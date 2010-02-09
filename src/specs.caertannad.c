      /* Caer tannad shout proc, nice and simple */
#include <ctype.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <list>
using namespace std;

#include "comm.h"
#include "db.h"
#include "events.h"
#include "interp.h"
#include "prototypes.h"
#include "spells.h"
#include "specs.prototypes.h"
#include "structs.h"
#include "utils.h"
#include "weather.h"
#include "justice.h"
#include "assocs.h"
#include "graph.h"
#include "damage.h"
#include "reavers.h"
#include "specs.caertannad.h"

extern P_char character_list;
extern P_desc descriptor_list;
extern P_index mob_index;
extern P_index obj_index;
extern P_room world;
extern P_obj justice_items_list;
extern char *coin_names[];
extern const char *command[];
extern const char *dirs[];
extern const char rev_dir[];
extern const struct stat_data stat_factor[];
extern int planes_room_num[];
extern int racial_base[];
extern int top_of_world;
extern int top_of_zone_table;
extern struct command_info cmd_info[MAX_CMD_LIST];
extern struct str_app_type str_app[];
extern struct time_info_data time_info;
extern struct zone_data *zone;
extern struct zone_data *zone_table;
extern const char *crime_list[];
extern const char *crime_rep[];
extern const char *specdata[][MAX_SPEC];
extern struct class_names class_names_table[];
int      range_scan_track(P_char ch, int distance, int type_scan);
extern P_obj    object_list;

int caertannad_summon(P_char ch, P_char tch, int cmd, char *arg)
{
  int      helpers[] = { 78478, 78480, 0 };
  if (cmd == -10)
    return TRUE;
  if (!tch && !number(0, 4))
    return shout_and_hunt(ch, 100,
                          "&+CMages! Soldiers! To arms! &+CAnnihilate &=LC%s&n&+C!&n",
                          NULL, helpers, 0, 0);

  return FALSE;
}

