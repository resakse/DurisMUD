#ifndef _TRADESKILL_H_

#define MINE_VNUM 193
#define HAMMER_VNUM 252
#define PICK_VNUM 253
#define POLE_VNUM 336
#define PARCHMENT_VNUM 251 
#define MAX_NEEDED_ORE 5
#define SMALL_IRON_ORE 194
#define MEDIUM_IRON_ORE 196
#define LARGE_IRON_ORE 197

#define SMALL_STEEL_ORE 198
#define MEDIUM_STEEL_ORE 199
#define LARGE_STEEL_ORE 200

#define SMALL_COPPER_ORE 201
#define MEDIUM_COPPER_ORE 202
#define LARGE_COPPER_ORE 219

#define SMALL_SILVER_ORE 220
#define MEDIUM_SILVER_ORE 221
#define LARGE_SILVER_ORE  222

#define SMALL_GOLD_ORE  223
#define MEDIUM_GOLD_ORE 224
#define LARGE_GOLD_ORE 225

#define SMALL_PLATINUM_ORE 226
#define MEDIUM_PLATINUM_ORE 229
#define LARGE_PLATINUM_ORE 230

#define SMALL_MITHRIL_ORE 231
#define MEDIUM_MITHRIL_ORE 232
#define LARGE_MITHRIL_ORE 233

#define MAX_ORE 21

#define ALLOW 1
#define ANTI 0

#define MINES_MAP_SURFACE   0
#define MINES_MAP_UD        1
#define MINES_MAP_THARNRIFT 2

struct forge_item {

  int id;
  char *keywords;
  char *long_desc;
  char *short_desc;
  int   ore_needed[5];  
  
  int   loc0;
  int   min0;
  int   max0;
  
  int   loc1;
  int   min1;
  int   max1;

  int skill_min;
  int how_rare;  

  int allow_anti;
  unsigned int classes;
  unsigned int wear_flags; 
  unsigned int aff1;
  unsigned int aff2;
  unsigned int aff3;
  unsigned int aff4;  
};

void mine_check(P_char);

struct mining_data {
  int room;
  int counter;
  int mine_quality;
};

/*

234 - crude iron bar
235 - refined iron bar
236 - crude steel bar
237 - refined steel bar
238 - crude copper bar
240 - refined copper bar
241 - crude silver bar
242 - refined silver bar
243 - crude gold bar
244 - refined gold bar
247 - crude platinum bar
248 - refined platinum bar
249 - crude mithril bar
250 - refined mithril bar
*/

int mines_properties(int map);
void initialize_tradeskills();
bool load_one_mine(int map);
void load_mines(bool set_event, bool load_all, int map);
int mine(P_obj obj, P_char ch, int cmd, char *arg);
void event_mine_check(P_char ch, P_char victim, P_obj, void *data);
void event_load_mines(P_char ch, P_char victim, P_obj, void *data);
bool invalid_mine_room(int rroom_id);


bool player_recipes_exists(char *charname);
void create_recipes_file(const char *dir, char *name);
void create_recipes_name(char *name);

#endif // _TRADESKILL_H_
