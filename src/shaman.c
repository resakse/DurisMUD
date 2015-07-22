/*
   ***************************************************************************
   *  File: shaman.c                                          Part of  Duris *
   *  Usage: routines for handling shamans                                   *
   *  Copyright  1990, 1991 - see 'license.doc' for complete information.    *
   *  Copyright 1994 - 2008 - Duris Systems Ltd.                             *
   ***************************************************************************
 */

#include <stdio.h>
#include <string.h>

#include "comm.h"
#include "db.h"
#include "events.h"
#include "interp.h"
#include "prototypes.h"
#include "spells.h"
#include "structs.h"
#include "utils.h"
#include "justice.h"
#include "mm.h"
#include "weather.h"
#include "objmisc.h"

extern P_char character_list;
extern P_desc descriptor_list;
extern P_index mob_index;
extern P_index obj_index;
extern P_room world;
extern const struct stat_data stat_factor[];
extern int spl_table[TOTALLVLS][MAX_CIRCLE];
extern struct zone_data *zone_table;
extern struct time_info_data time_info;

extern Skill skills[];
extern const char *spells[];

/*
   returns TRUE if object passed allows whoever to cast spell specified by spl
 */
bool checkTotem(P_char ch, P_obj obj, int skill)
{
  char     strn[MAX_STRING_LENGTH];
  int      sph, val0;

  if (!obj || (obj->type != ITEM_TOTEM) || (skill < 0) || !ch)
    return FALSE;

  if (IS_MULTICLASS_PC(ch))
  {
    return TRUE;
  }
  val0 = obj->value[0];

  if (IS_SET(skills[skill].targets, TAR_ANIMAL)) {
    if (get_spell_circle(ch, skill) >= 6)
      return (val0 & TOTEM_GR_ANIM);
    return (val0 & TOTEM_LESS_ANIM);
  }

  if (IS_SET(skills[skill].targets, TAR_ELEMENTAL)) {
    if (get_spell_circle(ch, skill) >= 6)
      return (val0 & TOTEM_GR_ELEM);
    return (val0 & TOTEM_LESS_ELEM);
  }

  if (IS_SET(skills[skill].targets, TAR_SPIRIT)) {
    if (get_spell_circle(ch, skill) >= 6)
      return (val0 & TOTEM_GR_SPIR);
    return (val0 & TOTEM_LESS_SPIR);
  }

  send_to_char("&+Werror: &nunrecognized sphere in checkTotem().  notify somebody.\n", ch);
  return FALSE;
}

/*
   check if character has right totem for spell
 */

bool hasTotem(P_char ch, int skill)
{
  if (!ch)
    return FALSE;
  if ((ch->equipment[WIELD] &&
       obj_index[ch->equipment[WIELD]->R_num].virtual_number == 25105) ||
      (ch->equipment[HOLD] &&
       obj_index[ch->equipment[HOLD]->R_num].virtual_number == 25105))
    return TRUE;
  if (checkTotem(ch, ch->equipment[WIELD], skill) || checkTotem(ch, ch->equipment[HOLD], skill) /*||
                                                                                                   checkTotem(ch, ch->equipment[WEAR_NECK_1], skill) ||
                                                                                                   checkTotem(ch, ch->equipment[WEAR_NECK_2], skill) */ )
    return TRUE;
  else
    return FALSE;
}
