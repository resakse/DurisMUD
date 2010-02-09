/*
 *  guildhall.h
 *  Duris
 *
 *  Created by Torgal on 1/29/10.
 *
 */

#ifndef _GUILDHALL_H_
#define _GUILDHALL_H_

#include <vector>
#include <string>
using namespace std;

#include "structs.h"

// mud room/object/mob vnums/etc

#define LOG_GUILDHALLS "logs/log/guildhalls"

#define GH_ZONE_NUMBER 480
#define GH_START_VNUM 48020
#define GH_END_VNUM 48999

#define IN_GH_ZONE(r_room) ( world[r_room].number >= GH_START_VNUM && world[r_room].number <= GH_END_VNUM )

#define GH_DOOR_VNUM        48000
#define GH_BOARD_START      48100
#define GH_PORTAL_VNUM      48001
#define GH_WINDOW_VNUM      48008
#define GH_HEARTSTONE_VNUM  48009
#define GH_FOUNTAIN_VNUM    48004

#define GH_ROOM_TEMPLATE_ENTRANCE 48000
#define GH_ROOM_TEMPLATE_HEARTSTONE 48001
#define GH_ROOM_TEMPLATE_LIBRARY 48002
#define GH_ROOM_TEMPLATE_HEAL 48003
#define GH_ROOM_TEMPLATE_PORTAL 48004
#define GH_ROOM_TEMPLATE_INN 48005
#define GH_ROOM_TEMPLATE_GENERIC 48006
#define GH_ROOM_TEMPLATE_WINDOW 48007
#define GH_ROOM_TEMPLATE_HEAL 48003

#define GH_GOLEM_WARRIOR    11002
#define GH_GOLEM_CLERIC     11003
#define GH_GOLEM_SORCERER   11004

// GH-specific defines
#define GH_TYPE_MAIN 1
#define GH_TYPE_OUTPOST 2

#define GH_ROOM_TYPE_GENERIC 0
#define GH_ROOM_TYPE_ENTRANCE 1
#define GH_ROOM_TYPE_HEARTSTONE 2
#define GH_ROOM_TYPE_INN 3
#define GH_ROOM_TYPE_PORTAL 4
#define GH_ROOM_TYPE_WINDOW 5
#define GH_ROOM_TYPE_HEAL 6
#define GH_ROOM_NUM_TYPES 7

/*
 rooms values are type-specific
*/
#define GH_ROOM_NUM_VALUES 8

// entrance rooms
#define GH_VALUE_ENTRANCE_DIR 0
#define GH_GOLEM_SLOT 1
#define GH_GOLEM_NUM_SLOTS 4

#define GH_GOLEM_TYPE_WARRIOR 1
#define GH_GOLEM_TYPE_CLERIC 2
#define GH_GOLEM_TYPE_SORCERER 3

#define IS_GH_GOLEM(ch) ( IS_NPC(ch) && \
  ( GET_VNUM(ch) == GH_GOLEM_TYPE_WARRIOR || \
  GET_VNUM(ch) == GH_GOLEM_TYPE_CLERIC || \
  GET_VNUM(ch) == GH_GOLEM_TYPE_SORCERER ) )

struct Guildhall; // stub declaration so following can refer to it
struct GuildhallRoom; // stub declaration so following can refer to it

//
// duris commands and subcommands
//
void do_construct(P_char ch, char *arg, int cmd);
void do_guildhall(P_char ch, char *arg, int cmd);

//
// guildhall procs
//
int guildhall_door(P_obj obj, P_char ch, int cmd, char *arg);
int guildhall_golem(P_char ch, P_char pl, int cmd, char *arg);
int guildhall_window_room(int room, P_char ch, int cmd, char *arg);
int guildhall_window(P_obj obj, P_char ch, int cmd, char *arg);
int guildhall_heartstone(P_obj obj, P_char ch, int cmd, char *arg);

int check_gh_home(P_char ch, int r_room);

//
// Guildhall classes
//

struct GuildhallRoom {
  // fields stored in DB
  int id;
  int vnum;
  int guildhall_id;
  string name;
  int type;
  unsigned int value[GH_ROOM_NUM_VALUES];
  int exits[NUM_EXITS];
  
  const int template_vnum;
  
  virtual bool init();
  virtual bool deinit();
  virtual bool can_upgrade() const 
  {
    return ( this->type == GH_ROOM_TYPE_GENERIC );
  }
  
  bool save();
  bool valid();
  bool destroy();
  
  bool has_exit(int dir) const
  {
    return( this->exits[dir] >= 0 );
  }
  
  Guildhall* guildhall;

  P_room room;
  
  GuildhallRoom() : vnum(-1), guildhall_id(0), type(0), room(NULL), guildhall(NULL), template_vnum(GH_ROOM_TEMPLATE_GENERIC)
  {
    for( int i = 0; i < GH_ROOM_NUM_VALUES; i++ )
    {
      this->value[i] = 0;
    }
    
    for( int i = 0; i < NUM_EXITS; i++ )
    {
      this->exits[i] = -1;
    }    
  }
  
  GuildhallRoom(int _template_vnum) : vnum(-1), guildhall_id(0), type(0), room(NULL), guildhall(NULL), template_vnum(_template_vnum)
  {
    GuildhallRoom();
  }
};

struct EntranceRoom : public GuildhallRoom
{
  P_obj door;
  P_char golems[GH_GOLEM_NUM_SLOTS];
  
  bool init();
  bool deinit();  

  bool golem_died(P_char golem);
  
  EntranceRoom() : GuildhallRoom(GH_ROOM_TEMPLATE_ENTRANCE), door(NULL)
  {
    for( int i = 0; i < GH_GOLEM_NUM_SLOTS; i++ )
    {
      this->golems[i] = NULL;
    }
  }
};

struct InnRoom : public GuildhallRoom
{
  P_obj board;
  
  bool init();
  bool deinit();
  
  InnRoom() : GuildhallRoom(GH_ROOM_TEMPLATE_INN), board(NULL) {}
};

struct PortalRoom : public GuildhallRoom
{
  bool init();
  bool deinit();
  
  PortalRoom() : GuildhallRoom(GH_ROOM_TEMPLATE_PORTAL) {}
};

struct HeartstoneRoom : public GuildhallRoom
{
  bool init();
  bool deinit();

  P_obj heartstone;
  
  HeartstoneRoom() : GuildhallRoom(GH_ROOM_TEMPLATE_HEARTSTONE), heartstone(NULL) {}
};

struct WindowRoom : public GuildhallRoom
{
  bool init();
  bool deinit();

  P_obj window;
  
  WindowRoom() : GuildhallRoom(GH_ROOM_TEMPLATE_WINDOW), window(NULL) {}
};

struct HealRoom : public GuildhallRoom
{
  bool init();
  bool deinit();
  
  P_obj fountain;
  
  HealRoom() : GuildhallRoom(GH_ROOM_TEMPLATE_HEAL), fountain(NULL) {}
};

//
// Main Guildhall class
//

struct Guildhall {
  static vector<Guildhall*> guildhalls;

  static void initialize();
  static void shutdown();
  
  static void add(Guildhall*);
  static void remove(Guildhall*);
  
  static Guildhall* find_by_id(int id);
  static Guildhall* find_by_vnum(int vnum);
  static Guildhall* find_by_outside_vnum(int vnum);
  static GuildhallRoom* find_room_by_id(int id);
  static GuildhallRoom* find_room_by_vnum(int vnum);
  static Guildhall* find_from_ch(P_char ch);
  
  static int count_by_assoc_id(int assoc_id, int type);
  
  static const int max_rooms = 50;
  
  // fields stored in DB
  int id;  
  int assoc_id;  
  int type;
  int outside_vnum;

  bool init();
  bool save();
  bool can_add_room();
  void add_room(GuildhallRoom*);
  bool valid();
  bool destroy();
  bool deinit();
  bool reload();
  
  void golem_died(P_char);
  
  vector<GuildhallRoom*> rooms;

  EntranceRoom* entrance_room;
  InnRoom* inn_room;
  PortalRoom* portal_room;
  HeartstoneRoom* heartstone_room;
    
  Guildhall() : id(0), assoc_id(0), type(0), outside_vnum(0), entrance_room(NULL), inn_room(NULL), portal_room(NULL), heartstone_room(NULL) {}
  
  ~Guildhall()
  {
    this->clear_rooms();
  }
  
  void clear_rooms()
  {
    // delete guildhall rooms
    for( vector<GuildhallRoom*>::iterator r = this->rooms.begin(); r != this->rooms.end(); r++ )
    {
      delete(*r);
    }        
    
    this->rooms.clear();    
  }

  int entrance_vnum()
  {
    if( !this->entrance_room )
      return 0;
    
    return this->entrance_room->vnum;
  }
  
  int inn_vnum()
  {
    if( !this->inn_room )
      return 0;
    
    return this->inn_room->vnum;
  }
  
  void init_main();
  void init_outpost();
  void deinit_main();
  void deinit_outpost();
};

#endif // _GUILDHALLS_H_
