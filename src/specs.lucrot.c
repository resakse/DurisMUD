// Lucrot's file of pain, sweat, and tears.
// 2009

#include <ctype.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <list>
#include <vector>
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
#include "justice.h"
#include "assocs.h"
#include "graph.h"
#include "damage.h"
#include "spells.h"

extern P_index obj_index;
extern char *spells[];
extern int top_of_world;
extern int char_is_on_plane(P_char);
extern P_room world;

int lucrot_mindstone(P_obj obj, P_char ch, int cmd, char *arg)
{
  int curr_time;

  if(cmd == CMD_SET_PERIODIC ||
    !(obj) ||
    !(ch) ||
    !OBJ_WORN_BY(obj, ch))
  {
    return FALSE;
  }
  
  if((cmd == CMD_SAY) &&
      arg)
  {
    if(isname(arg, "journey"))
    {
      curr_time = time(NULL);
      
      if(affected_by_spell(ch, TAG_PVPDELAY) ||
         IS_FIGHTING(ch) ||
         IS_IMMOBILE(ch))
      {
        send_to_char("&+WYour thoughts are too incohesive and disorganized.&n\r\n", ch);
        CharWait(ch, PULSE_VIOLENCE * 1);

        return false;
      }
      
      if(obj->timer[0] + 1200 <= curr_time)
      {
        int to_room = real_room(19401);

        act("You say 'Journey'",FALSE, ch, 0, 0, TO_CHAR);
        act("\n&+cYour&n $q &+cpulses!&n\n",
          FALSE, ch, obj, obj, TO_CHAR);

        act("$n says 'Journey'", TRUE, ch, obj, NULL, TO_ROOM);
        act("\n$n's &n$q &+cpulses!&n\n",
          TRUE, ch, obj, NULL, TO_ROOM);
      
        if(IS_SET(world[ch->in_room].room_flags, NO_RECALL))
        {
          send_to_char("&+WThere is something about this area that prevents your journey home.&n\r\n", ch);
          CharWait(ch, PULSE_VIOLENCE * 1);

          return false;
        }
          
        char_from_room(ch);
        char_to_room(ch, to_room, 0);

        act("$n materializes...", 0, ch, 0, 0, TO_ROOM);
        CharWait(ch, PULSE_VIOLENCE * 4);
          
        obj->timer[0] = curr_time;
        return true;
      }
    }
  }
  
  return false;
}

        
