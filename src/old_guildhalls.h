#ifndef _GUILDHALLS_H_
#define _GUILDHALLS_H_

/************************************************************************
*                                                                       *
*                                                                       *
* structs and defines for house building                                *
* 2008 Lom: renamed from kingdom.h and removed kingdom stuff            *
************************************************************************/

#define MAX_GUESTS	         2
#define MAX_HOUSE_ROOMS       10 
#define MAX_CONTROLLED_LAND   100
#define START_HOUSE_VNUM      48000
#define END_HOUSE_VNUM        48999

#define HMODE_ERROR     0
#define HMODE_PRIVATE   1
#define HMODE_OPEN      2

#define HCONTROL_ERROR     0
#define HCONTROL_HOUSE     1
#define HCONTROL_GUILD     2
#define HCONTROL_CASTLE    3
#define HCONTROL_OUTPOST   4

/* objects in the castle_room zones we may want to load */
#define HOUSE_OUTER_DOOR	11007
#define HOUSE_INNER_DOOR	11008
#define HOUSE_FLAG	    	11012

#define SHOPKEEPER_VNUM    11005

#define MAGE_GOLEM_VNUM    11004
#define GUARD_GOLEM_VNUM   11001
#define CLERIC_GOLEM_VNUM  11003
#define WARRIOR_GOLEM_VNUM 11002

#define IS_ASSOC_GOLEM(ch) ( IS_NPC(ch) && \
                           ( GET_VNUM(ch) == MAGE_GOLEM_VNUM || \
                             GET_VNUM(ch) == CLERIC_GOLEM_VNUM || \
                             GET_VNUM(ch) == WARRIOR_GOLEM_VNUM || \
                             GET_VNUM(ch) == GUARD_GOLEM_VNUM ) )

#define CHEST_VNUM         115
#define FOUNTAIN_VNUM      11000
#define TELEPORTER_VNUM    11001
#define MOUTH_VNUM         11002
#define GUILD_HOLY_FOUNT   11005
#define GUILD_UNHOLY_FOUNT 11006
#define CABINET_VNUM       11009
#define WINDOW_VNUM        11013
#define GUILD_BOARD_START  11100

#define HCONTROL_ADD_MAGE_GOLEM     1
#define HCONTROL_ADD_CLERIC_GOLEM   2
#define HCONTROL_ADD_WARRIOR_GOLEM  3
#define HCONTROL_ADD_CHEST          4
#define HCONTROL_ADD_TELEPORTER     5
#define HCONTROL_ADD_FOUNTAIN       6
#define HCONTROL_ADD_INN            7
#define HCONTROL_ADD_SHOP           8
#define HCONTROL_ADD_BOARD          9
#define HCONTROL_ADD_ROOM           10
#define HCONTROL_DESC_ROOM          11
#define HCONTROL_NAME_ROOM          12
#define HCONTROL_ADD_MOUTH          13
#define HCONTROL_ADD_HEAL           14
#define HCONTROL_ADD_HOLY           15
#define HCONTROL_ADD_UNHOLY         16
#define HCONTROL_INITIAL_HOUSE      17
#define HCONTROL_INITIAL_GUILD      18
#define HCONTROL_ADD_CABINET        19
#define HCONTROL_ADD_SECRET         20
#define HCONTROL_ADD_WINDOW         21
#define HCONTROL_ADD_MOVEDOOR       22
#define HCONTROL_ADD_GUARD_GOLEM    23

struct house_control_rec {
   int vnum;         /* vnum of atrium          */
   int exit_num;     /* direction of house's exit  */
   time_t built_on;     /* date this house was built  */
   sh_int mode;         /* mode of ownership    */
   sh_int type;         /* castle, guild, house    */
   byte construction;           /* check if its in construction */
   char *owner;         /* name of house's owner   */
   int owner_guild;             /* owners guild # (if any)      */
   int num_of_guests;      /* how many guests for house  */
   char *guests[MAX_GUESTS];  /* names of house's guests */
   int num_of_rooms;            /* # of rooms in the house      */
   int room_vnums[MAX_HOUSE_ROOMS];   /* a list of each vnum          */
   time_t last_payment;    /* date of last house payment   */
   ush_int size;     /* Size of castle, 1-9     */
   long upgrades;    /* Extras purchased     */
   int controlled_land[MAX_CONTROLLED_LAND];  /* map rooms owned */
   P_obj outer_door;
   P_obj inner_door;
   P_house next;

   /* for guildhalls, which dont' have normal entrance, just an "enter blah" */
   char *entrance_keyword;

   /* now we list the possible mobs, items, and rooms a guild could have */
   /* (so we can completely automate the loading of the guild hall at boot)*/
   sh_int wizard_golems;
   sh_int warrior_golems;
   sh_int cleric_golems;
   sh_int guard_golems;
   int inn_vnum;
   int fountain_vnum;
   int teleporter1_room;
   int teleporter1_dest;
   int teleporter2_room;
   int teleporter2_dest;
   int shop_vnum;
   int heal_vnum; 
   int board_vnum;
   int mouth_vnum;
   int holy_fount_vnum;
   int unholy_fount_vnum;
   int secret_entrance;
   int window_vnum;

   struct house_sack_rec *sack_list;
};

struct house_sack_rec {
  
  char *name;  /* who's sacking */
  long time;  /* when they started */
  struct house_sack_rec *next;  /* next in line */ 
};

struct house_upgrade_rec {
   int vnum;        /* vnum of house being upgraded */
   long time;       /* time (real time) it was started */
   int type;        /* for mob/room/obj upgrade */
   int location;    /* where the mob/obj/room is going to be */
   int guild;       /* for guildhalls, who's it is */
   
   /* for room upgrades */
   int exit_dir;
   int door;
   char *door_keyword;


   P_house_upgrade next;  /* next in line */
};

P_house find_house_by_owner(char *name);
int charge_char(P_char, int);
P_house_upgrade get_con_rec(void);
void update_kingdoms(void);
void check_for_kingdom_trespassing(P_char);
void do_name_room(P_house, P_char, char *);
void do_describe_room(P_house, P_char, char *);
bool valid_build_location(int room, P_char ch, int type);
int house_cost(P_char ch);
int guild_cost(P_char ch);
P_house get_house_from_room(int rnum);
void hcontrol_detail_house(P_char ch, char *arg);
void hcontrol_clear_house(P_char ch, char *arg);
void hcontrol_restore_house(P_char ch, char *arg);
void hcontrol_change_house(P_char ch, char *arg);
void do_construct_room(P_house, P_char, char *);
void do_construct_house(P_char, char *);
void do_show_q(P_char);
void do_build_golem(P_house, P_char, int);
void do_build_inn(P_house, P_char);
void do_build_fountain(P_house, P_char);
void do_build_window(P_house, P_char);
void do_build_movedoor(P_house, P_char, char*);
void do_build_chest(P_house, P_char);
void do_build_shop(P_house, P_char);
void stock_guild_shop(int);
void do_build_teleporter(P_house, P_char, char *);
void do_build_mouth(P_house, P_char);
void do_build_holy(P_house, P_char);
void do_build_heal(P_house, P_char);
void do_build_unholy(P_house, P_char);
P_char get_char_online(char *);
void sack_house(P_house, P_char);
void check_sacks(P_house house);
void nuke_portal(int rnum);
void nuke_doorways(int rnum);
P_house find_house(int);
int room_guild_no(int);
void House_boot(void);
void hcontrol_list_houses(P_char, char *);
void hcontrol_build_house(P_char, char *);
void hcontrol_destroy_house(P_char, char *);
void hcontrol_pay_house(P_char, char *);
void do_hcontrol(P_char, char *, int);
void do_house(P_char, char *, int);
void House_save_all(void);
int House_can_enter(P_char, int, int);
void house_crash_save(void);
int hcontrol_setup_room(P_house, int, int);
void do_construct(P_char, char *, int);
void do_sack(P_char, char *, int);
void clear_sacks(P_char);
P_house house_ch_is_in(P_char);
void construct_castle(P_house);
void do_stathouse(P_char ch, char *argument, int cmd);
void destroy_castle(P_house);
void read_guild_room(int, int);
void write_guild_room(int, int);
void process_construction_q(void);
void event_housekeeping(P_char ch, P_char victim, P_obj obj, void *data);
//void do_housekeeping(void);
void open_market_house(void);
void do_decree(P_char, char *, int);
void enemy_hall_check(P_char ch, int room);

int writeHouse(P_house);
int deleteHouse(char *);
int restoreHouse(char *);
void restore_house(void);
void writeHouseObj(P_obj, int);
void restoreHouseObj(void);
int writeConstructionQ(void);
int loadConstructionQ(void);
void boot_kingdoms(void);

#endif // _GUILDHALLS_H_
