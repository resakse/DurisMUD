#ifndef _NEXUS_STONES_H_
#define _NEXUS_STONES_H_

#include "structs.h"

#define VAL_STONE_ID 0
#define VAL_ALIGN 1
#define VAL_GUARDIAN_ALIVE 2
#define VAL_STAT_AFFECT 3

#define STONE_ID(stone) ( (stone)->value[VAL_STONE_ID] )
#define STONE_ALIGN(stone) ( (stone)->value[VAL_ALIGN] )
#define STONE_TURNED(stone) ( STONE_ALIGN(stone) >= STONE_ALIGN_GOOD || STONE_ALIGN(stone) <= STONE_ALIGN_EVIL )
#define STONE_GUARD_ALIVE(stone) ( (stone)->value[VAL_GUARDIAN_ALIVE] )
#define STONE_STAT_AFFECT(stone) ( (stone)->value[VAL_STAT_AFFECT] )
#define STONE_TIMER(stone) ( (stone)->timer[0] )
#define STONE_TURN_TIMER(stone) ( (stone)->timer[1] )
#define STONE_SAGE_TIMER(stone) ( (stone)->timer[2] )

#define STONE_ALIGN_EVIL -3
#define STONE_ALIGN_GOOD 3

#define NEXUS_STONE_MARDUK    1
#define NEXUS_STONE_ANTU      2
#define NEXUS_STONE_ENLIL     3
#define NEXUS_STONE_NINGISHZIDA 4
#define MAX_NEXUS_STONES      9

#define NEXUS_BONUS_NONE      0
#define NEXUS_BONUS_EPICS     1
#define NEXUS_BONUS_EXP       2
#define NEXUS_BONUS_CARGO     3
#define NEXUS_BONUS_PRESTIGE  4
#define NEXUS_BONUS_RANDDROPS 5

#define MOB_GOOD_GUARDIAN 65
#define MOB_EVIL_GUARDIAN 66
#define MOB_NEUTRAL_GUARDIAN 74

#define MOB_EVIL_SAGE 67
#define MOB_GOOD_SAGE 68

#define OBJ_NEXUS_STONE 394
#define OBJ_GUARDIAN_MACE 395

#define IS_NEXUS_GUARDIAN(ch) ( IS_NPC(ch) && \
															( GET_VNUM(ch) == MOB_GOOD_GUARDIAN || \
															  GET_VNUM(ch) == MOB_EVIL_GUARDIAN ) )

#define IS_NEXUS_SAGE(ch) ( IS_NPC(ch) && \
                          ( GET_VNUM(ch) == MOB_GOOD_SAGE || \
                            GET_VNUM(ch) == MOB_EVIL_SAGE ) )

#define IS_NEXUS_STONE(obj) ( GET_OBJ_VNUM(obj) == OBJ_NEXUS_STONE )

/*
 
 mysql> desc nexus_stones;
 +-------------+--------------+------+-----+---------+-------+
 | Field       | Type         | Null | Key | Default | Extra |
 +-------------+--------------+------+-----+---------+-------+
 | id          | int(11)      | NO   |     | 0       |       |
 | name        | varchar(255) | NO   |     |         |       |
 | room_vnum   | int(11)      | NO   |     | 0       |       |
 | align       | int(11)      | NO   |     | 0       |       |
 | stat_affect | int(11)      | NO   |     | -1      |       |
 | bonus       | int(11)      | NO   |     | 0       |       |
 +-------------+--------------+------+-----+---------+-------+
 6 rows in set (0.00 sec)

 create table nexus_stones (
                            id int not null primary key auto_increment,
                            name varchar(255) not null default '',
                            room_vnum int not null default 0,
                            align int not null default 0,
                            stat_affect int not null default -1,
                            affect_amount int not null default 0,
			    bonus int not null default 0
                            );
 
*/

struct NexusStone {
  NexusStone() : stone_id(0), stone(NULL), guardian(NULL), sage(NULL) {}
  int stone_id;
  P_obj stone;
  P_char guardian;
  P_char sage;
};

struct NexusStoneInfo {
  int id;
  string name;
  int room_vnum;
  int align;
  int stat_affect;
  int affect_amount;
  int last_touched_at;
};

int init_nexus_stones();
int load_nexus_stones();
bool nexus_stone_info(int stone_id, NexusStoneInfo *info);
int check_nexus_bonus(P_char ch, int amount, int type);
int update_nexus_stone_align(int stone_id, int align);
int nexus_stone(P_obj stone, P_char ch, int cmd, char *arg);
P_obj get_nexus_stone(int stone_id);
P_char get_nexus_guardian(int stone_id);
int guardian_stone_id(P_char ch);
int nexus_stone_guardian(P_char ch, P_char pl, int cmd, char *arg);
int nexus_guardian_pwn_mace(P_obj obj, P_char ch, int cmd, char *arg);
void nexus_guardian_nuke(P_char ch, P_char vict, int dam);
void nexus_guardian_energy_burst(P_char ch, P_char vict);
P_char load_guardian(int stone_id, int rroom_id, int align); 
P_char load_sage(int stone_id, int rroom_id, int align);
P_char get_nexus_sage(int stone_id);
int nexus_sage(P_char ch, P_char pl, int cmd, char *arg);
int nexus_sage_train(P_char ch, P_char pl, char *arg);
int nexus_sage_ask(P_char ch, P_char pl, char *arg);
int remove_nexus_sage(int stone_id);
bool load_nexus_stone(int stone_id, const char* name, int room_vnum, int align);
void world_echo(char *str);
void event_nexus_stone_hum(P_char __ch, P_char __victim, P_obj stone, void *data);
void update_nexus_stat_mods();
void nexus_stone_epics(P_char ch, P_obj stone);
void do_nexus(P_char ch, char *arg, int cmd);
void reset_nexus_stones(P_char ch);
void reload_nexus_stone(P_char ch, int stone_id);
void nexus_stone_god_list(P_char ch);
void nexus_stone_list(P_char ch);
bool nexus_stone_expired(int stone_id);
void expire_nexus_stone(int stone_id);
P_obj get_random_enemy_nexus(P_char ch);

#endif
