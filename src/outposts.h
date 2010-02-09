//
// Outposts and buildings are not attached to the mud, but is here in case someone wants to clean up and finish the code,
// which was started by Torgal in 2008 and then continued by Venthix in 2009.
// - Torgal 1/29/2010
//

#ifndef _OUTPOSTS_H_
#define _OUTPOSTS_H_

#include "structs.h"
#include "buildings.h"

#define OUTPOST_LEVEL_ONE    2
#define OUTPOST_LEVEL_TWO    6
#define OUTPOST_LEVEL_THREE 10

#define WOOD        1
#define STONE       2

#define WALLS_NONE           0
#define WALLS_WOOD           1
#define WALLS_RWOOD          2 // reenforced wood
#define WALLS_                 // add stone wall types when i make the ore setups

int init_outposts();
int load_outposts();
void do_outpost(P_char, char*, int);
void outpost_death(P_char, P_char, Building*);
int get_current_outpost_hitpoints(Building*);
void set_current_outpost_hitpoints(Building*);
int get_outpost_resources(Building*, int);
int get_guild_resources(int, int);
int get_outpost_appliedresources(Building*);
void outpost_update_resources(P_char, int, int);
void outpost_death(P_char, P_char);
void reset_one_outpost(Building*);
void reset_outposts(P_char);
int outpost_rubble(P_obj, P_char, int, char*);
void outpost_create_wall(int, int, int);
int outpost_generate_walls(Building*, int, int);
int outpost_load_gateguard(int, int);
void outpost_setup_gateguards(int, int, int);

// For op_resources.c
#define RES_COPSE_VNUM      97820
#define RES_TSTAND_VNUM     97821

#define TREE_RESOURCE          0
#define TREE_TOTAL             1
#define TREE_BUSY              2

void init_outpost_resources();
void load_trees(int);
void load_one_tree(int);
void event_tree_growth(P_char, P_char, P_obj, void*);
int resources_tree_proc(P_obj, P_char, int, char*);
void event_harvest_tree(P_char, P_char, P_obj, void*);
#endif
