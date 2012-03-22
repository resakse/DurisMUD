#ifndef __NECROMANCY_H__
#define __NECROMANCY_H__

#include "config.h"
#include "structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>

#define NECROPET      1201

#define DRAGONSCALE_VNUM 392

#define NECROPET_START      0
#define NECROPET_SKELETON   0
#define NECROPET_ZOMBIE     1
#define NECROPET_DKNIGHT    2
#define NECROPET_SPECTRE    3
#define NECROPET_VAMPIRE    4
#define NECROPET_WRAITH     5
#define NECROPET_LICH       6
#define NECROPET_SHADOW     7
#define NECROPET_END        7

#define THEURPET_START      8
#define THEURPET_SKELETON   8
#define THEURPET_ZOMBIE     9
#define THEURPET_SPECTRE    10
#define THEURPET_WRAITH     11
#define THEURPET_VAMPIRE    12
#define THEURPET_LICH       13
#define THEURPET_SHADOW     14
#define THEURPET_END        14

#define NECROPET_LAST       14

#define NECROGOLEM_FLESH 0
#define NECROGOLEM_BLOOD 1
#define NECROGOLEM_BONE  2
#define THEURGOLEM_VALOR 3
#define NECROGOLEM_LAST  3

#define CORPSEFORM_INNATE 0
#define CORPSEFORM_REG    1

/* defines for corpse objects values[] */
#define CORPSE_WEIGHT 0
#define CORPSE_FLAGS 1
#define CORPSE_LEVEL 2
#define CORPSE_VNUM 3
#define CORPSE_PID 3
#define CORPSE_EXP_LOSS 4
#define CORPSE_RACEWAR 5
#define CORPSE_SAVEID 6
#define CORPSE_RACE 7

void spell_corpseform(int, P_char, char *, int, P_char, P_obj);
void event_corpseform_wearoff(P_char, P_char, P_obj, void *);
void check_saved_corpse(P_char);
void event_saved_corpse(P_char ch, P_char vict, P_obj obj, void *data);
void create_saved_corpse(P_obj obj, P_char mob);

#endif // __NECROMANCY_H__

