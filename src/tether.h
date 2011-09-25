#include "structs.h"
#include "prototypes.h"
#include "comm.h"
#include "utils.h"

bool is_being_tethered( P_char victim );

// returns true iff ch is tethering someone.
bool is_tethering( P_char ch );

// tether to a person
void do_tether( P_char ch, char *argument, int cmd );

// This should be called when char leaves the room (and after everyone follows)
void tether_broken( struct char_link_data *cld );

void tetherheal( P_char ch, int damageamount );
