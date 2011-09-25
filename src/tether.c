#include "tether.h"

// returns true iff victim is tethered.
bool is_being_tethered( P_char victim )
{
   struct char_link_data *cld;

   for( cld = victim->linking; cld; cld = cld->next_linking )
      if( cld->type == LNK_TETHER )
         return TRUE;
   return FALSE;
}

// returns true iff ch is tethering someone.
bool is_tethering( P_char ch )
{
   struct char_link_data *cld;

   for( cld = ch->linked; cld; cld = cld->next_linked )
      if( cld->type == LNK_TETHER )
         return TRUE;
   return FALSE;
}

// returns target iff ch is tethering someone.
P_char tethering( P_char ch )
{
   struct char_link_data *cld;

   for( cld = ch->linked; cld; cld = cld->next_linked )
      if( cld->type == LNK_TETHER )
         return cld->linking;
   return NULL;
}

// tether to a person
void do_tether( P_char ch, char *argument, int cmd )
{
   P_char victim;

   if( !ch )
      return;

   // If doesn't have tether (i.e. not a cabalist)
   if( !GET_CLASS( ch, CLASS_CABALIST ) )
   {
      act( "You would not know where to begin.", FALSE, ch, NULL, NULL, TO_CHAR );
      return;
   }

   // Stop tethering if tether off.
   if( isname( argument, "off" ) )
   {
      clear_links( ch, LNK_TETHER );
      act( "You break your soul bond.", FALSE, ch, NULL, NULL, TO_CHAR );
      return;
   }

   victim = get_char_room_vis( ch, argument );
   if( victim == NULL )
   {
      act( "Tether who?", FALSE, ch, NULL, NULL, TO_CHAR );
      return;
   }

   // If victim is not a PC:
   if( IS_NPC( victim ) )
   {
      act( "You cannot connect with that.", FALSE, ch, NULL, NULL, TO_CHAR );
      return;
   }

   // If victim is already tethered:
   if( is_being_tethered( victim ) )
   {
      act( "That soul is too crowded.", FALSE, ch, NULL, NULL, TO_CHAR );
      return;
   }

   if( victim == ch )
   {
      act( "Yes, you are your soul mate; try again.", FALSE, ch, NULL, NULL, TO_CHAR);
      return;
   }

   // Success:
   // Add a message if stop tethering?
   clear_links( ch, LNK_TETHER );
   link_char( ch, victim, LNK_TETHER );
   act( "You reach out and embrace $N's soul.", FALSE, ch, NULL, victim, TO_CHAR );
   act( "$n latches on to your soul with a warm embrace.", FALSE, ch, NULL, victim, TO_VICT );

}

// This should be called when char leaves the room (and after everyone follows)
void tether_broken( struct char_link_data *cld )
{
   P_char ch = cld->linking;
   P_char victim = cld->linked;

   unlink_char(ch, victim, LNK_TETHER );
   act( "Your reach exceeded, you release $N's soul.", FALSE, ch, NULL, victim, TO_CHAR );
   act( "You feel a cold vacancy within.", FALSE, ch, NULL, victim, TO_VICT );
}

/*
When cabalist deals damage wth spells while in the same room as his 
tethered individual(s).  Anyone tethered gains healed hps (not vamp) at 
a percent equal to a new property.
*/
void tetherheal( P_char ch, int damageamount )
{
   P_char victim = tethering( ch );

   if( victim )
   {
      vamp( victim, damageamount * get_property( "innate.cabalist.healing_mod", 0.700), GET_MAX_HIT(ch) );
      act( "You are tethering $N.", FALSE, ch, NULL, victim, TO_CHAR );
      //single_tether_heal( ch, damageamount );
   }
   else
      //group_tether_heal( ch, damageamount );
      act( "You are not tethering anyone.", FALSE, ch, NULL, NULL, TO_CHAR );
}
