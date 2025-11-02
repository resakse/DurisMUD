#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>

#include "prototypes.h"
#include "structs.h"
#include "utils.h"
#include "config.h"
#include "db.h"
#include "spells.h"
#include "map.h"
#include "justice.h"

extern P_char char_in_room(int);
extern P_char character_list;
extern P_desc descriptor_list;
extern P_index mob_index;
extern P_index obj_index;
extern P_room world;
extern const char *dirs[];
extern const char rev_dir[];
extern int top_of_zone_table;

extern struct zone_data *zone;
extern struct zone_data *zone_table;
extern int LOADED_RANDOM_ZONES;
extern struct time_info_data time_info;

void random_gems(P_obj gem, int base_value);
void random_jewelry(P_obj obj, int base_value);

/* Called whenever, occasionally loads encounters based on ch passed */
void random_encounters(P_char ch)
{
  int      mobs_vnum = 0, num_to_load = 0, num_loaded = 0, done = 0, total = 0;
  int      chance = 0;
  P_char   mob, tch;
  struct group_list *gl;

  /* only activates in map world */
  if( !IS_MAP_ROOM(ch->in_room) )
    return;

  /* nothing in water rooms yet */
  if( IS_WATER_ROOM(ch->in_room) )
    return;

  /* nor fire, lava, mithril */
  if( world[ch->in_room].sector_type == SECT_FIREPLANE ||
      world[ch->in_room].sector_type == SECT_LAVA ||
      world[ch->in_room].sector_type == SECT_UNDRWLD_LIQMITH ||
      world[ch->in_room].sector_type == SECT_UNDRWLD_SLIME)
    return;

  /* Don't overload the room */
  for( tch = world[ch->in_room].people; tch; tch = tch->next_in_room )
    if( ++total >= 30 )
      return;

  if( number(0, 8) )
    return;

  /* if its a group, only load for the leader.
    * this way, we dont load kobolds and orcs in the same room
    */
  if (ch->group && (ch->group->ch != ch) &&
      (ch->group->ch->in_room == ch->in_room))
    return;
  
  if (world[ch->in_room].sector_type == SECT_UNDRWLD_WILD ||
      world[ch->in_room].sector_type == SECT_UNDRWLD_CITY ||
      world[ch->in_room].sector_type == SECT_UNDRWLD_INSIDE ||
      world[ch->in_room].sector_type == SECT_UNDRWLD_MOUNTAIN ||
      world[ch->in_room].sector_type == SECT_UNDRWLD_MUSHROOM)
    mobs_vnum = number(300, 330);
  else
    mobs_vnum = number(1, 168);
  
  mobs_vnum += 100000;
  
  if (!(mob = read_mobile(mobs_vnum, VIRTUAL)))
  {
    logit(LOG_MOB, "Random Encounters, mob #%d not in file!", mobs_vnum);
    return;
  }
  /* no aggros in justice */
  if (CHAR_IN_JUSTICE_AREA(ch) && IS_AGGRESSIVE(mob))
  {
    extract_char(mob);
    mob = NULL;
    return;
  }
  /* How many mobs of this type do we need? */
  if (IS_HUMANOID(mob))
    num_to_load = BOUNDED(1, (int) (GET_LEVEL(ch) / GET_LEVEL(mob)), 3);
  
  /*
   if (ch->group) {
     gl = ch->group;
     for (gl = gl->next; gl; gl = gl->next)
       num_to_load += ((GET_LEVEL(ch) * 0.75) / GET_LEVEL(mob));
   }
   */
  num_to_load = BOUNDED(1, num_to_load, 5);     /* arbitrary, just for now */
  
  /* ok random chance to load nasty shit */
  if (world[ch->in_room].sector_type == SECT_UNDRWLD_WILD ||
      world[ch->in_room].sector_type == SECT_UNDRWLD_CITY ||
      world[ch->in_room].sector_type == SECT_UNDRWLD_INSIDE ||
      world[ch->in_room].sector_type == SECT_UNDRWLD_MOUNTAIN ||
      world[ch->in_room].sector_type == SECT_UNDRWLD_MUSHROOM)
  {
    chance = (number(1, 425));
    switch (chance)
    {
      case 1:
        mobs_vnum = 4510;
        break;
      case 2:
        mobs_vnum = 4520;
        break;
        /*
         case 3:
           mobs_vnum = 4480;
           break;
           */
      default:
        mobs_vnum = 0;
        break;
    }
    if (mobs_vnum != 0)
    {
      extract_char(mob);
      mob = NULL;
      if (!(mob = read_mobile(mobs_vnum, VIRTUAL)))
      {
        logit(LOG_MOB, "Random Encounters, mob #%d not in file!", mobs_vnum);
        return;
      }
      char_to_room(mob, ch->in_room, -2);
      if (IS_SET(mob->specials.act, ACT_CANFLY) && ch->specials.z_cord > 0)
        mob->specials.z_cord = ch->specials.z_cord;
      else if (IS_SET(mob->specials.act, ACT_CANSWIM) &&
               ch->specials.z_cord < 0)
        mob->specials.z_cord = ch->specials.z_cord;
      else
        mob->specials.z_cord = 0;
      return;
    }
  }
  
  /* ok random chance to load nasty shit */
  if (ch->in_room >= real_room(140000) && ch->in_room <= real_room(149999))
  {
    chance = (number(1, 450));
    switch (chance)
    {
      case 1:
        mobs_vnum = 550;
        break;
      case 2:
        mobs_vnum = 551;
        break;
      default:
        mobs_vnum = 0;
        break;
    }
    if (mobs_vnum != 0)
    {
      extract_char(mob);
      mob = NULL;
      if (!(mob = read_mobile(mobs_vnum, VIRTUAL)))
      {
        logit(LOG_MOB, "Random Encounters, mob #%d not in file!", mobs_vnum);
        return;
      }
      char_to_room(mob, ch->in_room, -2);
      if (IS_SET(mob->specials.act, ACT_CANFLY) && ch->specials.z_cord > 0)
        mob->specials.z_cord = ch->specials.z_cord;
      else if (IS_SET(mob->specials.act, ACT_CANSWIM) &&
               ch->specials.z_cord < 0)
        mob->specials.z_cord = ch->specials.z_cord;
      else
        mob->specials.z_cord = 0;
      return;
    }
  }
  
  do
  {
    if (num_loaded)
      mob = read_mobile(mobs_vnum, VIRTUAL);
    
    if (!mob)
    {
      done = TRUE;
      continue;
    }
    /* equip them */
    if (IS_HUMANOID(mob))
      randobjs_to_mob(mob);
    
    /* ok Evil continent is harder */
    if (ch->in_room >= real_room(140000) && ch->in_room <= real_room(149999))
    {
      //       GET_LEVEL(mob) += 10;
      mob->player.level += 10;
      //       SET_BIT(mob->specials.act,ACT_AGGRESSIVE);
      SET_BIT(mob->only.npc->aggro_flags, AGGR_ALL);
    }
    
    /* load them */
    char_to_room(mob, ch->in_room, -2);
    if (IS_SET(mob->specials.act, ACT_CANFLY) && ch->specials.z_cord > 0)
      mob->specials.z_cord = ch->specials.z_cord;
    else if (IS_SET(mob->specials.act, ACT_CANSWIM) &&
             ch->specials.z_cord < 0)
      mob->specials.z_cord = ch->specials.z_cord;
    else
      mob->specials.z_cord = 0;
    
    /* count them */
    num_loaded++;
    
    if (num_to_load <= num_loaded)
      done = TRUE;
    
  }
  while (!done);
}

/* Here we semi randomly pick a type of item based on mob class and level */
int randobjs_to_mob(P_char mob)
{
  int      i, x;
  ulong    which = 0;
  P_obj    obj = NULL;
  
  i = number(5, (MAX(5, GET_LEVEL(mob) / 2)));
  for (x = 1; x <= i; x++)
  {
    switch (x)
    {
      case 1:
        if (!number(0, 1))
          which = (ITEM_WEAR_BODY);
        else
          which = (ITEM_WEAR_ABOUT);
        break;
      case 2:
        if (IS_WARRIOR(mob))
          which = (ITEM_WIELD);
        else
          which = (ITEM_HOLD);
        break;
      case 3:
        which = (ITEM_WEAR_FEET);
        break;
      case 4:
        which = (ITEM_HOLD);
        break;
      case 5:
        which = (ITEM_WEAR_LEGS);
        break;
      case 6:
        which = (ITEM_WEAR_ARMS);
        break;
      case 7:
        which = (ITEM_WEAR_WAIST);
        break;
      case 8:
        which = (ITEM_WEAR_EARRING);
        break;
      case 9:
        if (GET_CLASS(mob, CLASS_WARRIOR))
          which = (ITEM_WEAR_SHIELD);
        else
          which = (ITEM_HOLD);
        break;
      case 10:
        which = (ITEM_WEAR_NECK);
        break;
      case 11:
        which = (ITEM_WEAR_WRIST);
        break;
      case 12:
        which = (ITEM_WEAR_FINGER);
        break;
      case 13:
        which = (ITEM_WEAR_FINGER);
        break;
      case 14:
        which = (ITEM_WIELD);
        break;
      default:
        which = (ITEM_HOLD);
        break;
    }
    obj = ran_obj(mob, which);
    if (obj)
    {
      obj_to_char(obj, mob);
      CheckEqWorthUsing(mob, obj);
    }
  }
  return TRUE;
}

char    *strip_color(const char *string)
{
  static char colorless[MAX_STRING_LENGTH];
  int      x;
  
  if (string[0] != '&')
  {
    strcpy(colorless, string);
    return colorless;
  }
  
  for (x = 0; x <= strlen(string); x++)
    colorless[x] = string[x + 3];
  
  return colorless;
}

/* here we generate some minor potion, treasure, scroll, bag */

P_obj ran_magical(P_char mob)
{
  P_obj    obj = NULL;
  int      temp = 0;
  char     buf[MAX_STRING_LENGTH];
  
  const char *lights[] = {
    "&+ytorch", "&+Llantern", "&+Wstrange fungus", "&+Wglowing crystal"
  };
  int      nlights = 3;
  const char *scrolls[] = {
    "tattered", "small", "&+Ldusty", "vellum", "&+yparchment",
    "frail", "&+Wglowing"
  };
  int      nscrolls = 6;
  const char *potions[] = {
    "&+Ldark", "light", "&+ggreen", "&+Yyellow", "&+Bbubbly", "&+Lsmoking",
    "clear", "cloudy", "&+mpurple"
  };
  int      npotions = 8;
  const char *bags[] = {
    "&+yleather", "&+ybrown", "&+Lblack", "&+gsnakeskin", "large",
    "&+Wruffled", "&+mvelvet", "torn", "&+Rdragonscale", "&+yfurred",
    "silverish"
  };
  int      nbags = 10;
  const char *drinks[] = {
    "&+ybottle", "flagon", "jar", "&+Yvase", "&+Lwineskin",
    "&+Wdrinking horn", "&+ymug"
  };
  int      ndrinks = 6;
  const char *foods[] = {
    "pastry", "&+yloaf of bread", "&+Ltravel ration",
    "&+Rleg of beef", "&+Ljuicy steak", "&+gpear", "&+Rcarrot",
    "&+ysuspicious-looking mushroom",
    "&+Rapple", "&+Lwyvern jerky", "&+Ldragon-gut", "&+ypiece of bread"
  };
  int      nfoods = 11;
  
  buf[0] = '\0';
  if (!(obj = read_object(1295, VIRTUAL)))      /* standard held object */
    return NULL;
  obj->str_mask = (STRUNG_KEYS | STRUNG_DESC1 | STRUNG_DESC2);
  switch (number(0, 19))
  {                             /* weight this chance to taste */
    case 0:                      /* light */
      temp = number(0, nlights);
      snprintf(buf, MAX_STRING_LENGTH, "a %s&n", lights[temp]);
      obj->short_description = str_dup(buf);
      snprintf(buf, MAX_STRING_LENGTH, "A %s &nlies here.", lights[temp]);
      obj->description = str_dup(buf);
      snprintf(buf, MAX_STRING_LENGTH, "%s RANOBJ", strip_color(lights[temp]));
      obj->name = str_dup(buf);
      obj->type = ITEM_LIGHT;
      obj->value[2] = 8;          /* 8 hour burn time */
      /* have to set value flags for item cat specifics */
      break;
    case 1:                      /* scroll */
      temp = number(0, nscrolls);
      snprintf(buf, MAX_STRING_LENGTH, "a %s scroll", scrolls[temp]);
      obj->short_description = str_dup(buf);
      snprintf(buf, MAX_STRING_LENGTH, "A %s scroll &nlies here.", scrolls[temp]);
      obj->description = str_dup(buf);
      snprintf(buf, MAX_STRING_LENGTH, "scroll RANOBJ");
      obj->name = str_dup(buf);
      obj->type = ITEM_SCROLL;
      obj->value[0] = GET_LEVEL(mob);
      obj->value[1] = number(1, 44);      /* NEED SPELL LIST */
      break;
    case 2:                      /* treasure */
      random_gems(obj, number(1, 100));
      break;
    case 3:                      /* potion */
      temp = number(0, npotions);
      snprintf(buf, MAX_STRING_LENGTH, "a %s potion", potions[temp]);
      obj->short_description = str_dup(buf);
      snprintf(buf, MAX_STRING_LENGTH, "A %s potion &nlies here.", potions[temp]);
      obj->description = str_dup(buf);
      strcat(buf, "potion RANOBJ");
      obj->name = str_dup(buf);
      obj->type = ITEM_POTION;
      obj->value[0] = GET_LEVEL(mob);
      obj->value[1] = number(1, 44);      /* NEED SPELL LIST */
      break;
    case 4:                      /* container */
    case 5:
    case 6:
      temp = number(0, nbags);
      snprintf(buf, MAX_STRING_LENGTH, "a %s bag", bags[temp]);
      obj->short_description = str_dup(buf);
      snprintf(buf, MAX_STRING_LENGTH, "A %s bag &nlies here.", bags[temp]);
      obj->description = str_dup(buf);
      snprintf(buf, MAX_STRING_LENGTH, "bag %s RANOBJ", strip_color(bags[temp]));
      obj->name = str_dup(buf);
      obj->type = ITEM_CONTAINER;
      obj->value[1] = 1;
      obj->value[2] = -1;
      obj->value[0] = number(1, GET_LEVEL(mob) / 2);
      break;
    case 7:                      /* drinkcon */
    case 8:
    case 9:
      temp = number(0, ndrinks);
      snprintf(buf, MAX_STRING_LENGTH, "a %s&n", drinks[temp]);
      obj->short_description = str_dup(buf);
      snprintf(buf, MAX_STRING_LENGTH, "A %s &nlies here.", drinks[temp]);
      obj->description = str_dup(buf);
      snprintf(buf, MAX_STRING_LENGTH, "%s RANOBJ", strip_color(drinks[temp]));
      obj->name = str_dup(buf);
      obj->type = ITEM_DRINKCON;
      obj->value[0] = 2;
      obj->value[1] = 2;
      obj->value[2] = number(1, 20);
      break;
    case 10:                     /* food */
    case 11:
    case 12:
    case 13:
      temp = number(0, nfoods);
      snprintf(buf, MAX_STRING_LENGTH, "a %s&n", foods[temp]);
      obj->short_description = str_dup(buf);
      snprintf(buf, MAX_STRING_LENGTH, "A %s &nlies here.", foods[temp]);
      obj->description = str_dup(buf);
      snprintf(buf, MAX_STRING_LENGTH, "%s RANOBJ", strip_color(foods[temp]));
      obj->name = str_dup(buf);
      obj->type = ITEM_FOOD;
      obj->value[0] = number(1, 5);
      break;
    case 14:
      random_jewelry(obj, number(1, 100));
      break;
    default:                     /* money */
      snprintf(buf, MAX_STRING_LENGTH, "a pile of coins");
      obj->short_description = str_dup(buf);
      snprintf(buf, MAX_STRING_LENGTH, "A small pile of coins &nlie here.");
      obj->description = str_dup(buf);
      snprintf(buf, MAX_STRING_LENGTH, "pile coins RANOBJ");
      obj->name = str_dup(buf);
      obj->type = ITEM_MONEY;
      obj->value[3] = number(0, GET_LEVEL(mob) / 2);
      obj->value[2] = number(1, GET_LEVEL(mob));
      break;
  }

return obj;
}

/* returns a random object, given a type, and a mob */

P_obj ran_obj(P_char mob, ulong which)
{
  P_obj    obj = NULL;
  char     buf[MAX_STRING_LENGTH];
  int      temp = 0, temp2 = 0, temp3 = 0;
  
  const char *earrings[] = {
    "&+Wdiamond", "&+gemerald", "&+ytopaz", "&+ywooden", "&+gjade",
    "&+Wwhite gold", "&+Lonyx", "tin", "glass", "&+Wmarble", "&+Lblack",
    "&+Lgranite"
  };
  int      nearrings = 11;
  const char *rings[] = {
    "tarnished", "jeweled", "silver", "&+Ygolden",
    "&+Wshiny", "dull", "sparkling", "attractive", "&+Lblack", "glass"
  };
  int      nrings = 9;
  const char *generics[] = {
    /* These MUST have blank at end */
    "", "", "", "", "",         /* not all needs describing :) */
    "splendid ", "ancient ", "dusty ", "scratched ", "antique ", "old ",
    "flawed ", "sooty ", "plain ", "ornate ", "dull ", "shiny ",
    "odd-looking ", "tarnished ", "unusual ", "spiffy "
  };
  int      ngenerics = 20;
  const char *necks[] = {
    "necklace", "collar", "cape", "amulet", "pendant"
  };
  int      nnecks = 4;
  const char *bodys[] = {
    "breastplate", "coat", "jacket", "chain shirt", "splint mail"
  };
  int      nbodys = 4;
  const char *heads[] = {
    "hat", "headdress", "cap", "helm", "coif", "derby"
  };
  int      nheads = 5;
  const char *feets[] = {
    "sandals", "boots", "shoes", "riding boots", "leathers"
  };
  int      nfeets = 4;
  const char *shields[] = {
    "dented", "round", "oval", "center-grip", "heater",
    "door", "shiny", "embossed", "coffin-shaped", "curved"
  };
  int      nshields = 9;
  const char *weps[] = {
    "axe", "sword", "mace", "club", "bardiche", "pike",
    "whip", "maul", "bone", "staff", "dagger", "spear"
  };
  int      nweps = 11;
  const char *color[] =         /* better colors are duped often */
  {
    "&+L", "&+g", "&+y", "&+m", "&+b", "&+c", "&+w", "&+r",
    "&+G", "&+Y", "&+M", "&+B", "&+C", "&+W", "&+R", "&+L",
    "&+L", "&+L", "&+y", "&+y", "&+y", "&+y", "&+W", "&+c"
  };
  int      ncolors = 23;
  
  
  /* What we'll do is load Standard God Item <blah> then String it,
    and stat is as neccessary. */
  buf[0] = '\0';
  temp2 = number(0, ngenerics);
  temp3 = number(0, ncolors);
  
  switch (which)
  {
    case ITEM_WEAR_FINGER:       /* 1293 */
      temp = number(0, nrings);
      if (!(obj = read_object(1293, VIRTUAL)))
        return NULL;
        obj->str_mask = (STRUNG_KEYS | STRUNG_DESC1 | STRUNG_DESC2);
      snprintf(buf, MAX_STRING_LENGTH, "a %s ring", rings[temp]);
      obj->short_description = str_dup(buf);
      snprintf(buf, MAX_STRING_LENGTH, "A %s ring &nlies here.", rings[temp]);
      obj->description = str_dup(buf);
      obj->name = str_dup("ring RANOBJ");
      /* Now that we have name setup, set stats */
      if ((GET_LEVEL(mob) > 10) && (GET_LEVEL(mob) < 25))
      {
        /*      obj->affected[0].location = APPLY_ARMOR;
        obj->affected[0].modifier = number(1, 5);*/
      }
        else if (GET_LEVEL(mob) > 24)
        {
          /*      obj->affected[0].location = APPLY_ARMOR;
          obj->affected[0].modifier = number(2, 6);*/
          obj->affected[1].location = number(1, 30);        /*some minor ability */
          if ((obj->affected[1].location == 7) || (obj->affected[1].location == 8) ||       /* can't use these */
            (obj->affected[1].location == 6) ||
            (obj->affected[1].location == 15) ||
            (obj->affected[1].location == 16))
obj->affected[1].location = APPLY_HIT;
obj->affected[1].modifier = number(1, 5);
        };
break;
case ITEM_WEAR_NECK:         /* 1285 */
temp = number(0, nnecks);
if (!(obj = read_object(1285, VIRTUAL)))
return NULL;
obj->str_mask = (STRUNG_KEYS | STRUNG_DESC1 | STRUNG_DESC2);
snprintf(buf, MAX_STRING_LENGTH, "a %s%s%s&n", color[temp3], generics[temp2], necks[temp]);
obj->short_description = str_dup(buf);
snprintf(buf, MAX_STRING_LENGTH, "A %s%s%s &nlies here.", color[temp3], generics[temp2],
        necks[temp]);
obj->description = str_dup(buf);
snprintf(buf, MAX_STRING_LENGTH, "%s RANOBJ", strip_color(necks[temp]));
obj->name = str_dup(buf);
break;
case ITEM_WEAR_BODY:         /* 1286 */
temp = number(0, nbodys);
if (!(obj = read_object(1286, VIRTUAL)))
return NULL;
obj->str_mask = (STRUNG_KEYS | STRUNG_DESC1 | STRUNG_DESC2);
snprintf(buf, MAX_STRING_LENGTH, "a %s%s%s&n", color[temp3], generics[number(0, nbodys)],
        bodys[temp]);
obj->short_description = str_dup(buf);
snprintf(buf, MAX_STRING_LENGTH, "A %s%s%s &nlies here.", color[temp3], generics[temp2],
        bodys[temp]);
obj->description = str_dup(buf);
snprintf(buf, MAX_STRING_LENGTH, "%s RANOBJ", strip_color(bodys[temp]));
obj->name = str_dup(buf);
/* Now that we have name setup, set stats */
/*    obj->affected[0].location = APPLY_ARMOR;
obj->affected[0].modifier = 0 - (GET_LEVEL(mob) / 2 + number(0, 5) - number(0, 5));*/
break;
case ITEM_WEAR_HEAD:         /* 1281 */
temp = number(0, nheads);
if (!(obj = read_object(1281, VIRTUAL)))
return NULL;
obj->str_mask = (STRUNG_KEYS | STRUNG_DESC1 | STRUNG_DESC2);
snprintf(buf, MAX_STRING_LENGTH, "a %s%s%s&n", color[temp3], generics[temp2], heads[temp]);
obj->short_description = str_dup(buf);
snprintf(buf, MAX_STRING_LENGTH, "A %s%s%s &nlies here.", color[temp3], generics[temp2],
        heads[temp]);
obj->description = str_dup(buf);
snprintf(buf, MAX_STRING_LENGTH, "%s RANOBJ", strip_color(heads[temp]));
obj->name = str_dup(buf);
/* Now that we have name setup, set stats */
/*    obj->affected[0].location = APPLY_ARMOR;
obj->affected[0].modifier = 0 - number(1, 5);*/
break;
case ITEM_WEAR_LEGS:         /* 1296 */
temp = number(0, ngenerics);
if (!(obj = read_object(1296, VIRTUAL)))
return NULL;
obj->str_mask = (STRUNG_KEYS | STRUNG_DESC1 | STRUNG_DESC2);
snprintf(buf, MAX_STRING_LENGTH, "some %s%sleggings&n", color[temp3], generics[temp]);
obj->short_description = str_dup(buf);
snprintf(buf, MAX_STRING_LENGTH, "Some %s%sleggings &nlie here.", color[temp3],
        generics[temp]);
obj->description = str_dup(buf);
obj->name = str_dup("leggings RANOBJ");
/* Now that we have name setup, set stats */
/*    obj->affected[0].location = APPLY_ARMOR;
obj->affected[0].modifier = 0 - number(1, 8);*/
break;
case ITEM_WEAR_FEET:         /* 1297 */
temp = number(0, nfeets);
if (!(obj = read_object(1297, VIRTUAL)))
return NULL;
obj->str_mask = (STRUNG_KEYS | STRUNG_DESC1 | STRUNG_DESC2);
sprintf(buf, "a pair of %s%s%s&n", color[temp3], generics[temp2],
        feets[temp]);
obj->short_description = str_dup(buf);
sprintf(buf, "A pair of %s%s%s &nlies here.", color[temp3],
        generics[temp2], feets[temp]);
obj->description = str_dup(buf);
sprintf(buf, "%s RANOBJ", strip_color(feets[temp]));
obj->name = str_dup(buf);
/* Now that we have name setup, set stats */
/*    obj->affected[0].location = APPLY_ARMOR;
obj->affected[0].modifier = 0 - ((GET_LEVEL(mob) / 5) + number(1, 5));*/
break;
case ITEM_WEAR_ARMS:         /* 1289 */
temp = number(0, ngenerics);
if (!(obj = read_object(1289, VIRTUAL)))
return NULL;
obj->str_mask = (STRUNG_KEYS | STRUNG_DESC1 | STRUNG_DESC2);
sprintf(buf, "a pair of %s%ssleeves&n", color[temp3], generics[temp]);
obj->short_description = str_dup(buf);
sprintf(buf, "A pair of %s%ssleeves &nlie here.", color[temp3],
        generics[temp]);
obj->description = str_dup(buf);
obj->name = str_dup("sleeves RANOBJ");
/* Now that we have name setup, set stats */
/*    obj->affected[0].location = APPLY_ARMOR;
obj->affected[0].modifier = 0 - ((GET_LEVEL(mob) / 5) + number(1, 5));*/
break;
case ITEM_WEAR_SHIELD:       /* 1290 */
temp = number(0, nshields);
if (!(obj = read_object(1290, VIRTUAL)))
return NULL;
obj->str_mask = (STRUNG_KEYS | STRUNG_DESC1 | STRUNG_DESC2);
sprintf(buf, "a %s shield", shields[temp]);
obj->short_description = str_dup(buf);
sprintf(buf, "A %s shield &nlies here.", shields[temp]);
obj->description = str_dup(buf);
obj->name = str_dup("shield RANOBJ");
/* Now that we have name setup, set stats */
/*    obj->affected[0].location = APPLY_ARMOR;
obj->affected[0].modifier = 0 - ((GET_LEVEL(mob) / 4) + number(1, 5));*/
break;
case ITEM_WEAR_ABOUT:        /* 1287 */
temp = number(0, ngenerics);
if (!(obj = read_object(1287, VIRTUAL)))
return NULL;
obj->str_mask = (STRUNG_KEYS | STRUNG_DESC1 | STRUNG_DESC2);
sprintf(buf, "a %s%scloak&n", color[temp3], generics[temp]);
obj->short_description = str_dup(buf);
sprintf(buf, "A %s%scloak &nlies here.", color[temp3], generics[temp]);
obj->description = str_dup(buf);
obj->name = str_dup("cloak RANOBJ");
/* Now that we have name setup, set stats */
/*    obj->affected[0].location = APPLY_ARMOR;
obj->affected[0].modifier = 0 - (GET_LEVEL(mob) / 4);*/
obj->affected[1].location = number(1, 30);  /*some minor ability */
if ((obj->affected[1].location == 7) || (obj->affected[1].location == 8) || /* can't use these */
(obj->affected[1].location == 6) ||
(obj->affected[1].location == 15) ||
(obj->affected[1].location == 16))
obj->affected[1].location = APPLY_HIT;
obj->affected[1].modifier = number(1, 5);
break;
case ITEM_WEAR_WAIST:       /* 1288 */
temp = number(0, ngenerics);
if (!(obj = read_object(1288, VIRTUAL)))
return NULL;
obj->str_mask = (STRUNG_KEYS | STRUNG_DESC1 | STRUNG_DESC2);
sprintf(buf, "a %s%sbelt&n", color[temp3], generics[temp]);
obj->short_description = str_dup(buf);
sprintf(buf, "A %s%sbelt &nlies here.", color[temp3], generics[temp]);
obj->description = str_dup(buf);
obj->name = str_dup("belt RANOBJ");
break;
case ITEM_WEAR_WRIST:        /* 1291 */
temp = number(0, ngenerics);
if (!(obj = read_object(1291, VIRTUAL)))
return NULL;
obj->str_mask = (STRUNG_KEYS | STRUNG_DESC1 | STRUNG_DESC2);
sprintf(buf, "a %s%sbracelet&n", color[temp3], generics[temp]);
obj->short_description = str_dup(buf);
sprintf(buf, "A %s%sbracelet &nlies here.", color[temp3], generics[temp]);
obj->description = str_dup(buf);
obj->name = str_dup("bracelet RANOBJ");
/* Now that we have name setup, set stats */
obj->affected[1].location = number(1, 30);  /* some minor ability */
if ((obj->affected[1].location == 7) || (obj->affected[1].location == 8) || /* can't use these */
(obj->affected[1].location == 6) ||
(obj->affected[1].location == 15) ||
(obj->affected[1].location == 16))
obj->affected[1].location = APPLY_HIT;
obj->affected[1].modifier = number(1, 5);
break;
case ITEM_WEAR_HANDS:        /* 1292 */
temp = number(0, ngenerics);
if (!(obj = read_object(1292, VIRTUAL)))
return NULL;
obj->str_mask = (STRUNG_KEYS | STRUNG_DESC1 | STRUNG_DESC2);
sprintf(buf, "some %s%sgloves&n", color[temp3], generics[temp]);
obj->short_description = str_dup(buf);
sprintf(buf, "Some %s%sgloves &nlie here.", color[temp3], generics[temp]);
obj->description = str_dup(buf);
obj->name = str_dup("gloves RANOBJ");
/* Now that we have name setup, set stats */
/*    obj->affected[0].location = APPLY_ARMOR;
obj->affected[0].modifier = 0 - number(1, 5);*/
break;
case ITEM_WIELD:             /* 1294 */
temp = number(0, nweps);
if (!(obj = read_object(1294, VIRTUAL)))
return NULL;
obj->str_mask = (STRUNG_KEYS | STRUNG_DESC1 | STRUNG_DESC2);
sprintf(buf, "a %s%s&n", generics[temp2], weps[temp]);
obj->short_description = str_dup(buf);
sprintf(buf, "A %s%s &nlies here.", generics[temp2], weps[temp]);
obj->description = str_dup(buf);
sprintf(buf, "%s RANOBJ", strip_color(weps[temp]));
obj->name = str_dup(buf);
/* Now that we have name setup, set stats */
obj->value[1] = number(1, 2);
obj->value[2] = number(2, 6);
obj->value[3] = 4;          /* all slashing, sorry.  */
if (GET_LEVEL(mob) > 10)
{
  obj->affected[0].location = APPLY_HIT;
  obj->affected[0].modifier = number(1, 2);
}
break;
case ITEM_HOLD:              /* 1295 Some minor magical item or pouch */
obj = ran_magical(mob);
break;
case ITEM_THROW:             /* 1298 */
/* ?? */
break;
case ITEM_WEAR_EARRING:
  temp = number(0, nearrings);
  if (!(obj = read_object(1283, VIRTUAL)))
    return NULL;
    obj->str_mask = (STRUNG_KEYS | STRUNG_DESC1 | STRUNG_DESC2);
  sprintf(buf, "a %s earring", earrings[temp]);
  obj->short_description = str_dup(buf);
  sprintf(buf, "A %s earring &nlies here.", earrings[temp]);
  obj->description = str_dup(buf);
  obj->name = str_dup("earring RANOBJ");
  break;
  }
if (!obj)
return NULL;

/* Will have to make these items mortal usable, and remove certain
flags that are same across all objects. */
obj->extra_flags = obj->extra_flags & !ITEM_NOSELL
& !ITEM_NOLOCATE & !ITEM_NOIDENTIFY & !ITEM_TRANSIENT;

convertObj(obj);
return (obj);
}

/* Give these next two a clean object, a basic value of 1-14, and out
pops a nice little item! */

void random_gems(P_obj gem, int base_value)
{
  float    bv;
  int      bv_die, orig_base_value_place, current_base_value_place;
  int      ignore_results_below_2 = 0, inc_or_dec_die, prop;
  int      get_out = FALSE;
  char     buf[MAX_STRING_LENGTH];
  static int bv_table[13][2] = {
  {25, 10},
  {50, 50},
  {70, 100},
  {90, 500},
  {99, 1000},
  {100, 5000},
  {0, 10000},
  {0, 25000},
  {0, 50000},
  {0, 100000},
  {0, 250000},
  {0, 500000},
  {0, 1000000}
  };
  static float decreased_bv_table[6] = { 5.0, 1.0, 0.5, 0.25, 0.05 };
  const char *properties[6][14] = {
  {                           /* Ornamental Stones, base value 1 GP */
    "&+bazurite&n",
    "&+yba&n&+Bn&n&+yded a&N&+Rg&N&+yate&n",
    "&+Bblue quartz&n",
    "&+Weye a&n&+Lg&n&+Wate&n",
    "&+Lhematite&n",
    "&+bla&+Bp&n&+bis la&n&+Bz&n&+buli&n",
    "&+gm&+Ga&n&+gl&+Ga&n&+gc&+Gh&n&+gi&+Gt&N&+ge&n",
    "&+Wmo&n&+gs&+Ws ag&n&+ga&+Wte&n",
    "&+Lobsidian&n",
    "&+Rrhodochrosite&n",
    "&+ytiger &+Yeye&n",
    "&+Bturquiose&n",
    "",
    ""},
    
  {                           /* Semi-precious Stones, base value 5 GP */
    "&+Yamber&n",
    "&+Rcarnelian&n",
    "&+Wchalcedony&n",
    "&+gchrysoprase&n",
    "&+ycitrine&n",
    "&+bjasper&n",
    "&+Wmoon&n&+Bstone&n",
    "&+Lonyx&n",
    "rock crystal",
    "&+Rs&n&+War&n&+Rdo&n&+Wny&n&+Rx&n",
    "smoky quartz",
    "&+Rstar &n&+Wrose&n&+R quartz&n",
    "&+Bzircon&n",
    ""},

{                           /* Fancy Stones, base value 10 GP */
"&+yamber&n",
"&+Yalexandrite&n",
"&+mamethyst&n",
"",
"&+gchrysoberyl&n",
"&+Rcoral&n",
"&+rgarnet&n",
"&+gjade&n",
"&+Ljet&n",
"&+Wpearl&n",
"",
"&+gspinel&n",
"",
"&+Btourmaline&n"},

{                           /* Fancy Stones, base value 50 GP */
"",
"",
"",
"&+gaquamarine&n",
"",
"",
"&+rgarnet&n",
"",
"",
"&+Wpearl&n",
"&+gperidot&n",
"&+Rspinel&n",
"&+Ytopaz&n",
""},

{                           /* Gem Stones, base value 100 GP */
"&+Lblack opal&n",
"",
"",
"&+gemerald&n",
"&+Rfire opal",
"",
"&+Wopal&n",
"&+Moriental amethyst&n",
"",
"&+Yoriental topaz&n",
"",
"&+Bsapphire&n",
"&+Rstar &n&+Wr&n&+Ruby&n",
"&+Bstar &n&+Ws&n&+Bapphire&n"},

{                           /* Gem Stones, base value 500 GP */
"&+Lblack opal&n",
"&+Lblack sapphire&n",
"&+Wdiamond&n",
"&+gemerald&n",
"&+rfire opal&n",
"&+Rjacinth&n",
"&+Wopal&n",
"&+Boriental amethyst&n",
"&+goriental emerald&n",
"&+Yoriental topaz&n",
"&+Rruby&n",
"&+Bsapphire&n",
"&+Rstar &n&+Wr&n&+Ruby&n",
"&+Bstar &n&+Ws&n&+Bapphire&n"}
  };

if (!gem)
return;

buf[0] = '\0';
gem->str_mask = (STRUNG_KEYS | STRUNG_DESC1 | STRUNG_DESC2);

if (base_value)
{
  bv = base_value;
  /* Calculate orig_base_value_place for later, since no gem may *
  * increase more than 7 places beyond its original base value  */
  orig_base_value_place = 0;
  while ((orig_base_value_place < 12)
         && (base_value > bv_table[orig_base_value_place][1]))
    ++orig_base_value_place;
  if (orig_base_value_place >= 6)
    orig_base_value_place = 5;
  /* randomize base value */
}
else
{
  bv_die = number(1, 100);
  orig_base_value_place = 0;
  while ((orig_base_value_place < 5)
         && (bv_die >= bv_table[orig_base_value_place][0]))
    orig_base_value_place++;
  
  bv = bv_table[orig_base_value_place][1];
  /* Remember orig_base_value_place for later, since no gem may increase *
    * more than 7 places beyond its original base value              */
}
current_base_value_place = orig_base_value_place;

ignore_results_below_2 = 0;   /* Later, if we roll a 10, we ignore 1's */

while (!get_out)
{
  inc_or_dec_die = number(1, 10);
  
  switch (inc_or_dec_die)
  {
    case 1:
      if ((current_base_value_place <= 12)
          && (current_base_value_place <= orig_base_value_place + 6)
          && !ignore_results_below_2)
        bv = bv_table[++current_base_value_place][1];
      break;
    case 2:
      bv = 2.0 * bv;
      get_out = TRUE;
      break;
      
    case 3:
      bv = bv * (1.0 + (((float) number(0, 6)) / 10.0));
      get_out = TRUE;
      break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
      get_out = TRUE;
      break;
    case 9:
      bv = bv * (1.0 - (((float) number(0, 4)) / 10.0));
      get_out = TRUE;
      break;
    case 10:
      ignore_results_below_2 = 1;
      if (current_base_value_place >= orig_base_value_place - 4)
      {
        if (current_base_value_place > 0)
          bv = bv_table[--current_base_value_place][1];
        else
          bv = decreased_bv_table[-1 - (--current_base_value_place)];
      }
  }
} /* end switch */ ;

prop = number(0, 14);
while (strlen(properties[orig_base_value_place][prop]) == 0)
prop = number(0, 14);

/* description and name */
snprintf(buf, MAX_STRING_LENGTH, "a %s", properties[orig_base_value_place][prop]);
gem->short_description = str_dup(buf);
snprintf(buf, MAX_STRING_LENGTH, "A %s lies here.", properties[orig_base_value_place][prop]);
gem->description = str_dup(buf);
gem->name = str_dup("gem RANGEM");

/* randomize some more */
if (!number(0, 5))
{
  SET_BIT(gem->extra_flags, ITEM_NOSELL);
}
gem->cost = (int) (bv * 100);
}

/* base value 1-6 */
void random_jewelry(P_obj obj, int base_value)
{
  long     bv;
  int      bv_die, base_value_place, increase_from_gems, temp;
  char     buf[MAX_STRING_LENGTH];
  const char *nice_work, *nice_gems;
  static int bv_table[7][5] = {
    /*  %     n   d     m  g  Jewelry value is n dice of size d, multiplied by m *
    *---    --  --  ----  -  If g = 1, this piece has gems and may increase     */
  {10, 10, 10, 1, 0},         /* base value place 0 */
  {20, 2, 6, 100, 0},         /* base value place 1 */
  {40, 3, 6, 100, 0},         /* base value place 2 */
  {50, 5, 6, 100, 0},         /* base value place 3 */
  {70, 1, 6, 1000, 1},        /* base value place 4 */
  {90, 2, 4, 1000, 1},        /* base value place 5 */
  {100, 2, 6, 1000, 1}
  };                            /* base value place 6 */
  const char *descriptions[7] = {
    "wrought silver",
    "&+Ywrought&n silver and &+Ygold",
    "&+Ywrought gold",
    "&+Wwrought platinum",
    "silver and &+Rg&n&+Ye&n&+gm&n encrusted",
    "&+Ygold&n and &+Rg&n&+Ye&n&+gm&n encrusted&+Y",
    "&+Wplatinum and &+Rg&n&+Ye&n&+gm&n encrusted&+W"
  };
  const char *kind[51] = {
    "anklet",
    "arm band",
    "belt",
    "box(small)",
    "bracelet",
    "brooch",
    "buckle",
    "chain",
    "chalice",
    "choker",
    "clasp",
    "collar",
    "comb",
    "coronet",
    "crown",
    "decanter",
    "diadem",
    "earring",
    "fob",
    "goblet",
    "headband(fillet)",
    "idol",
    "locket",
    "medal",
    "medallion",
    "necklace",
    "pendant",
    "pin",
    "orb",
    "ring",
    "sceptre",
    "seal",
    "statuette",
    "tiara",
    "bracers",
    "cup",
    "dagger",
    "flask",
    "helm",
    "horn",
    "mirror",
    "periapt",
    "phylactery",
    "rod",
    "scarab",
    "skull",
    "shield",
    "staff",
    "sword",
    "talisman",
    "trident"
  };

  if (base_value)
  {
    bv = base_value;
    /* Calculate base_value_place for later, since no jewelry may *
    * increase more than 7 places beyond its original base value  */
    base_value_place = 0;
    while ((base_value_place < 12)
           && (base_value > bv_table[base_value_place][1]))
      ++base_value_place;
    if (base_value_place >= 6)
      base_value_place = 5;
    
  }
  else
  {
    bv_die = number(0, 100);
    base_value_place = 0;
    while ((base_value_place < 6)
           && (bv_die >= bv_table[base_value_place][0]))
      base_value_place++;
  }

  nice_work = "";

/*roll_bv_dice:*/
  if (base_value)
    bv = base_value;
  else
  {
    /* Next, roll the dice and determine bv */
    bv = 0;
    for (bv_die = 1; bv_die <= bv_table[base_value_place][1]; bv_die++)
      bv += number(0, bv_table[base_value_place][2])
        * bv_table[base_value_place][3];
  }


/*check_for_exceptional:*/
  /* Check for exceptional workmanship */
  if (number(0, 10) == 1)
  {
    nice_work = " (exquisite workmanship)";
    if (bv == bv_table[base_value_place][1] * bv_table[base_value_place][2]
        * bv_table[base_value_place][3])
      if (base_value_place < 6)
      {
        ++base_value_place;
        /*      goto roll_bv_dice;*/
      }
        else;                     /* If already at max, do nothing */
    else
    {
      /* Set bv to maximum for this base_value_place */
      bv = bv_table[base_value_place][1] * bv_table[base_value_place][2]
      * bv_table[base_value_place][3];
      /*      goto check_for_exceptional;*/
    }
  }
  nice_gems = "";
  increase_from_gems = 0;
  if ((bv_table[base_value_place][4]) && (number(0, 6) == 1))
  {
    increase_from_gems = 5000;
    nice_gems = " (quality gems)";
    while ((number(0, 6) == 1) && (increase_from_gems <= 200000))
      increase_from_gems = increase_from_gems * 2;
  }
  bv += increase_from_gems;

  temp = number(0, 50);
  obj->str_mask = (STRUNG_KEYS | STRUNG_DESC1 | STRUNG_DESC2);
  sprintf(buf, "%s jewelry RANJEWELRY", strip_color(kind[temp]));
  obj->name = str_dup(buf);
  sprintf(buf, "a %s %s &n%s%s", descriptions[base_value_place],
          kind[temp], nice_gems, nice_work);
  obj->short_description = str_dup(buf);
  sprintf(buf, "A %s %s &nlies here.", descriptions[base_value_place],
          kind[temp]);
  obj->description = str_dup(buf);
  /* randomize some more */
  if (!number(0, 5))
  {
    SET_BIT(obj->extra_flags, ITEM_NOSELL);
  }
  obj->cost = bv * 100;
}

