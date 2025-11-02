/*
 * ***************************************************************************
 *  File: smoke.c                                              Part of Duris *
 *  Usage: Command to implement smoking and its affects.                     * 
 *  Copyright  1990, 1991 - see 'license.doc' for complete information.      *
 *  Copyright 1994 - 2016 - Duris Systems Ltd.                               *
 *                                                                           *
 * ***************************************************************************
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "comm.h"
#include "db.h"
#include "events.h"
#include "interp.h"
#include "prototypes.h"
#include "spells.h"
#include "structs.h"
#include "weather.h"
#include "utils.h"
#include "objmisc.h"
#include "necromancy.h"
#include "sql.h"
#include "ctf.h"       // Can remove?

extern P_room world;
extern struct sector_data *sector_table;

void herb_ocularius( int level, int duration, P_char smoker );
void herb_blue_haze( int level, int duration, P_char smoker );
void herb_medicus( int level, int duration, P_char smoker );
void herb_black_kush( int level, int duration, P_char smoker );
void herb_gootwiet( int level, int duration, P_char smoker );

void obj_cast_spell( int spell, int level, int duration, P_char smoker )
{
  switch( spell )
  {
    case HERB_OCULARIUS:
      herb_ocularius( level, duration, smoker );
      break;
    case HERB_BLUE_HAZE:
      herb_blue_haze( level, duration, smoker );
      break;
    case HERB_MEDICUS:
      herb_medicus( level, duration, smoker );
      break;
    case HERB_BLACK_KUSH:
      herb_black_kush( level, duration, smoker );
      break;
    case HERB_GOOTWIET:
      herb_gootwiet( level, duration, smoker );
      break;
    default:
      send_to_char( "Your stash was fake!\n", smoker );
      break;
  }
}

/* Originally coded for Basternae II by Sniktiorg Blackhaven.  This version has been cleaned up, re-coded, and
 *   expanded for use on Duris, also by Sniktiorg Blackhaven with help from Lohrr.  This ability is used in
 *   conjunction with the TYPE_HERB and TYPE_PIPE objects.  It is one of the few systems to utilize the weather
 *   code.  July 27th, 2016 Era Vulgaris.
 */
void do_smoke( P_char ch, char *argument, int cmd )
{
  int room;
  struct weather_data *weather_info = &sector_table[in_weather_sector(ch->in_room)].conditions;

  // The Drawbridge - Determines whether to kill the process before we begin looking into whether the player
  //   has the proper items or how to calculate the Spell_Levels.
  // Dead chars don't smoke (ok.. maybe w/fireball, but not herbs).
  if( !IS_ALIVE(ch) )
  {
    return;
  }

  // Cancel out unless their lungs can handle it.
  if( IS_AFFECTED5(ch, AFF5_STONED) )
  {
    send_to_char( "&nYour &+Ml&+mu&+Mn&+mg&+Ms&n definitely couldn't handle that right now.\r\n", ch );
    return;
  }

  // Animals do not smoke.  Awww... "put pipe bunny mouth".
  if( IS_ANIMAL(ch) && !IS_TRUSTED(ch) )
  {
    send_to_char( "&nYou need a proper opposable thumb.\r\n", ch );
    return;
  }

  // Demons and Dragons Smoke in Different Ways.
  if( (IS_DEMON(ch) || IS_DRAGON(ch)) && !IS_TRUSTED(ch) )
  {
    send_to_char( "&nYou begin to &+Ws&+wt&+We&+wa&+Wm&n like a &+ckettle&n.\r\n", ch );
    act( "Strangely, &+Ws&+wm&+Wo&+wk&+We&n begins to pour forth from $n&n.", FALSE, ch, NULL, NULL, TO_ROOM );
    return;
  }

  // Illithids are too proud to smoke.
  if( (GET_RACE(ch) == RACE_ILLITHID) && !IS_TRUSTED(ch) )
  {
    send_to_char( "Ugh!  The very thought of such plebian an act revolts you.\r\n", ch );
    return;
  }

  // Undead and Angelic Races do not Smoke.
  if( (IS_UNDEADRACE(ch) || IS_ANGEL(ch)) && !IS_TRUSTED(ch) )
  {
    send_to_char( "&nThe idea of &+Wsmoking&n is anathema to you.\r\n", ch );
    return;
  }

  // Assuming Ch left room before Smoke got to this point, thus terminating the action.
  if( (room = ch->in_room) == NOWHERE )
  {
    send_to_char( "&nThe &+cw&+Ci&+cnd&n kicked up by your movement hinders making &+rfire&n.\r\n", ch );
    return;
  }

  // Easier thing is to call a singular procedure can_move() as this is an oft called snippet.
  // It's a function call that can cover lots of things that you might not have considered.
  if( IS_AFFECTED2( ch, AFF2_MINOR_PARALYSIS ) || IS_AFFECTED2( ch, AFF2_MAJOR_PARALYSIS) )
  {
    send_to_char( "&nIt appears that you are already couchlocked!\n\r", ch );
    return;
  }

  // The blind cannot see to light the pipe.
  if (IS_AFFECTED(ch, AFF_BLIND))
  {
    send_to_char( "&nYou are &+Lblind&n, how can you light a &+ypipe&n?\n\r", ch );
    return;
  }

  // Can't smoke in the rain, snow, or blizzards.
  if( (weather_info != NULL) && (weather_info->precip_rate > 0) &&
    !(( world[room].sector_type == SECT_INSIDE ) || ( world[room].sector_type == SECT_UNDRWLD_INSIDE )) )
  {
    if( !IS_ROOM(room, ( ROOM_INDOORS | ROOM_NO_PRECIP | ROOM_TUNNEL )) )
    {
      send_to_char( "&nThere is no way you can &+Wsmoke&n in this &+cwe&+Ca&+ct&+Che&+cr&n.\n\r", ch );
      return;
    }
  }

  // Trolls are a sticky wicket as they shouldn't smoke as it is bad for their health.  There are some caveats,
  //   however.  These include their home terrains of Swamps, as well as sometimes in Sect_Water_Swim.
  // Can't smoke in the water, under water, on the ocean, or in the Water Plane. 
  if( GET_RACE(ch) != RACE_TROLL )
  {
    if( IS_WATER_ROOM(room) )
    {
      send_to_char( "The &+cwa&+Ct&+ce&+Cr&n makes that an impossiblity.\n\r", ch );
      return;
    }
  }
  else if( IS_SECT(room, SECT_WATER_SWIM) )
  {
    // Only a 66% chance a Troll can stay dry enough while swimming to light a pipe.
    if( number(1, 100) < 33 ) // ~1/3 as the code doesn't currently take lev/fly/rafts into account.
    {
      send_to_char( "&nYou are still too &+cd&+Ca&+cm&+Cp&n to &+rspark&n a &+Rflame&n.\n\r", ch );
      return;
      // The Troll kept his hands dry enough to spark the flint.
    }
    else
    {
      send_to_char( "&nYou manage to keep your &+Lflint&n dry enough to &+rspark&n a &+Rflame&n.\n\r", ch );
    }
  }
  // Trolls can smoke if they are in a Swamp/Underworld_Slime sector.  Capillary action helps to protect them.
  else if( IS_SECT(room, SECT_SWAMP) || IS_SECT(room, SECT_UNDRWLD_SLIME) )
  {
    send_to_char( "Luckily, you are just &+cd&+Ca&+cm&+Cp&n enough to dare a &+Rflame&n.\n\r", ch );
  }
  else if( IS_WATER_ROOM(room) )
  {
    send_to_char( "The &+cwa&+Ct&+ce&+Cr&n makes that an impossiblity.\n\r", ch );
    return;
  }
  // Otherwise Trolls may not smoke at all.
  else
  {
    // Wet Trolls are an exception.
    if( IS_AFFECTED5(ch, AFF5_WET) )
    {
      send_to_char( "Luckily, you are just &+cd&+Ca&+cm&+Cp&n enough to dare a &+Rflame&n.\n\r", ch );
    }
    else
    {
      send_to_char( "Are you mad!?  &+gTrolls&n and &+rfire&n do not mix.\n\r", ch );
      return;
    }
  } // End RACE_TROLL Checks

  // The Gates - Now we can get to the meat of the procedure, insomuch as the following sections require variables
  //   to keep track or calculate different states and modifiers.
  P_obj herb, pipe;
  char  arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];

  // Break Up Argument into Arguments
  argument = one_argument( argument, arg1 );
  one_argument( argument, arg2 );

  // Check for Presence of Pipe and Herb Arguments
  if( arg1[0] == '\0' )
  {
    send_to_char( "&+WSmoke&n what?\n\r", ch );
    return;
  }

  if( arg2[0] == '\0' )
  {
    send_to_char( "&nWhat do you want to &+Wsmoke&n through?\n\r", ch );
    return;
  }

  // Is Character Carrying Herb in Hands or Inv?
  if( (( herb = get_obj_in_list_vis(ch, arg1, ch->carrying) ) == NULL)
    && (( (herb = ch->equipment[HOLD]) == NULL ) || ( !isname(arg1, herb->name) ))
    && (( (herb = ch->equipment[WIELD]) == NULL ) || ( !isname(arg1, herb->name) ))
    && (( (herb = ch->equipment[WIELD2]) == NULL ) || ( !isname(arg1, herb->name) ))
    && (( (herb = ch->equipment[WIELD3]) == NULL ) || ( !isname(arg1, herb->name) ))
    && (( (herb = ch->equipment[WIELD4]) == NULL ) || ( !isname(arg1, herb->name) )) )
  {
    snprintf(buf, MAX_STRING_LENGTH, "&nYou do not have any &+g%s&n in your inventory or hands.\n\r", arg1 );
    send_to_char( buf, ch );
    return;
  }

  // Check that It is Actually an Herb.
  if( GET_ITEM_TYPE(herb) != ITEM_HERB )
  {
    send_to_char( "It is unsafe to &+Wsmoke&n anything but certain &+gh&+Ge&+gr&+Gb&+gs&n.\n\r", ch );
    return;
  }

  // Is character holding Pipe?  This should also check 3rd and 4th hands.
  if( (( pipe = get_obj_in_list_vis(ch, arg2, ch->carrying) ) == NULL)
    && (( (pipe = ch->equipment[HOLD]) == NULL ) || ( !isname(arg2, pipe->name) ))
    && (( (pipe = ch->equipment[WIELD]) == NULL ) || ( !isname(arg2, pipe->name) ))
    && (( (pipe = ch->equipment[WIELD2]) == NULL ) || ( !isname(arg2, pipe->name) ))
    && (( (pipe = ch->equipment[WIELD3]) == NULL ) || ( !isname(arg2, pipe->name) ))
    && (( (pipe = ch->equipment[WIELD4]) == NULL ) || ( !isname(arg2, pipe->name) )) )
  {
    snprintf(buf, MAX_STRING_LENGTH, "&nYou are not holding a &+L%s&n.\n\r", arg2 );
    send_to_char( buf, ch );
    return;
  }

  // Check that it is actually a Pipe.
  if( GET_ITEM_TYPE(pipe) != ITEM_PIPE )
  {
    act( "&nYou need a &+Lpipe&n of some sort, $p&n just won't do.", FALSE, ch, pipe, NULL, TO_CHAR );
    return;
  }

  // Thri-Kreen cannot smoke a Nostril Pipe.
  if( (GET_RACE(ch) == RACE_THRIKREEN) && (pipe->value[0] == 3) )
  {
    act( "&+GThri-&+YKreen&n lack nostrils through which to use $p&n.", FALSE, ch, pipe, NULL, TO_CHAR );
    return;
  }

  // If on Fireplane, Lava or liquid mithril, there's a chance the herb will burn to ash before you can smoke it.
  if( IS_SECT(room, SECT_FIREPLANE) || IS_SECT(room, SECT_LAVA) || IS_SECT(room, SECT_UNDRWLD_LIQMITH) )
  {
    if( number(1, 100) <= 25 )
    {
      act( "&nThe &+rheat&n is too much for $p&n.", FALSE, ch, herb, NULL, TO_CHAR );
      act( "&nYou drop the &+Lashes&n of $p&n and sigh.", FALSE, ch, herb, NULL, TO_CHAR );
      act( "$n&n discards $p&n which was ruined by the &+rheat&n.", FALSE, ch, herb, NULL, TO_ROOM );
      extract_obj( herb, TRUE ); // Artifact herbs.. mebbe.
      return;
    }
  }

  // The Throne Room - calculate the adjustements to the default affects of the herb.  Where prior we sought to
  // kill the procedure, from here on we assume it has succeeded and calculate just how well.
  int herb_level = herb->value[0];

  /* Initial Visualization of Smoking the Herb.  All ACT statements are sending FALSE to HIDE_INVIS as even
   * someone without DI should be able to see a flame and smoke.  This and it's counterpart below can be
   * refactored into their own view procedure, but I am keeping it whole in case it is incorporated into
   * actobj.c instead of remaining on its own.
   */
  switch( pipe->value[0] )
  {
    case PIPE_ROLLING_PAPERS:
      act( "&nYou roll $p&n with $P&n into a nice &+Wspliff&n.", FALSE, ch, herb, pipe, TO_CHAR );
      act( "&nYou &+rflame&n $p&n and power inhale the &+ws&+Wm&+wo&+Wk&+we&n from the &+Wspliff&n.", FALSE, ch, herb, NULL, TO_CHAR );
      act( "$n&n rolls $p&n into a &+Wspliff&n and &+rsparks&n it.", FALSE, ch, herb, NULL, TO_ROOM );
      herb_level -= 2; // Not the best method as the papers are primitive.
      break;
    case PIPE_CHILLUM:
      act( "&nYou tamp $p&n into the end of $P&n.", FALSE, ch, herb, pipe, TO_CHAR );
      act( "&nYou &+rlight&n $p&n and huff $P&n &+ws&+Wm&+wo&+Wk&+we&n from your fists.", FALSE, ch, pipe, herb, TO_CHAR );
      act( "$n&n &+rlights&n $p&n and huff $P&n &+ws&+Wm&+wo&+Wk&+we&n from $s fists.", FALSE, ch, pipe, herb, TO_ROOM );
      herb_level -= 1; // Your hands filter more than just contaminants.
      break;
    case PIPE_HOOKAH:
      act( "&nYou pack $p&n into $P&n.", FALSE, ch, herb, pipe, TO_CHAR );
      act( "&nYou &+rkindle&n $P&n and contently puff the &+Ws&+wm&+Wo&+wk&+We&n from $p&n.", FALSE, ch, pipe, herb, TO_CHAR );
      act( "$n&n &+rkindles&n $P&n and contently puffs the &+Ws&+wm&+Wo&+wk&+We&n from $p&n.", FALSE, ch, pipe, herb, TO_ROOM );
      herb_level += 1; // Water purifies. 
      break;
    case PIPE_NOSE_PIPE:
      act( "&nYou press $p&n into the bowl of $P&n.", FALSE, ch, herb, pipe, TO_CHAR );
      act( "&nYou &+rignite&n $p&n and inhale $P&n &+ws&+Wm&+wo&+Wk&+we&n through your nostrils.", FALSE, ch, pipe, herb, TO_CHAR );
      act( "$n&n inserts $p&n into $s nostrils and inhales deep $P&n &+ws&+Wm&+wo&+Wk&+we&n.", FALSE, ch, pipe, herb, TO_ROOM );
      herb_level += 2; // The smoke is exposed to veins in the nose as well as lungs, absorbing faster. 
      break;
    case PIPE_REGULAR:  // Pipe
      act( "&nYou pack $p&n into $P&n.", FALSE, ch, herb, pipe, TO_CHAR );
      act( "&nYou &+rspark&n $P&n and inhale deep the &+Ws&+wm&+Wo&+wk&+We&n from $p&n.", FALSE, ch, pipe, herb, TO_CHAR );
      act( "$n&n &+rsparka&n $P&n and inhales deep the &+Ws&+wm&+Wo&+wk&+We&n from $p&n.", FALSE, ch, pipe, herb, TO_ROOM );
      break;
    // Should an undefined pipe really work?
    default:  // Catch all.
      act( "&nYou stuff $P&n with $p&n.", FALSE, ch, herb, pipe, TO_CHAR );
      act( "&nYou &+rlight&n $P&n and inhale deep the $p&n &+Ws&+wm&+Wo&+wk&+We&n.", FALSE, ch, pipe, herb, TO_CHAR );
      act( "$n&n &+rlights&n $P&n and inhales deep the $p&n &+Ws&+wm&+Wo&+wk&+We&n.", FALSE, ch, pipe, herb, TO_ROOM );
      break;
  }

  /* Ethermancers and their Specializations get a slight bonus due to the nature of their class.
   * Prototype if decided to add other classes/races to the list.  In reality, I would prefer this
   * section check for "fire" or "pyro" or "smoke" or "ether" as part of the class and spec names
   * and then apply the bonus of +2.  This way, any present and future class dealing with smoke or
   * fire would gain a bonus.  This, however, is good enough.  For some reason I chose not to give
   * a message with this modifier.
   */
  if( GET_CLASS(ch, CLASS_ETHERMANCER | CLASS_ALCHEMIST) )
  {
    IS_SPECIALIZED(ch) ? herb_level += 2 : herb_level += 1; // !Spec = +1, Spec = +2
  }

  // Let's switch it up to check for sector modifiers.
  switch( world[room].sector_type )
  {
    // Swamps/Underworld_Slime sectors decrease the effectiveness of the smoke by 2/3 levels unless you are a troll.
    // Putting it here instead of directly after the intial visualization is more forgiving to the final herb_level (for the player).
    case SECT_SWAMP:
    case SECT_UNDRWLD_SLIME:
      if (GET_RACE(ch) != RACE_TROLL)
      {
        herb_level *= 2/3;
        act( "&nThe &+cmo&+Ci&+cs&+Ctu&+cr&+Ce&n is affecting the &+Cdank&+cness&n of $p&n.",
          FALSE, ch, herb, NULL, TO_CHAR );
      }
      break;
    // If on Fireplane or Lava sectors, the herb's effectiveness is decreased by the heat.
    case SECT_FIREPLANE:
    case SECT_LAVA:
      herb_level *= 3/5;
      act( "&nThe &+rheat&n has ruined the &+mte&+cr&+mp&+cen&+mo&+ci&+mds&n of $p&n.", FALSE, ch, herb, NULL, TO_CHAR );
      break;
    default:
      // Every other sector type is smoke friendly at this point.
    break;
  }

  // Let's use a switch to apply a bunch of modifiers to different races.
  switch ( GET_RACE(ch) )
  {
    case RACE_TROLL:
      herb_level += 5;  // Highest bonus given at all.  But that is because they can almost never smoke.
      send_to_char( "&nYou always feel excited when you get to &+ws&+Wm&+wo&+Wk&+we&n.\n\r", ch );
      break;
    case RACE_KOBOLD:
    case RACE_HALFLING:
      herb_level += 2; // All halflings and kobolds love their pipes.  It's a historical homage thing.
      send_to_char( "&nYou feel wonderful inhaling deep the &+Ws&+wm&+Wo&+wk&+We&n.\n\r", ch );
      break;
    case RACE_HUMAN:
    case RACE_ORC:
      herb_level += 1; // Humans and Orcs just need some love.  It is why most humans smoke in meatspace.
      send_to_char( "&nYou feel your tensions begin to ease.\n\r", ch);
      break;
    case RACE_MOUNTAIN:
    case RACE_DUERGAR:
      herb_level -= 1; // Dwarves and Duergars are too damn healthy.
      send_to_char( "&nYou're dwarven constitution mitigates the affects of the &+gh&+Ge&+gr&+Gb&+gs&n upon you.\n\r", ch);
      break;
    case RACE_THRIKREEN:
    case RACE_DRIDER:
      herb_level -= 2; // Their insect body presents its own issues.
      send_to_char( "&nThe affects of the &+Gh&+ge&+Gr&+gb&+Gs&n are diminished by your insectoid metabolism.\n\r", ch);
      break;
    case RACE_DROW:
    case RACE_GREY:
      herb_level -= 5;  // These fuckers have enough bonus with their Shrug.
      send_to_char( "&nYour innate elven magic resists the affects of the &+gh&+Ge&+gr&+Gb&+gs&n.\n\r", ch);
      break;
    default:
      // Everyone else has no modifiers.
    break;
  }

  // Cast the Spells.  Should be the same as if potion were used... not casting of a spell.
  obj_cast_spell( herb->value[1], herb_level, herb->value[7], ch );
  if( herb->value[2] > 0 )
  {
    obj_cast_spell( herb->value[2], herb_level / 2, herb->value[7], ch );
    if( herb->value[3] > 0 )
      obj_cast_spell( herb->value[3], herb_level / 3, herb->value[7], ch );
  }

  // Final Visualization of Smoking the Herb.
  if( pipe->value[0] == PIPE_ROLLING_PAPERS )
  {
    send_to_char( "&nYou toss the &+Lroach&n down your throat as you finish the &+Wspliff&n.\n\r", ch );
    act( "$n&n swallows the &+Lend&n of $s &+Wspliff&n as $e finishes &+Ws&+wmo&+Wk&+wi&+Wng&n.", FALSE, ch, NULL, NULL, TO_ROOM );
    // Spliff destruction.
    extract_obj( pipe, TRUE );
  }
  else
  {
    act( "&nYou finish &+Wsm&+wo&+Wk&+win&+Wg&n $p&n.", FALSE, ch, herb, NULL, TO_CHAR );
    act( "$n&n finishes &+wsm&+Wo&+wk&+Win&+wg&n $s $p&n.", FALSE, ch, pipe, NULL, TO_ROOM );
    // Check Damage to Pipe and Remove if Needed.
    // Pipe damaged?
    if( number(1, 100) < pipe->value[2] )
    {
      // Subtract a HP from the Pipe.
      // Destroy Pipe?
      if( --(pipe->value[1]) <= 0 )
      {
        // Pipe destruction messages.
        act( "&nYour $p&n &+Lcracks&n and becomes useless from too much &+rheat&n.", FALSE, ch, pipe, NULL, TO_CHAR );
        act( "&nYou toss $p&n violently into the distance.", FALSE, ch, pipe, NULL, TO_CHAR );
        act( "$n&n throws away $p&n, as &+Lcracks&n have made it useless.", FALSE, ch, pipe, NULL, TO_ROOM );
        extract_obj( pipe, TRUE );
      }
    }
  }

  send_to_char("You don't think your &+Ml&+mu&+Mn&+mg&+Ms&n could handle &+Wsmoking&n for a bit.\n\r", ch );
  if( number(1, 100) < 23 ) // 23% Chance to cough.  Just needed a reference to 23 somewhere.
  {
    act("$n&n coughs as $e exhales the &+Wsmoke&n.", FALSE, ch, 0, 0, TO_ROOM);
  }
  else
  {
    act("$n&n exhales the &+Wsmoke&n with a grin.", FALSE, ch, 0, 0, TO_ROOM);
  }

  // Apply the lag on next time player can smoke.  Some herbs hurt the lungs more than others.
  struct affected_type af;
  bzero(&af, sizeof(af));

  af.type = HERB_SMOKED;
  af.duration = herb->value[6];
  af.bitvector5 = AFF5_STONED;
  affect_to_char(ch, &af);

  // Remove Herb from Inventory of PCs and PC-controlled mobs, but not mud-controlled mobs (so they can continue to smoke
  // and players can kill mob for the items it carries).  
  if( IS_PC(ch) || IS_PC_PET(ch) )
    extract_obj( herb, FALSE ); // Assuming the False means do not report anything.

  if( !number(0, 420) )
  {
    send_to_char( "&+wYou think you hear a siren.&N\n", ch );
  }
}

/*****************************************************************************
 *  File: herb_spells.c - swallowed by smoke.c                 Part of Duris *
 *  Usage: Contains the herb "spells".                                       *
 *  Copyright  1990, 1991 - see 'license.doc' for complete information.      *
 *  Copyright 1994 - 2016 - Duris Systems Ltd.                               *
 *                                                                           *
 * ***************************************************************************/

/* NOTE:  I fully expect this will just be appended to the bottom of magic.c, as the header stuff is outright
 *        murder.  However, it would be nice to keep them separate from the standard spells even in the magic.c
 *        file.  It's just cleaner that way.  Herb spells are designed to be augmentive only, not that they
 *        cannot have deletorious affects on the user.  They should also kind of imitate the effects of real
 *        world herbs/drugs more than magical potion effects like fireshield.  Also, I consider each herb
 *        spell to be a strain of genetics (which is why secondary and tertiary effects are less, hybrids
 *        don't get all of their parents' genes).  So, once the basics are covered there should be no need
 *        to keep adding herb spells.  3 slots on an herb with 5 current herb spells gives a shit load of
 *        combinations of affects for area designers.  Currently, the herb spells cover augmented senses
 *        (herb_ocularius), offensive augmentation (herb_blue_haze), augmented healing (herb_medicus),
 *        mental augmentation (herb_black_kush), and defensive augmentation (herb_gootwiet).
 *        - Sniktiorg Blackhaven, 7.28.16 Era Vulgaris.
 */


/* HERB_OCULARIUS simulates the effects of a landrace sativa insofar as tries to emulate the
 * paranoia of users not used to very potent sativa strains.  This heightened awareness is manifest
 * through DETECT_GOOD|DETECT_EVIL|AFF4_SENSE_HOLINESS|AFF_INFRAVISION and the additional benefit(?)
 * of AFF2_SLOW to emulate the increased concentration and attention that accompanies the paranoia.
 * The effects are not renewable. - Sniktiorg Blackhaven, 7.28.16EV.
 */
void herb_ocularius(int level, int duration, P_char smoker )
{
  struct affected_type af;

  if( !IS_ALIVE(smoker) )
    return;

  // Check to see if the player is already affected by Herb_Ocularius.
  if( affected_by_spell(smoker, HERB_OCULARIUS) )
  {
    send_to_char( "&nYou can't possibly get any more paranoid than you already are.\n\r", smoker );
    return;
  }

  // Strip Haste.
  if( IS_AFFECTED(smoker, AFF_HASTE) )
    affect_from_char(smoker, AFF_HASTE);

  // Set the Herb's Affects.
  bzero(&af, sizeof(af));

  af.type = HERB_OCULARIUS;
  // duration mud hours + 1/10th herb level seems okay for now.  ~10+ RL mins.
  af.duration = duration + level / 10;

  af.bitvector4 = AFF4_SENSE_HOLINESS;
  // AFF2_SLOW for the obligatory negative affect.
  if( IS_EVIL(smoker) )
  {
    af.bitvector2 = AFF2_DETECT_GOOD | AFF2_SLOW;
  }
  else if( IS_GOOD(smoker) )
  {
    af.bitvector2 = AFF2_DETECT_EVIL | AFF2_SLOW;
  }
  else
  {
    af.bitvector2 = AFF2_DETECT_EVIL | AFF2_DETECT_GOOD | AFF2_SLOW;
  }
  af.bitvector = AFF_SENSE_LIFE | AFF_INFRAVISION;
  affect_to_char(smoker, &af);

  send_to_char( "&nYou feel the &+Ws&+wm&+Wo&+wk&+We&n strengthening your paranoia.\n\r", smoker );

} // End HERB_OCULARIUS


/* HERB_BLUE_HAZE simulates the pain numbing affects common to various real world substances.
 * What this actual means is it gives you an AC boost of 4/5 herb->level (which stacks with other
 * armor-type spells), strips AFF2_SLOW (unless caused by HERB_OCULARIUS), and gives a 
 * small vitality (which does not stack with other vitalities and self-strips if they are present).
 * The effects of Herb_Blue_Haze are renewable.  This herb is contraindicated by Herb_Black_Kush, 
 * which causes the duration of the herb to be halved.- Sniktiorg Blackhaven, 7.28.16EV.
 */
void herb_blue_haze(int level, int duration, P_char smoker )
{
  struct affected_type af;

  // Check for CH and smoker.
  if( !IS_ALIVE(smoker) )
    return;

  // Check to see if the player is already affected by Herb_Black_Kush.  If so, halve the duration of the herb.
  if( affected_by_spell(smoker, HERB_BLACK_KUSH) )
  {
    send_to_char( "&nThe &+Lk&+Gu&+Ls&+Gh&n tempers the effects of the &+ch&+Ca&+cz&+Ce&n.", smoker);
    duration /= 2;
  }

  // Check to see if the player is already affected by Herb_Blue_Haze.
  if( affected_by_spell(smoker, HERB_BLUE_HAZE) )
  {
    struct affected_type *af1;

    // Renew the affect (although if the last time they smoked it didn't apply the vitality, resmoking doesn't either).
    for (af1 = smoker->affected; af1; af1 = af1->next)
    {
      if (af1->type == HERB_BLUE_HAZE)
      {
        af1->duration += duration;
        break;
      }
    }
    send_to_char( "&nYou feel the &+Ws&+wm&+Wo&+wk&+We&n bolster your &+ch&+Ca&+cz&+Ce&n.\n\r", smoker );
    return;
  }

  // Strip Slow (unless it is from HERB_OCULARIUS).
  if( IS_AFFECTED2(smoker, AFF2_SLOW) && !affected_by_spell(smoker, HERB_OCULARIUS) )
    affect_from_char(smoker, AFF2_SLOW);

  // Set the Herb's Affects.
  bzero(&af, sizeof(af));

  af.type = HERB_BLUE_HAZE;
  af.duration = duration;
  // The APPLY_AC does stack.  It's not much, in bump or time, and it's just more expedient than checking
  //   for every existing armor spell as we do for vitality portion below.
  // Actually, we could use the AFF_ARMOR flag, but meh.
  af.modifier = (int) (-1 * ((4 / 5) * level)); // -20 AC at herb->level = 25
  af.location = APPLY_AC;
  affect_to_char(smoker, &af);
  // Slight decrease in Apply_Casting_Pulse.
  af.modifier = number(1, 3);
  af.location = APPLY_SPELL_PULSE;
  affect_to_char(smoker, &af);
  // Do not stack Vitalities!
  if( affected_by_spell(smoker, SPELL_MIELIKKI_VITALITY) || affected_by_spell(smoker, SPELL_FALUZURES_VITALITY) || affected_by_spell(smoker, SPELL_ESHABALAS_VITALITY)
    || affected_by_spell(smoker, SPELL_VITALITY) )
  {
    send_to_char( "&nThe &+Ws&+wm&+Wo&+wk&+We&n heralds a light &+Ch&+ca&+Cz&+ce&n within you.\n\r", smoker );
  }
  else
  {
    af.modifier = 3 * level; // 75hp at herb->level = 25 (Standard vitality is 100hp)
    af.location = APPLY_HIT;
    affect_to_char(smoker, &af);
    af.modifier = (int) (3 + level / 10); // +5 at herb->level = 25 (Standard Vitality is +6)
    af.location = APPLY_HITROLL;
    affect_to_char(smoker, &af);
    update_pos(smoker);
    send_to_char( "&nThe &+Ws&+wm&+Wo&+wk&+We&n brings a &+Ch&+ca&+Cz&+ce&n down upon you.\n\r", smoker );
  }
} // End HERB_BLUE_HAZE


/* HERB_MEDICUS simulates the healing affects attributed to real and legendary real world botanicals.
 * What this actual means is it gives you an APPLY_HIT_REG boost to healing factor.  It is not stackable
 * with other regenerative spells.  No herb affect should ever compete directly with a spell or ability
 * given to a player class (and why if they emulate a spell it is usually at a weaker level).  
 * - Sniktiorg Blackhaven, 7.29.16EV.
 */
void herb_medicus( int level, int duration, P_char smoker )
{
  struct affected_type af;

  // Check for CH and smoker.
  if( !IS_ALIVE(smoker) )
    return;

  // Sorry, no stacking with other accelerated healing type spells.
  if( affected_by_spell(smoker, SPELL_ACCEL_HEALING) || affected_by_spell(smoker, SKILL_REGENERATE) ||
      affected_by_spell(smoker, SPELL_REGENERATION) || affected_by_spell(smoker, HERB_MEDICUS) )
  {
    // So sorry, you just wasted the money or time it took to get that herb.
    send_to_char( "As good as the &+ch&+Ge&+cr&+Gb&+cs&n are, you can't heal any faster.\n\r", smoker );
    return;
  }

  // Set the Herb's Affects.
  bzero(&af, sizeof(af));

  af.type = HERB_MEDICUS;
  af.duration = SECS_PER_MUD_HOUR * 5; // 5 Hours seems okay for now.  ~6.5 RL mins.
  af.location = APPLY_HIT_REG;
  af.modifier = (level * level * 4) / 3; // Standard SPELL_REGENERATION is level * level * 2.
  affect_to_char(smoker, &af);
  send_to_char( "&nThe &+Ws&+wm&+Wo&+wk&+We&n begins to help your body mend itself.\n\r", smoker );

} // End HERB_MEDICUS


/* HERB_BLACK_KUSH simulates the mental augmentation of low dose psychotropics or perhaps certain
 * amphetamines.  What this translates to in game terms is a 1/3 bump in "caster" attributes, a
 * slight boost in mana regeneration for psionicists, and a 1/3 drop in "fighter" attributes.  This
 * herb is contraindicated by Herb_Blue_Haze, which causes the duration of the herb to be halved.
 * - Sniktiorg Blackhaven, 7.29.16EV.
 */
void herb_black_kush(int level, int duration, P_char smoker )
{
  struct affected_type af;

  // Check for CH and smoker.
  if( !IS_ALIVE(smoker) )
    return;

  // Check to see if the player is already affected by Herb_Blue_Haze.  If so, halve the duration of the herb.
  if( affected_by_spell(smoker, HERB_BLUE_HAZE) )
  {
    send_to_char( "&nYour &+ch&+Ca&+cz&+Ce&n tempers the effects of the &+Gk&+Lu&+Gs&+Lh&n.", smoker );
    duration /= 2;
  }

  // Check to see if the player is already affected by Herb_Black_Kush.
  if( affected_by_spell(smoker, HERB_BLACK_KUSH) )
  {
    struct affected_type *af1;

    // Renew the affect.
    for( af1 = smoker->affected; af1; af1 = af1->next )
    {
      if (af1->type == HERB_BLACK_KUSH )
      {
        af1->duration += duration;
        break;
      }
    }
    send_to_char( "&nYou feel your mind expand with the &+Ws&+wm&+Wo&+wk&+We&n.\n\r", smoker );
    return;
  }

  // Set the Herb's Affects.
  bzero(&af, sizeof(af));

  af.type = HERB_BLACK_KUSH;
  af.duration = duration;
  // General boost to each "caster" stat.
  af.modifier = (GET_C_POW(smoker) / 10) + (level / 10) - 5;
  af.location = APPLY_POW;
  affect_to_char(smoker, &af);
  af.modifier = (GET_C_INT(smoker) / 10) + (level / 10) - 5;
  af.location = APPLY_INT;
  affect_to_char(smoker, &af);
  af.modifier = (GET_C_WIS(smoker) / 10) + (level / 10) - 5;
  af.location = APPLY_WIS;
  affect_to_char(smoker, &af);

  // Give Psionicists a boost to mana regen.
  if( GET_CLASS(smoker, CLASS_PSIONICIST) )
  {
    af.location = APPLY_MANA_REG;
    af.modifier = BOUNDED(1, GET_LEVEL(smoker) / 10, 5);
    affect_to_char(smoker, &af);
  }
  // General drops to AGI and STR.
  af.modifier = -(GET_C_STR(smoker) / 10) - (level / 10) - 5;
  af.location = APPLY_STR;
  affect_to_char(smoker, &af);
  af.modifier = -(GET_C_AGI(smoker) / 10) - (level / 10) - 5;
  af.location = APPLY_AGI;
  affect_to_char(smoker, &af);
  // Slight decrease in Apply_Combat_Pulse.
  af.modifier = number(1, 3);
  af.location = APPLY_COMBAT_PULSE;
  affect_to_char(smoker, &af);

  send_to_char( "&nYou feel the &+Ws&+wm&+Wo&+wk&+We&n improve your mental accuity.\n\r", smoker );
} // End HERB_BLACK_KUSH


/* HERB_GOOTWIET (Dutch for "gutter weed" and pronounced "Goatwheat") simulates a defensive augmentation 
 * in the form of random improved saves and slight boosts to AGI and DEX.  It's negatives are in the form 
 * of its interactions with other herbs voiding out the various save boosts.
 * - Sniktiorg Blackhaven, 7.30.16EV.
 */
void herb_gootwiet(int level, int duration, P_char smoker )
{
  struct affected_type af;

  // Check for CH and smoker.
  if( !IS_ALIVE(smoker) )
    return;

  // Check to see if the player is already affected by Herb_Gootwiet.
  if( affected_by_spell(smoker, HERB_GOOTWIET) )
  {
    struct affected_type *af1;

    // Renew the affect.
    for( af1 = smoker->affected; af1; af1 = af1->next )
    {
      if( af1->type == HERB_GOOTWIET )
      {
        af1->duration = duration;
      }
    }
    send_to_char( "&nYou cough harshly as the &+Ws&+wm&+Wo&+wk&+We&n expands in your &+ml&+Mu&+mn&+Mg&+ms&n.\n\r", smoker );
    return;
  }

  // Set the Herb's Affects.
  bzero(&af, sizeof(af));

  af.type = HERB_GOOTWIET;
  af.duration = duration;

  // 75% chance to add a small boost to Save Para so long as smoker isn't affected by Herb_Ocularius.
  if( (number(1, 100) <= 75) && !affected_by_spell(smoker, HERB_OCULARIUS) )
  {
    // Apply_Saving_Para
    af.modifier = -(level / 10) - number(1, 3); // herb->level = 25 gives -3 to -5 Save Para
    af.location = APPLY_SAVING_PARA;
    affect_to_char(smoker, &af);
    send_to_char( "&nYou feel your muscles relax as you become more aware of each of them.\n\r", smoker );
  }

  // 75% chance to add a small boost to Save Spell so long as smoker isn't affected by Herb_Blue_Haze.
  if( (number(1, 100) <= 75) && !affected_by_spell(smoker, HERB_BLUE_HAZE) )
  {
    // Apply_Saving_Spell
    af.modifier = -(level / 10) - number(1, 3); // herb->level = 25 gives -3 to -5 Save Spell
    af.location = APPLY_SAVING_SPELL;
    affect_to_char(smoker, &af);
    send_to_char( "&nYour body tingles with magical resistance.\n\r", smoker );
  }

  // 75% chance to add a small boost to Save Breath so long as smoker isn't affected by Herb_Medicus.
  if( (number(1, 100) <= 75) && !affected_by_spell(smoker, HERB_MEDICUS) )
  {
    // Apply_Saving_Breath
    af.modifier = -(level / 10) - number(1, 3); // herb->level = 25 gives -3 to -5 Save Breath
    af.location = APPLY_SAVING_BREATH;
    affect_to_char(smoker, &af);
    send_to_char( "&nYour &+Ml&+mu&+Mn&+mg&+Ms&n have never felt so powerful.\n\r", smoker );
  }

  // 75% chance to add a small boost to Save Fear so long as smoker isn't affected by Herb_Black_Kush.
  if( (number(1, 100) <= 75) && !affected_by_spell(smoker, HERB_BLACK_KUSH) )
  {
    // Apply_Saving_Fear
    af.modifier = -(level / 10) - number(1, 3); // herb->level = 25 gives -3 to -5 Save Fear
    af.location = APPLY_SAVING_FEAR;
    affect_to_char(smoker, &af);
    send_to_char( "&nYou can't remember ever feeling this invulnerable.\n\r", smoker );
  }

  // General boosts to AGI and DEX.
  int stat_modifier = number(9, 13);
  af.modifier = GET_C_AGI(smoker) / stat_modifier;  // Add 1/9, 1/10, 1/11, 1/12, or 1/13 of a stat boost.
  af.location = APPLY_AGI;
  affect_to_char(smoker, &af);
  af.modifier = GET_C_DEX(smoker) / stat_modifier;
  af.location = APPLY_DEX;
  affect_to_char(smoker, &af);

  send_to_char( "&nThe &+Ws&+wm&+Wo&+wk&+We&n tastes floral and pungent on the tongue.\n\r", smoker );
} // End HERB_GOOTWIET

