/*
 ***************************************************************************
 *  File: kingdom.c
 *  Usage: troops, castles, land aquisition
 *  Copyright 1994 - 2008 - Duris Systems Ltd.
 ***************************************************************************
 */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

#include "events.h"
#include "spells.h"
#include "structs.h"
#include "comm.h"
#include "prototypes.h"
#include "db.h"
#include "interp.h"
#include "utils.h"
#include "config.h"
#include "mm.h"
#include "justice.h"
#include "sound.h"
#include "assocs.h"
#include "specs.prototypes.h"
#include "map.h"
#include "old_guildhalls.h"
#include "utility.h"

/* external variables */
extern struct kingdom_global kingdom[];
extern int mini_mode;
extern P_char character_list;
extern struct mm_ds *dead_mob_pool;
extern struct mm_ds *dead_pconly_pool;

extern P_desc descriptor_list;
extern P_event current_event;
extern P_event event_list;
extern P_index mob_index;
extern P_index obj_index;
extern P_room world;
extern const char *dirs[];
extern int MobSpellIndex[MAX_SKILLS];
extern int equipment_pos_table[CUR_MAX_WEAR][3];
extern int no_specials;
extern int spl_table[TOTALLVLS][MAX_CIRCLE];
extern int top_of_world;
extern const int rev_dir[];
extern struct zone_data *zone_table;
extern char *troop_types[];
extern char *troop_levels[];
extern char *troop_offense[];
extern char *troop_defense[];
extern int troop_costs[NUM_TROOP_TYPES][4];

/* misc crap */
struct mm_ds *dead_house_pool = NULL;
struct mm_ds *dead_construction_pool = NULL;
P_house  first_house = NULL;
P_house_upgrade house_upgrade_list = NULL;
//struct kingdom_global kingdom[MAX_KINGDOMS];
//struct troop_info_rec *deployed_troops;
//char* kingdom_num_name(int, int);
//void update_kingdom_score(int kingdom_num, int int_arg, int type);

void parse_connection(P_house, P_char, char *);
void do_build_secret(P_house house, P_char ch);
void write_pc_land(int room);
void do_construct_guild(P_char, int);
void do_delete_room(P_house house, P_char ch, char *arg);
int moveHouse(P_house house, int new_vnum);

const int upgrade_times[] = {   /* these are measured in mud hours */
  0,                            /* null */
  10,                           /* add mage golem */
  10,                           /* add cleric golem */
  10,                           /* add warrior golem */
  15,                           /* add chest */
  15,                           /* add teleporter */
  15,                           /* add fountain */
  5,                            /* add inn */
  5,                            /* add shop */
  15,                           /* add board */
  8,                            /* add room */
  1,                            /* describe room */
  1,                            /* name room */
  1,                            /* magic mouth */
  2,                            /* holy */
  3,                            /* unholy */
  4,                            /* heal room */
  1,                            /* build initial house */
  1,                            /* build initial guildhall */
  1,                            /* cabinet */
  1,                            /* secret entrance */
  1,                            /* window entrance */
  1,                            /* move entrance */
  1,                            /* guard golem */
  0
};                              /* last record */

const int upgrade_costs[] = {   /* costs in platinum */
  0,                            /* null */
  25000,                        /* add mage golem */
  20000,                        /* add cleric golem */
  15000,                        /* add warrior golem */
  10000,                         /* add chest */
  20000,                        /* add teleporter */
  1500,                          /* add fo untain */
  30000,                          /* add inn */
  10000,                         /* add shop */
  500,                          /* add board */
  20000,                         /* add room */
  100,                           /* describe room */
  500,                           /* name room */
  500,                          /* magic mouth */
  5000,                         /* holy */
  5000,                         /* unholy */
  10000,                         /* heal room */
  5000,                         /* initial house, shouldnt be called here */
  100000,                        /* initial guild, shouldnt be called here */
  5000,                          /* cabinet */
  10000,                        /* secret entrance */
  10000,                        /* Window entrance */
  150000,                        /* move entrance */
  10000,                         /* guard */
  0
};                              /* last record */

/* CONSTRUCT syntax */
const char *CONSTRUCT_FORMAT_OLD =
  "\r\n"
  " &+YConstruct command syntax&n\r\n"
  " &+y========================&n\r\n"
  " &+LHouses are disabled!&n\r\n"
  "  construct room exit_dir door|nodoor [door keyword]  <&+BG&n|&+YC&n>\r\n"
  "  construct name <room name>                          <&+wA&n>\r\n"
  "  construct describe                                  <&+wA&n>\r\n"
  "  construct cleric | mage | warrior                   <&+BG&n|&+YC&n>\r\n"
  "  construct guard                                     <&+wA&n>\r\n"
  "  construct inn                                       <&+BG&n|&+YC&n>\r\n"
  "  construct chest                                     <&+BG&n|&+YC&n>\r\n"
  "  construct cabinet                                   <&+LH&n>\r\n"
  "  construct teleporter <target>                       <&+BG&n|&+YC&n>\r\n"
  "  construct fountain                                  <&+BG&n|&+YC&n>\r\n"
  "  construct mouth                                     <&+BG&n|&+YC&n>\r\n"
  "  construct board                                     <&+BG&n|&+YC&n>\r\n"
  "  construct holy                                      <&+BG&n|&+YC&n>\r\n"
  "  construct unholy                                    <&+BG&n|&+YC&n>\r\n"
  "  construct heal                                      <&+BG&n|&+YC&n>\r\n"
  "  construct shop                                      <&+BG&n|&+YC&n>\r\n"
  "  construct secret entrance                           <&+BG&n|&+LH&n>\r\n"
  "  construct window entrance                           <&+BG&n|&+LH&n>\r\n"
  "  construct move entrance                             <&+BG&n|&+LH&n>\r\n"
  " &+y========================&n\r\n"
  "  <&+wA&n> command allowed for all types\r\n"
  "  <&+BG&n> guildhalls only\r\n"
  "  <&+YC&n> castle only\r\n"
  "  <&+LH&n> house only\r\n";

const char *CONSTRUCT_FORMAT =
  "\r\n"
  " &+YConstruct command syntax&n\r\n"
  " &+y========================&n\r\n"
  "  construct room exit_dir\r\n"
  "  construct name <room name>\r\n"
  "  construct describe\r\n"
  "  construct cleric | mage | warrior\r\n"
  "  construct inn\r\n"
  "  construct chest\r\n"
  "  construct fountain\r\n"
  "  construct holy\r\n"
  "  construct unholy\r\n"
  "  construct heal\r\n";


void do_construct(P_char ch, char *arg, int cmd)
{
  P_house  house, house2;
  uint     temp;
  char     buff[MAX_STRING_LENGTH];
  struct house_upgrade_rec *cur_rec;

  if (!ch)
    return;

  //TODO: implement construct

  if (!arg  || !*arg)
  {
    send_to_char(CONSTRUCT_FORMAT, ch);
    return;
  }
  arg = one_argument(arg, buff);

  if (is_abbrev(buff, "show") && IS_TRUSTED(ch))
  {                             /* show construction q */
    do_show_q(ch);
    return;
  }

  wizlog(GET_LEVEL(ch), "%s construct %s at #%d", GET_NAME(ch), buff, world[ch->in_room].number);

  // god only commands
  if( is_abbrev(buff, "guild") || is_abbrev(buff, "outpost") )
  {
     if( !IS_TRUSTED(ch) )
     {
       send_to_char("Sorry?\n", ch);
       return;    
     }
     // ok god can do it
     if (is_abbrev(buff, "guild"))
     {
       do_construct_guild(ch, HCONTROL_GUILD);
       return;
     }
     if (is_abbrev(buff, "outpost"))
     {
       send_to_char("Outposts are disabled.\n", ch);
       return;
       do_construct_guild(ch, HCONTROL_OUTPOST);
       return;
     }
     return;
  }

  // free for all commands
  
  house = house_ch_is_in(ch);
  if (!house)
  {
    send_to_char("You cannot construct here.\r\n", ch);
    return;
  }

  if ((GET_A_NUM(ch) != house->owner_guild) && (!IS_TRUSTED(ch)) &&
      (house->type == HCONTROL_GUILD || house->type == HCONTROL_OUTPOST))
  {
    send_to_char("You don't have the authority to build here.\r\n", ch);
    return;
  }
  
  // rank/level check
  temp = GET_M_BITS(GET_A_BITS(ch), A_RK_MASK);
  if (!(IS_LEADER(temp) || IS_TRUSTED(ch) || GT_LEADER(temp)) &&
      (house->type == HCONTROL_GUILD))
  {
    send_to_char("You are not high enough rank to build here.\r\n", ch);
    return;
  }
  else if (GET_LEVEL(ch) < 25)
  {
    send_to_char
      ("Ye must achieve more experience in the world to build on to yer abode.\r\n",
       ch);
    return;
  }

  cur_rec = house_upgrade_list;
  while (cur_rec)
  {
    house2 = find_house(cur_rec->vnum);
    if (house2 && (house == house2))
      if (!str_cmp(house2->owner, J_NAME(ch)) && !IS_TRUSTED(ch))
      {
#if 0
        send_to_char
          ("You are already building something. Finish it first.\r\n", ch);
        return;
#endif
      }
    cur_rec = cur_rec->next;
  }
  /* Okay, valid place/person, parse the arg */

  if (is_abbrev(buff, "process"))
  {                             /* process the q */
    if (IS_TRUSTED(ch))
      process_construction_q();
    return;
  }

  if (is_abbrev(buff, "connect"))
  {                             /* connect 2 rooms, god only to */
    if (IS_TRUSTED(ch))         /* prevent goofyness */
      parse_connection(house, ch, arg);
    return;
  }
  if (is_abbrev(buff, "name"))
  {                             /* name current room */
    do_name_room(house, ch, arg);
    return;
  }
  if (is_abbrev(buff, "describe"))
  {                             /* describe current room */
    do_describe_room(house, ch, arg);
    return;
  }
  if (is_abbrev(buff, "cabinet"))
  {
    do_build_chest(house, ch);
    return;
  }

  if (is_abbrev(buff, "guard"))
  {
    //do_build_golem(house, ch, HCONTROL_ADD_GUARD_GOLEM);
    return;
  }
  if (is_abbrev(buff, "fixroom"))
  {                             /* construct room */
    do_construct_room(house, ch, "n nodoor");
    return;
  }

//  if (house->type == HCONTROL_GUILD || house->type == HCONTROL_HOUSE)
//  {
//    if (is_abbrev(buff, "secret_disabled"))
//    {                           /* construct secret */
//      return;
//      do_build_secret(house, ch);
//      return;
//    }
//  }

  if (house->type == HCONTROL_GUILD || house->type == HCONTROL_OUTPOST)
  {
    if (is_abbrev(buff, "teleporter"))
    {
      return;
      do_build_teleporter(house, ch, arg);
      return;
    }
    if (is_abbrev(buff, "cleric"))
    {
      do_build_golem(house, ch, HCONTROL_ADD_CLERIC_GOLEM);
      return;
    }
    if (is_abbrev(buff, "mage"))
    {
      do_build_golem(house, ch, HCONTROL_ADD_MAGE_GOLEM);
      return;
    }
    if (is_abbrev(buff, "warrior"))
    {
      do_build_golem(house, ch, HCONTROL_ADD_WARRIOR_GOLEM);
      return;
    }
    if (is_abbrev(buff, "mouth"))
    {
      return;
      send_to_char("Construct a mouth? Dont you already have one? .\r\n", ch);
      //do_build_mouth(house, ch);
      return;
    }
  }

  /* next are castle/guild only */
  if (house->type == HCONTROL_GUILD)
  {
    if (is_abbrev(buff, "shop"))
    {
      return;
      send_to_char("You cannot buy a shop!\n", ch);
      //do_build_shop(house, ch);
      return;
    }
    if (is_abbrev(buff, "room"))
    {                           /* construct room */
      do_construct_room(house, ch, arg);
      return;
    }
    if (is_abbrev(buff, "delete"))
    { 
      send_to_char("You cannot trust the workers.!\r\n", ch);
          return;
             /* delete room */
      do_delete_room(house, ch, arg);
      return;
    }
    if (is_abbrev(buff, "chest"))
    {
      do_build_chest(house, ch);
      return;
    }
    if (is_abbrev(buff, "holy"))
    {
      do_build_holy(house, ch);
      return;
    }
    if (is_abbrev(buff, "unholy"))
    {
      do_build_unholy(house, ch);
      return;
    }
    if (is_abbrev(buff, "heal"))
    {
      do_build_heal(house, ch);
      return;
    }
    if (is_abbrev(buff, "fountain"))
    {
      do_build_fountain(house, ch);
      return;
    }
    if (is_abbrev(buff, "window"))
    {
      return;
      do_build_window(house, ch);
      return;
    }
    if (is_abbrev(buff, "move"))
    {
      return;
      do_build_movedoor(house, ch, arg);
      return;
    }
    if (is_abbrev(buff, "board"))
    {                           /* this one simple */
      house->board_vnum = world[ch->in_room].number;
      writeHouse(house);
      send_to_char("Board location moved to here.\r\n", ch);
      return;
    }
    if (is_abbrev(buff, "inn"))
    {
            
      do_build_inn(house, ch);
      return;
    }
  }

  // invalid command
  send_to_char(CONSTRUCT_FORMAT, ch);
  return;
}

void parse_connection(P_house house, P_char ch, char *arg)
{
  int      v1, v2, dir;
  char     buf[MAX_INPUT_LENGTH];

  arg = one_argument(arg, buf);
  if (!buf || !is_number(buf))
  {
    send_to_char("Format: construct connection room1 room2 direction.\r\n",
                 ch);
    send_to_char("You appear to have left out room1s virtual number.\r\n",
                 ch);
    return;
  }
  else
    v1 = atoi(buf);
  arg = one_argument(arg, buf);
  if (!buf || !is_number(buf))
  {
    send_to_char("Format: construct connection room1 room2 direction.\r\n",
                 ch);
    send_to_char("You appear to have left out room2s virtual number.\r\n",
                 ch);
    return;
  }
  else
    v1 = atoi(buf);
  arg = one_argument(arg, buf);
  if (!(dir = old_search_block(buf, 0, strlen(buf), dirs, 0)))
  {
    send_to_char("Format: construct connection room1 room2 direction.\r\n",
                 ch);
    send_to_char("You appear to have left out the direction.\r\n", ch);
    return;
  }
  connect_rooms(v1, v2, dir);
}

/* Parse 3 args from hcontrol
 * cmd <house|guild|castle> <name> <dir> <guild_number>
 */
void hcontrol_parser(char *arg, int *type, char *name, int *direction,
                     int *guild)
{
  char     buf[MAX_STRING_LENGTH];

  buf[0] = '\0';

  /* determine type */
  arg = one_argument(arg, buf);
  if (is_abbrev(buf, "guild"))
    *type = HCONTROL_GUILD;
  else if (is_abbrev(buf, "outpost"))
    *type = HCONTROL_OUTPOST;
  else
  {
    *type = HCONTROL_ERROR;
    return;
  }

  /* find name */
  arg = one_argument(arg, name);

  switch (*type)
  {
  case HCONTROL_OUTPOST:
  case HCONTROL_GUILD:
    arg = one_argument(arg, buf);
    if (!(*direction = old_search_block(buf, 0, strlen(buf), dirs, 0)))
      return;
    arg = one_argument(arg, buf);
    *guild = atoi(buf);
    break;
  }
  if (*direction <= 0 || *direction >= 3)
    *direction = 0;

  return;
}


int remove_a_golem_from_house(int vnum)
{
  P_house  thouse = NULL;

  thouse = get_house_from_room(vnum);
    if(!thouse){
    return FALSE;
    wizlog(56, "Cannot find house of golem %d", vnum);
    }
  if(thouse->wizard_golems){
    thouse->wizard_golems--;
                        }
  else if (thouse->warrior_golems)
  {
     thouse->warrior_golems--;
  }
   else if(thouse->cleric_golems){
     thouse->cleric_golems--;
   }
   writeHouse(thouse);

return 1;
}

/******************************************************************
 *  Functions for house administration (creation, deletion, etc.  *
 *****************************************************************/

P_house find_house(int vnum)
{
  P_house  house = NULL;
  int      x;

  if (!first_house)
    return NULL;

  for (house = first_house; house; house = house->next)
  {
    if (house->vnum == vnum)
      return (house);
    for (x = 0; x < MAX_HOUSE_ROOMS; x++)
      if (house->room_vnums[x] == vnum)
        return (house);
  }
  return NULL;
}

/* copy of above, except it looks based on ownership */
P_house find_house_by_owner(char *name)
{
  P_house  house = NULL;

  if (!first_house)
    return NULL;

  for (house = first_house; house; house = house->next)
    if (!strcmp(name, house->owner))
      return (house);

  return NULL;
}

int room_guild_no(int rnum)
{                               /* returns guild # of guild room */
  P_house  house;
  int      x;

  for (house = first_house; house; house = house->next)
    for (x = 0; x < MAX_HOUSE_ROOMS; x++)
      if (real_room(house->room_vnums[x]) == rnum)
        return house->owner_guild;

  return 0;
}

P_house get_house_from_room(int rnum)
{
  P_house  house;
  int      x;

  for (house = first_house; house; house = house->next)
    for (x = 0; x < MAX_HOUSE_ROOMS; x++)
      if (real_room(house->room_vnums[x]) == rnum)
        return house;
  return 0;
}

/****************************************************/
/* "House Control" functions */

const char *HCONTROL_FORMAT =
  "\r\n"
  "Usage: hcontrol build house <owner> <entrance direction>\r\n"
  "       hcontrol build castle <owner> <entrance dir> <assoc #>\r\n"
  "       hcontrol build guild <owner> <entrance dir> <assoc #>\r\n"
  "       hcontrol destroy <house vnum>\r\n"
  "       hcontrol pay <house vnum>\r\n"
  "       hcontrol list (main listing)\r\n"
  "       hcontrol detail (lists rooms)\r\n"
  "       hcontrol move <house vnum> <new map entrance vnum>\r\n"
  "       hcontrol clear (removes teleporters)\r\n"
  "       hcontrol restore <house vnum> (reboots building)\r\n"
  "       hcontrol change <house vnum> <aspect/field> <new value>\r\n"
  "       hcontrol portals <house vnum> [portal #] [r|d] [new #]\r\n"
  "Type is one of house, guild or castle\r\n"
  "Owner is the primary controlling character\r\n"
  "Entrance is the direction from your current room, into the structure\r\n"
  "Field is one of: owner, guild, upgrade, type, room <x>, numrooms, land\r\n"
  "House vnum is shown via show\r\n" "\r\n";

void hcontrol_list_houses(P_char ch, char *arg)
{
  int      j;
  char     timestr[256];
  char     built_on[128], last_pay[128], own_name[128];
  P_house  house;
  char     buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  char     the_type;

  if (!first_house)
  {
    send_to_char("No houses have been defined.\r\n", ch);
    return;
  }
  strcpy(buf, "\r\n"
         "&+GAddress  Guild Upgr Build Date  Rooms  Owner       Last Paymt Type MainRoom&N\r\n");
  strcat(buf,
         "&+G-------  ----- ---- ----------  ----- ------------ ---------- ---- ------&N\r\n");

  send_to_char(buf, ch);
  buf2[0] = '\0';

  for (house = first_house; house; house = house->next)
  {
    buf[0] = '\0';

    if (*arg)
      if (is_number(arg))
      {
        if ((atoi(arg)) != house->owner_guild)
          continue;
      }
      else if (house->owner && str_cmp(house->owner, arg))
        continue;

    if (house->built_on)
    {
      strcpy(timestr, asctime(localtime(&(house->built_on))));
      *(timestr + 10) = 0;
      strcpy(built_on, timestr);
    }
    else
      strcpy(built_on, "Unknown");

    if (house->last_payment)
    {
      strcpy(timestr, asctime(localtime(&(house->last_payment))));
      *(timestr + 10) = 0;
      strcpy(last_pay, timestr);
    }
    else
      strcpy(last_pay, "None");

    if (house->owner)
      strcpy(own_name, house->owner);
    else
      strcpy(own_name, "(null)");

    if (house->type == HCONTROL_CASTLE)
      the_type = 'C';
    else if (house->type == HCONTROL_HOUSE)
      the_type = 'H';
    else if (house->type == HCONTROL_GUILD)
      the_type = 'G';
    else if (house->type == HCONTROL_OUTPOST)
      the_type = 'O';
    else
      the_type = ' ';

    sprintf(buf + strlen(buf), "%7d %5d %3d  %-10s     %-12s %-10s  %c   %d",
            (int) house->vnum, (int) house->owner_guild,
            (int) house->upgrades, built_on, own_name, last_pay, the_type,
            house->room_vnums[0]);

    if (house->construction > 0)
      sprintf(buf + strlen(buf), " &+R*&N\r\n");
    else
      sprintf(buf + strlen(buf), "\r\n");

    if (house->num_of_guests > 0)
    {
      sprintf(buf + strlen(buf), "     &+LGuests (%d):&n ",
              house->num_of_guests);
      for (j = 0; j < house->num_of_guests; j++)
      {
        sprintf(buf + strlen(buf), "&+L%s&n ", house->guests[j]);
      }
      strcat(buf, "\r\n");
    }
    strcat(buf2 + strlen(buf2), buf);
  }
  page_string(ch->desc, buf2, 1);
}

bool can_acquire_hall(P_char ch, int type)
{
  int      outposts;
  bool     has_main = FALSE;
  P_house  house;

  for (outposts = 0, house = first_house; house; house = house->next)
  {
    if (house->owner_guild == GET_A_NUM(ch))
    {
      if (house->type == HCONTROL_GUILD)
        has_main = TRUE;
      else
        outposts++;
    }
  }

  if (type == HCONTROL_GUILD)
  {
    if (!has_main)
      return TRUE;
  }
  else if (type == HCONTROL_OUTPOST)
  {
    if (outposts < get_property("guild.maxOutposts", 1))
      return TRUE;
  }

  return FALSE;
}


/* For icon purposes, we don't want these built near a maps edge. We need
   only 2 squares surrounding free, but lets check 3 in case something changes
   later on. Pass this a real room!
 */
bool valid_build_location(int room, P_char ch, int type)
{
  int      zone_num, temp1, temp2;
  int      vroom;
  P_house  house;

  zone_num = world[room].zone;

  if(!IS_HOMETOWN(room))
  {
     if(ch)
        send_to_char("Can build only inside hometowns now!\r\n", ch);

     return FALSE;
  }

  return TRUE;
  
  if (ch)
  {
    if (IS_TRUSTED(ch))
      return TRUE;

    if (world[room].sector_type == SECT_INSIDE ||
        world[room].sector_type == SECT_WATER_SWIM ||
        world[room].sector_type == SECT_WATER_NOSWIM ||
        world[room].sector_type == SECT_NO_GROUND ||
        world[room].sector_type == SECT_UNDERWATER ||
        world[room].sector_type == SECT_UNDERWATER_GR ||
        world[room].sector_type == SECT_FIREPLANE ||
        world[room].sector_type == SECT_OCEAN ||
        world[room].sector_type == SECT_UNDRWLD_INSIDE ||
        world[room].sector_type == SECT_UNDRWLD_WATER ||
        world[room].sector_type == SECT_UNDRWLD_NOSWIM ||
        world[room].sector_type == SECT_UNDRWLD_NOGROUND ||
        world[room].sector_type == SECT_AIR_PLANE ||
        world[room].sector_type == SECT_WATER_PLANE ||
        world[room].sector_type == SECT_EARTH_PLANE ||
        world[room].sector_type == SECT_ETHEREAL ||
        world[room].sector_type == SECT_ASTRAL ||
        world[room].sector_type == SECT_CASTLE ||
        world[room].sector_type == SECT_CASTLE_WALL ||
        world[room].sector_type == SECT_CASTLE_GATE)
    {
      send_to_char
        ("Gee bucko, can't you think of a better place to build this?\r\n",
         ch);
      return FALSE;
    }
    if ((world[ch->in_room].room_flags & INDOORS) && !IS_UNDERWORLD(ch->in_room))
    {
      send_to_char
        ("Umm, this area is already indoors. We don't build things like that around these parts.\r\n",
         ch);
      return FALSE;
    }
    if (IS_SET(world[ch->in_room].room_flags, GUILD_ROOM))
    {
      send_to_char("Not inside a guild. Puhlease!\r\n", ch);
      return FALSE;
    }
    if (CHAR_IN_TOWN(ch))
    {
      send_to_char("Sorry, you can't build inside towns right now.\r\n", ch);
      return FALSE;
    }
    if ((IS_GOOD_MAP(room) && !GOOD_RACE(ch)) ||
        (IS_EVIL_MAP(room) && !EVIL_RACE(ch)) ||
        (IS_UD_MAP(room) && !EVIL_RACE(ch)))
    {
      send_to_char
        ("You can't acquire outposts so deep in the land of your enemies.\r\n",
         ch);
      return FALSE;
    }

  }

/****** this is wrong. room is a real, the +300 is only valid on vnum ******/
  vroom = world[room].number;
/*
  if ((room + 300 > top_of_world) || (room + 3 > top_of_world) ||
      (room - 3 < 0) || (room - 300 < 0))
    return FALSE;
*/
  if (!(real_room0(vroom + 300) && real_room0(vroom + 3) &&
        real_room0(vroom - 300) && real_room0(vroom - 3)))
  {
    send_to_char("This area is a little unstable.\n\r", ch);
    return FALSE;
  }
  if ((world[real_room(vroom + 300)].zone == zone_num) &&
      (world[real_room(vroom - 300)].zone == zone_num) &&
      (world[real_room(vroom + 3)].zone == zone_num) &&
      (world[real_room(vroom - 3)].zone == zone_num))
    return TRUE;
/**** end of errors *****/

  temp1 = GET_A_NUM(ch);
  temp2 = GET_A_BITS(ch);

  if (type == HCONTROL_CASTLE || type == HCONTROL_GUILD)
  {
/*    if (world[real_room(room)].kingdom_type == KINGDOM_TYPE_NPC ||
	world[real_room(room)].kingdom_type == KINGDOM_TYPE_TOWN) {
      send_to_char("This land has already been claimed by another kingdom.\r\n", ch);
      return FALSE;
    } else if (world[real_room(room)].kingdom_type == KINGDOM_TYPE_PC && (!IS_MEMBER(temp2) || !temp1)) {
      send_to_char("This land has already been claimed by another kingdom.\r\n", ch);
      return FALSE;
    }*/
  }
  return FALSE;
}

void construct_castle(P_house temp_house)
{
  int      gate_room, rdir;
  int      virt_house, real_house;
  int      x, j;
  P_obj    entrance_obj;
  P_char   golem;
  char     buff[MAX_STRING_LENGTH], golem_block = 'a';
  char     tbuff[MAX_STRING_LENGTH];
  P_obj    temp_obj;
  P_obj    window_obj;

  virt_house = temp_house->vnum;
  real_house = real_room0(temp_house->vnum);
  if (!real_room0(temp_house->vnum))
  {
    wizlog(AVATAR,
           "Castle/guild %d cannot be built, due to this being a bugged room!.\r\n",
           temp_house->vnum);
    return;
  }
  if (!valid_build_location
      (real_room0(temp_house->vnum), NULL, temp_house->type))
    return;
  /* now, we know where we are talking about, so lets get busy building it */

  SET_BIT(world[real_room0(temp_house->vnum)].room_flags, ROOM_ATRIUM);

  /* BUILD ROOMS */
  for (x = 0; x < MAX_HOUSE_ROOMS; x++)
    if (temp_house->room_vnums[x] != -1)
      read_guild_room(temp_house->room_vnums[x], temp_house->owner_guild);

  /* New opened faced guildhalls! */
  if (temp_house->type == HCONTROL_GUILD ||
      temp_house->type == HCONTROL_OUTPOST)
  {
    connect_rooms(temp_house->vnum, temp_house->room_vnums[0], DOWN);
    disconnect_exit(temp_house->vnum, DOWN);
    SET_BIT(world[real_room0(temp_house->room_vnums[0])].room_flags, NO_MOB);
/*
    if ((entrance_obj = read_object(HOUSE_OUTER_DOOR, VIRTUAL)))
    {
       REMOVE_BIT(entrance_obj->extra_flags, ITEM_NOSHOW);
       entrance_obj->str_mask = (STRUNG_KEYS | STRUNG_DESC1);
       sprintf(buff, "hall guild building %s", temp_house->owner);
       entrance_obj->name = strdup(buff);
       sprintf(buff, "A Guild Hall of %s is here.", get_assoc_name(temp_house->owner_guild).c_str());
       entrance_obj->description = strdup(buff);
       entrance_obj->value[0] = temp_house->room_vnums[0];
       nuke_doorways(real_room0(temp_house->vnum));
       obj_to_room(entrance_obj, real_room0(temp_house->vnum));
    }
*/  
    temp_obj = read_object(TELEPORTER_VNUM, VIRTUAL);
    if (temp_obj)
    {
      temp_obj->value[0] = temp_house->room_vnums[0];
      obj_to_room(temp_obj, real_room0(temp_house->vnum));
    }
  }

  /* error control */
  if (temp_house->upgrades > 99)
    temp_house->upgrades = 0;
  /* inn */

  if (temp_house->room_vnums[0])
  {
    world[real_room0(temp_house->inn_vnum)].funct = inn;
  }

// INN AT SECOND ROOM
  /* guild board */
  if (temp_house->owner_guild && (temp_house->type == HCONTROL_GUILD))
  {
    temp_obj =
      read_object(GUILD_BOARD_START + temp_house->owner_guild, VIRTUAL);
    if (temp_obj)
    {
      if (temp_house->board_vnum)
      {
        obj_to_room(temp_obj, real_room0(temp_house->board_vnum));
      }
      else
      {
        obj_to_room(temp_obj, real_room0(temp_house->room_vnums[1]));
      }
    }
  }
  /* fountain */
  if (temp_house->fountain_vnum)
  {
    temp_obj = read_object(FOUNTAIN_VNUM, VIRTUAL);
    if (temp_obj)
    {
      obj_to_room(temp_obj, real_room0(temp_house->fountain_vnum));
    }
  }
  /* holy */
  if (temp_house->holy_fount_vnum)
  {
    temp_obj = read_object(GUILD_HOLY_FOUNT, VIRTUAL);
    if (temp_obj)
      obj_to_room(temp_obj, real_room0(temp_house->holy_fount_vnum));
  }
  if (temp_house->unholy_fount_vnum)
  {
    temp_obj = read_object(GUILD_UNHOLY_FOUNT, VIRTUAL);
    if (temp_obj)
      obj_to_room(temp_obj, real_room0(temp_house->unholy_fount_vnum));
  }
  if (temp_house->heal_vnum)
  {
    SET_BIT(world[real_room0(temp_house->heal_vnum)].room_flags, HEAL);
  }
  /* teleporters */
  if (temp_house->teleporter1_room)
  {
    temp_obj = read_object(TELEPORTER_VNUM, VIRTUAL);
    if (temp_obj)
    {
      temp_obj->value[0] = temp_house->teleporter1_dest;
      obj_to_room(temp_obj, real_room0(temp_house->teleporter1_room));
    }
  }
  if (temp_house->teleporter2_room)
  {
    temp_obj = read_object(TELEPORTER_VNUM, VIRTUAL);
    if (temp_obj)
    {
      temp_obj->value[0] = temp_house->teleporter2_dest;
      obj_to_room(temp_obj, real_room0(temp_house->teleporter2_room));
    }
  }
  if (temp_house->shop_vnum)
  {
    stock_guild_shop(temp_house->shop_vnum);
  }
  /* chest */
  if (temp_house->mouth_vnum)
  {
    temp_obj = read_object(MOUTH_VNUM, VIRTUAL);
    if (temp_obj)
    {
      temp_obj->value[0] = temp_house->owner_guild;
      obj_to_room(temp_obj, real_room0(temp_house->mouth_vnum));
    }
  }
  /* set up golems */
  if ((temp_house->type == HCONTROL_GUILD) ||
      (temp_house->type == HCONTROL_CASTLE) ||
      temp_house->type == HCONTROL_HOUSE)
  {

    for (x = 0; x < temp_house->wizard_golems; x++)
    {
      golem = read_mobile(MAGE_GOLEM_VNUM, VIRTUAL);
      if (!golem)
        break;

      if (temp_house->exit_num == -1)
        golem_block = 'a';
      else
        golem_block = *dirs[temp_house->exit_num];

      sprintf(buff, "golem br_c assoc%d_%c_0", temp_house->owner_guild,
              golem_block);
      golem->player.name = str_dup(buff);
      char_to_room(golem, real_room0(temp_house->room_vnums[0]), 0);
    }
    for (x = 0; x < temp_house->warrior_golems; x++)
    {
      golem = read_mobile(WARRIOR_GOLEM_VNUM, VIRTUAL);
      if (!golem)
        break;

      if (temp_house->exit_num == -1)
        golem_block = 'a';
      else
        golem_block = *dirs[temp_house->exit_num];

      sprintf(buff, "golem br_f assoc%d_%c_0", temp_house->owner_guild,
              golem_block);
      golem->player.name = str_dup(buff);
      char_to_room(golem, real_room0(temp_house->room_vnums[0]), 0);
    }
    for (x = 0; x < temp_house->cleric_golems; x++)
    {
      golem = read_mobile(CLERIC_GOLEM_VNUM, VIRTUAL);
      if (!golem)
        break;

      if (temp_house->exit_num == -1)
        golem_block = 'a';
      else
        golem_block = *dirs[temp_house->exit_num];

      sprintf(buff, "golem br_p assoc%d_%c_0", temp_house->owner_guild,
              golem_block);
      golem->player.name = str_dup(buff);
      char_to_room(golem, real_room0(temp_house->room_vnums[0]), 0);
    }
  }
}

void destroy_castle(P_house temp_house)
{
  int      real_house, virt_house;
  int      temp, i;

  virt_house = temp_house->vnum;
  real_house = real_room0(virt_house);
  if (!real_house)
  {
    return;
  }

  /* remove doorway objects */
  nuke_doorways(real_house);
  nuke_doorways(real_room0(temp_house->room_vnums[0]));

  if (temp_house->type == HCONTROL_GUILD ||
      temp_house->type == HCONTROL_OUTPOST ||
      temp_house->type == HCONTROL_HOUSE)
  {
    world[real_house].dir_option[DOWN] = 0;
  }
  else
  {
    world[real_house].dir_option[temp_house->exit_num] = 0;
  }
  for (temp = 0; temp < MAX_HOUSE_ROOMS; temp++)
  {
    if (temp_house->room_vnums[temp] != -1)
      for (i = 0; i < NUM_EXITS; i++)
        world[real_room0(temp_house->room_vnums[temp])].dir_option[i] = 0;
  }
  if (temp_house->type == HCONTROL_HOUSE)
  {
    REMOVE_BIT(world[real_house].room_flags, ROOM_HOUSE);
    REMOVE_BIT(world[real_house].room_flags, NO_TELEPORT);
    REMOVE_BIT(world[real_house].room_flags, NO_SUMMON);
    REMOVE_BIT(world[real_house].room_flags, NO_GATE);
    REMOVE_BIT(world[real_house].room_flags, TWILIGHT);
    return;
  }
  if (temp_house->type == HCONTROL_CASTLE)
  {
    REMOVE_BIT(world[real_house].room_flags, ROOM_HOUSE);
    REMOVE_BIT(world[real_house].room_flags, NO_TELEPORT);
    REMOVE_BIT(world[real_house].room_flags, NO_SUMMON);
    REMOVE_BIT(world[real_house].room_flags, NO_GATE);
    REMOVE_BIT(world[real_house].room_flags, TWILIGHT);
    world[real_house].sector_type = SECT_FIELD;

    /* remove the 8 walls */
    world[real_room0(virt_house - 99)].sector_type = SECT_FIELD;
    world[real_room0(virt_house - 100)].sector_type = SECT_FIELD;
    world[real_room0(virt_house - 101)].sector_type = SECT_FIELD;
    world[real_room0(virt_house - 1)].sector_type = SECT_FIELD;
    world[real_room0(virt_house + 1)].sector_type = SECT_FIELD;
    world[real_room0(virt_house + 99)].sector_type = SECT_FIELD;
    world[real_room0(virt_house + 100)].sector_type = SECT_FIELD;
    world[real_room0(virt_house + 101)].sector_type = SECT_FIELD;
/*    world[world[real_house].dir_option[temp_house->exit_num]->to_room].sector_type = SECT_FIELD;*/
  }
}

void hcontrol_build_house(P_char ch, char *arg)
{
  char     name[MAX_INPUT_LENGTH];
  char     keyword[MAX_INPUT_LENGTH];
  char     buff[MAX_INPUT_LENGTH];
  P_house  temp_house;
  int      virt_house, real_house, real_atrium, virt_atrium, i;
  int      guild = 0, direction = 0, type = 0;
  int      instant;

  name[0] = '\0';

  instant = IS_TRUSTED(ch);

  hcontrol_parser(arg, &type, name, &direction, &guild);
  if ((type == HCONTROL_ERROR) || !name ||
      !name[0] /*|| !direction || !keyword || !keyword[0] */ )
  {
    if (IS_TRUSTED(ch))
    {
      send_to_char(HCONTROL_FORMAT, ch);
      return;
    }
    /* when automated, only error out if type is invalid */

    else
    {                           /*if (type == HCONTROL_ERROR) */
      send_to_char("Error constructing. Try again.\r\n", ch);
      return;
    }
  }
  /* gets the array back on track */
  /* direction --; */

  real_atrium = ch->in_room;
  virt_atrium = world[real_atrium].number;

  if (!dead_house_pool)
    dead_house_pool = mm_create("HOUSE",
                                sizeof(struct house_control_rec),
                                offsetof(struct house_control_rec, next), 1);

  temp_house = (struct house_control_rec *) mm_get(dead_house_pool);

  if (!instant)
    temp_house->construction = 1;
  else
    temp_house->construction = 0;

  for (i = 0; i < MAX_HOUSE_ROOMS; i++)
    temp_house->room_vnums[i] = -1;

  if (type == HCONTROL_GUILD || type == HCONTROL_HOUSE ||
      type == HCONTROL_OUTPOST)
    direction = NORTH;

  virt_house = hcontrol_setup_room(temp_house, virt_atrium, -1);
  hcontrol_setup_room(temp_house, virt_house, direction);

  real_house = real_room0(virt_house);
  if (!real_house)
  {
    send_to_char("Couldnt allocate house room!\r\n", ch);
    return;
  }
  temp_house->mode = HMODE_OPEN;
  temp_house->type = type;
  temp_house->vnum = virt_atrium;
  temp_house->exit_num = direction;
  temp_house->built_on = time(0);
  temp_house->last_payment = 0;
  temp_house->owner = str_dup(name);
  temp_house->num_of_guests = 0;
  temp_house->size = 1;
  temp_house->upgrades = 0;
  temp_house->mouth_vnum = 0;
  temp_house->holy_fount_vnum = 0;
  temp_house->window_vnum = 0;
  temp_house->unholy_fount_vnum = 0;
  temp_house->secret_entrance = 0;
  temp_house->inn_vnum = 0;
  temp_house->warrior_golems = 0;
  temp_house->wizard_golems = 0;
  temp_house->cleric_golems = 0;
  temp_house->guard_golems = 0;
  temp_house->heal_vnum = 0;
  for (i = 0; i < MAX_CONTROLLED_LAND; i++)
    temp_house->controlled_land[i] = -1;
  if (temp_house->type == HCONTROL_HOUSE ||
      temp_house->type == HCONTROL_GUILD)
    temp_house->entrance_keyword = str_dup(J_NAME(ch));
  else
    temp_house->entrance_keyword = 0;
  /* houses start with 1 room, guilds/castles start with 2 */
  //  temp_house->num_of_rooms = 2;

  if (instant)
    temp_house->owner_guild = guild;    /* GUILD */
  temp_house->sack_list = 0;

  for (i = 0; i < MAX_GUESTS; i++)
    temp_house->guests[i] = NULL;

  temp_house->next = first_house;
  first_house = temp_house;

  writeConstructionQ();

  construct_castle(temp_house);

  if (!writeHouse(temp_house))
    wizlog(56, "Problem saving house %d", temp_house->vnum);
}


int delete_guild_room(int vnum)
{
  char     Gbuf1[MAX_STRING_LENGTH], Gbuf2[MAX_STRING_LENGTH];

  if (vnum < 0)
  {
    logit(LOG_EXIT, "invalid guild room %d in delete_guild_room", vnum);
    raise(SIGSEGV);
  }
  sprintf(Gbuf1, "%s/House/HouseRoomData/%d", SAVE_DIR, vnum);
  strcpy(Gbuf2, Gbuf1);
  strcat(Gbuf2, ".bak");
  unlink(Gbuf1);
  unlink(Gbuf2);

  return TRUE;
}
void hcontrol_destroy_house(P_char ch, char *arg)
{
  int      real_atrium, real_house;
  int      house_nr, i;
  P_house  house = NULL, temp_house;
  char     buf[MAX_STRING_LENGTH], housefile[MAX_STRING_LENGTH];

  if (!first_house)
  {
    debug("Attempt to destroy a house with an empty house list.");
    return;
  }

  if (!*arg)
  {
    if (ch)
      send_to_char(HCONTROL_FORMAT, ch);
    return;
  }

  if (is_number(arg))
  {
    if (!(house = find_house(atoi(arg))))
    {
      if (ch)
        send_to_char("Unknown house.\r\n", ch);
      return;
    }
  }
  else
  {
    if (ch)
      send_to_char(HCONTROL_FORMAT, ch);
    return;
  }

  real_house = real_room0(house->vnum);
  if (!real_room0(house->vnum))
  {
    logit(LOG_HOUSE, "House had invalid vnum!");
    return;
  }
  else
    REMOVE_BIT(world[real_house].room_flags, ROOM_ATRIUM);

  house_nr = house->vnum;
  for (i = 0; i < MAX_HOUSE_ROOMS; i++)
  {
    if (house->room_vnums[i] != -1 &&
        !delete_guild_room(house->room_vnums[i]))
      logit(LOG_HOUSE, "Could not delete room number %d!",
            house->room_vnums[i]);
  }

  destroy_castle(house);        /* Does most of the flag and sector work */
  str_free(house->owner);
  for (i = 0; i < MAX_GUESTS; i++)
    str_free(house->guests[i]);

  if (house == first_house)
  {
    first_house = house->next;
  }
  else
  {
    for (temp_house = first_house; temp_house && temp_house->next != house;
         temp_house = temp_house->next)
      ;
    if (temp_house)
      temp_house->next = house->next;
  }

  sprintf(housefile, "%s/House/HouseRoom/house.%d", SAVE_DIR, house->vnum);
  mm_release(dead_house_pool, house);
  unlink(housefile);
  if (ch)
    send_to_char("House deleted.\r\n", ch);

  /*
   * Now, reset the ROOM_ATRIUM flag on all existing houses' atriums,
   * just in case the house we just deleted shared an atrium with another
   * house.
   */
  for (house = first_house; house; house = house->next)
    if ((real_atrium = real_room(house->room_vnums[0])) >= 0)
      SET_BIT(world[real_atrium].room_flags, ROOM_ATRIUM);
}


void hcontrol_pay_house(P_char ch, char *arg)
{
  P_house  house;
  char     buf[MAX_STRING_LENGTH];

  if (!*arg)
    send_to_char(HCONTROL_FORMAT, ch);
  else if (!(house = find_house(atoi(arg))))
    send_to_char("Unknown house.\r\n", ch);
  else
  {
    sprintf(buf, "Payment for house %s collected by %s.", arg, GET_NAME(ch));
    wizlog(56, buf);

    house->last_payment = time(0);
    send_to_char("Payment recorded.\r\n", ch);
    if (!writeHouse(house))
      wizlog(56, "Problem saving house %d", house->vnum);
  }
}

void hcontrol_clear_house(P_char ch, char *arg)
{
  P_house  house;

  if (!arg)
  {
    send_to_char("Supply a house vnum please.\r\n", ch);
    return;
  }
  house = find_house(atoi(arg));
  if (!house)
    return;
  house->teleporter1_room = 0;
  house->teleporter1_dest = 0;
  house->teleporter2_room = 0;
  house->teleporter2_dest = 0;
  send_to_char("Teleporters cleared.\r\n", ch);
  writeHouse(house);
  return;
}

void hcontrol_portals_house(P_char ch, char *arg)
{
  P_house  house;
  char     rest[MAX_STRING_LENGTH], current[MAX_STRING_LENGTH];
  char     out[MAX_STRING_LENGTH];
  int      newnum = 0, portal = 0, type = 0;

  half_chop(arg, current, rest);

  if (!current || !*current)
  {
    send_to_char("Supply a house vnum please.\r\n", ch);
    return;
  }
  house = find_house(atoi(current));
  if (!house)
  {
    send_to_char("Invalid house num.\r\n", ch);
    return;
  }

  if (!rest || !*rest)
  {
    sprintf(out, "Teleporter 1: Room[%d] Destination[%d]\r\n"
            "Teleporter 2: Room[%d] Destination[%d]\r\n",
            house->teleporter1_room, house->teleporter1_dest,
            house->teleporter2_room, house->teleporter2_dest);
    send_to_char("hcontrol portals <address> [portal #] [r|d] [new #]\r\n",
                 ch);
    send_to_char(out, ch);
    return;
  }
  else
  {
    half_chop(rest, current, rest);
    if (!current || !*current || !(portal = atoi(current)))
    {
      send_to_char("Choose portal 1 or 2.\r\n", ch);
      return;
    }
    half_chop(rest, current, rest);
    if (current && *current == 'r')
      type = 1;
    if (current && *current == 'd')
      type = 2;
    if (!type)
    {
      send_to_char("Choose [r]oom or [d]estination\r\n", ch);
      return;
    }
    half_chop(rest, current, rest);
    if (!current || !*current || !(newnum = atoi(current)))
    {
      send_to_char("Invalid room number (last arg)\r\n", ch);
      return;
    }
    if (!real_room(newnum))
    {
      send_to_char("That room does not exist!\r\n", ch);
      return;
    }
    if (portal == 1)
    {
      if (type == 1)
        house->teleporter1_room = newnum;
      else
        house->teleporter1_dest = newnum;
    }
    else if (portal == 2)
    {
      if (type == 1)
        house->teleporter2_room = newnum;
      else
        house->teleporter2_dest = newnum;
    }
    else
    {
      send_to_char("Invalid portal number. Choose 1 or 2.\r\n", ch);
      return;
    }
  }

  send_to_char
    ("Portal modified.\r\nYou must restore this house to load changes.\r\n",
     ch);
  writeHouse(house);
  return;
}

void hcontrol_restore_house(P_char ch, char *arg)
{
  P_house  house;
  char     id[80];


  if (!arg)
  {
    send_to_char("Supply a house vnum please.\r\n", ch);
    return;
  }
  house = find_house(atoi(arg));
  if (!house)
  {
    send_to_char("House not found.\r\n", ch);
    return;
  }
  sprintf(id, "%d", house->vnum);
  destroy_castle(house);
  restoreHouse(id);
}

void hcontrol_detail_house(P_char ch, char *arg)
{
  char     buf[256];
  P_house  house;
  int      x;

  if (!arg)
  {
    send_to_char("Supply a house vnum please.\r\n", ch);
    return;
  }
  house = find_house(atoi(arg));
  if (!house)
  {
    send_to_char("Can't find that house.\r\n", ch);
    return;
  }
  sprintf(buf, "&+RHouse:&n &+Y%d&n\r\n", atoi(arg));
  send_to_char(buf, ch);
  for (x = 0; x < MAX_HOUSE_ROOMS; x++)
  {
    if (house->room_vnums[x] != -1)
    {
/*      sprintf(buf, "&+RRoom:&n %d  &+RRoom:&n %d  &+RRoom:&N %d\r\n ", house->room_vnums[x], 
	house->room_vnums[x + 1], house->room_vnums[x + 2]);
      x += 2;
    } else if (x < (house->num_of_rooms - 1)) {
      sprintf(buf, "&+RRoom:&n %d  &+RRoom:&n %d\r\n", house->room_vnums[x], 
	house->room_vnums[x + 1]);
      x += 1;
    } else */
      sprintf(buf, "&+RRoom:&n %d\r\n", house->room_vnums[x]);
      send_to_char(buf, ch);
    }
  }

  sprintf(buf, "Exit Number:%d\r\n", house->exit_num);
  send_to_char(buf, ch);
//  sprintf(buf, "Number of room:%d\r\n", house->num_of_rooms);
//  send_to_char(buf, ch);

  if (house->type == HCONTROL_CASTLE)
  {
    send_to_char("&+RControlled Lands:&n\r\n", ch);
    for (x = 0; x < MAX_CONTROLLED_LAND; x++)
      if (house->controlled_land[x] != -1)
      {
        sprintf(buf, "%d ", house->controlled_land[x]);
        send_to_char(buf, ch);
      }
  }
  send_to_char("\r\n", ch);
}

void hcontrol_move_house(P_char ch, char *arg)
{
  P_house  house;
  char     arg0[MAX_STRING_LENGTH], arg1[MAX_STRING_LENGTH];
  char     arg2[MAX_STRING_LENGTH], arg3[MAX_STRING_LENGTH];
  char     arg4[MAX_STRING_LENGTH];
    
  if (!arg)
  {
    send_to_char("Supply a house vnum please.\r\n", ch);
    return;
  }
  
  half_chop(arg, arg1, arg0);
  half_chop(arg0, arg2, arg3);
  
  if (!*arg1 || !*arg2)
  {
    send_to_char(HCONTROL_FORMAT, ch);
    return;
  }
  /* arg1 vnum arg2 field arg3 value */
  
  int old_vnum = atoi(arg1);
  
  house = find_house(old_vnum);
  if (!house)
  {
    send_to_char("Can't find that house number.\r\n", ch);
    return;
  }
    
  if( !is_number(arg2) )
  {
    send_to_char("Invalid destination.\r\n", ch);
    return;    
  }
  
  int new_vnum = atoi(arg2);
  int new_rnum = real_room0(new_vnum);
  
  if( !new_rnum )
  {
    send_to_char("Invalid destination!\r\n", ch);
    return;
  }
    
  if( !valid_build_location( new_rnum, NULL, house->type ) )
  {
    send_to_char("You can't build anything there!\r\n", ch);
    return;
  }
  
  if( find_house(new_vnum) )
  {
    send_to_char("There is already a house there!\r\n", ch);
    return;
  }

  if( !moveHouse(house, new_vnum ) )
  {
    send_to_char("Error renaming house file. House not moved.\r\n", ch);
    return;
  }  

  house->vnum = new_vnum;

  if (!writeHouse(house))
  {
    send_to_char("Problem saving house. Not moved.\r\n", ch);
    house->vnum = old_vnum;
    return;    
  }
  
  disconnect_exit(old_vnum, DOWN);
  connect_rooms(house->vnum, house->room_vnums[0], DOWN);

  logit(LOG_WIZ, "House %d moved to %d by %s", old_vnum, new_vnum, GET_NAME(ch)); 
  wizlog(56, "House %d moved to %d by %s", old_vnum, new_vnum, GET_NAME(ch)); 
  send_to_char("House moved.\r\n", ch);
}

void hcontrol_change_house(P_char ch, char *arg)
{
  P_house  house;
  char     arg0[MAX_STRING_LENGTH], arg1[MAX_STRING_LENGTH];
  char     arg2[MAX_STRING_LENGTH], arg3[MAX_STRING_LENGTH];
  char     arg4[MAX_STRING_LENGTH];
  int      x;

  if (!arg)
  {
    send_to_char("Supply a house vnum please.\r\n", ch);
    return;
  }
  half_chop(arg, arg1, arg0);
  half_chop(arg0, arg2, arg3);
  half_chop(arg3, arg3, arg4);

  if (!*arg1 || !*arg2 || !*arg3 || !*arg)
  {
    send_to_char(HCONTROL_FORMAT, ch);
    return;
  }
  /* arg1 vnum arg2 field arg3 value */

  house = find_house(atoi(arg1));
  if (!house)
  {
    send_to_char("Use build to create a new house type.\r\n", ch);
    return;
  }
  /* field check */
  if (is_abbrev(arg2, "owner"))
  {
    house->owner = str_dup(arg3);
  }
  else if (is_abbrev(arg2, "guild"))
  {
    if (!is_number(arg3))
    {
      send_to_char("Value for that field appears wrong.\r\n", ch);
      return;
    }
    house->owner_guild = atoi(arg3);
  }
  else if (is_abbrev(arg2, "upgrades"))
  {
    if (!is_number(arg3))
    {
      send_to_char("Value for that field appears wrong.\r\n", ch);
      return;
    }
    house->upgrades = atoi(arg3);
  }
  else if (is_abbrev(arg2, "land"))
  {
    if (!is_number(arg3))
    {
      send_to_char("Value for that field appears wrong.\r\n", ch);
      return;
    }
    for (x = 0; x < MAX_CONTROLLED_LAND; x++)
    {
      if (house->controlled_land[x] == -1)
      {
        house->controlled_land[x] = atoi(arg3);
        break;
      }
      else if (house->controlled_land[x] == atoi(arg3))
      {
        house->controlled_land[x] = -1;
        break;
      }
    }
  }                             /*else if (is_abbrev(arg2, "numrooms")) {
                                   if (!is_number(arg3)) {
                                   send_to_char("Value for that field appears wrong.\r\n", ch);
                                   return;
                                   }
                                   house->num_of_rooms = atoi(arg3);
                                   } */
  else if (is_abbrev(arg2, "room"))
  {
    if (!is_number(arg3))
    {
      send_to_char("Value for that field appears wrong.\r\n", ch);
      return;
    }
    for (x = 0; x < MAX_HOUSE_ROOMS; x++)
    {
      if (house->room_vnums[x] == -1)
      {
        house->room_vnums[x] = atoi(arg3);
        break;
      }
      else if (house->room_vnums[x] == atoi(arg3))
      {
        house->room_vnums[x] = -1;
        break;
      }
    }
//    send_to_char("Be sure to change num_of_rooms!!!\r\n", ch);
  }
  else if (is_abbrev(arg2, "type"))
  {
    if (is_abbrev(arg3, "house"))
      house->type = HCONTROL_HOUSE;
    else if (is_abbrev(arg3, "guild"))
      house->type = HCONTROL_GUILD;
    else if (is_abbrev(arg3, "outpost"))
      house->type = HCONTROL_OUTPOST;
    else if (is_abbrev(arg3, "castle"))
      house->type = HCONTROL_CASTLE;
  }
  if (!writeHouse(house))
    wizlog(56, "Problem saving house %d", house->vnum);

  send_to_char("Data changed.\r\n", ch);
}

/* The hcontrol command itself, used by imms to create/destroy houses */
void do_hcontrol(P_char ch, char *arg, int cmd)
{
  char     arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

  half_chop(arg, arg1, arg2);

  if (is_abbrev(arg1, "build"))
    hcontrol_build_house(ch, arg2);
  else if (is_abbrev(arg1, "destroy"))
    hcontrol_destroy_house(ch, arg2);
  else if (is_abbrev(arg1, "pay"))
    hcontrol_pay_house(ch, arg2);
  else if (is_abbrev(arg1, "list"))
    hcontrol_list_houses(ch, arg2);
  else if (is_abbrev(arg1, "detail"))
    hcontrol_detail_house(ch, arg2);
  else if (is_abbrev(arg1, "clear"))
    hcontrol_clear_house(ch, arg2);
  else if (is_abbrev(arg1, "change"))
    hcontrol_change_house(ch, arg2);
  else if (is_abbrev(arg1, "restore"))
    hcontrol_restore_house(ch, arg2);
  else if (is_abbrev(arg1, "portals"))
    hcontrol_portals_house(ch, arg2);
  else if (is_abbrev(arg1, "move"))
    hcontrol_move_house(ch, arg2);
  else
    send_to_char(HCONTROL_FORMAT, ch);
}

/* Simply determines how much a house will cost a player to initially build */
int house_cost(P_char ch)
{
  double   cost = 0;

  if (!ch)
    return (int) cost;

  /* Base cost is x plat */
  cost = 2000;

  /* Modify by justice */
  if (CHAR_IN_JUSTICE_AREA(ch))
  {
    if (PC_TOWN_JUSTICE_FLAGS(ch, CHAR_IN_JUSTICE_AREA(ch)) ==
        JUSTICE_IS_CITIZEN)
    {
      cost /= 3;
    }
    else if (PC_TOWN_JUSTICE_FLAGS(ch, CHAR_IN_JUSTICE_AREA(ch)) ==
             JUSTICE_IS_CITIZEN_BUY)
    {
      cost /= 2;
    }
    else if (PC_TOWN_JUSTICE_FLAGS(ch, CHAR_IN_JUSTICE_AREA(ch)) ==
             JUSTICE_IS_NORMAL)
    {
      cost /= 1.5;
    }
    else if (PC_TOWN_JUSTICE_FLAGS(ch, CHAR_IN_JUSTICE_AREA(ch)) ==
             JUSTICE_IS_OUTCAST)
    {
      cost *= 2;
    }
  }

  /* Modify by terrain */
  if (world[ch->in_room].sector_type == SECT_MOUNTAIN)
    cost *= 3;
  if (world[ch->in_room].sector_type == SECT_HILLS)
    cost *= 2;
  if (world[ch->in_room].sector_type == SECT_FOREST)
    cost *= 1.5;

  return (int) cost;
}

/* Simply determines how much a guildhall will cost to initially build */
int guild_cost(P_char ch)
{
  double   cost = 0;

  if (!ch)
    return (int) cost;

  /* Base cost is x plat */
  cost = 7000;

  /* Modify by justice */
  if (CHAR_IN_JUSTICE_AREA(ch))
  {
    if (PC_TOWN_JUSTICE_FLAGS(ch, CHAR_IN_JUSTICE_AREA(ch)) ==
        JUSTICE_IS_CITIZEN)
    {
      cost /= 1.5;
    }
    else if (PC_TOWN_JUSTICE_FLAGS(ch, CHAR_IN_JUSTICE_AREA(ch)) ==
             JUSTICE_IS_CITIZEN_BUY)
    {
      cost /= 1.25;
    }
    else if (PC_TOWN_JUSTICE_FLAGS(ch, CHAR_IN_JUSTICE_AREA(ch)) ==
             JUSTICE_IS_NORMAL)
    {
      cost *= 1.5;
    }
    else if (PC_TOWN_JUSTICE_FLAGS(ch, CHAR_IN_JUSTICE_AREA(ch)) ==
             JUSTICE_IS_OUTCAST)
    {
      cost *= 2;
    }
  }

  /* Modify by terrain */
  if (world[ch->in_room].sector_type == SECT_MOUNTAIN)
    cost *= 3;
  if (world[ch->in_room].sector_type == SECT_HILLS)
    cost *= 2;
  if (world[ch->in_room].sector_type == SECT_FOREST)
    cost *= 1.5;

  return (int) cost;
}

/* The house command, used by mortal house owners to assign guests */
void do_house(P_char ch, char *arg, int cmd)
{
  int      cost;
  char     arg1[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  P_house  house;
  int      j = 0;

  // if (IS_NPC(ch))
  //  return;                   /* NPCs bad */

  one_argument(arg, arg1);

  if (arg1 && !strcmp(arg1, "cost"))
  {
    if (!valid_build_location(ch->in_room, ch, 0))
      return;
    cost = house_cost(ch);
    cost *= 1000;
    sprintf(buf, "Building a house here would cost %s,", coin_stringv(cost));
    send_to_char(buf, ch);
    cost = guild_cost(ch);
    cost *= 1000;
    sprintf(buf, " and a guildhall would be %s.\r\n", coin_stringv(cost));
    send_to_char(buf, ch);
    return;
  }
  else if (arg1 && !strcmp(arg1, "sell"))
  {
    send_to_char("None would want to buy your house..\r\n", ch);
    return;
    half_chop(arg1, arg, arg1);
    if (!IS_SET(world[ch->in_room].room_flags, ROOM_HOUSE))
    {
      send_to_char("You must be in your house to sell it.\r\n", ch);
      return;
    }
    else if (!(house = find_house(world[ch->in_room].number)))
    {
      send_to_char("Um.. this house seems to be screwed up.\r\n", ch);
      return;
    }
    else if (str_cmp(house->owner, J_NAME(ch)))
    {
      send_to_char("Only the primary owner may sell a house.\r\n", ch);
      return;
    }
    if (!*arg1)
    {
      /* open market sell */
      sprintf(buf, "%d", house->vnum);
      // what in the hell is this?
      //AddEvent(EVENT_SPECIAL, number(10, 100), TRUE, open_market_house, buf);
      send_to_char
        ("Your building has been placed upon the open market. Good luck!\r\n",
         ch);
      return;
    }
    else
    {
      if (!is_number(arg1))
      {
        house->owner = str_dup(arg1);
        sprintf(buf, "Buildings deed transfered to %s.\r\n", house->owner);
        send_to_char(buf, ch);
        return;
      }
      else
      {
        send_to_char("Eh?!? Who the hell is that?\r\n", ch);
        return;
      }
    }
#if 1
    /* no guests anymore. Kepp though, just in case someday... */
  }
  else if (!IS_SET(world[ch->in_room].room_flags, ROOM_HOUSE))
  {
    send_to_char("You must be in your house to set guests.\r\n", ch);
    return;
  }
  else if (!(house = find_house(world[ch->in_room].number)))
  {
    send_to_char("Um.. this house seems to be screwed up.\r\n", ch);
    return;
  }
  else if (str_cmp(house->owner, GET_NAME(ch)))
  {
    send_to_char("Only the primary owner can set guests.\r\n", ch);
    return;
  }
  else if (!*arg1)
  {
    strcpy(buf, "&+GGuests of your house:&N\r\n");
    if (house->num_of_guests == 0)
      strcat(buf, " None.\r\n");
    else
      for (j = 0; j < house->num_of_guests; j++)
      {
        sprintf(buf + strlen(buf), "%s\r\n", house->guests[j]);
      }
    send_to_char(buf, ch);

    return;
  }
  else
  {
    for (j = 0; j < house->num_of_guests; j++)
    {
      if (!str_cmp(house->guests[j], arg1))
      {
        for (; j < house->num_of_guests; j++)
          house->guests[j] = house->guests[j + 1];

        house->num_of_guests--;
        send_to_char("Guest deleted.\r\n", ch);

        if (!writeHouse(house))
          wizlog(56, "Problem saving house %d", house->vnum);

        return;
      }
    }

    j = house->num_of_guests;
    if (j < MAX_GUESTS)
    {
      if (GET_MONEY(ch) < 100000)
      {
        send_to_char("Adding a guest will cost you a 100 platinum tax.\r\n",
                     ch);
        return;
      }
      SUB_MONEY(ch, 100000, 0);
      house->guests[j] = str_dup(arg1);
      send_to_char("Guest added, 100 platinum tax paid.\r\n", ch);
      house->num_of_guests++;

      if (!writeHouse(house))
        wizlog(56, "Problem saving house %d", house->vnum);

      return;
    }
    else
    {
      send_to_char("You may not add any more guests to your house.\r\n", ch);

      return;
    }
#endif
  }
}

/* note: arg passed must be house vnum, so there. */
int House_can_enter(P_char ch, int vnum, int exitnumb)
{
  int      j;
  P_house  house;

  /* exitnumb is -1 if we're dealing with teleporter entrance object,
     as opposed to a true door */

  house = find_house(vnum);

  if (IS_TRUSTED(ch))
    return TRUE;
  if (!house && (exitnumb != -1))
    return TRUE;
  if (IS_NPC(ch) && IS_SET(ch->only.npc->spec[2], MOB_SPEC_JUSTICE))
    return TRUE;
  if (house && (exitnumb != -1) && (exitnumb != house->exit_num))
    return TRUE;
  if (house)
  {
    switch (house->mode)
    {
    case HMODE_PRIVATE:
      if (!str_cmp(house->owner, J_NAME(ch)))
        return TRUE;
      for (j = 0; j < house->num_of_guests; j++)
        if (!str_cmp(house->guests[j], J_NAME(ch)))
          return TRUE;
      return FALSE;
      break;
    case HMODE_OPEN:
      return TRUE;
      break;
    }
  }
  return FALSE;
}

/* guild room data stored as follows:
   filename is 
   Name~
   Desc~
   exit info (same as .wld file)
 */

void read_guild_room(int vnum, int guild)
{
  FILE    *hfile;
  char     buffer[MAX_STRING_LENGTH];
  int      room_nr, x;
  char    *tmp;
  int      dir, temp;

  sprintf(buffer, "%s/House/HouseRoomData/%d", SAVE_DIR, vnum);
  if (!(hfile = fopen(buffer, "r")))
    return;
  room_nr = real_room0(vnum);
  if (!room_nr)
  {
    logit(LOG_HOUSE, "Room %d didn't exist?", vnum);
    return;
  }
  fscanf(hfile, " %d \n", &temp);
  if (temp != guild)
  {
    logit(LOG_HOUSE,
          "read_guild_room: room #%d belonged to guild #%d instead of %d",
          vnum, temp, guild);
    return;
  }

  tmp = fread_string(hfile);
  world[room_nr].name = tmp;
  tmp = fread_string(hfile);
  world[room_nr].description = tmp;
  SET_BIT(world[room_nr].room_flags, ROOM_HOUSE);
  SET_BIT(world[room_nr].room_flags, NO_TELEPORT);
  SET_BIT(world[room_nr].room_flags, NO_RECALL);
  SET_BIT(world[room_nr].room_flags, NO_SUMMON);
  SET_BIT(world[room_nr].room_flags, NO_GATE);
  SET_BIT(world[room_nr].room_flags, TWILIGHT);
  SET_BIT(world[room_nr].room_flags, GUILD_ROOM);
  for (x = 0; x < NUM_EXITS; x++)
    world[room_nr].dir_option[x] = 0;
  for (;;)
  {
    fscanf(hfile, " %s \n", buffer);
    if (*buffer == 'D')
    {
      dir = atoi(buffer + 1);
      CREATE(world[room_nr].dir_option[dir], room_direction_data, 1, MEM_TAG_DIRDATA);

      world[room_nr].dir_option[dir]->general_description =
        fread_string(hfile);
      world[room_nr].dir_option[dir]->keyword = fread_string(hfile);
      fscanf(hfile, " %d ", &temp);
      temp = 0;                 //Disable doors in GH
      if (temp)
      {
        world[room_nr].dir_option[dir]->exit_info = EX_ISDOOR;
        if (temp == 2)
          world[room_nr].dir_option[dir]->exit_info |= EX_PICKABLE;
        if (temp == 3)
          world[room_nr].dir_option[dir]->exit_info |= EX_PICKPROOF;
        if (temp > 3)
          world[room_nr].dir_option[dir]->exit_info |= EX_SECRET;
      }
      else
      {
        world[room_nr].dir_option[dir]->exit_info = 0;
      }
      fscanf(hfile, " %d ", &temp);
      world[room_nr].dir_option[dir]->key = temp;
      fscanf(hfile, " %d ", &temp);
      world[room_nr].dir_option[dir]->to_room = real_room(temp);
#if 0
      if (world[world[room_nr].dir_option[dir]->to_room].zone !=
          world[room_nr].zone)
        logit(LOG_HOUSE,
              "read_guild_room: room #%d belonged to guild #%d instead of %d",
              vnum, world[room_nr].zone, guild);
#endif
      *buffer = 'X';
    }
    else
      break;
  }
  fclose(hfile);
};

void write_guild_room(int vnum, int guild)
{
  FILE    *hfile;
  int      x, room_nr, info;
  char     buffer[MAX_STRING_LENGTH];

  room_nr = real_room0(vnum);
  if (!room_nr)
  {
    logit(LOG_HOUSE, "House %d appears bogus in write_guild_room", vnum);
    return;
  }
  sprintf(buffer, "%s/House/HouseRoomData/%d", SAVE_DIR, vnum);
  if (!(hfile = fopen(buffer, "w")))
  {
    logit(LOG_HOUSE, "Cannot open %s for writing guild rooms.", buffer);
    return;
  }
  fprintf(hfile, "%d\n", guild);
  if (strstr(world[room_nr].name, "Unnamed"))
    world[room_nr].name = str_dup("A Barren Room");
  fprintf(hfile, "%s~\n", world[room_nr].name);
  fprintf(hfile, "%s~\n", world[room_nr].description);
  for (x = 0; x < NUM_EXITS; x++)
  {
    if (world[room_nr].dir_option[x] &&
        world[world[room_nr].dir_option[x]->to_room].number >= 48000 &&
        world[world[room_nr].dir_option[x]->to_room].number <= 49999)
    {
      fprintf(hfile, "D%d\n", x);
      if (!world[room_nr].dir_option[x]->general_description)
        fprintf(hfile, "~\n");
      else
        fprintf(hfile, "%s~\n",
                world[room_nr].dir_option[x]->general_description);
      if (!world[room_nr].dir_option[x]->keyword)
        fprintf(hfile, "~\n");
      else
        fprintf(hfile, "%s~\n", world[room_nr].dir_option[x]->keyword);
      info = 0;
      if (IS_SET(world[room_nr].dir_option[x]->exit_info, EX_ISDOOR))
        info = 1;
      if (IS_SET(world[room_nr].dir_option[x]->exit_info, EX_PICKABLE))
        info = 2;
      if (IS_SET(world[room_nr].dir_option[x]->exit_info, EX_PICKPROOF))
        info = 3;
      if (IS_SET(world[room_nr].dir_option[x]->exit_info, EX_SECRET))
        info = 4;
      fprintf(hfile, "%d %d %d\n", info,
              world[room_nr].dir_option[x]->key,
              world[world[room_nr].dir_option[x]->to_room].number);
    }
  }
  fclose(hfile);
};


int hcontrol_get_free_room()
{
  int      i, nr;

  for (i = START_HOUSE_VNUM; i < END_HOUSE_VNUM; i++)
  {
    if (strstr(world[real_room0(i)].name, "Unnamed"))
      break;
  }

  if (!strstr(world[real_room0(i)].name, "Unnamed"))
    return -1;                  /* no more rooms! */

  nr = real_room0(i);
  if (!nr)
    return -1;
  else
    return nr;
}

int hcontrol_setup_room(P_house house, int cur_room, int exit_dir)
{
  /* setup the default name, dir, etc for room */
  int      virt_nr, i, nr;
  P_room   room;

  for (i = 0; i < MAX_HOUSE_ROOMS && house->room_vnums[i] != -1; i++)
    ;

  if (i == MAX_HOUSE_ROOMS)
    return -1;

  if ((nr = hcontrol_get_free_room()) == -1)
    return -1;

  virt_nr = world[nr].number;
  house->room_vnums[i] = virt_nr;

  room = &world[nr];

  virt_nr = room->number;

  room->name = str_dup("A Barren Room");
  SET_BIT(room->room_flags, ROOM_HOUSE);
  SET_BIT(room->room_flags, NO_TELEPORT);
  SET_BIT(room->room_flags, NO_SUMMON);
  SET_BIT(room->room_flags, NO_GATE);
  SET_BIT(room->room_flags, TWILIGHT);
  if (house->type == HCONTROL_GUILD)
    SET_BIT(room->room_flags, GUILD_ROOM);
  room->description = str_dup("This room is empty.\r\n");
  /* setup exits to/from atrium to first room */
  if (exit_dir >= 0)
    connect_rooms(cur_room, virt_nr, exit_dir);

  return virt_nr;
}

P_house house_ch_is_in(P_char ch)
{
  int      x, flag;
  P_house  temp_house;

  temp_house = first_house;
  flag = 0;

  while (!flag && temp_house)
  {
    for (x = 0; x < MAX_HOUSE_ROOMS; x++)
    {
      if (temp_house->room_vnums[x] == world[ch->in_room].number)
      {
        flag = 1;
        return temp_house;

      }
    }
    temp_house = temp_house->next;
  }
  return temp_house;
}

/* Construction Commands */

void do_delete_room(P_house house, P_char ch, char *arg)
{
  char     buff[MAX_STRING_LENGTH];
  int      i, exit_dir;
  int      door;
  int      numb_exits;
  int      room_in_progress = 0;
  char    *door_keyword;
  struct house_upgrade_rec *con_rec;
  P_room   target;

  arg = one_argument(arg, buff);
  if ((exit_dir = dir_from_keyword(buff)) == -1)
  {
    send_to_char(CONSTRUCT_FORMAT, ch);
    return;
  };

  if (!world[ch->in_room].dir_option[exit_dir])
  {                             /* no exit there */
    send_to_char("There is no room there.\r\n", ch);
    return;
  }

  target = &world[world[ch->in_room].dir_option[exit_dir]->to_room];

  if (target->people || target->contents)
  {
    send_to_char("Room must be empty to be deleted.\r\n", ch);
    return;
  }

  numb_exits = 0;
  for (i = 0; i < NUM_EXITS; i++)
    if (target->dir_option[i])
      numb_exits++;

  if (numb_exits != 1)
  {
    send_to_char("There are other rooms connected to that room.\r\n", ch);
    return;
  }

  if (world[ch->in_room].number == house->room_vnums[0])
  {
    send_to_char("This is your golem room, you cannot delete it.\r\n", ch);
    return;
  }

  FREE(target->dir_option[rev_dir[exit_dir]]);
  FREE(target->name);
  target->name = str_dup("Unnamed");
  FREE(target->description);
  target->description = NULL;
  FREE(world[ch->in_room].dir_option[exit_dir]);
  target->dir_option[rev_dir[exit_dir]] = 0;
  world[ch->in_room].dir_option[exit_dir] = 0;
  delete_guild_room(target->number);
  for (i = 0; i < MAX_HOUSE_ROOMS; i++)
    if (house->room_vnums[i] == target->number)
    {
      house->room_vnums[i] = -1;
      break;
    }
  writeHouse(house);
  send_to_char("Room deleted.\r\n", ch);
}

int charge_char(P_char ch, int plat)
{
  char     buff[256];

  if (IS_TRUSTED(ch))
    return TRUE;

  if (!plat)
  {
    send_to_char("Ye don't have the funds.\r\n", ch);
    return FALSE;
  }
  plat *= 1000;

  if (GET_MONEY(ch) < plat)
  {
    sprintf(buff, "Ye don't have the %s required.\r\n", coin_stringv(plat));
    send_to_char(buff, ch);
    return FALSE;
  }
  SUB_MONEY(ch, plat, 0);
  return TRUE;
}

P_house_upgrade get_con_rec()
{

  if (!dead_construction_pool)
    dead_construction_pool = mm_create("CONSTRUCTION",
                                       sizeof(struct house_upgrade_rec),
                                       offsetof(struct house_upgrade_rec,
                                                next), 1);
  return (P_house_upgrade) mm_get(dead_construction_pool);
}

void do_build_window(P_house house, P_char ch)
{
  struct house_upgrade_rec *con_rec;

  if (house->window_vnum)
  {
    send_to_char("Window moved to here.\r\n", ch);
    house->window_vnum = world[ch->in_room].number;
    writeHouse(house);
    return;
  }
  if (!charge_char(ch, upgrade_costs[HCONTROL_ADD_WINDOW]))
    return;

  con_rec = get_con_rec();
  con_rec->vnum = house->vnum;
  con_rec->type = HCONTROL_ADD_WINDOW;
  con_rec->time = time(0);
  con_rec->location = world[ch->in_room].number;
  con_rec->next = house_upgrade_list;
  house_upgrade_list = con_rec;
  writeConstructionQ();
  send_to_char("Window construction begun.\r\n", ch);
}

void do_build_movedoor(P_house house, P_char ch, char *arg)
{
  struct house_upgrade_rec *con_rec;
  P_house  thouse = NULL;
  P_char   target_char = NULL;

  if (!IS_TRUSTED(ch))
  {
    send_to_char("You're not godly enough to move guildhalls!\r\n", ch);
    return;
  }
  
  send_to_char("construct move disabled - use hcontrol move instead.\n", ch);
  return;  

  target_char = get_char_vis(ch, arg);
  if (!target_char)
  {
    send_to_char("You need a valid target in the guildhall!\r\n", ch);
    return;
  }

  if (GET_A_NUM(target_char) != GET_A_NUM(ch))
  {
    send_to_char("Target must be in your guild.\r\n", ch);
    return;
  }
  thouse = house_ch_is_in(ch);
  if (!thouse)
  {
    send_to_char("Target is not in a valid guild hall.\r\n", ch);
    return;
  }

  if (!charge_char(ch, upgrade_costs[HCONTROL_ADD_MOVEDOOR]))
    return;


  con_rec = get_con_rec();
  con_rec->vnum = house->vnum;
  con_rec->type = HCONTROL_ADD_MOVEDOOR;
  con_rec->time = time(0);
  con_rec->location = world[target_char->in_room].number;
  con_rec->next = house_upgrade_list;
  house_upgrade_list = con_rec;
  writeConstructionQ();
  send_to_char("Door movement begun.\r\n", ch);
}

void do_build_heal(P_house house, P_char ch)
{
  struct house_upgrade_rec *con_rec;

  if (house->heal_vnum)
  {
    send_to_char("Heal Room moved to here.\r\n", ch);
    house->heal_vnum = world[ch->in_room].number;
    writeHouse(house);
    return;
  }
  if (!charge_char(ch, upgrade_costs[HCONTROL_ADD_HEAL]))
    return;

  con_rec = get_con_rec();
  con_rec->vnum = house->vnum;
  con_rec->type = HCONTROL_ADD_HEAL;
  con_rec->time = time(0);
  con_rec->location = world[ch->in_room].number;
  con_rec->next = house_upgrade_list;
  house_upgrade_list = con_rec;
  writeConstructionQ();
  send_to_char("Healing room construction begun.\r\n", ch);
}

void do_build_secret(P_house house, P_char ch)
{
  struct house_upgrade_rec *con_rec;

  if (house->secret_entrance)
  {
    send_to_char("Entrance is already hidden.\r\n", ch);
    return;
  }
  if (!charge_char(ch, upgrade_costs[HCONTROL_ADD_SECRET]))
    return;
  con_rec = get_con_rec();
  con_rec->vnum = house->vnum;
  con_rec->type = HCONTROL_ADD_SECRET;
  con_rec->time = time(0);
  con_rec->location = world[ch->in_room].number;
  con_rec->next = house_upgrade_list;
  house_upgrade_list = con_rec;
  writeConstructionQ();
  send_to_char("Secret entrance construction begun.\r\n", ch);
}

void do_build_holy(P_house house, P_char ch)
{
  struct house_upgrade_rec *con_rec;

  if (house->holy_fount_vnum)
  {
    send_to_char("Holy water fountain location moved to here.\r\n", ch);
    house->holy_fount_vnum = world[ch->in_room].number;
    writeHouse(house);
    return;
  }
  if (!charge_char(ch, upgrade_costs[HCONTROL_ADD_HOLY]))
    return;
  con_rec = get_con_rec();
  con_rec->vnum = house->vnum;
  con_rec->type = HCONTROL_ADD_HOLY;
  con_rec->time = time(0);
  con_rec->location = world[ch->in_room].number;
  con_rec->next = house_upgrade_list;
  house_upgrade_list = con_rec;
  writeConstructionQ();
  send_to_char("Holy water fountain construction begun.\r\n", ch);
}

void do_build_unholy(P_house house, P_char ch)
{
  struct house_upgrade_rec *con_rec;

  if (house->unholy_fount_vnum)
  {
    send_to_char("Unholy water fountain location moved to here.\r\n", ch);
    house->unholy_fount_vnum = world[ch->in_room].number;
    writeHouse(house);
    return;
  }
  if (!charge_char(ch, upgrade_costs[HCONTROL_ADD_UNHOLY]))
    return;
  con_rec = get_con_rec();
  con_rec->vnum = house->vnum;
  con_rec->type = HCONTROL_ADD_UNHOLY;
  con_rec->time = time(0);
  con_rec->location = world[ch->in_room].number;
  con_rec->next = house_upgrade_list;
  house_upgrade_list = con_rec;
  writeConstructionQ();
  send_to_char("Unholy water fountain construction started.\r\n", ch);
}

void do_build_mouth(P_house house, P_char ch)
{
  struct house_upgrade_rec *con_rec;

  if (house->mouth_vnum)
  {
    house->mouth_vnum = world[ch->in_room].number;
    send_to_char("Mouth location moved to here.\r\n", ch);
    writeHouse(house);
    return;
  }
  if (!charge_char(ch, upgrade_costs[HCONTROL_ADD_MOUTH]))
  {
    return;
  }
  con_rec = get_con_rec();
  con_rec->vnum = house->vnum;
  con_rec->type = HCONTROL_ADD_MOUTH;
  con_rec->time = time(0);
  con_rec->location = world[ch->in_room].number;
  con_rec->next = house_upgrade_list;
  house_upgrade_list = con_rec;
  writeConstructionQ();
  send_to_char("Magic Mouth Construction begun.\r\n", ch);
}

void do_build_teleporter(P_house house, P_char ch, char *arg)
{
  struct house_upgrade_rec *con_rec;
  P_char   target;
  P_house  thouse;

  if (house->teleporter1_room && house->teleporter2_room)
  {
    send_to_char("You can only have 2 magic portals per place.\r\n", ch);
    return;
  }
  target = get_char_vis(ch, arg);
  if (!target)
  {
    send_to_char("I don't see that target.\r\n", ch);
    return;
  }
  if (GET_A_NUM(target) != GET_A_NUM(ch))
  {
    send_to_char("Target must be in your guild.\r\n", ch);
    return;
  }
  thouse = house_ch_is_in(target);
  if (!thouse)
  {
    send_to_char("Target is not in a valid guild hall.\r\n", ch);
    return;
  }
  if (thouse->owner_guild != house->owner_guild)
  {
    send_to_char("Target house is not owned by your guild.\r\n", ch);
    return;
  }
  if (thouse->teleporter1_room && thouse->teleporter2_room)
  {
    send_to_char("Target house has no room for teleporters.\r\n", ch);
    return;
  }
//  if ((thouse->num_of_rooms < 9) || (house->num_of_rooms < 9)) {
//    send_to_char("You have to have at least 10 rooms in each house to build.\r\n", ch);
//    return;
//  }
  if (!charge_char(ch, upgrade_costs[HCONTROL_ADD_TELEPORTER]))
    return;

  /* okay, build it */

  con_rec = get_con_rec();
  con_rec->vnum = house->vnum;
  con_rec->type = HCONTROL_ADD_TELEPORTER;
  con_rec->time = time(0);
  con_rec->location = world[ch->in_room].number;
  con_rec->door = world[target->in_room].number;
  con_rec->exit_dir = thouse->vnum;
  con_rec->next = house_upgrade_list;
  house_upgrade_list = con_rec;
  writeConstructionQ();
  send_to_char("Teleporter construction begun.\r\n", ch);
}

void stock_guild_shop(int loc)
{
  int      items[] = {
    204, 412, 3050, 95547, 6089, 53, 203, 5725, 377, 399, 0
  };

  P_obj    tobj;
  P_char   tmob;
  int      x;

  // disabled
  return;

  tmob = read_mobile(SHOPKEEPER_VNUM, VIRTUAL);
  if (!tmob)
    return;
  char_to_room(tmob, real_room0(loc), 0);
  x = 0;
  while (items[x] != 0)
  {
    tobj = read_object(items[x], VIRTUAL);
    if (tobj)
      obj_to_char(tobj, tmob);
    x++;
  }
}

void do_build_shop(P_house house, P_char ch)
{
  struct house_upgrade_rec *con_rec;

  if (house->shop_vnum)
  {
    house->shop_vnum = world[ch->in_room].number;
    writeHouse(house);
    send_to_char("Shop Location moved to here.\r\n", ch);
    return;
  }
  if (!charge_char(ch, upgrade_costs[HCONTROL_ADD_SHOP]))
    return;
  con_rec = get_con_rec();
  con_rec->vnum = house->vnum;
  con_rec->type = HCONTROL_ADD_SHOP;
  con_rec->time = time(0);
  con_rec->location = world[ch->in_room].number;
  con_rec->next = house_upgrade_list;
  house_upgrade_list = con_rec;
  writeConstructionQ();
  send_to_char("Shop Construction begun.\r\n", ch);
}

void do_build_chest(P_house house, P_char ch)
{
  struct house_upgrade_rec *con_rec;

  if (!charge_char(ch, upgrade_costs[HCONTROL_ADD_CHEST]))
  {
    return;
  }
  con_rec = get_con_rec();
  con_rec->vnum = house->vnum;
  con_rec->type = HCONTROL_ADD_CHEST;
  con_rec->time = time(0);
  con_rec->location = world[ch->in_room].number;
  con_rec->next = house_upgrade_list;
  house_upgrade_list = con_rec;
  writeConstructionQ();
  send_to_char("Chest construction begun.\r\n", ch);
}

void do_build_fountain(P_house house, P_char ch)
{
  struct house_upgrade_rec *con_rec;

  if (house->fountain_vnum)
  {
    house->fountain_vnum = world[ch->in_room].number;
    writeHouse(house);
    send_to_char("Fountain location moved to here.\r\n", ch);
    return;
  }
  if (!charge_char(ch, upgrade_costs[HCONTROL_ADD_FOUNTAIN]))
  {
    return;
  }
  con_rec = get_con_rec();
  con_rec->vnum = house->vnum;
  con_rec->type = HCONTROL_ADD_FOUNTAIN;
  con_rec->time = time(0);
  con_rec->location = world[ch->in_room].number;
  con_rec->next = house_upgrade_list;
  house_upgrade_list = con_rec;
  writeConstructionQ();
  send_to_char("Fountain construction begun.\r\n", ch);
}

void do_build_inn(P_house house, P_char ch)
{
  struct house_upgrade_rec *con_rec;

  if (house->inn_vnum)
  {
    house->inn_vnum = world[ch->in_room].number;
    writeHouse(house);
    send_to_char("Inn location moved to here.\r\n", ch);
    return;
  }
  if (!charge_char(ch, upgrade_costs[HCONTROL_ADD_INN]))
  {
    return;
  }
  con_rec = get_con_rec();
  con_rec->vnum = house->vnum;
  con_rec->type = HCONTROL_ADD_INN;
  con_rec->time = time(0);
  con_rec->location = world[ch->in_room].number;
  con_rec->next = house_upgrade_list;
  house_upgrade_list = con_rec;
  writeConstructionQ();
  send_to_char("Inn construction begun.\r\n", ch);
}
void do_build_golem(P_house house, P_char ch, int type)
{
  struct house_upgrade_rec *con_rec;

  if (house->type == HCONTROL_GUILD &&
      (house->wizard_golems + house->warrior_golems + house->cleric_golems) ==
      get_property("guild.hall.maxGolem", 4.))
  {
    send_to_char("No more golems will fit in this room!\r\n", ch);
    return;
  }
  else if (house->type == HCONTROL_OUTPOST &&
           (house->wizard_golems + house->warrior_golems +
            house->cleric_golems) == get_property("guild.outpost.maxGolem",
                                                  2.))
  {
    send_to_char("No more golems will fit in this room!\r\n", ch);
    return;
  }

  if (!charge_char(ch, upgrade_costs[type]))
    return;
  con_rec = get_con_rec();
  con_rec->vnum = house->vnum;
  con_rec->type = type;
  con_rec->time = time(0);
  con_rec->next = house_upgrade_list;
  house_upgrade_list = con_rec;
  writeConstructionQ();
  send_to_char("Guard/Golem construction begun.\r\n", ch);
}

void do_describe_room(P_house house, P_char ch, char *arg)
{
  struct house_upgrade_rec *con_rec;

  if (!charge_char(ch, upgrade_costs[HCONTROL_DESC_ROOM]))
    return;
  send_to_char("Enter new room description, save with /s, /h for help.\r\n",
               ch);
  ch->desc->str = NULL;
  ch->desc->max_str = MAX_STRING_LENGTH;

  con_rec = get_con_rec();
  con_rec->door_keyword = str_dup("\n");
  *con_rec->door_keyword = 0;
  ch->desc->str = &(con_rec->door_keyword);
  con_rec->vnum = house->vnum;
  con_rec->time = time(0);
  con_rec->type = HCONTROL_DESC_ROOM;
  con_rec->location = world[ch->in_room].number;
  con_rec->next = house_upgrade_list;
  house_upgrade_list = con_rec;
  writeConstructionQ();
  send_to_char("Room description begun.\r\n", ch);
}

void do_name_room(P_house house, P_char ch, char *arg)
{
  struct house_upgrade_rec *con_rec;

  arg++;
  if (!*arg || !arg)
    return;

  if (!charge_char(ch, upgrade_costs[HCONTROL_NAME_ROOM]))
    return;

  con_rec = get_con_rec();
  con_rec->vnum = house->vnum;
  con_rec->time = time(0);
  con_rec->type = HCONTROL_NAME_ROOM;
  con_rec->location = world[ch->in_room].number;
  con_rec->door_keyword = str_dup(arg);
  con_rec->next = house_upgrade_list;
  house_upgrade_list = con_rec;
  writeConstructionQ();
  send_to_char("Room naming begun.\r\n", ch);
}

int hcontrol_count_rooms(P_house house)
{
  int      i, n;

  for (i = 0, n = 0; i < MAX_HOUSE_ROOMS; i++)
    if (house->room_vnums[i] != -1)
      n++;

  return n;
}

/* construct one room of the house */
void do_construct_room(P_house house, P_char ch, char *arg)
{
  char     buff[MAX_STRING_LENGTH];
  int      exit_dir;
  int      door;
  int      room_count;
  int      room_in_progress = 0;
  char    *door_keyword;
  struct house_upgrade_rec *con_rec;

/*  if (!IS_TRUSTED(ch)) {
    send_to_char("No new rooms right now. Try back later...\r\n", ch);
    return;
  }*/

  room_count = hcontrol_count_rooms(house);

  con_rec = house_upgrade_list;

  while (con_rec)
  {
    if ((con_rec->vnum == house->vnum) &&
        (con_rec->type == HCONTROL_ADD_ROOM))
    {
      room_in_progress++;
    }
    con_rec = con_rec->next;
  }


  if ((room_count + room_in_progress) >= MAX_HOUSE_ROOMS)
  {
    send_to_char("This house has max # of rooms already.\n\r", ch);
    return;
  }

  arg = one_argument(arg, buff);
  if (!(exit_dir = old_search_block(buff, 0, strlen(buff), dirs, 0)))
  {
    send_to_char(CONSTRUCT_FORMAT, ch);
    return;
  };
  exit_dir--;                   /* to put array back on track */
  if (world[ch->in_room].dir_option[exit_dir])
  {                             /* already exit there */
    send_to_char("There is already an exit there.\r\n", ch);
    return;
  }

  if (world[ch->in_room].number == house->room_vnums[0] &&
      (house->type == HCONTROL_GUILD))
  {
    send_to_char("This is your golem room, you cannot build rooms here.\r\n",
                 ch);

    return;
  }

  if (!charge_char(ch, upgrade_costs[HCONTROL_ADD_ROOM]))
  {
    return;
  }

  arg = one_argument(arg, buff);
  door = 0;

#if 0
/* buggy */
  if (is_abbrev(buff, "door"))
  {
    door = 1;
    if (arg[0])
      door_keyword = str_dup(arg);
    else
      door_keyword = str_dup("none");
  }
#endif

  sprintf(buff, "Building a room, exit %s", dirs[exit_dir]);
  send_to_char(buff, ch);
  if (door)
  {
    sprintf(buff, " with door keyword %s.\r\n", door_keyword);
    send_to_char(buff, ch);
  }
  else
    send_to_char(" with no door.\r\n", ch);

  con_rec = get_con_rec();
  con_rec->vnum = house->vnum;
  con_rec->time = time(0);
  con_rec->type = HCONTROL_ADD_ROOM;
  con_rec->location = world[ch->in_room].number;
  con_rec->exit_dir = exit_dir;
  con_rec->door = door;
  if (door)
    con_rec->door_keyword = door_keyword;
  con_rec->next = house_upgrade_list;
  house_upgrade_list = con_rec;
  writeConstructionQ();
}


void do_show_q(P_char ch)
{
  struct house_upgrade_rec *cur_rec;
  char     buff[MAX_STRING_LENGTH];
  char     temp[20];

  cur_rec = house_upgrade_list;

  send_to_char("House    Type    Location   Complete Time\r\n", ch);
  send_to_char("-----    ----    --------   -------------\r\n", ch);
  while (cur_rec)
  {
    switch (cur_rec->type)
    {
    case HCONTROL_ADD_ROOM:
      sprintf(temp, "%s", "Room");
      break;
    case HCONTROL_NAME_ROOM:
      sprintf(temp, "%s", "Name");
      break;
    case HCONTROL_DESC_ROOM:
      sprintf(temp, "%s", "Descr");
      break;
    case HCONTROL_ADD_GUARD_GOLEM:
      sprintf(temp, "%s", "Guard");
      break;
    case HCONTROL_ADD_MAGE_GOLEM:
    case HCONTROL_ADD_CLERIC_GOLEM:
    case HCONTROL_ADD_WARRIOR_GOLEM:
      sprintf(temp, "%s", "Golem");
      break;
    case HCONTROL_ADD_INN:
      sprintf(temp, "%s", "Inn");
      break;
    case HCONTROL_ADD_MOUTH:
      sprintf(temp, "%s", "Mouth");
      break;
    case HCONTROL_ADD_SHOP:
      sprintf(temp, "%s", "Shop");
      break;
    case HCONTROL_ADD_CHEST:
      sprintf(temp, "%s", "Chest");
      break;
    case HCONTROL_ADD_CABINET:
      sprintf(temp, "%s", "Cabinet");
      break;
    case HCONTROL_ADD_TELEPORTER:
      sprintf(temp, "%s", "Tele");
      break;
    case HCONTROL_ADD_FOUNTAIN:
      sprintf(temp, "%s", "Fount");
      break;
    case HCONTROL_ADD_HOLY:
      sprintf(temp, "%s", "Holy");
      break;
    case HCONTROL_ADD_UNHOLY:
      sprintf(temp, "%s", "Unholy");
      break;
    case HCONTROL_ADD_HEAL:
      sprintf(temp, "%s", "Heal");
      break;
    case HCONTROL_ADD_WINDOW:
      sprintf(temp, "%s", "Window");
      break;
    case HCONTROL_ADD_MOVEDOOR:
      sprintf(temp, "%s", "Door Move");
      break;
    case HCONTROL_INITIAL_HOUSE:
      sprintf(temp, "%s", "Home");
      break;
    case HCONTROL_INITIAL_GUILD:
      sprintf(temp, "%s", "GuildHall");
      break;
    default:
      sprintf(temp, "%s", "????");
    }
    sprintf(buff, "%5d     %s    %5d    %14lu\r\n", cur_rec->vnum, temp,
            cur_rec->location,
            (upgrade_times[cur_rec->type] -
             (time(0) - cur_rec->time) / SECS_PER_MUD_HOUR));
    send_to_char(buff, ch);
    cur_rec = cur_rec->next;
  }
}

void process_construction_q()
{
  struct house_upgrade_rec *cur_rec, *prev_rec;
  P_house  house = NULL, house2 = NULL;
  int      flag, temp;
  char     golem_block = 'A';
  char     buff[MAX_STRING_LENGTH];
  P_char   golem;
  P_obj    temp_obj, entrance_obj;

  cur_rec = house_upgrade_list;
  prev_rec = NULL;

  while (cur_rec)
  {
    flag = 0;
    if (1 ||
        ((time(0) - cur_rec->time) >
         (upgrade_times[cur_rec->type] * SECS_PER_MUD_HOUR)))
    {
      switch (cur_rec->type)
      {                         /* time to build */
      case HCONTROL_ADD_ROOM:
        house = find_house(cur_rec->vnum);
        if (house)
        {
          temp =
            real_room0(hcontrol_setup_room
                       (house, cur_rec->location, cur_rec->exit_dir));
          if (!temp)
            return;
          if (cur_rec->door && cur_rec->door_keyword)
          {                     /* setup door */
            if (world[real_room0(cur_rec->location)].
                dir_option[cur_rec->exit_dir])
            {
              SET_BIT(world[real_room0(cur_rec->location)].
                      dir_option[cur_rec->exit_dir]->exit_info, EX_ISDOOR);
              world[real_room0(cur_rec->location)].dir_option[cur_rec->
                                                              exit_dir]->
                keyword = str_dup(cur_rec->door_keyword);
            }
            if (world[real_room0(temp)].
                dir_option[rev_dir[cur_rec->exit_dir]])
            {
              SET_BIT(world[real_room0(temp)].
                      dir_option[rev_dir[cur_rec->exit_dir]]->exit_info,
                      EX_ISDOOR);
              world[temp].dir_option[rev_dir[cur_rec->exit_dir]]->keyword =
                str_dup(cur_rec->door_keyword);
            }
          }                     /* setup door */
          writeHouse(house);
        }                       /* if house */
        break;
      case HCONTROL_NAME_ROOM:
        house = find_house(cur_rec->vnum);
        if (house)
        {
          world[real_room0(cur_rec->location)].name =
            str_dup(cur_rec->door_keyword);
          writeHouse(house);
        }
        break;
      case HCONTROL_DESC_ROOM:
        house = find_house(cur_rec->vnum);
        if (house && cur_rec->door_keyword)
        {
          world[real_room0(cur_rec->location)].description =
            str_dup(cur_rec->door_keyword);
          writeHouse(house);
        }
        break;
      case HCONTROL_ADD_SECRET:
        house = find_house(cur_rec->vnum);
        if (house)
        {
          house->secret_entrance = 1;
          writeHouse(house);
        }
        break;
      case HCONTROL_ADD_WARRIOR_GOLEM:
      case HCONTROL_ADD_MAGE_GOLEM:
      case HCONTROL_ADD_CLERIC_GOLEM:
        house = find_house(cur_rec->vnum);
        if (house)
        {
          if ((house->wizard_golems + house->warrior_golems +
               house->cleric_golems) == (int) ((house->type ==
                                                HCONTROL_GUILD) ?
                                               get_property
                                               ("guild.hall.maxGolem",
                                                4.) :
                                               get_property
                                               ("guild.outpost.maxGolem",
                                                2.)))
          {
            break;
          }
          if (cur_rec->type == HCONTROL_ADD_CLERIC_GOLEM)
          {
            house->cleric_golems++;
            golem = read_mobile(CLERIC_GOLEM_VNUM, VIRTUAL);
          }
          else if (cur_rec->type == HCONTROL_ADD_WARRIOR_GOLEM)
          {
            house->warrior_golems++;
            golem = read_mobile(WARRIOR_GOLEM_VNUM, VIRTUAL);
          }
          else if (cur_rec->type == HCONTROL_ADD_MAGE_GOLEM)
          {
            house->wizard_golems++;
            golem = read_mobile(MAGE_GOLEM_VNUM, VIRTUAL);
          }

          writeHouse(house);
          if (!golem)
            break;
          if (house->exit_num == -1)
            golem_block = 'a';
          else
            golem_block = *dirs[house->exit_num];
          sprintf(buff, "golem assoc%d_%c_0", house->owner_guild,
                  golem_block);
          golem->player.name = str_dup(buff);
          char_to_room(golem, real_room0(house->room_vnums[0]), 0);
        }
        break;
      case HCONTROL_ADD_MOVEDOOR:
        break; // disabled
        house = find_house(cur_rec->vnum);
        if (house)
        {
          if (house->inner_door)
          {
            temp_obj = house->inner_door;
            temp_obj->value[0] = cur_rec->location;
            temp_obj = house->outer_door;
            obj_from_room(temp_obj);
            obj_to_room(temp_obj, real_room0(cur_rec->location));
          }

          // delete old house (9/5/00 - Tavril)

          //sprintf(buff, "%d", house->vnum);
          sprintf(buff, "%s/House/HouseRoom/house.%d", SAVE_DIR, house->vnum);
          unlink(buff);

          // write new house
          house->vnum = cur_rec->location;
          writeHouse(house);
        }
        break;
      case HCONTROL_ADD_INN:
        house = find_house(cur_rec->vnum);
        if (house)
        {
          if (house->inn_vnum)
            break;
          house->inn_vnum = cur_rec->location;
          world[real_room0(house->inn_vnum)].funct = inn;
          writeHouse(house);
        }
        break;
      case HCONTROL_ADD_FOUNTAIN:
        house = find_house(cur_rec->vnum);
        if (house)
        {
          if (house->fountain_vnum)
            break;
          house->fountain_vnum = cur_rec->location;
          writeHouse(house);
          temp_obj = read_object(FOUNTAIN_VNUM, VIRTUAL);
          if (temp_obj)
          {
            obj_to_room(temp_obj, real_room0(cur_rec->location));
          }
        }
        break;
      case HCONTROL_ADD_HOLY:
        house = find_house(cur_rec->vnum);
        if (house)
        {
          if (house->holy_fount_vnum)
            break;
          house->holy_fount_vnum = cur_rec->location;
          writeHouse(house);
          temp_obj = read_object(GUILD_HOLY_FOUNT, VIRTUAL);
          if (temp_obj)
            obj_to_room(temp_obj, real_room0(cur_rec->location));
        }
        break;
      case HCONTROL_ADD_UNHOLY:
        house = find_house(cur_rec->vnum);
        if (house)
        {
          if (house->unholy_fount_vnum)
            break;
          house->unholy_fount_vnum = cur_rec->location;
          writeHouse(house);
          temp_obj = read_object(GUILD_UNHOLY_FOUNT, VIRTUAL);
          if (temp_obj)
            obj_to_room(temp_obj, real_room0(cur_rec->location));
        }
        break;
      case HCONTROL_ADD_HEAL:
        house = find_house(cur_rec->vnum);
        if (house)
        {
          if (house->heal_vnum)
            break;
          house->heal_vnum = cur_rec->location;
          writeHouse(house);
          SET_BIT(world[real_room0(cur_rec->location)].room_flags, HEAL);
          break;
        }
        break;
      case HCONTROL_ADD_SHOP:
        house = find_house(cur_rec->vnum);
        if (house)
        {
          stock_guild_shop(cur_rec->location);
          house->shop_vnum = cur_rec->location;
          writeHouse(house);
        }
        break;
      case HCONTROL_ADD_CHEST:
        house = find_house(cur_rec->vnum);
        temp_obj = read_object(CHEST_VNUM, VIRTUAL);
        if (house && temp_obj)
        {
          obj_to_room(temp_obj, real_room0(cur_rec->location));
          writeHouse(house);
        }
        break;
      case HCONTROL_ADD_CABINET:
        house = find_house(cur_rec->vnum);
        temp_obj = read_object(CABINET_VNUM, VIRTUAL);
        if (house && temp_obj)
        {
          obj_to_room(temp_obj, real_room0(cur_rec->location));
          writeHouse(house);
        }
        break;
      case HCONTROL_ADD_WINDOW:
        house = find_house(cur_rec->vnum);
        temp_obj = read_object(WINDOW_VNUM, VIRTUAL);
        if (house && temp_obj)
        {
          if (house->window_vnum)
            break;
          house->window_vnum = cur_rec->location;
          writeHouse(house);
          temp_obj->value[0] = house->vnum;
          obj_to_room(temp_obj, real_room0(cur_rec->location));
        }
        break;
      case HCONTROL_ADD_MOUTH:
        house = find_house(cur_rec->vnum);
        temp_obj = read_object(MOUTH_VNUM, VIRTUAL);
        if (house && temp_obj)
        {
          if (house->mouth_vnum)
            break;
          house->mouth_vnum = cur_rec->location;
          writeHouse(house);
          temp_obj->value[0] = house->owner_guild;
          obj_to_room(temp_obj, real_room0(cur_rec->location));
        }
        break;
      case HCONTROL_INITIAL_HOUSE:
        house = find_house(cur_rec->vnum);
        if (house)
        {
          if ((entrance_obj = read_object(HOUSE_OUTER_DOOR, VIRTUAL)))
          {
            entrance_obj->str_mask = (STRUNG_KEYS | STRUNG_DESC1);
            entrance_obj->name = str_dup(house->owner);
            sprintf(buff, "A door leading into %s's house is here.",
                    house->owner);
            entrance_obj->description = str_dup(buff);
            entrance_obj->value[0] = house->room_vnums[0];
            nuke_doorways(real_room0(house->vnum));
            obj_to_room(entrance_obj, real_room0(house->vnum));
          }
          send_to_room("Construction of a new home completed.\r\n",
                       real_room0(house->vnum));
          house->owner_guild = 0;
          house->upgrades = 0;
          house->inn_vnum = house->room_vnums[1];
          world[real_room0(house->inn_vnum)].funct = inn;
          house->construction = 0;
          writeHouse(house);
        }
        break;
      case HCONTROL_INITIAL_GUILD:
        house = find_house(cur_rec->vnum);
        if (house)
        {
/*
          if ((entrance_obj = read_object(HOUSE_OUTER_DOOR, VIRTUAL)))
          {
             entrance_obj->str_mask = (STRUNG_KEYS | STRUNG_DESC1);
             sprintf(buff, "hall guild building %s", house->owner);
             entrance_obj->name = str_dup(buff);
             sprintf(buff, "A Guild Hall of %s is here.", get_assoc_name(house->owner_guild).c_str());
             entrance_obj->description = strdup(buff);
             entrance_obj->value[0] = house->room_vnums[0];
             nuke_doorways(real_room0(house->vnum));
             obj_to_room(entrance_obj, real_room0(house->vnum));
          }
*/
          temp_obj = read_object(TELEPORTER_VNUM, VIRTUAL);
          if (temp_obj)
          {
            temp_obj->value[0] = house->room_vnums[0];
            obj_to_room(temp_obj, real_room0(house->vnum));
          }

          house->owner_guild = cur_rec->guild;
          send_to_room("Construction of a new guildhall completed.\r\n",
                       real_room0(house->vnum));
          house->upgrades = 0;
          house->construction = 0;
          writeHouse(house);
        }
        break;
      case HCONTROL_ADD_TELEPORTER:
        house = find_house(cur_rec->vnum);
        house2 = find_house(cur_rec->exit_dir);
        if (house->teleporter1_room && house->teleporter2_room)
          break;
        if (house2->teleporter2_room && house2->teleporter2_room)
          break;
        if (house->teleporter1_room)
        {
          house->teleporter2_room = cur_rec->location;
          house->teleporter2_dest = cur_rec->door;
        }
        else
        {
          house->teleporter1_room = cur_rec->location;
          house->teleporter1_dest = cur_rec->door;
        }
        writeHouse(house);
        if (house2->teleporter1_room)
        {
          house2->teleporter2_room = cur_rec->door;
          house2->teleporter2_dest = cur_rec->location;
        }
        else
        {
          house2->teleporter1_room = cur_rec->door;
          house2->teleporter1_dest = cur_rec->location;
        }
        writeHouse(house2);
        temp_obj = read_object(TELEPORTER_VNUM, VIRTUAL);
        if (!temp_obj)
          break;
        temp_obj->value[0] = cur_rec->door;
        obj_to_room(temp_obj, real_room0(cur_rec->location));
        temp_obj = read_object(TELEPORTER_VNUM, VIRTUAL);
        temp_obj->value[0] = cur_rec->location;
        obj_to_room(temp_obj, real_room0(cur_rec->door));
        break;
      }                         /* switch */
      /* count the upgrades */
      if (house)
        house->upgrades += 1;
      if (house2)
        house2->upgrades += 1;
      /* now delete this rec from Q */
      flag = 1;                 /* don't advance cur_rec */
      if (prev_rec == NULL)
      {                         /* first rec */
        prev_rec = cur_rec;
        cur_rec = cur_rec->next;
        house_upgrade_list = cur_rec;
        mm_release(dead_construction_pool, prev_rec);
        prev_rec = NULL;
      }
      else
      {
        prev_rec->next = cur_rec->next;
        mm_release(dead_construction_pool, cur_rec);
        cur_rec = prev_rec->next;
      }
      writeConstructionQ();
    }                           /* if time to build */
    if (!flag)
    {
      prev_rec = cur_rec;
      cur_rec = cur_rec->next;
    }
  }
  writeConstructionQ();
}

void clear_sacks(P_char ch)
{
  P_house  house;
  struct house_sack_rec *sack_rec;

  house = first_house;
  while (house)
  {
    sack_rec = house->sack_list;
    while (sack_rec)
    {
      if (!strcmp(sack_rec->name, GET_NAME(ch)))
      {
        sack_rec->name = str_dup("xxxx");
      }
      sack_rec = sack_rec->next;
    }
    house = house->next;
  }
}

/* sack commands */
void do_sack(P_char ch, char *arg, int cmd)
{

  P_house  house;
  P_char   mob;
  struct house_sack_rec *sack_rec;
  char     buf[256];

  if(!(ch))
    return;

  arg = one_argument(arg, buf);
  if ((buf[0] != '\000') && is_abbrev(buf, "off"))
  {
    send_to_char("You stop your rampage.\r\n", ch);
    REMOVE_BIT(ch->specials.affected_by4, AFF4_SACKING);
    clear_sacks(ch);
    return;
  }
  if ((buf[0] != '\000') && IS_TRUSTED(ch) && is_abbrev(buf, "show"))
  {
    house = first_house;
    send_to_char("House     Sacker     Time Left\r\n", ch);
    while (house)
    {
      sack_rec = house->sack_list;
      while (sack_rec)
      {
        sprintf(buf, "%d    %s       %d\r\n", house->vnum, sack_rec->name,
                (int) (150 * SECS_PER_MUD_HOUR -
                       (time(0) - sack_rec->time)) / SECS_PER_MUD_HOUR);
        send_to_char(buf, ch);
        sack_rec = sack_rec->next;
      }
      house = house->next;
    }
    return;
  }
  house = house_ch_is_in(ch);
  if (!house)
  {
    send_to_char("Sack what? You're not in a guildhall or castle!\r\n", ch);
    return;
  }
  if (GET_A_NUM(ch) == house->owner_guild)
  {
    send_to_char("Sack your own property!  You can't be that bored.\r\n", ch);
    return;
  }
  if (IS_AFFECTED4(ch, AFF4_SACKING))
  {
    send_to_char("You're already sacking something.\r\n", ch);
    send_to_char("Use 'sack off' to stop.\r\n", ch);
    return;
  }
  if (world[ch->in_room].number == house->room_vnums[0])
  {
    send_to_char("You need to move slightly farther in to begin sacking.\r\n",
                 ch);
    return;
  }

  if (!can_acquire_hall(ch, house->type))
  {
    send_to_char
      ("Sorry, your guild has reached the limit for halls already.\r\n", ch);
    return;
  }

  // this check produces a message already
  if (!valid_build_location(real_room(house->vnum), ch, HCONTROL_GUILD))
    return;

  for (mob = world[real_room0(house->room_vnums[0])].people; mob;
       mob = mob->next_in_room)
  {
    if (isname("golem", GET_NAME(mob)))
    {
      send_to_char
        ("The golems might have something to say about that, go kill em.\r\n",
         ch);
      return;
    }
  }
  // valid sack command

  SET_BIT(ch->specials.affected_by4, AFF4_SACKING);
  CREATE(sack_rec, house_sack_rec, 1, MEM_TAG_SACKREC);
  sack_rec->name = str_dup(GET_NAME(ch));
  sack_rec->time = time(0);
  sack_rec->next = house->sack_list;
  house->sack_list = sack_rec;

  send_to_char("&+RRAMPAGE!  DESTROY!&N You are now sacking this hall.\r\n",
               ch);

  logit(LOG_HOUSE, "House %d started sacking by %s", house->vnum,
        GET_NAME(ch));
}

void nuke_portal(int rnum)
{

  P_obj    obj;

  obj =
    get_obj_in_list_num(real_object(TELEPORTER_VNUM), world[rnum].contents);
  if (!obj)
    return;
  obj_from_room(obj);
  extract_obj(obj, TRUE);
}

void nuke_doorways(int rnum)
{

  P_obj    obj = NULL;

  obj =
    get_obj_in_list_num(real_object(HOUSE_INNER_DOOR), world[rnum].contents);
  if (obj)
  {
    obj_from_room(obj);
    extract_obj(obj, TRUE);
  }
  /* dont second guess, just look for and yank any type */
  obj =
    get_obj_in_list_num(real_object(HOUSE_OUTER_DOOR), world[rnum].contents);
  if (obj)
  {
    obj_from_room(obj);
    extract_obj(obj, TRUE);
  }
  return;
}

void sack_house(P_house house, P_char ch)
{
  struct house_sack_rec *sack_rec, *prev;
  int      temp1, temp2;

  /* P_obj tobj;  for finding objects to be nuked */
  P_house  thouse;

  /* When house is sacked we're going to:
     1) remove all golems 
     2) set guild_no of house to new guild
     3) clear sacking list completely
     4) remove teleporter objects, and teleporters
     5) remove board
     6) remove magic mouth
   */
  house->warrior_golems = 0;
  house->wizard_golems = 0;
  house->cleric_golems = 0;
/* first teleporter */
  if (house->teleporter1_room)
  {
    temp1 = house->teleporter1_room;
    temp2 = house->teleporter1_dest;
    nuke_portal(real_room0(temp1));
    nuke_portal(real_room0(temp2));
    thouse = get_house_from_room(real_room0(temp2));
    if (thouse)
    {                           /* should always be true, but just in case */
      if (thouse->teleporter1_room == temp2)
      {
        thouse->teleporter1_room = 0;
        thouse->teleporter1_dest = 0;
      }
      else
      {
        thouse->teleporter2_room = 0;
        thouse->teleporter2_dest = 0;
      }
      writeHouse(thouse);
    }
    house->teleporter1_room = 0;
    house->teleporter1_dest = 0;
  }
/* second teleporter */
  if (house->teleporter2_room)
  {
    temp1 = house->teleporter2_room;
    temp2 = house->teleporter2_dest;
    nuke_portal(real_room0(temp1));
    nuke_portal(real_room0(temp2));
    thouse = get_house_from_room(real_room0(temp2));
    if (thouse)
    {
      if (thouse->teleporter1_room == temp2)
      {
        thouse->teleporter1_room = 0;
        thouse->teleporter1_dest = 0;
      }
      else
      {
        thouse->teleporter2_room = 0;
        thouse->teleporter2_dest = 0;
      }
      writeHouse(thouse);
    }
    house->teleporter2_room = 0;
    house->teleporter2_dest = 0;
  }
/* mouth */
  if (house->mouth_vnum)
  {
    house->mouth_vnum = 0;
  }
/* board */

  sack_rec = house->sack_list;
  house->sack_list = 0;
  while (sack_rec)
  {
    prev = sack_rec->next;
    FREE(sack_rec);
    sack_rec = prev;
  }
  house->owner_guild = GET_A_NUM(ch);
  writeHouse(house);
  return;
}

void check_sacks(P_house house)
{
  struct house_sack_rec *sack_rec_list, *sack_rec, *prev;
  P_char   ch;
  P_house  thouse, house_list;
  P_desc   i;
  int      isacking = 0;
  char     buf[128];

  if (!house)
    return;                     // howd we get here?
  if (!house->sack_list)
    return;                     // nothing going on

  sack_rec = house->sack_list;
  prev = NULL;

  while (sack_rec)
  {
    // okay, check to make sure player is still in the house
    ch = get_char_online(sack_rec->name);
    if (!ch || !IS_AFFECTED4(ch, AFF4_SACKING))
    {                           // not online or not sacking
      if (prev)
      {
        prev->next = sack_rec->next;
        FREE(sack_rec);
        sack_rec = prev->next;
      }
      else
      {                         // delete first
        house->sack_list = sack_rec->next;
        FREE(sack_rec);
        sack_rec = house->sack_list;
      }
      break;
    }
    thouse = house_ch_is_in(ch);
    if (thouse != house)
    {                           // left hosue
      REMOVE_BIT(ch->specials.affected_by4, AFF4_SACKING);
      clear_sacks(ch);
      send_to_char("How can you sack something when you're not there?\r\n",
                   ch);
      break;
    }
    // if been there 168 mud hours (a week) SACK IT
    send_to_char("&+RYou continue your RAMPAGE.&N\r\n", ch);

    house_list = first_house;
    while (house_list)
    {
      sack_rec_list = house_list->sack_list;
      while (sack_rec_list)
      {
      //  house->vnum, sack_rec->tim;
        if (house->vnum == house_list->vnum)
        {
          isacking++;
        }
        sack_rec_list = sack_rec_list->next;
      }
      house_list = house_list->next;
    }

    if (isacking && !number(0, 4))
    {
      if (isacking > 10)
        isacking = 10;
      sack_rec->time = sack_rec->time - (isacking * SECS_PER_MUD_HOUR);
      isacking = 0;
    }

    sprintf(buf, "&+CGuild Alert! %s is SACKING the hall!\r\n",
            sack_rec->name);
    logit(LOG_HOUSE, "%s sacking hall %d", sack_rec->name, house->vnum);
    for (i = descriptor_list; i; i = i->next)
      if (!i->connected &&
          !is_silent(i->character, TRUE) &&
          IS_SET(i->character->specials.act, PLR_GCC) &&
          (GET_A_NUM(i->character) == house->owner_guild))
        send_to_char(buf, i->character);

    if ((time(0) - sack_rec->time) > (140 * SECS_PER_MUD_HOUR))
    {
      sack_house(house, ch);
      REMOVE_BIT(ch->specials.affected_by4, AFF4_SACKING);
      clear_sacks(ch);
      logit(LOG_PLAYER, "%s sacked hall %d", GET_NAME(ch), house->vnum);
      send_to_char("Sacking complete, welcome home!\r\n", ch);
      return;
    }
    prev = sack_rec;
    sack_rec = sack_rec->next;
  }
}

/* timed housing calls */

void event_housekeeping(P_char ch, P_char victim, P_obj obj, void *data)
{
  P_house  house;

  process_construction_q();
  /*
  house = first_house;
  while (house)
  {
    check_sacks(house);
    house = house->next;
  }
  */
  add_event(event_housekeeping, PULSES_IN_TICK, NULL, NULL, NULL, 0, NULL, 0);
}

void do_construct_guild(P_char ch, int type)
{
  int      cost, temp1, temp2;
  char     buf[MAX_STRING_LENGTH];
  struct house_upgrade_rec *con_rec;

  temp1 = GET_A_NUM(ch);
  temp2 = GET_A_BITS(ch);
  if (!IS_MEMBER(temp2) || !temp1)
  {
    send_to_char("Sorry, only guildmembers may initiate construction.\r\n",
                 ch);
    return;
  }
  if (!GT_NORMAL(temp2))
  {
    send_to_char
      ("You need to be a senior member to order new construction!\r\n", ch);
    return;
  }

  if (!can_acquire_hall(ch, type))
  {
    send_to_char
      ("Sorry, your guild has reached the limit for halls already.\r\n", ch);
    return;
  }

  if (!valid_build_location(ch->in_room, ch, type))
    return;

  if (find_house(world[ch->in_room].number))
  {
    send_to_char
      ("The area is already crowded enough with buildings as it is.\r\n", ch);
    return;
  }
  /* check construction queue as well */

  con_rec = house_upgrade_list;

  while (con_rec)
  {
    if ((con_rec->vnum == world[ch->in_room].number) &&
        con_rec->type == HCONTROL_INITIAL_GUILD)
    {
      send_to_char
        ("The area is already crowded enough with buildings as it is.\r\n",
         ch);
      return;
    }
    con_rec = con_rec->next;
  }

  cost = guild_cost(ch);

  if (!charge_char(ch, cost))
    return;

  con_rec = get_con_rec();

  con_rec->vnum = world[ch->in_room].number;
  con_rec->type = HCONTROL_INITIAL_GUILD;
  con_rec->time = time(0);
  con_rec->location = world[ch->in_room].number;
  con_rec->guild = GET_A_NUM(ch);
  con_rec->door_keyword = str_dup(J_NAME(ch));
  con_rec->next = house_upgrade_list;

  house_upgrade_list = con_rec;

  if (type == HCONTROL_GUILD)
    sprintf(buf, "guild %s n %d", J_NAME(ch), GET_A_NUM(ch));
  else
    sprintf(buf, "outpost %s n %d", J_NAME(ch), GET_A_NUM(ch));
  hcontrol_build_house(ch, buf);
  send_to_char
    ("GuildHall construction begun - this is going to take a while.\r\n", ch);

  return;
}

void do_stathouse(P_char ch, char *argument, int cmd)
{
  return;
}

int guild_chest(P_obj obj, P_char ch, int cmd, char *argument)
{
  P_house  house = NULL;
  P_obj    s_obj = NULL;
  char     GBuf1[MAX_STRING_LENGTH], GBuf2[MAX_STRING_LENGTH];
  
  *GBuf1 = '\0';
  *GBuf2 = '\0';
  
  if (cmd == CMD_SET_PERIODIC)               /*
                                              Events have priority
                                              */
    return FALSE;
  
  if (!ch || !obj)              /*
                                 If the player ain't here, why are we?
                                 */
    return FALSE;
  
  if (argument && cmd == CMD_GET || cmd == CMD_TAKE)
  {
    argument_interpreter(argument, GBuf1, GBuf2);
    if (!*GBuf2)
      return FALSE;
    s_obj = get_obj_in_list_vis(ch, GBuf2, ch->carrying);
    if (!s_obj)
      s_obj = get_obj_in_list_vis(ch, GBuf2, world[ch->in_room].contents);
    if (s_obj != obj)
      return FALSE;
    
    /* ok they are attempting to get something from this chest */
    house = house_ch_is_in(ch);
    if (!house)
      return FALSE;
    if ((GET_A_NUM(ch) != house->owner_guild) && (!IS_TRUSTED(ch)) &&
        (house->type == HCONTROL_GUILD))
    {
      act("&+L$n &+Lis &+Rzapped&+L as $e tries to get something from $p!",
          FALSE, ch, obj, 0, TO_ROOM);
      act("&+LYou are &+Rzapped&+L as you try to get something from $p!",
          FALSE, ch, obj, 0, TO_CHAR);
      return TRUE;
    }
    
  }
  return FALSE;
}

void enemy_hall_check(P_char ch, int room)
{
  P_char   owner;
  P_house  house = NULL;
  char     buf1[MAX_STR_NORMAL];
  char     buf2[MAX_STR_NORMAL];
  int      i1, i2, i3, i4;
  uint     u1;
  FILE    *f;
  
  // first unhome if someone is homed in not his guildhall
  // this is to get rid of unsolicited guests at gh
  if (GET_BIRTHPLACE(ch) > 0 && GET_BIRTHPLACE(ch) < top_of_world)
    house = find_house(world[GET_BIRTHPLACE(ch)].number);
  if (house && house->type == HCONTROL_GUILD &&
      GET_A_NUM(ch) != house->owner_guild)
    GET_BIRTHPLACE(ch) = GET_ORIG_BIRTHPLACE(ch);
  
  if (room <= 0 || room > top_of_world)
    return;
  
  house = find_house(world[room].number);
  if (!house || house->type != HCONTROL_GUILD)
    return;
  
  sprintf(buf1, "%sasc.%u", ASC_DIR, house->owner_guild);
  f = fopen(buf1, "r");
  
  if (f == NULL)
    return;
  
  for (i1 = 0; i1 < 11; i1++)
  {
    fgets(buf1, MAX_STR_NORMAL, f);
  }
  while (fgets(buf1, MAX_STR_NORMAL, f))
  {
    sscanf(buf1, "%s %u %i %i %i %i\r\n", buf2, &u1, &i1, &i2, &i3, &i4);
    if (!IS_ENEMY(u1) && IS_MEMBER(u1))
    {
      owner = (struct char_data *) mm_get(dead_mob_pool);
      if (!owner)
        break;
      owner->only.pc = (struct pc_only_data *) mm_get(dead_pconly_pool);
      if (restoreCharOnly(owner, skip_spaces(buf2)) >= 0)
      {
        //send_to_char("You voided in a hall....!\r\n", ch);
        //wizlog(56, "%s moved to his original birthplace cause he voided in hall. ",                                      GET_NAME(ch), GET_NAME(owner));
        room = real_room(GET_ORIG_BIRTHPLACE(ch));
        
        if (opposite_racewar(ch, owner) && FALSE) // NOTICE && FALSE
        {
          send_to_char("You were rented in an enemy hall!\r\n", ch);
          room = real_room(GET_ORIG_BIRTHPLACE(ch));
          wizlog(56, "%s moved to his original birthplace due to "
                 "racewar conflict with guildhall owner - %s",
                 GET_NAME(ch), GET_NAME(owner));
        }
        free_char(owner);
      }
      else
      {
        mm_release(dead_pconly_pool, owner->only.pc);
        mm_release(dead_mob_pool, owner);
      }
      break;
    }
  }
  fclose(f);
}

/*
 * This is the golem for associations.
 * keyword "assocX_Y_Z" expected in char names, X gives association
 * number, Y dir to block (neswud) and Z the #vnum of badge
 * Z=0 is used as sign that not badge but rather player info
 * is checked
 */

int assoc_golem(P_char ch, P_char pl, int cmd, char *arg)
{
  ush_int  assoc;
  uint     bits;
  char    *tmp;
  int      dir = -1;
  int      badge = 0;
  int      allowed = 0, allowed2 = 0;
  P_char   tch, next_ch;
  P_desc   desc;
  char     buf[MAX_STRING_LENGTH];
  char     temp[MAX_STRING_LENGTH];
  int      is_avatar = FALSE;
  int      virt = 0;
  int      online;
  
  /*
   * try to set up this mobs "assoc" number now, so it can be used
   * in future checks...
   */
  
  if (cmd == CMD_SET_PERIODIC)
    return TRUE;
  
  /* 
   if(cmd == CMD_DRAG && pl)
   {
   send_to_char("The golem whisper to you 'Drag your self in or out'", pl);
   return TRUE;
   }
   */  
  if (!GET_A_NUM(ch))
  {
    tmp = strstr(GET_NAME(ch), "assoc");
    if (!tmp)
    {
      logit(LOG_MOB,
            "assoc_golem() assigned to %s in %d without assocX_Y_Z keyword!",
            ch->player.short_descr, world[ch->in_room].number);
      REMOVE_BIT(ch->specials.act, ACT_SPEC);
      return FALSE;
    }
    assoc = (ush_int) atoi(tmp + 5);
    if ((assoc < 1) || (assoc > MAX_ASC))
    {
      logit(LOG_MOB,
            "assoc_golem() assigned to %s in %d has bad association number %u!",
            ch->player.short_descr, world[ch->in_room].number, assoc);
      REMOVE_BIT(ch->specials.act, ACT_SPEC);
      return FALSE;
    }
    GET_A_NUM(ch) = assoc;
    SET_MEMBER(GET_A_BITS(ch));
    SET_NORMAL(GET_A_BITS(ch));
  }
  /* find the association keyword */
  tmp = strstr(GET_NAME(ch), "assoc");
  
  assoc = GET_A_NUM(ch);
  sprintf(temp, "%d", world[ch->in_room].zone);
  // ENBALE NEXT LINE WHEN SACKING IS ENABLED
  
  //  assoc = readGuildFile(ch, world[ch->in_room].zone);
  sprintf(temp, "%d", assoc);
  /* now decode which direction to block */
  
  tmp = strchr(tmp, '_');
  if (!tmp)
  {
    logit(LOG_MOB,
          "assoc_golem() assigned to %s in %d has no blocking direction.",
          ch->player.short_descr, world[ch->in_room].number);
    REMOVE_BIT(ch->specials.act, ACT_SPEC);
    return (FALSE);
  }
  switch (*(++tmp))
  {
    case 'a':
      dir = 'a';
      break;                      /* handled later. stands for 'all' */
    case 'n':
      dir = CMD_NORTH;
      break;
    case 'e':
      dir = CMD_EAST;
      break;
    case 's':
      dir = CMD_SOUTH;
      break;
    case 'w':
      dir = CMD_WEST;
      break;
    case 'u':
      dir = CMD_UP;
      break;
    case 'd':
      dir = CMD_DOWN;
      break;
    default:
      logit(LOG_MOB,
            "assoc_iron_golem() assigned to %s in %d has wrong direction.",
            ch->player.short_descr, world[ch->in_room].number);
      REMOVE_BIT(ch->specials.act, ACT_SPEC);
      return (FALSE);
  }
  
  /* now look for the vnum of the badge */
  tmp = strchr(tmp, '_');
  if (!tmp)
  {
    logit(LOG_MOB,
          "assoc_iron_golem() assigned to %s in %d has no badge to check.",
          ch->player.short_descr, world[ch->in_room].number);
    REMOVE_BIT(ch->specials.act, ACT_SPEC);
    return (FALSE);
  }
  /* If I'm fighting, alert the guild! */
  if (IS_FIGHTING(ch) && !cmd && !number(0, 3))
  {
    strcpy(buf, " The guildhall is under attack!  Come at once!");
    do_gcc(ch, buf, CMD_GCC);
    return TRUE;
  }
  badge = atoi(tmp + 1);
  
  if (!cmd && !pl && !IS_FIGHTING(ch))
  {                             /* periodic call */
    /*
     * okay.. loop through all the characters in the room, and attack the first
     * one we find thats an enemy of the guild
     */
    
    //Demand cash
    
    /*if(!number(0,20))
     {
     if(withdraw_asc(ch, 1, 0, 0 , 0 ) || withdraw_asc(ch, 0, 15, 0 , 0 ) ||
     withdraw_asc(ch, 0, 0, 165 , 0 ) || withdraw_asc(ch, 0, 0, 0 , 1815))
     {
     if(!number(0,20)){
     sprintf(buf, "Thank you for my salary master.");
     do_gcc(ch, buf, CMD_GCC); 
     return 0;         
     }
     }
     else
     {
     if(number(0,10)){
     sprintf(buf, "Where is my cash master, pay me or i shall leave!");
     do_gcc(ch, buf, CMD_GCC);
     }
     else{
     sprintf(buf, "I'm tired of this, go find your self a new golem, bye, bye!");
     do_gcc(ch, buf, CMD_GCC);
     remove_a_golem_from_house(ch->in_room);
     send_to_room
     ("&+yThe ground begins to shake as the golem leaves the room.&n\n",
     ch->in_room);
     
     extract_char(ch);
     ch = NULL;
     return 0;                        
     //LEAVE
     }
     
     }
     
     }*/
    
    for (tch = world[ch->in_room].people; tch; tch = next_ch)
    {
      next_ch = tch->next_in_room;
      
      if (find_enemy(tch, (ush_int) (assoc)) && CAN_SEE(ch, tch))
      {
        sprintf(buf, " Alert, my masters, %s is invading!", GET_NAME(tch));
        do_gcc(ch, buf, CMD_GCC);
        MobStartFight(ch, tch); /* attack..  */
        if (IS_FIGHTING(ch))    /* if attack succeeded, break out of the loop */
          return (TRUE);
      }
    }
    
    online = 0;
    for (desc = descriptor_list; desc; desc = desc->next)
    {
      if (!desc->connected && GET_A_NUM(desc->character) == GET_A_NUM(ch))
      {
        forget(ch, desc->character);
        online++;
      }
    }
    
    if (online < 10)
    {
      SET_BIT(ch->specials.affected_by3, AFF3_INERTIAL_BARRIER);
      SET_BIT(ch->specials.affected_by2, AFF2_VAMPIRIC_TOUCH);
      SET_BIT(ch->specials.affected_by4, AFF4_VAMPIRE_FORM);
    }
    else
    {
      REMOVE_BIT(ch->specials.affected_by3, AFF3_INERTIAL_BARRIER);
      REMOVE_BIT(ch->specials.affected_by2, AFF2_VAMPIRIC_TOUCH);
      REMOVE_BIT(ch->specials.affected_by4, AFF4_VAMPIRE_FORM);
    }
    
    return (FALSE);
  }
  /* okay.. its not periodic...  */
  /* bogus call */
  if (!cmd || !pl)
    return (FALSE);
  
  if (IS_NPC(pl))
  {
    virt = GET_VNUM(pl);
    if (virt == EVIL_AVATAR_MOB || virt == GOOD_AVATAR_MOB)
      is_avatar = TRUE;
  }
  
  
  /* if it is not the blocked direction command, we don't give a dang...  */
  if ((cmd != dir) &&
      !((dir == 'a') &&
        ((cmd == CMD_NORTH) || (cmd == CMD_WEST) || (cmd == CMD_EAST) ||
         (cmd == CMD_SOUTH) || (cmd == CMD_DOWN) || (cmd == CMD_UP) ||
         (cmd == CMD_NORTHWEST) || (cmd == CMD_SOUTHWEST) ||
         (cmd == CMD_NORTHEAST) || (cmd == CMD_SOUTHEAST) || (cmd == CMD_NW)
         || (cmd == CMD_SW) || (cmd == CMD_SE) || (cmd == CMD_NE))) &&
      (is_avatar == FALSE))
    return (FALSE);
  
  /*
   * if badge isn't zero, we check if player has the right badge, else we check
   * player assoiation info
   */
  allowed = 0;
  if (badge)
  {
    if (pl->equipment[GUILD_INSIGNIA])
      allowed =
      (badge ==
       obj_index[pl->equipment[GUILD_INSIGNIA]->R_num].virtual_number);
  }
  else
  {
    P_char   t_ch = pl;
    
    /*
     * we allow entry if player has the right association number and is member
     * and is higher in rank than parole or if it's a god
     */
    
    if (IS_PC_PET(pl))
      t_ch = pl->following;
    
    if(!t_ch)
      return FALSE;
    
    bits = GET_A_BITS(t_ch);
    
    
    allowed = (((GET_A_NUM(t_ch) == assoc) && IS_MEMBER(bits) && GT_PAROLE(bits)) || IS_TRUSTED(t_ch)); 
		/*                                                                                       (GET_CLASS(t_ch) == CLASS_ROGUE && IS_AFFECTED(t_ch, AFF_SNEAK)));                            */
    
    struct alliance_data *alliance = get_alliance(GET_A_NUM(ch));
    
    if (alliance)
    {
      allowed2 = ((GET_A_NUM(t_ch) == alliance->forging_assoc_id) || 
                  (GET_A_NUM(t_ch) == alliance->joining_assoc_id)); 
    }
    
  }
  if (is_avatar)
    allowed = 0;
  
  if (allowed || allowed2)
  {
    if( IS_TRUSTED(pl) )
    {
      return FALSE;
    }
    
    if (IS_PC(pl) && GT_DEPUTY(bits))
    {
      act("$N snaps to attention and salutes as $n passes by.",
          FALSE, pl, 0, ch, TO_ROOM);
      act("$N respectfully salutes you as you pass by.",
          FALSE, pl, 0, ch, TO_CHAR);
    }
    else
    {
      act("$N nods in acknowledgment, stands aside and lets $n pass.",
          FALSE, pl, 0, ch, TO_ROOM);
      act("$N nods at you and stands aside to let you pass.",
          FALSE, pl, 0, ch, TO_CHAR);
    }
    return (FALSE);
  }
  /*
   * BLOCK!
   */
  act("$N pushes you back so hard that you fall down.",
      FALSE, pl, 0, ch, TO_CHAR);
  act("$n is sent to the floor by $N's mighty push.",
      FALSE, pl, 0, ch, TO_NOTVICT);
  SET_POS(pl, POS_PRONE + GET_STAT(pl));
  CharWait(pl, (int) get_property("guild.golems.pushDuration.secs", (PULSE_VIOLENCE * 2)));
  if (is_avatar)
    MobStartFight(ch, pl);
  
  return (TRUE);
}

int house_guard(P_char ch, P_char pl, int cmd, char *arg)
{
  ush_int  assoc;
  uint     bits;
  char    *tmp;
  int      dir = -1;
  int      badge = 0;
  int      allowed = 0;
  P_char   tch, next_ch;
  char     buf[MAX_STRING_LENGTH];
  char     temp[MAX_STRING_LENGTH];
  char     tmp2[MAX_STRING_LENGTH];
  int      is_avatar = FALSE;
  int      virt = 0;
  
  if (cmd == CMD_SET_PERIODIC)
    return TRUE;
  
  /* if it is not the blocked direction command, we don't give a dang...  */
  
  /*
   * if badge isn't zero, we check if player has the right badge, else we check
   * player assoiation info
   */
  
  allowed = 0;
  
  if (!ch)
    return 0;
  
  if (!pl)
    return 0;
  
  sprintf(tmp2, "%s_123", GET_NAME(pl));
  
  *tmp2 = tolower(*tmp2);
  
  
  
  tmp = strstr(GET_NAME(ch), tmp2);
  if (!(cmd == CMD_NORTH))
    return 0;
  
  if (!tmp)
    allowed = 0;
  else
    allowed = 1;
  
  if (GET_LEVEL(pl) > MINLVLIMMORTAL)
    allowed = 1;
  
  if (allowed)
  {
    if ((IS_TRUSTED(pl) || GT_DEPUTY(bits)) && IS_PC(pl))
    {
      act("$N snaps to attention and salutes as $n passes by.",
          FALSE, pl, 0, ch, TO_ROOM);
      act("$N respectfully salutes you as you pass by.",
          FALSE, pl, 0, ch, TO_CHAR);
    }
    else
    {
      act("$N nods in acknowledgment, stands aside and lets $n pass.",
          FALSE, pl, 0, ch, TO_ROOM);
      act("$N nods at you and stands aside to let you pass.",
          FALSE, pl, 0, ch, TO_CHAR);
    }
    return (FALSE);
  }
  /*
   * BLOCK!
   */
  act("$N pushes you back so hard that you fall down.",
      FALSE, pl, 0, ch, TO_CHAR);
  act("$n is sent to the floor by $N's mighty push.",
      FALSE, pl, 0, ch, TO_NOTVICT);
  SET_POS(pl, POS_PRONE + GET_STAT(pl));
  CharWait(pl, PULSE_VIOLENCE * 3);
  if (is_avatar)
    MobStartFight(ch, pl);
  
  return (TRUE);
}
