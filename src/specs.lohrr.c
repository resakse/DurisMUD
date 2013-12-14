#include "structs.h"
#include "graph.h"
#include "utils.h"
#include "interp.h"
#include "comm.h"
#include "prototypes.h"
#include "ships.h"
extern P_room world;

int adjacent_room_nesw(P_char ch, int num_rooms );
P_ship leviathan_find_ship( P_char leviathan, int room, int num_rooms );

// This is an old proc for Lohrr's eq..
void proc_lohrr( P_obj obj, P_char ch, int cmd, char *argument )
{
   int locwearing;

   // First, verify that it was called properly
   if( !obj || !ch )
      return;

   // Verify object is worn by cy
   if( !OBJ_WORN( obj ) || obj->loc.wearing != ch)
      return;

   for(locwearing = 0;locwearing < MAX_WEAR; locwearing++ )
   {
      if( ch->equipment[locwearing] == obj )
         break;
   }
   // obj is not worn !  This must be a bug if true.
   if( locwearing = MAX_WEAR )
      return;

   switch( locwearing )
   {
      // For his quiver first
      case WEAR_QUIVER:
	// Heal if down more than 10 hps
	if( GET_HIT(ch) < GET_MAX_HIT(ch) - 10 )
	   spell_full_heal( 60, ch, 0, 0, ch, 0);
      break;
   }
}
/*
// It's a percentage chance to make them attack a few extra times.
// It's size dependdent: < medium = 10, medium/large = 6, > large = 4
void dagger_of_wind( P_obj obj, P_char ch, int cmd, char *argument )
{
   int numhits = 0;
   int i = 0;

   // Verify that obj is dagger of wind and being wielded by ch.
   if( cmd != CMD_MELEE_HIT || !ch || !obj || !OBJ_WORN(obj) || obj->loc.wearing != ch )
      return;
   // Verify that ch is in battle with someone.
   if( !IS_FIGHTING(ch) || !ch->specials.fighting )
      return;

   // 50% chance to proc.
   if( number(1,100) > 50 )
   {
       act("You move with a blur of speed!",
          FALSE, ch, obj, 0, TO_CHAR);
       act("$n moves with a blur of speed!",
          FALSE, ch, obj, 0, TO_ROOM);

      // Calculate number of hits based on size.
      if( GET_SIZE(ch) < SIZE_MEDIUM )
         numhits = 10;
      else if( GET_SIZE(ch) == SIZE_MEDIUM || GET_SIZE(ch) == SIZE_LARGE )
         numhits = 6;
      else
         numhits = 4;

      while( i < numhits )
      {
         // Stop hitting if no one to hit.
         if( !ch->specials.fighting )
            break;
         hit(ch, ch->specials.fighting, obj );
         i++;
      }
   }
}
*/
// Alright, so... I made a first attempt at trying to hack some code together and
//  wanted to see if I did it correct. I'm going to cut/paste the proc I put together.
// Basicly, what I was intending, is for a proc that works on command, with a cooldown
//  (yea, the current cooldown is to fast for this item, but i can tweak)... lemme know
//  if it looks right, or what needs to change...
int sphinx_prefect_crown( P_obj obj, P_char ch, int cmd, char *arg )
{
   int curr_time;
   char first_arg256;

   if( cmd == CMD_SET_PERIODIC )
      return TRUE;
   if( !OBJ_WORN_POS( obj, WEAR_HEAD ) )
      return FALSE;

   curr_time = time(NULL);

   if( arg && (cmd == CMD_SAY) && isname(arg, "sphinx"))
   {

      // Set timer here: 5 min = 60 * 5 sec = 300
      // Note: This will be replaced by get_property("timer.proc.crownXXX", ??? )
      // Note: 300 is a magic number here and below.
      if( curr_time >= obj->timer[0] + 300 )
      {
         act("You say 'sphinx'", FALSE, ch, 0, 0, TO_CHAR);
         act("$n says 'sphinx'", TRUE, ch, obj, NULL, TO_ROOM);
         act("&+LYour crown seems to &+Yp&+yu&+Yl&+ys&+Ye &+Lwith a vibrant &+Ymagic&+L!&n", FALSE, ch, obj, obj, TO_CHAR);
         act("&+LA misty haze of &+yun&+Learthly &+Cknowledge &+Lflows from your crown!&n", FALSE, ch, obj, obj, TO_CHAR);
         act("&+mS&+Mw&+mi&+Mr&+ml&+Mi&+mn&+Mg &+Lthoughts and words of &+Bmagic &+Lseem to &+rsear &+Linto your mind as the wisdom of the &+Ys&+yp&+Yh&+yi&+Yn&+yx &+Cinvigorates &+Lyou with &+Gpower&+L!&n", FALSE, ch, obj, obj, TO_CHAR); 
         act("$n's &+Lcrown glows with a &+Gv&+gi&+Gb&+gr&+Ga&+gn&+Gt &+Ylight&+L!&n", TRUE, ch, obj, NULL, TO_ROOM);
         act("&+LThe image of a wise &+Ys&+yp&+Yh&+yi&+Yn&+yx &+Lseems to &+Cshimmer &+Laround $n's &+Lhead!&n", TRUE, ch, obj, NULL, TO_ROOM);
         act("&+LIn a rush of displaced &+Cair&+L, the image of the &+Ys&+yp&+Yh&+yi&+Yn&+yx &+Levaporates into a misty essence infused with &+mmagic&+L!&n", TRUE, ch, obj, NULL, TO_ROOM);
         act("&+LThe misty essence of the &+Ys&+yp&+Yh&+yi&+Yn&+yx &+Lenvelopes $n!", TRUE, ch, obj, NULL, TO_ROOM);
         act("&+LA wicked &+rgrin &+Lpasses across $n's &+Lface as they are &+me&+Mm&+mp&+Mo&+mw&+Me&+mr&+Me&+md &+Lwith &+rancient &+gwisdom &+Land &+Cknowledge&+L!&n", TRUE, ch, obj, NULL, TO_ROOM);
         spell_mordenkainens_lucubration(60, ch, 0, 0, ch, NULL);
         obj->timer[0] = curr_time;
         obj->timer[1] = 0;														      return FALSE;
         return TRUE;
      }
   }

   // Send a message (once) to the char when the crown becomes usable again.
   if (cmd == CMD_PERIODIC && obj->timer[0] + 300 <= curr_time && obj->timer[1] == 0 )
   {
      act("&+LYour legendary &+ycrown &+Lof the &+Ys&+yp&+Yh&+yi&+Yn&+yx &+Gp&+gr&+Ge&+gf&+Ge&+gc&+Gt&+gs &+Cglows &+Lwith an unearthly &+Wlight &+Land starts to &+mpulse &+Lgently in time to your &+rheartbeat&+L.&n",
         TRUE, obj->loc.wearing, obj, 0, TO_CHAR);
      obj->timer[1] = 1;														      return FALSE;
   }
}

int adjacent_room_nesw(P_char ch, int num_rooms )
{
   int dir = number( 0, 3 );
   int i, j;
   room_direction_data *exit;

   if( !ch )
      return -1;

   for( i = 0; i < 3; i++ )
   {
      exit = EXIT( ch, (dir+i)%4 );
      if( exit && exit->to_room )
         // Move num_rooms away (if possible).
         for ( j = 1; j < num_rooms;j++ )
            if( world[exit->to_room].dir_option[(dir+i)%4] )
               exit = world[exit->to_room].dir_option[(dir+i)%4];
      if( exit && exit->to_room )
         return exit->to_room;
   }

   // If no exit found, return -1;
   return -1;
}

// This is a proc for the Leviathan mob.
int leviathan( P_char ch, P_char pl, int cmd, char *arg )
{
   P_char tch;
   int to_room;
   P_ship ship;
   int ram_damage, heading;

   if( cmd == CMD_SET_PERIODIC )
      return TRUE;

   if( !ch )
      return TRUE;

   // Attack nearby ships (within 3 rooms ) if not fighting
   if( cmd == CMD_PERIODIC && !number( 0, 1 ) && !IS_FIGHTING(ch) )
      if( ship = leviathan_find_ship( ch, ch->in_room, 3 ) )
      {
          act_to_all_in_ship( ship,"$N speeds into your ship!", ch );
          act( "$N speeds into a ship!", FALSE, ch, NULL, ch, TO_ROOM );
          ram_damage = number( 10, 15 );
          heading = number( 0, 3 );
          ch_damage_hull( ch, ship, ram_damage, heading, number( 0, 99 ) );
          update_ship_status( ship );
          return TRUE;
      }

   if( cmd == CMD_PERIODIC && !number( 0, 1 ) )
   {
      switch( number( 1, 2 ) )
      {
      case 1:
         act( "$N lifts up out of the water, then splashes back down, causing a massive wave!", FALSE, ch, NULL, ch, TO_ROOM );

         // To each char in room, chance of knockdown.
         for( tch = world[ch->in_room].people;tch;tch = tch->next_in_room )
         {
            if( tch != ch )
            {
               if( number( 0, 1 ) )
                  SET_POS( tch, POS_SITTING + GET_STAT(tch));
               // if not knocked down, chance to get moved 1 room away.
               else if( number( 0, 1 ) && (to_room = adjacent_room_nesw(ch, 1)) )
               {
                  // Move char 1 room
                  char_from_room(tch);
                  char_to_room(tch, to_room, -1);
               }
            }
         }
      break;
      case 2:
         tch = ch->specials.fighting;
         if( tch )
         {
            act( "$N lashes out with a tentacle, wrapping it around you, lifts and quickly slams you upon the water surface!", FALSE, tch, NULL, ch, TO_CHAR );
            act( "$N lashes out grabbing $n with a tentacle, thrashing $m into the water!", FALSE, tch, NULL, ch, TO_ROOM );
            // Move victim 1-3 rooms away
            if( to_room = adjacent_room_nesw(ch, number( 1, 3 )) )
            {
               char_from_room(tch);
               char_to_room(tch, to_room, -1);
            }
            stop_fighting( tch );
            // Stun for 3-5 sec
            CharWait( tch, number( 3, 5 ) );
         }
      default:
      break;
      }
   }

   return FALSE;
}

// Returns a ship if it's near Leviathan
P_ship leviathan_find_ship( P_char leviathan, int room, int num_rooms )
{
   int i;
   P_obj obj;
   P_ship ship;
   room_direction_data *exit;
   char msg[100];

   if( !leviathan )
      return NULL;

   // Look through contents
   for(obj = world[room].contents;obj;obj = obj->next_content )
      // If found a ship && percent >= 50
      if( obj && (GET_ITEM_TYPE(obj) == ITEM_SHIP) && (obj->value[6] == 1) && number( 0, 1) )
         return shipObjHash.find(obj);

   // This is a bit repetative, but that's ok, it's for a small number.
   if( num_rooms > 0 )
   {
      // We only check exits N, S, E and W.
      for(i = 0; i < 4; i++ )
      {
         exit = world[room].dir_option[i];
         if( exit && exit->to_room )
         {
            ship = leviathan_find_ship( leviathan, exit->to_room, num_rooms - 1 );
            if( ship )
               return ship;
         }
      }
   }
   return NULL;
}

// Dependent on ch's weight and str.  Pretty simple atm.
int siege_move_wait( P_char ch )
{
  // Base: 100 str, 100 weight, 2 secs
  int retval = 100*100*2*WAIT_SEC;

  retval /= GET_C_STR( ch );
  retval /= ch->player.weight;

  return retval;
}

// Having to write this is a pain! Ok.. so not so bad.
bool can_move( P_char ch, int dir )
{
  if( IS_FIGHTING(ch) )
    return FALSE;

  if( !EXIT( ch, dir ) )
    return FALSE;
  if( !can_enter_room(ch, EXIT(ch, dir)->to_room, FALSE) )
    return FALSE;

  if( IS_CLOSED( ch->in_room, dir ) )
    return FALSE;
  if( IS_HIDDEN( ch->in_room, dir ) )
    return FALSE;
  if( IS_WALLED( ch->in_room, dir ) )
    return FALSE;

  return TRUE;
}

// This proc is for a ballista(OBJ # 461).
int ballista( P_obj obj, P_char ch, int cmd, char *arg )
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int dir;

  if( cmd == CMD_SET_PERIODIC )
    return TRUE;
  if( cmd == CMD_PUSH )
  {
    // Parse argument.
    argument_interpreter(arg, arg1, arg2);
    if(  isname( arg1, "ballista" )
      || isname( arg1, obj->name ) )
    {
      dir = -1;
      // Figure out what direction.
      if( is_abbrev(arg2, "north") )
        dir = NORTH;
      else if( is_abbrev(arg2, "south") )
        dir = SOUTH;
      else if( is_abbrev(arg2, "east") )
        dir = EAST;
      else if( is_abbrev(arg2, "west") )
        dir = WEST;
      if( dir == -1 || *arg2 == '\0' )
        send_to_char( "Move the ballista what direction?\n", ch );
      else
      {
        // If there is no exit in that direction, or it's blocked.
        if( !can_move( ch, dir ) )
        {
          send_to_char( "You can't leave that way right now.\n", ch );
          return TRUE;
        }
        // Move it that direction.
        sprintf( buf, "You begin to push $p %sward.", dirs[dir] );
        act( buf, FALSE, ch, obj, NULL, TO_CHAR );
        sprintf( buf, "$n starts pushing $p %sward.", dirs[dir] );
        act( buf, TRUE, ch, obj, 0, TO_ROOM);
        // Yes, this lags you to stop you from spamming.
        CharWait(ch, siege_move_wait(ch) );
        add_event(event_move_engine, siege_move_wait(ch), ch, NULL, 
          obj, 0, &dir, sizeof(int) );
      }
      return TRUE;
    }
  }
  return FALSE;
}

// This proc is for a battering ram(OBJ # 462).
int battering_ram( P_obj obj, P_char ch, int cmd, char *arg )
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int dir;

  if( cmd == CMD_SET_PERIODIC )
    return TRUE;
  if( cmd == CMD_PUSH )
  {
    // Parse argument.
    argument_interpreter(arg, arg1, arg2);
    if(  isname( arg1, "battering ram" )
      || isname( arg1, obj->name ) )
    {
      dir = -1;
      // Figure out what direction.
      if( is_abbrev(arg2, "north") )
        dir = NORTH;
      else if( is_abbrev(arg2, "south") )
        dir = SOUTH;
      else if( is_abbrev(arg2, "east") )
        dir = EAST;
      else if( is_abbrev(arg2, "west") )
        dir = WEST;
      if( dir == -1 || *arg2 == '\0' )
        send_to_char( "Move the battering ram what direction?\n", ch );
      else
      {
        // If there is no exit in that direction, or it's blocked.
        if( !can_move( ch, dir ) )
        {
          send_to_char( "You can't leave that way right now.\n", ch );
          return TRUE;
        }
        // Move it that direction.
        sprintf( buf, "You begin to push $p %sward.", dirs[dir] );
        act( buf, FALSE, ch, obj, NULL, TO_CHAR );
        sprintf( buf, "$n starts pushing $p %sward.", dirs[dir] );
        act( buf, TRUE, ch, obj, 0, TO_ROOM);
        // Yes, this lags you to stop you from spamming.
        CharWait(ch, siege_move_wait(ch) );
        add_event(event_move_engine, siege_move_wait(ch), ch, NULL, 
          obj, 0, &dir, sizeof(int) );
      }
      return TRUE;
    }
  }
  return FALSE;
}

// This proc is for a catapult(OBJ # 463).
int catapult( P_obj obj, P_char ch, int cmd, char *arg )
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int dir;

  if( cmd == CMD_SET_PERIODIC )
    return TRUE;
  if( cmd == CMD_PUSH )
  {
    // Parse argument.
    argument_interpreter(arg, arg1, arg2);
    if(  isname( arg1, "catapult" )
      || isname( arg1, obj->name ) )
    {
      dir = -1;
      // Figure out what direction.
      if( is_abbrev(arg2, "north") )
        dir = NORTH;
      else if( is_abbrev(arg2, "south") )
        dir = SOUTH;
      else if( is_abbrev(arg2, "east") )
        dir = EAST;
      else if( is_abbrev(arg2, "west") )
        dir = WEST;
      if( dir == -1 || *arg2 == '\0' )
        send_to_char( "Move the catapult what direction?\n", ch );
      else
      {
        // If there is no exit in that direction, or it's blocked.
        if( !can_move( ch, dir ) )
        {
          send_to_char( "You can't leave that way right now.\n", ch );
          return TRUE;
        }
        // Move it that direction.
        sprintf( buf, "You begin to push $p %sward.", dirs[dir] );
        act( buf, FALSE, ch, obj, NULL, TO_CHAR );
        sprintf( buf, "$n starts pushing $p %sward.", dirs[dir] );
        act( buf, TRUE, ch, obj, 0, TO_ROOM);
        // Yes, this lags you to stop you from spamming.
        CharWait(ch, siege_move_wait(ch) );
        add_event(event_move_engine, siege_move_wait(ch), ch, NULL, 
          obj, 0, &dir, sizeof(int) );
      }
      return TRUE;
    }
  }
  return FALSE;
}

// Attempt to move the siege engine a direction.
void event_move_engine(P_char ch, P_char victim, P_obj obj, void *data)
{
  char buf[MAX_STRING_LENGTH];
  int dir = *((int *)data);
  int to_room;

  // If there's an exit and it's unblocked.
  if( can_move( ch, dir ) )
  {
    sprintf( buf, "You push $p %sward.", dirs[dir] );
    act( buf, FALSE, ch, obj, NULL, TO_CHAR );
    sprintf( buf, "$n pushes $p %sward.", dirs[dir] );
    act( buf, TRUE, ch, obj, 0, TO_ROOM);

    to_room = EXIT(ch, dir)->to_room;
    obj_from_room(obj);
    obj_to_room(obj, to_room);

    char_from_room( ch );
    char_to_room( ch, to_room, dir );

    sprintf( buf, "$n pushes $p in from the %s.", dirs[rev_dir[dir]] );
    act( buf, TRUE, ch, obj, 0, TO_ROOM);
  }
  else
    send_to_char( "You can't leave that way right now.\n", ch );

}
