
/*
   ***************************************************************************
   *  File: specs.realm.c                                      Part of Duris *
   *  Usage: Special procs for Faerie Realm                                    *
   *  Copyright  1994, 1995 - Micah (Xeade) and Duris Systems Ltd.           *
   *************************************************************************** 
 */

#include <stdio.h>

#include "comm.h"
#include "db.h"
#include "events.h"
#include "interp.h"
#include "prototypes.h"
#include "specs.prototypes.h"
#include "structs.h"
#include "utils.h"

/*
   external variables 
 */

extern P_room world;

int tree_spirit(P_char ch, P_char pl, int cmd, char *arg)
{
  static int count = 0;
  static int helper_called = 0;
  P_char   mob = NULL;

  /*
     check for periodic event calls 
   */
  if (cmd == CMD_SET_PERIODIC)
    return FALSE;

  if (cmd && (cmd != CMD_DOWN) && (cmd != -1))
    return (FALSE);

  if (cmd == CMD_DOWN)
  {
    act("$N grabs you and throws you across the room before you could leave!",
        FALSE, pl, 0, ch, TO_CHAR);
    act("$N grabs $n and throws $m across the room!", FALSE, pl, 0, ch,
        TO_NOTVICT);
    return (TRUE);
  }
  else if (cmd == 0)
  {
    if (GET_OPPONENT(ch))
    {
      count++;
      /*
         this is where I changed some stuff around... does it work? 
       */
      if (count >= 3)
      {                         /*
                                   every 10 rounds => 12*10/40=3. 
                                 */
        switch (number(1, 2))
        {
        case 1:
          if (helper_called < 4)
          {
            mob = read_mobile(14024, VIRTUAL);
            break;
          }
        case 2:
          if (helper_called < 8)
            mob = read_mobile(14023, VIRTUAL);
          else
            return (FALSE);
          break;
        }

        if (!mob)
          return TRUE;

        helper_called++;
        act
          ("$n incants a powerful spell of creation and $N breaks through the wall of the chamber in aid!",
           FALSE, ch, 0, mob, TO_ROOM);
        char_to_room(mob, ch->in_room, 0);
        MobStartFight(mob, GET_OPPONENT(ch));
        count = 0;
        return (TRUE);
      }
    }
  }
  else
  {                             /*
                                   cmd == -1, when mob died 
                                 */
    count = 0;
    helper_called = 0;
  }
  return (FALSE);
}

int finn(P_char ch, P_char pl, int cmd, char *arg)
{
  /*
     check for periodic event calls 
   */
  if (cmd == CMD_SET_PERIODIC)
    return TRUE;

  if (pl)
  {
    return (FALSE);
  }
  else if (!IS_FIGHTING(ch))
  {
    switch (number(1, 15))
    {
    case 1:
      mobsay(ch, "If you are new to this realm, please go "
             "to Anna's cottage... she will be of much help to you in your journeys here.");
      break;

    case 2:
      mobsay(ch, "I wish I could leave this blasted realm.");
      break;

    case 3:
      mobsay(ch, "If only I had remembered a means of "
             "returning magically to my home, ... damn! I can't believe I lost my ring");
      break;

    case 4:
      act("The Legendary Finn searches through his travel gear, "
          " fruitlessly, and then sighs.", TRUE, ch, 0, 0, TO_ROOM);
      break;

    case 5:
      act("The Legendary Finn smiles at you and says 'If you "
          "are new to this realm, go to Anna's cottage in the Faerie Forest."
          "  It's over the hill north of here, and then north into the woods..."
          " before the highlands.'", TRUE, ch, 0, 0, TO_ROOM);
      break;

    case 6:
      mobsay(ch, "Make sure you travel this realm with care...");
      break;

    default:
      break;
    }
  }
  else
  {                             /*
                                   position == fighting 
                                 */
    switch (number(1, 15))
    {
    case 1:
      mobsay(ch, "You think you can actually beat me?  I"
             " laugh at your attempt.");
      break;

    case 2:
      mobsay(ch, "Pray I do not mortally harm you, but you"
             " have called this doom upon yourself.  Cease now in this foolishness!");
      break;

    case 3:
      mobsay(ch, "You have left me little option but to"
             " destroy you.  Where shall I instruct my page to deliver your corpse?");
      break;

    default:
      break;
    }
  }
  return (FALSE);
}

/*
   for mob 14048... a cricket, duh! 
 */

int cricket(P_char ch, P_char pl, int cmd, char *arg)
{
  /*
     check for periodic event calls 
   */
  if (cmd == CMD_SET_PERIODIC)
    return TRUE;

  if (pl)
  {
    return (FALSE);
  }
  else
  {
    if (!IS_FIGHTING(ch))
    {
      switch (number(1, 8))
      {
      case 1:
        act("*chirp-chirp*... *chirp-chirp*... *chirp-chirp*...", FALSE, ch,
            0, 0, TO_ROOM);
        break;

      case 2:
        act("The chirping of an insect somewhere in the underbrush"
            " can be heard near by.", FALSE, ch, 0, 0, TO_ROOM);
        break;

      default:
        break;
      }
    }
  }
  return (FALSE);
}

/*
   picks a random mortal in the room 
 */

int faerie(P_char ch, P_char pl, int cmd, char *arg)
{
  /*
     check for periodic event calls 
   */
  if (cmd == CMD_SET_PERIODIC)
    return TRUE;

  if (pl)
  {
    return (FALSE);
  }
  else if (!IS_FIGHTING(ch))
  {
    switch (number(1, 15))
    {
    case 1:
      if (!(pl = char_in_room(ch->in_room)))
        break;
      act("$n tickles you - Hee Ha Hehe Ha Hee hee Ha", FALSE, ch, 0, pl,
          TO_VICT);
      act("$n tickles $N into a fit of hysterics.", TRUE, ch, 0, pl,
          TO_NOTVICT);
      break;

    case 2:
      if (!(pl = char_in_room(ch->in_room)))
        break;
      act("$n dances around you in a merry, little jig.  "
          "This guy must be starved for attention.",
          FALSE, ch, 0, pl, TO_VICT);
      act("$n dances around $N in a merry, little jig.",
          TRUE, ch, 0, pl, TO_NOTVICT);
      break;

    case 3:
      if (!(pl = char_in_room(ch->in_room)))
        break;
      npc_steal(ch, pl);
      act("The woodland faerie whistles innocently and then "
          "grins mischievously.", TRUE, ch, 0, 0, TO_ROOM);
      break;

    case 4:
      if (!(pl = char_in_room(ch->in_room)))
        break;
      act("With a cry of laughter, $n falls down giggling "
          "at you.", FALSE, ch, 0, pl, TO_VICT);
      act("With a cry of laugher, $n falls down giggling at $N.",
          TRUE, ch, 0, pl, TO_NOTVICT);
      break;

    case 5:
      if (!(pl = char_in_room(ch->in_room)))
        break;
      act("$n looks at you and says, 'You're new here, eh?'",
          FALSE, ch, 0, pl, TO_VICT);
      act("$n looks at $N and says, 'You're new here, eh?'",
          TRUE, ch, 0, pl, TO_NOTVICT);
      break;

    default:
      break;
    }
  }
  return (FALSE);
}
