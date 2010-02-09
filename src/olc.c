
/* copyright (c) 1996 by Gary Dezern */


/* For the online creation system...

   This file should contain only two things:  skeleton code, and
   "generic" functions used by more then 1 part of the olc system */


#define _OLC_SOURCE_            /* to get that extra Kick from olc.h */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "comm.h"
#include "olc.h"
#include "prototypes.h"
#include "utils.h"

const char *olc_dirs[] = {
  "North",
  "East",
  "South",
  "West",
  "Up",
  "Down",
  "Northwest",
  "Southwest",
  "Northeast",
  "Southeast"
};


/* generic functions used by parts of the olc system */

void olc_build_flag_menu8(char *str, ubyte value, const char *names[])
{
  byte     ttl = 0;
  byte     i;
  char     buf[128];

  /* two pasess... first to see whats there, second to build the table
     NOTE: This only makes available to the users those bit flags
     which appear in 'names' before the "\n" entry.  */

  while (*names[ttl] != '\n')
    ttl++;

  if (!ttl)
  {
    strcat(str, "&+R&-LNO FLAGS AVAILABLE!  This is an error!&N");
    return;
  }
  /* set ttl to its half */

  ttl = (ttl + 1) / 2;

  for (i = 0; i <= ttl; i++)
  {
    sprintf(buf, " %s &+W%2d.&N %s%-33.33s&N",
            value == i ? "&+M*&N" : " ",
            i + 1, value == i ? "&+C" : "", names[i]);

    if (*names[i + ttl] != '\n')
      sprintf(buf + strlen(buf),
              " %s &+W%2d.&N %s%-33.33s&N\r\n",
              value == (i + ttl) ? "&+M*&N" : " ",
              i + ttl + 1, value == (i + ttl) ? "&+C" : "", names[i + ttl]);
    else
      strcat(buf, "\r\n");
    strcat(str, buf);
  }
}


void olc_build_bitflag_menu32(char *str, ulong value, const char *names[])
{
  int      ttl = 0;
  int      i;
  char     buf[128];

  /* two pasess... first to see whats there, second to build the table
     NOTE: This only makes available to the users those bit flags
     which appear in 'names' before the "\n" entry.  */

  while (*names[ttl] != '\n')
    ttl++;

  if (!ttl)
  {
    strcat(str, "&+R&-LNO FLAGS AVAILABLE!  This is an error!&N");
    return;
  }
  /* set ttl to its half */

  ttl = (ttl + 1) / 2;

  /* okay.. two columns, each of "ttl/2" length.  Make it look like:
   * A. flag0                   * D. flag3
   B. flag1                     E. flag4
   C. flag2
   (assuming flags 0 and 3 are set).  
   */

  for (i = 0; i <= ttl; i++)
  {
    sprintf(buf, " %s &+W%2d.&N %s%-33.33s&N",
            IS_SET(value, 1 << i) ? "&+M*&N" : " ",
            i + 1, IS_SET(value, 1 << i) ? "&+C" : "", names[i]);

    if (*names[i + ttl] != '\n')
      sprintf(buf + strlen(buf),
              " %s &+W%2d.&N %s%-33.33s&N\r\n",
              IS_SET(value, 1 << (i + ttl)) ? "&+M*&N" : " ",
              i + ttl + 1, IS_SET(value, 1 << (i + ttl)) ? "&+C" : "",
              names[i + ttl]);
    else
      strcat(buf, "\r\n");
    strcat(str, buf);
  }
}

void olc_del_exit_menu(struct olc_data *data)
{
  int      i;
  char     buf1[MAX_STRING_LENGTH];

  buf1[0] = 0;

  for (i = 0; i < NUM_EXITS; i++)
    if (world[data->rnum].dir_option[i])
      sprintf(buf1 + strlen(buf1),
              " &+W%d.&N Delete %s exit\r\n", i, dirs[i]);
  if (*buf1)
  {
    SEND_TO_Q("\r\n\r\n", data->desc);
    SEND_TO_Q(buf1, data->desc);
  }

}

void olc_show_menu(struct olc_data *data)
{
  char     buf1[MAX_STRING_LENGTH];

  buf1[0] = 0;

  if (data->mode == OLC_MODE_EXIT_DEL)
    olc_del_exit_menu(data);
  else if (data->mode <= OLC_MODE_LAST_ROOM)
    olc_room_menu(buf1, data);
  else
    strcpy(buf1, "&+R&-LUnknown OLC mode!!!&N\r\n");

  if (*buf1)
  {
    SEND_TO_Q("\r\n\r\n", data->desc);
    SEND_TO_Q(buf1, data->desc);
  }
}


/* parts of OLC which are called from "the outside" */

void olc_string_add(struct olc_data *data, char *str)
{
  if (!*str)
  {
    str = NULL;
  }
  else if (*str == '?')
  {
    olc_show_menu(data);
    return;
  }
  if (data->mode <= OLC_MODE_LAST_ROOM)
    olc_room_string_add(data, str);
  else
    SEND_TO_Q("&+R&-LUnknown OLC mode, how can I parse commands?!&N\r\n",
              data->desc);
}

/* outside "interface" to olc.  This is the only function called from
   the outside world to initiate olc actions. (with the exception of
   comm shit).  In particular, its the "hook" between olc and the
   command interpreter. */

void do_olc(P_char ch, char *arg, int cmd)
{
  /* for now, just have 'room' and 'zone' */
  /*
     edit <what> [num]
     room
     zone
     save
   */

  char     buf1[MAX_STRING_LENGTH];
  int      i;

  arg = one_argument(arg, buf1);
  if (!*buf1)
    buf1[0] = '?';
  if (!strn_cmp("edit", buf1, strlen(buf1)))
  {
    /* its an edit command! */
    arg = one_argument(arg, buf1);
    if (!*buf1)
      buf1[0] = '?';
    if (!strn_cmp("room", buf1, strlen(buf1)))
    {
      int      room_no;
      struct olc_data *data;

      /* editting a room! */
      /* get an OPTIONAL room number.  If none, then we edit the room
         the char is standing in */
      arg = one_argument(arg, buf1);
      if (*buf1 && is_number(buf1))
        room_no = real_room(atoi(buf1));
      else
        room_no = ch->in_room;

      if (room_no == -1)
      {
        send_to_char("No such room number!\r\n", ch);
        return;
      }
      /* Now would be a good place to check if they are allowed to
         edit this room! */
      if (!olc_check_rnum_access(ch->desc, room_no))
      {
        send_to_char("&+R&-LYou don't have access!!\r\n", ch);
        return;
      }
      CREATE(data, struct olc_data, 1, MEM_TAG_OLCDATA);
      data->desc = ch->desc;
      data->mode = OLC_MODE_ROOM0;
      data->rnum = room_no;
      ch->desc->olc = data;
      olc_show_menu(data);
      return;

    }
    else if (!strn_cmp("zone", buf1, strlen(buf1)))
    {
      /* editting zone data! */
    }
    else
    {
      send_to_char("Unknown olc edit directive\r\n"
                   " Try \"olc edit <room|zone|mobile|object>\"\r\n", ch);
    }

  }
  else if (!strncmp("save", buf1, strlen(buf1)))
  {
    for (i = 0; i <= top_of_zone_table; i++)
    {
      if (world[i].zone)
        olc_save_wld(world[i].zone);
    }
    send_to_char("All .wld's saved!\r\n", ch);
  }
  else
  {
    send_to_char("Unknown olc command!\r\n", ch);
  }
}

/* functions called from comm.c... */


void olc_prompt(struct olc_data *data)
{
  char     prompt[MAX_STRING_LENGTH];

  /* this function should generate a prompt while a person is in
     OLC... and then display it to the user.  */

  prompt[0] = '\0';
  switch (data->mode)
  {
  case OLC_MODE_ROOM0:
    sprintf(prompt, "OLC Room Editor (#%d): ", world[data->rnum].number);
    break;
  case OLC_MODE_ROOM1:
    sprintf(prompt, ">");
    break;
  case OLC_MODE_ROOM_FLAGS:
    sprintf(prompt, "OLC Room Flag Editor (#%d): ", world[data->rnum].number);
    break;
  case OLC_MODE_ROOM_SECT:
    sprintf(prompt, "OLC Room Sector Editor (#%d): ",
            world[data->rnum].number);
    break;
  case OLC_MODE_XTRA:
    sprintf(prompt, "OLC Keyword Editor (#%d): ", world[data->rnum].number);
    break;
  case OLC_MODE_XTRA1:
    sprintf(prompt, "OLC Keyword Editor (#%d) [%s]: ",
            world[data->rnum].number,
            ((struct extra_descr_data *) data->misc)->keyword);
    break;
  case OLC_MODE_XTRA2:
    sprintf(prompt, ">");
    break;

  case OLC_MODE_XTRA_DEL:
    sprintf(prompt, "OLC Keyword Removal (#%d): ", world[data->rnum].number);
    break;

  case OLC_MODE_EXIT:
  case OLC_MODE_EXIT_DEL:
  case OLC_MODE_EXIT1:
    sprintf(prompt, "OLC Exit Editor (#%d): ", world[data->rnum].number);
    break;

  default:
    strcpy(prompt, "Unknown OLC Mode:");
  }

  if (write_to_descriptor(data->desc->descriptor, prompt) < 0)
  {
    logit(LOG_COMM, "Closing socket on write error");
    close_socket(data->desc);   /* let close_socket deal with clearing
                                   us out */
  }
}

void olc_end(struct olc_data *data)
{
  /* if we are called, the user had BETTER not be in the editor as
     well.... */
  P_desc   d = data->desc;

  SEND_TO_Q("\r\n\r\n", d);

  FREE(data);
  d->olc = NULL;
}

int olc_check_rnum_access(P_desc desc, int rnum)
{
  char    *name;
  int      i;

  if (!desc->character)
    return 0;

  if (GET_LEVEL(desc->character) >= 59)
    return 1;

  name = desc->character->player.name;

  /* walk through the zone table, and figure out which zone owns this
     vnum. */

  for (i = 0; i <= top_of_zone_table; i++)
    if (rnum <= zone_table[i].real_top)
    {
      if (str_cmp(name, zone_table[i].owner))
        return 0;
      else
        return 1;
    }
  return 0;
}
