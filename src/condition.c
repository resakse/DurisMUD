
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "comm.h"
#include "db.h"
#include "events.h"
#include "interp.h"
#include "justice.h"
#include "new_combat_def.h"
#include "objmisc.h"
#include "prototypes.h"
#include "spells.h"
#include "structs.h"
#include "utils.h"
#include "weather.h"
#include "damage.h"

extern P_char character_list;
extern P_desc descriptor_list;
extern P_index mob_index;
extern P_index obj_index;
extern P_obj object_list;
extern P_room world;
extern int proccing_slots[];
extern struct material_data materials[];

char *item_condition( P_obj obj )
{
  int         value;
  // I can't imagine anything longer than "&N&=LR\0" which is only 7 chars.
  char        bufCOLOR[8];
  // I can't imagine having a condition string longer than " [&N&=LR100%%&N]\0" which is only 16 chars.. 32 def safe.
  static char buf[32];

  if( obj->condition > 90 )
    snprintf(bufCOLOR, 8, "&+G");
  else if( obj->condition > 70 )
    snprintf(bufCOLOR, 8, "&+g");
  else if( obj->condition > 50 )
    snprintf(bufCOLOR, 8, "&+y");
  else if( obj->condition > 20 )
    snprintf(bufCOLOR, 8, "&+r");
  else if( obj->condition > 0 )
    snprintf(bufCOLOR, 8, "&+R");
  else
    snprintf(bufCOLOR, 8, "&=LR");

  // Print either the [20%] or spaces of equivalent length.
  if( obj->condition < 90 )
    snprintf(buf, 32, " [%s%2d&n%%]", bufCOLOR , obj->condition);
  else
    buf[0] = '\0';

  /* Default conditon is 100, and reflects average quality. This can be
   * raised, through _special_ and _expensive_ smiths, tailors, etc to a max
   * of 125, reflecting exceptional quality and manufacture. Code only
   * cares about 1-12, so lets chop the name down a bit...
   */

  /*  This isn't used atm. - We use the actual % now.
#ifndef NEW_COMBAT
  value = BOUNDED(1, obj->condition / 10, 12);
#else
  value = BOUNDED(1, (int) (((float) obj->curr_sp / (float) obj->max_sp) * 10.0), 12);
#endif

  // So these item types wont be shown with a condition.
  if( obj->type == ITEM_CORPSE )
    value = 12;

  switch (value)
  {
  case 1:
    snprintf(buf, MAX_STRING_LENGTH, " [ruined]         ");
    break;
  case 2:
    snprintf(buf, MAX_STRING_LENGTH, " [well worn]      ");
    break;
  case 3:
    snprintf(buf, MAX_STRING_LENGTH, " [worn]           ");
    break;
  case 4:
    snprintf(buf, MAX_STRING_LENGTH, " [partially worn] ");
    break;
  case 5:
    snprintf(buf, MAX_STRING_LENGTH, " [fair]           ");
    break;
  case 6:
    snprintf(buf, MAX_STRING_LENGTH, " [used]           ");
    break;
  case 7:
    snprintf(buf, MAX_STRING_LENGTH, " [good]           ");
    break;
  case 8:
    snprintf(buf, MAX_STRING_LENGTH, " [very good]      ");
    break;
  case 9:
    snprintf(buf, MAX_STRING_LENGTH, " [almost new]     ");
    break;
  default:
    snprintf(buf, MAX_STRING_LENGTH, "");
  }
  */

  return buf;
}

const char *item_damage_messages[][2] = {
  {"was damaged from the massive blow!",
   "was completely destroyed by the massive blow!"},
  {"is burned from the blast!",
   "melted from the intense heat!"},
  {"is weakened from the intense cold!",
   "freezes and shatters into million pieces from the intense cold!"},
  {"is filled with electrical energy!",
   "exploded spreading charges of electrical energy!"},
  {"was corroded by gas.",
   "crumbled as it was consumed by the toxic gas."},
  {"was corroded by acid.",
   "melted corroded by the acid."},
  {"is blasted by the negative energy!",
   "was disintegrated by the negative energy!"},
  {"is burned by the light!",
   "shattered into a million pieces by the divine fury!"},
  {"cracks as the tortured body writhes!",
   "shattered in bits as the tortured body spasms and quivers!"},
  {"is corrupted by spiritual anger!",
   "was destroyed by the blast!"},
  {"cracks attacked by the sound wave.",
   "shattered into a million pieces as the sound wave hit it!"},
  {"cracks from the bombardment of &+yearth&n.",
   "is smashed to bits by &+Lrocks&n and &+ydebris&n!"},

};

int DamageOneItem(P_char ch, int dam_type, P_obj obj, bool destroy)
{
  int      num;
  char     buf[MAX_STRING_LENGTH];
  int      objtype;
  bool     force_destroy = destroy;

  if( (objtype == ITEM_TOTEM) || (objtype == ITEM_KEY) || (objtype == ITEM_SPELLBOOK) || IS_ARTIFACT(obj) )
  {
    return 0;
  }

  num = number(3, 9);

  if(materials[obj->material].dam_res[dam_type])
  {
    num *= materials[obj->material].dam_res[dam_type];
  }

  // physical being the most common is less harsh
  if(dam_type == SPLDAM_GENERIC)
  {
    num >>= 1;
  }

  // Lets reduce the amount of damage a lance takes.
  if( objtype == ITEM_WEAPON && obj->value[0] == 16 )
  {
    num = number(1, 3);
  }

  obj->condition -= num;

  if( obj->condition < 0 )
  {
    destroy = TRUE;
  }

  objtype = GET_ITEM_TYPE(obj);

  snprintf(buf, MAX_STRING_LENGTH, "Your $q %s", item_damage_messages[dam_type - 1][destroy ? 1 : 0]);
  act(buf, TRUE, ch, obj, 0, TO_CHAR);

  snprintf(buf, MAX_STRING_LENGTH, "$n's $q %s", item_damage_messages[dam_type - 1][destroy ? 1 : 0]);
  act(buf, TRUE, ch, obj, 0, TO_ROOM);

  if(destroy)
    if(!force_destroy)
      MakeScrap(ch, obj);
    else
      extract_obj(obj);

  return destroy;
}


void MakeScrap(P_char ch, P_obj obj)
{
  char     buf[MAX_STRING_LENGTH];
  P_obj    t, x;
  int      pos;


  if (!ch || !obj || (ch->in_room == NOWHERE))
    return;

  act("$p falls to the ground in scraps.", TRUE, ch, obj, 0, TO_CHAR);
  act("$p falls to the ground in scraps.", TRUE, ch, obj, 0, TO_ROOM);

  t = read_object(9, VIRTUAL);

  if (!t)
    return;

  snprintf(buf, MAX_STRING_LENGTH, "Scraps from %s&n lie in a pile here.",
          obj->short_description);

  t->description = str_dup(buf);

  snprintf(buf, MAX_STRING_LENGTH, "a pile of scraps from %s", obj->short_description);

  t->short_description = str_dup(buf);

  snprintf(buf, MAX_STRING_LENGTH, "%s scraps pile", obj->name);

  t->name = str_dup(buf);

  t->str_mask = STRUNG_DESC1 | STRUNG_DESC2 | STRUNG_KEYS;

  if (OBJ_CARRIED(obj))
  {
    obj_from_char(obj);
  }
  else if (OBJ_WORN(obj))
  {
    for (pos = 0; pos < MAX_WEAR; pos++)
      if (ch->equipment[pos] == obj)
        break;
    if (pos >= MAX_WEAR)
    {
      logit(LOG_DEBUG, "MakeScrap(), can't find worn object in equip");
      raise(SIGSEGV);
    }
    obj = unequip_char(ch, pos);
  }

  set_obj_affected(t, 400, TAG_OBJ_DECAY, 0);
  obj_to_room(t, ch->in_room);

  while (obj->contains)
  {
    x = obj->contains;
    obj_from_obj(x);
    obj_to_room(x, ch->in_room);
  }

  if (OBJ_FALLING(t))
    falling_obj(t, 1, false);

  extract_obj(obj);
}

void DamageAllStuff(P_char ch, int dam_type)
{
  int      j;
  P_obj    obj, next;

  /* this procedure takes all of the items in equipment and inventory
     and damages the ones that should be damaged */

  /* equipment */

  for (j = 0; j < MAX_WEAR; j++)
  {
    if (ch->equipment[j] && (j != WIELD) && (j != WIELD2))
    {
      obj = ch->equipment[j];
      DamageOneItem(ch, dam_type, obj, FALSE);
    }
  }

  /* inventory */
  for (obj = ch->carrying; obj; obj = next)
  {
    next = obj->next_content;
    DamageOneItem(ch, dam_type, obj, FALSE);
  }
}


void DamageStuff(P_char v, int type)
{
  int      slot;
  P_obj    obj;

  slot = number(2, CUR_MAX_WEAR);
  if (v->equipment[slot] && slot != WIELD && slot != WIELD2)
  {
    DamageOneItem(v, type, v->equipment[slot], FALSE);
  }
}
