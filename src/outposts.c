//
// Outposts and buildings are not attached to the mud, but is here in case someone wants to clean up and finish the code,
// which was started by Torgal in 2008 and then continued by Venthix in 2009.
// - Torgal 1/29/2010
//

/* outposts.c
   - Property of Duris
     Mar 09

*/

#include <stdlib.h>
#include <cstring>
#include <vector>
using namespace std;

#include "prototypes.h"
#include "defines.h"
#include "utils.h"
#include "structs.h"
#include "nexus_stones.h"
#include "racewar_stat_mods.h"
#include "sql.h"
#include "interp.h"
#include "comm.h"
#include "spells.h"
#include "db.h"
#include "damage.h"
#include "epic.h"
#include "buildings.h"
#include "outposts.h"
#include "assocs.h"
#include "specs.prototypes.h"

extern P_index mob_index;
extern P_index obj_index;
extern P_room world;
extern P_desc descriptor_list;
extern P_char character_list;
extern P_obj object_list;
extern const char *apply_names[];
extern BuildingType building_types[];
extern const char *dirs[];
extern const int rev_dir[NUM_EXITS];
extern bool create_walls(int room, int exit, P_char ch, int level, int type,
                         int power, int decay, char *short_desc, char *desc,
                         ulong flags);

#ifdef __NO_MYSQL__
int init_outposts()
{
    // load nothing
}

void do_outpost(P_char ch, char *arg, int cmd) 
{
    // do nothing
}

int get_current_outpost_hitpoints(Building *building)
{
  return 0;
}

void outpost_update_resources(P_char ch, int wood, int stone)
{
  
}

int outpost_rubble(P_obj obj, P_char ch, int cmd, char *arg)
{
  return 0;
}
void set_current_outpost_hitpoints(Building *building)
{
  
}
void outpost_death(P_char outpost, P_char killer)
{
  
}
#else

extern MYSQL* DB;

int outpost_locations[] = {
        // Outpost ID - 1
  0,    // 0
  0     // 1
};

int init_outposts()
{
  fprintf(stderr, "-- Booting outposts\r\n");
  
  // Outpost specs, (can be found in building.c)
  //mob_index[real_mobile(0)].func.mob = ;
  //obj_index[real_object(0)].func.obj = ;

  //load_outposts();
  init_outpost_resources();
}

int load_outposts()
{
  // load outposts from DB
  if( !qry("SELECT id, owner_id, level, walls FROM outposts") )
  {
    debug("load_outposts() can't read from db");
    return FALSE;
  }
  
  MYSQL_RES *res = mysql_store_result(DB);
 
  if( mysql_num_rows(res) < 1 )
  {
    mysql_free_result(res);
    return FALSE;
  }
 
  Building* building;

  MYSQL_ROW row;
  while( row = mysql_fetch_row(res) )
  {
    int id = atoi(row[0]);
    int guild = atoi(row[1]);
    int level = atoi(row[2]);
    int walls = atoi(row[3]);

    if (building = load_building(guild, BUILDING_OUTPOST, outpost_locations[id], level))
    {
      if (walls) outpost_generate_walls(building, walls, NORTH);
    }
        
  }
  
  mysql_free_result(res);

  return TRUE;
}

int get_current_outpost_hitpoints(Building *building)
{
  if (!qry("SELECT id, hitpoints FROM outposts WHERE id = %d", building->id-1))
  {
    debug("get_current_outpost_hitpoints() cant read from db");
    return FALSE;
  }

  MYSQL_RES *res = mysql_store_result(DB);

  if (mysql_num_rows(res) < 1)
  {
    mysql_free_result(res);
    return FALSE;
  }

  MYSQL_ROW row = mysql_fetch_row(res);

  int hitpoints = atoi(row[1]);

  mysql_free_result(res);

  return hitpoints;
}

int get_outpost_resources(Building *building, int type)
{
  if (!qry("SELECT id, owner_id FROM outposts WHERE id = %d", building->id-1))
  {
    debug("get_outpost_resources() cant read from db");
    return FALSE;
  }

  MYSQL_RES *res = mysql_store_result(DB);

  if (mysql_num_rows(res) < 1)
  {
    mysql_free_result(res);
    return FALSE;
  }

  MYSQL_ROW row = mysql_fetch_row(res);

  int id = atoi(row[1]);

  mysql_free_result(res);

  int resources = get_guild_resources(id, type);

  return resources;
}

int get_guild_resources(int id, int type)
{
  if ((type != WOOD) && (type != STONE))
  {
    debug("get_guild_resources() passed invalid type %d", type);
    return FALSE;
  }
  if (!id)
  {
    debug("get_guild_resources() passed invalid guild id %d", id);
    return FALSE;
  }
  
  if (!qry("SELECT id, wood, stone FROM associations WHERE id = %d", id));
  {
    // WHY IS THIS FAILING?
    debug("get_guild_resources() cant read from db");
    return FALSE;
  }

  MYSQL_RES *res = mysql_store_result(DB);

  if (mysql_num_rows(res) < 1)
  {
    mysql_free_result(res);
    return FALSE;
  }
  
  MYSQL_ROW row = mysql_fetch_row(res);

  int resources = atoi(row[type]);

  mysql_free_result(res);
  
  return resources;
}

void set_current_outpost_hitpoints(Building *building)
{
  db_query("UPDATE outposts SET hitpoints='%d' WHERE id='%d'", ((GET_HIT(building->mob) < 0) ? 0 : GET_HIT(building->mob)), building->id-1);
}
  
void do_outpost(P_char ch, char *arg, int cmd)
{
  char buff2[MAX_STRING_LENGTH];
  char buff3[MAX_STRING_LENGTH];
  P_char op;

  if( !ch || IS_NPC(ch) )
    return;

  argument_interpreter(arg, buff2, buff3);
  
  if( IS_TRUSTED(ch) && !str_cmp("reset", buff2) )
  {
    if (GET_LEVEL(ch) != OVERLORD)
    {
      send_to_char("Ask an overlord, this command will reset all active outposts.\n", ch);
      return;
    }

    send_to_char("Resetting all outposts.\n", ch);
    reset_outposts(ch);

    return;
  }
  
  if( IS_TRUSTED(ch) && !str_cmp("reload", buff2) )
  {
    if( strlen(buff3) < 1 )
    {
      send_to_char("syntax: outpost reload <outpost id>\r\n", ch);
      return;
    }

    if( !isdigit(*buff3) )
    {  
      send_to_char("outpost id needs to be a number.\r\n", ch);
      return;
    }
    
    debug("outpost reload option not implemented yet.");
    //reload_outpost(ch, atoi(buff3));
    
    return;
  }

  if (!str_cmp("repair", buff2))
  {
    for (op = world[ch->in_room].people; op; op = op->next_in_room)
    {
      if (affected_by_spell(op, TAG_BUILDING) && GET_VNUM(op) == BUILDING_OUTPOST_MOB)
      {
        send_to_char("You begin repairing the outpost.\r\n", ch);
        //add_event(event_outpost_repair, )
        return;
      }
    }
  }
 
  send_to_char("options available: repair\r\n", ch);

  //show_outposts(ch);

  if( IS_TRUSTED(ch) ) ;
    //show_outposts_wiz(ch);
  else;
    //show_outposts(ch);
    
}

void outpost_death(P_char outpost, P_char killer)
{
  Building *building = get_building_from_char(outpost);
 //handle materials from old outpost
  int wood = building_types[BUILDING_OUTPOST-1].req_wood * building->level;
  int stone = building_types[BUILDING_OUTPOST-1].req_stone * building->level;
  wood = (int)((float)wood * get_property("outpost.materials.death.convert.wood", 0.500));
  stone = (int)((float)stone * get_property("outpost.materials.death.convert.stone", 0.500));
  P_obj rubble = read_object(97800, VIRTUAL);
  rubble->value[0] = wood;
  rubble->value[1] = stone;
  obj_to_room(rubble, outpost->in_room);

 //reset outpost data in the db
  reset_one_outpost(building);

  GET_HIT(building->mob) = 0;
  set_current_outpost_hitpoints(building);
}

// Add resources to a player's guild's current resource pool
void outpost_update_resources(P_char ch, int wood, int stone)
{
  if (!qry("SELECT id, wood, stone FROM associations WHERE id = %d", GET_A_NUM(ch)))
  {
    debug("outpost_update_resources() cant read from db");
    return;
  }
 
  MYSQL_RES *res = mysql_store_result(DB);

  if (mysql_num_rows(res) < 1)
  {
    mysql_free_result(res);
    return;
  }
  
  MYSQL_ROW row = mysql_fetch_row(res);

  int cur_wood = atoi(row[1]);
  int cur_stone = atoi(row[2]);

  mysql_free_result(res);
  
  db_query("UPDATE associations SET wood='%d', stone='%d' WHERE id='%d'", (int)(wood+cur_wood), (int)(stone+cur_stone), GET_A_NUM(ch));
}

void reset_one_outpost(Building *building)
{
  P_char op;
  int id;

  if (!building->id)
  {
    debug("error calling reset_one_outpost, no building ID available");
    return;
  }
  id = building->id-1;

  db_query("UPDATE outposts SET owner_id = '0', level = '1', walls = '0', archers = '0', hitpoints = '%d', portal_room = '0', portal_dest = '0' WHERE id = '%d'", building_types[BUILDING_OUTPOST-1].hitpoints, id);

  GET_MAX_HIT(building->mob) = building->mob->points.base_hit = GET_HIT(building->mob) = building_types[BUILDING_OUTPOST-1].hitpoints;

  SET_POS(building->mob, POS_STANDING + STAT_NORMAL);
}

void reset_outposts(P_char ch)
{
  P_desc d;
  P_char op;
  Building *building;

  for (d = descriptor_list; d; d = d->next)
  {
    if (!d->character)
      continue;

    op = d->character;

    if (!affected_by_spell(op, TAG_BUILDING))
      continue;

    building = get_building_from_char(op);

    reset_one_outpost(building);
  }
}

int outpost_rubble(P_obj obj, P_char ch, int cmd, char *arg)
{
  char buff[MAX_STRING_LENGTH], buff2[MAX_STRING_LENGTH];
  P_obj t_obj;
  int wood = 0;
  int stone = 0;

  if (cmd == CMD_SET_PERIODIC || cmd == CMD_PERIODIC)
    return FALSE;

  if (!obj)
    return FALSE;

  if (cmd == CMD_GET || cmd == CMD_TAKE)
  {
    arg = one_argument(arg, buff);
    if (!*buff)
      return FALSE;
    
    t_obj = get_obj_in_list_vis(ch, buff, world[ch->in_room].contents);
    
    if (t_obj = obj)
    {
      if (!GET_A_NUM(ch))
      {
        send_to_char("You need to be guilded to make use of outpost resources!\r\n", ch);
        return TRUE;
      }

      wood = obj->value[0];
      stone = obj->value[1];
      outpost_update_resources(ch, wood, stone);

      sprintf(buff2, "You receive %d wood and %d stone in outpost resources.\r\n", wood, stone);
      send_to_char(buff2, ch);
      
      extract_obj(obj, TRUE);

      return TRUE;
    }
  }
  return FALSE;
}

void outpost_create_wall(int location, int direction, int type)
{
  // type here more determines if we have stronger or weaker walls.. ie setting
  // the power of the wall.  hit wall does 1 damage to WALL_OUTPOST.
  int value;

  // set this later based upon type
  value = 300;

  if (create_walls(location, direction, NULL, 64, WALL_OUTPOST, value, -1,
       "&+Wan outpost wall&n",
       "&+WAn outpost wall is here to the %s.&n", 0))
  {
    SET_BIT(world[location].dir_option[direction]->exit_info, EX_BREAKABLE);
    SET_BIT(VIRTUAL_EXIT
        ((world[location].dir_option[direction])->to_room,
	  rev_dir[direction])->exit_info, EX_BREAKABLE);
  }
}

int outpost_generate_walls(Building* building, int type, int gate)
{
  int location = 0;
  int walllocal[10];
  int x;

  if (building->mob->in_room)
    location = building->mob->in_room;

  if (!location)
  {
    debug("outpost_generate_walls() can't find location");
    return FALSE;
  }

  // assign rooms
  walllocal[NORTH] = world[location].dir_option[NORTH]->to_room;
  walllocal[EAST] = world[location].dir_option[EAST]->to_room;
  walllocal[SOUTH] = world[location].dir_option[SOUTH]->to_room;
  walllocal[WEST] = world[location].dir_option[WEST]->to_room;
  walllocal[NORTHWEST] = world[walllocal[NORTH]].dir_option[WEST]->to_room;
  walllocal[NORTHEAST] = world[walllocal[NORTH]].dir_option[EAST]->to_room;
  walllocal[SOUTHEAST] = world[walllocal[SOUTH]].dir_option[WEST]->to_room;
  walllocal[SOUTHWEST] = world[walllocal[SOUTH]].dir_option[EAST]->to_room;
  walllocal[UP] = 0;
  walllocal[DOWN] = 0;

  for (x = 0; x < NUM_EXITS; x++)
  {
    if (x == gate)
    {
      outpost_setup_gateguards(world[walllocal[x]].dir_option[gate]->to_room, OUTPOST_GATEGUARD_WAR, 4);
      continue;
    }

    switch(x)
    {
    case NORTH:
    case EAST:
    case SOUTH:
    case WEST:
      debug("creating wall in room %d, towards the %s", walllocal[x], dirs[x]);
      outpost_create_wall(walllocal[x], x, type);
      break;
    case NORTHWEST:
      debug("creating wall in room %d, towards the west", walllocal[x]);
      outpost_create_wall(walllocal[x], WEST, type);
      debug("creating wall in room %d, towards the north", walllocal[x]);
      outpost_create_wall(walllocal[x], NORTH, type);
      break;
    case NORTHEAST:
      debug("creating wall in room %d, towards the north", walllocal[x]);
      outpost_create_wall(walllocal[x], NORTH, type);
      debug("creating wall in room %d, towards the east", walllocal[x]);
      outpost_create_wall(walllocal[x], EAST, type);
      break;
    case SOUTHEAST:
      debug("creating wall in room %d, towards the east", walllocal[x]);
      outpost_create_wall(walllocal[x], WEST, type);
      debug("creating wall in room %d, towards the south", walllocal[x]);
      outpost_create_wall(walllocal[x], SOUTH, type);
      break;
    case SOUTHWEST:
      debug("creating wall in room %d, towards the south", walllocal[x]);
      outpost_create_wall(walllocal[x], SOUTH, type);
      debug("creating wall in room %d, towards the west", walllocal[x]);
      outpost_create_wall(walllocal[x], EAST, type);
      break;
    default:
      break;
    }
  }
}

int outpost_load_gateguard(int location, int type)
{
  P_char guard;
  
  guard = read_mobile(type, VIRTUAL);

  //mob_index[real_mobile0(OUTPOST_GATEGUARD)].func.mob = outpost_gateguard_proc;

  if (!guard)
  {
    debug("outpost_load_gateguards() error reading mobile");
    return FALSE;
  }

  char_to_room(guard, location, -1);

  return TRUE;
}

void outpost_setup_gateguards(int location, int type, int amnt)
{
  int i;

  for (i = 0; i < amnt; i++)
  {
    outpost_load_gateguard(location, type);
  }
}

#endif

