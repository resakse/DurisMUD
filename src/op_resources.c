//
// Outposts and buildings are not attached to the mud, but is here in case someone wants to clean up and finish the code,
// which was started by Torgal in 2008 and then continued by Venthix in 2009.
// - Torgal 1/29/2010
//

/* op_resources.c
   - Property of Duris
     Apr 09

*/

#include <stdlib.h>
#include <cstring>
#include <vector>
using namespace std;

#include "prototypes.h"
#include "buildings.h"
#include "outposts.h"
#include "utils.h"
#include "interp.h"
#include "assocs.h"

extern P_room world;
extern P_index obj_index;
extern P_obj object_list;
extern const char *dirs[];

struct forest_range_data {
  char *name;
  char *abbrev;
  int start;
  int end;
  int growth_rate;
} forest_data[] = {
  {"Surface Map", "map", 500000, 659999, 30},
  {0}
};

struct forest_event_data {
  int map;
  P_char ch;
};

void init_outpost_resources()
{
  int i;

  obj_index[real_object0(RES_COPSE_VNUM)].func.obj = resources_tree_proc;
  obj_index[real_object0(RES_TSTAND_VNUM)].func.obj = resources_tree_proc;

  for (i = 0; forest_data[i].name; i++)
  {
    load_trees(i);
  }
}

void load_trees(int map)
{
  int i;
  struct forest_event_data fdata;

  for (i = forest_data[map].start; i <= forest_data[map].end; i++)
  {
    if (world[real_room(i)].sector_type != SECT_FOREST)
      continue;

    load_one_tree(real_room(i));
  }
  
  fdata.map = map;

  if (forest_data[map].growth_rate)
    add_event(event_tree_growth, (WAIT_SEC * 60 * forest_data[map].growth_rate), NULL, NULL, 0, 0, &fdata, sizeof(fdata));

}

void load_one_tree(int room)
{
  P_obj tree;
  int i, edge = 0;

  for (i = 0; i < NUM_EXITS; i++)
  {
    if (world[room].dir_option[i] && world[room].dir_option[i]->to_room > 0)
      if (world[world[room].dir_option[i]->to_room].sector_type != SECT_FOREST)
        edge = TRUE;
  }

  if (edge)
  {
    tree = read_object(RES_COPSE_VNUM, VIRTUAL);
    if (!tree)
    {
      debug("Error loading tree obj [%d]", RES_COPSE_VNUM);
      return;
    }
  }
  else
  {
    tree = read_object(RES_TSTAND_VNUM, VIRTUAL);
    if (!tree)
    {
      debug("Error loading tree obj [%d]", RES_TSTAND_VNUM);
      return;
    }
  }

  // Set resource worth
  if (edge)
  {
    tree->value[TREE_RESOURCE] = number(1, 20);
    tree->value[TREE_TOTAL] = tree->value[TREE_RESOURCE];
  }
  else
  {
    tree->value[TREE_RESOURCE] = number(10, 40);
    tree->value[TREE_TOTAL] = tree->value[TREE_RESOURCE];
  }

  obj_to_room(tree, room); 
}

void event_tree_growth(P_char ch, P_char victim, P_obj xobj, void *data)
{
  struct forest_event_data *fdata = (struct forest_event_data*)data;

  if (!fdata)
  {
    debug("passed null pointer to event_forest_growth()");
    return;
  }

  int i, treehere = FALSE;
  int room, troom;
  P_obj obj, tobj;
  int map = fdata->map;

  // run through all objects in game
  for (obj = object_list; obj; obj = obj->next)
  {
    room = 0;
    // see if it's a resource tree
    if (obj &&
        ((GET_OBJ_VNUM(obj) == RES_COPSE_VNUM) ||
	 (GET_OBJ_VNUM(obj) == RES_TSTAND_VNUM)))
    {
      if (obj->loc.room)
        room = obj->loc.room;
      else
	continue;
    }

    // if the tree still exists but isn't at it's full resource, let it grow too
    if (obj->value[TREE_RESOURCE] < obj->value[TREE_TOTAL])
    {
      obj->value[TREE_RESOURCE] += number(1, 3);
    }      

    if (!room)
      continue;

    // Ok, so this room has a tree in it, run through exits of this room
    for (i = 0; i < NUM_EXITS; i++)
    {
      troom = 0;
      
      if (world[room].dir_option[i] && world[room].dir_option[i]->to_room > 0)
        troom = world[room].dir_option[i]->to_room;

      if (troom <= 0)
        continue;
      
      // and find a forest
      if (world[troom].sector_type != SECT_FOREST)
        continue;
    
      // see if theres a tree in the room
      for (tobj = world[troom].contents; tobj; tobj = tobj->next_content)
      {
        if (tobj->R_num == obj->R_num)
          treehere = TRUE;
      }

      if (treehere)
        continue;

      if (!number(0, 1))
        load_one_tree(troom);
    }
  }
// add_event(event_tree_growth, (WAIT_SEC * 60 * forest_data[map].growth_rate), NULL, NULL, 0, 0, &fdata, sizeof(fdata));
}

int resources_tree_proc(P_obj tree, P_char ch, int cmd, char *arg)
{
  if (cmd == CMD_SET_PERIODIC || cmd == CMD_PERIODIC)
    return FALSE;

  if (!ch || !tree)
    return FALSE;

  if (!IS_PC(ch))
    return FALSE;

  if (cmd == CMD_HARVEST)
  {
    if (!GET_A_NUM(ch))
    {
      send_to_char("You must belong to an association to harvest resources!\r\n", ch);
      return TRUE;
    }

    if (tree->value[TREE_BUSY])
    {
      send_to_char("This tree is already being harvested!\r\n", ch);
      return TRUE;
    }

    // ok so let's begin...
    tree->value[TREE_BUSY] = GET_PID(ch);

    send_to_char("You begin chopping down some trees...\r\n", ch);

    add_event(event_harvest_tree, WAIT_SEC * 3, ch, NULL, 0, 0, 0, 0);
    
    return TRUE;
  }

  return FALSE;
}

void event_harvest_tree(P_char ch, P_char victim, P_obj obj, void *data)
{
  if (!ch)
  {
    debug("event_harvest_tree() called with no ch");
    return;
  }

  if (!IS_PC(ch))
  {
    debug("event_harvest_tree() called with npc ch");
    return;
  }

  P_obj tobj, tree;
  
  for (tobj = world[ch->in_room].contents; tobj; tobj = tobj->next_content)
  {
    if (((GET_OBJ_VNUM(tobj) == RES_COPSE_VNUM) ||
         (GET_OBJ_VNUM(tobj) == RES_TSTAND_VNUM)) &&
        (tobj->value[TREE_BUSY] == GET_PID(ch)))
      tree = tobj;
      break;
  }

  if (!tree)
  {
    send_to_char("You must be in the same room as the tree to harvest it!\r\n", ch);
    return;
  }

  send_to_char("You fell a &+ytree&n and gather the &+ywood&n resouces.\r\n", ch);

  outpost_update_resources(ch, 1, 0);
  //debug("Wood resources for guild %d: %d", GET_A_NUM(ch), get_guild_resources(GET_A_NUM(ch), WOOD));

  if (!--tree->value[TREE_RESOURCE])
  {
    send_to_char("You have rendered this area barren of trees!\r\n", ch);
    extract_obj(tree, TRUE);
  }
  else
    add_event(event_harvest_tree, WAIT_SEC * 3, ch, NULL, 0, 0, 0, 0);
}  
