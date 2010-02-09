//
// Outposts and buildings are not attached to the mud, but is here in case someone wants to clean up and finish the code,
// which was started by Torgal in 2008 and then continued by Venthix in 2009.
// - Torgal 1/29/2010
//

#ifndef _BUILDINGS_H_
#define _BUILDINGS_H_

#include <vector>
using namespace std;

#include "map.h"
#include "structs.h"

#define BUILDING_START_ROOM 97800
#define BUILDING_END_ROOM 97899

#define IS_BUILDING_ROOM(room) ( world[room].number >= BUILDING_START_ROOM && world[room].number <= BUILDING_END_ROOM )

// Building types for building_types array
#define BUILDING_OUTPOST 1

// Building structure mob vnums
#define BUILDING_OUTPOST_MOB     97800

// Normal mobs relating to outposts
#define OUTPOST_GATEGUARD_WAR    97801

struct Building
{
  Building();
  Building(int _guild_id, int _type, int _room_vnum, int _level);
  ~Building();
    
  int load();
  int unload();

  int id;
  
  static int next_id;
  
  int guild_id;
  int type;
  int room_vnum;
  int level;

  bool loaded;  
  
  P_char mob;
  mob_proc_type mob_proc;  
  
  vector<P_room> rooms;
  
  int gate_room() 
  {
    if( rooms.size() < 1 )
      return 0;
    
    if( !rooms[0] )
      return 0;
    
    return real_room0(rooms[0]->number);
  }
  
  int location() { real_room0(room_vnum); }
};

// building generation functions
typedef int (*building_generator_type)(Building*);

struct BuildingType
{
  int type;
  int mob_vnum;
  int req_wood;
  int req_stone;
  int hitpoints;
  building_generator_type generator; // function to set up building, instantiate rooms, set flags, etc
};

// utility functions

int initialize_buildings(); // called from comm.c
BuildingType get_type(int type);
Building* get_building_from_char(P_char ch);
Building* get_building_from_room(int rroom);
Building* load_building(int guildid, int type, int location, int level);
void do_build(P_char ch, char *argument, int cmd);
int building_mob_proc(P_char ch, P_char pl, int cmd, char *arg);
int check_outpost_death(P_char, P_char);

//
// individual building procs
//

// OUTPOST
int outpost_generate(Building* building);
int outpost_mob(P_char ch, P_char pl, int cmd, char *arg);
int outpost_inside(int room, P_char ch, int cmd, char *arg);

#endif

