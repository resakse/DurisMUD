#include <string.h>
#include "prototypes.h"
#include "utils.h"
#include "makeexit.h"

extern const int rev_dir[];
extern P_room world;

void do_makeexit(P_char ch, char *arg, int cmd)
{
  char buf[MAX_STRING_LENGTH];

  if( !IS_TRUSTED(ch) )
  {
    return;
  }

  arg = one_argument(arg, buf);

  if( buf[0] == '?' || !strcmp(buf, "help") )
  {
    send_to_char("Syntax:\r\nmakeexit <direction> <destination room vnum> ['none' for no reverse exit]\r\n", ch);
    return;
  }

  int dir = dir_from_keyword(buf);

  if( dir < 0 || dir >= NUM_EXITS )
  {
    send_to_char("Invalid direction.\r\n", ch);
    return;
  }

  arg = one_argument(arg, buf);

  int to_room_vnum = atoi(buf);

  int from_room = ch->in_room;
  int to_room = real_room(to_room_vnum);

  arg = one_argument(arg, buf);
  
  bool rev_exit;
  if( !strcmp(buf, "none") )
    rev_exit = false;
  else
    rev_exit = true;

  if( to_room == -1 )
  {
    send_to_char("Invalid destination room.\r\n", ch);
    return;
  }

  debug("makeexit %d to %d dir %d", world[from_room].number, world[to_room].number, dir);

  if( world[from_room].dir_option[dir] )
  {
    send_to_char("Sorry, direction is already taken.\r\n", ch);
    return;
  }

  if( rev_exit && world[to_room].dir_option[rev_dir[dir]] )
  {
    send_to_char("Sorry, direction at destination already taken.\r\n", ch);
    return;
  }
 
  link_room(from_room, to_room, dir);

  arg = one_argument(arg, buf);
  
  if( rev_exit )
  { 
    link_room(to_room, from_room, rev_dir[dir]);
  }
 
  wizlog(56, "%s made exit from %d to %d", ch->player.name, world[from_room].number, world[to_room].number);
  logit(LOG_WIZ, "%s made exit from %d to %d", ch->player.name, world[from_room].number, world[to_room].number);
 
  send_to_char("Exit created.\r\n", ch);

  return;
}

int link_room(int from_r, int to_r, int dir)
{
  struct room_direction_data *dir_data;
  CREATE( dir_data, room_direction_data, 1, MEM_TAG_DIRDATA);

  if( !dir_data )
  {
    return FALSE;
  }

  dir_data->general_description = str_dup("An otherworldy door stands here.");
  dir_data->keyword = str_dup("door");
  dir_data->exit_info = EX_ISDOOR | EX_CLOSED | EX_LOCKED | EX_SECRET | EX_PICKPROOF;
  dir_data->key = 0;
  dir_data->to_room = to_r;

  world[from_r].dir_option[dir] = dir_data;
  
  return TRUE;
}
