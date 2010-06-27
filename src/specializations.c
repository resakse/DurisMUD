#include <stdio.h>
#include <cstring>
#include <vector>
using namespace std;

#include "comm.h"
#include "interp.h"
#include "utils.h"
#include "prototypes.h"
#include "spells.h"
#include "structs.h"
#include "sql.h"
#include "epic.h"
#include "specializations.h"

extern P_room world;
extern char *specdata[][MAX_SPEC];

#define SPEC_ALL 0

// this is the list of class specialziations blocked for certain races
struct allowed_race_spec_struct {
  int race;
  uint m_class;
  int spec;
} allowed_race_specs[] =
{
  /* Lets take care of who has access to all specs first */
    {RACE_HUMAN, CLASS_WARRIOR, SPEC_ALL},
    {RACE_HUMAN, CLASS_SORCERER, SPEC_ALL},
    {RACE_HUMAN, CLASS_ETHERMANCER, SPEC_ALL},
    {RACE_HUMAN, CLASS_SHAMAN, SPEC_ALL},
    {RACE_HUMAN, CLASS_ILLUSIONIST, SPEC_ALL},
    {RACE_HUMAN, CLASS_CLERIC, SPEC_ALL},
    {RACE_HUMAN, CLASS_PALADIN, SPEC_ALL},
    {RACE_HUMAN, CLASS_CONJURER, SPEC_ALL},
    {RACE_HUMAN, CLASS_NECROMANCER, SPEC_ALL},
    {RACE_HUMAN, CLASS_DRUID, SPEC_ALL},
    {RACE_HUMAN, CLASS_BARD, SPEC_ALL},
//    {RACE_HUMAN, CLASS_MONK, SPEC_ALL},
    {RACE_HUMAN, CLASS_MERCENARY, SPEC_ALL},
    //  {RACE_HUMAN, CLASS_ROGUE, SPEC_ALL},   Swashbucklers were removed
    {RACE_HUMAN, CLASS_ROGUE, SPEC_SHARPSHOOTER},
    {RACE_HUMAN, CLASS_ROGUE, SPEC_ASSASSIN},
    {RACE_HUMAN, CLASS_ROGUE, SPEC_THIEF},
    {RACE_HUMAN, CLASS_RANGER, SPEC_ALL},
    {RACE_ORC, CLASS_WARRIOR, SPEC_ALL},
    {RACE_ORC, CLASS_SORCERER, SPEC_ALL},
    {RACE_ORC, CLASS_ETHERMANCER, SPEC_ALL},
    {RACE_ORC, CLASS_SHAMAN, SPEC_ALL},
    {RACE_ORC, CLASS_ILLUSIONIST, SPEC_ALL},
    {RACE_ORC, CLASS_CLERIC, SPEC_ALL},
    {RACE_ORC, CLASS_ANTIPALADIN, SPEC_ALL},
    {RACE_ORC, CLASS_CONJURER, SPEC_ALL},
    {RACE_ORC, CLASS_NECROMANCER, SPEC_ALL},
    {RACE_ORC, CLASS_BARD, SPEC_ALL},
    {RACE_ORC, CLASS_BERSERKER, SPEC_ALL},
    {RACE_ORC, CLASS_MERCENARY, SPEC_ALL},
    //{RACE_ORC, CLASS_ROGUE, SPEC_ALL},    Swashbucklers were removed
    {RACE_ORC, CLASS_ROGUE, SPEC_SHARPSHOOTER},
    {RACE_ORC, CLASS_ROGUE, SPEC_ASSASSIN},
    {RACE_ORC, CLASS_ROGUE, SPEC_THIEF},
    {RACE_ORC, CLASS_REAVER, SPEC_ALL},
    {RACE_GITHYANKI, CLASS_PSIONICIST, SPEC_ALL},
    {RACE_PVAMPIRE, CLASS_DREADLORD, SPEC_ALL},
    {RACE_HUMAN, CLASS_AVENGER, SPEC_ALL},
    // {RACE_HALFELF, CLASS_AVENGER, SPEC_ALL},
    {RACE_PLICH, CLASS_NECROMANCER, SPEC_ALL},
    {RACE_MINOTAUR, CLASS_WARRIOR, SPEC_GUARDIAN},
    {RACE_MINOTAUR, CLASS_WARRIOR, SPEC_SWORDSMAN},
    {RACE_HALFLING, CLASS_WARRIOR, SPEC_ALL},
    {RACE_GNOME, CLASS_WARRIOR, SPEC_ALL},
    // {RACE_HALFELF, CLASS_WARRIOR, SPEC_GUARDIAN},
    // {RACE_HALFELF, CLASS_WARRIOR, SPEC_SWORDSMAN},
    {RACE_THRIKREEN, CLASS_WARRIOR, SPEC_SWORDSMAN},
    {RACE_TROLL, CLASS_WARRIOR, SPEC_GUARDIAN}, //Troll Allowed: Guardian
    {RACE_OGRE, CLASS_WARRIOR, SPEC_SWORDSMAN}, //Ogre Allowed: Swordsman
    {RACE_DROW, CLASS_WARRIOR, SPEC_GUARDIAN}, //Drow Allowed: Swordsman
    {RACE_DROW, CLASS_WARRIOR, SPEC_SWORDSMAN}, 
    {RACE_GITHYANKI, CLASS_WARRIOR, SPEC_SWORDSMAN}, //Githyanki Allowed: Swordsman
    {RACE_GITHYANKI, CLASS_WARRIOR, SPEC_GUARDIAN},
    {RACE_DUERGAR, CLASS_WARRIOR, SPEC_SWORDSMAN},
    {RACE_OROG, CLASS_WARRIOR, SPEC_GUARDIAN},
    {RACE_CENTAUR, CLASS_WARRIOR, SPEC_GUARDIAN}, //Centaur Allowed: Guardian, Swordsman
    {RACE_CENTAUR, CLASS_WARRIOR, SPEC_SWORDSMAN},
    {RACE_MOUNTAIN, CLASS_WARRIOR, SPEC_GUARDIAN},
    {RACE_BARBARIAN, CLASS_WARRIOR, SPEC_GUARDIAN},
    {RACE_BARBARIAN, CLASS_WARRIOR, SPEC_SWORDSMAN},
    {RACE_GREY, CLASS_WARRIOR, SPEC_SWORDSMAN},
    {RACE_GREY, CLASS_WARRIOR, SPEC_GUARDIAN},
    {RACE_GOBLIN, CLASS_WARRIOR, SPEC_SWASHBUCKLER},
    {RACE_GOBLIN, CLASS_WARRIOR, SPEC_GUARDIAN},
    {RACE_GITHZERAI, CLASS_WARRIOR, SPEC_GUARDIAN},
    /* End Warrior Restrictions */
    /* Clerics Second -Zion */
    {RACE_DROW, CLASS_CLERIC, SPEC_ZEALOT}, //Drow Allowed: Zealot
    {RACE_DROW, CLASS_CLERIC, SPEC_HEALER}, 
    {RACE_DUERGAR, CLASS_CLERIC, SPEC_HOLYMAN}, //Duergar Allowed: Holyman, Zealot
    {RACE_DUERGAR, CLASS_CLERIC, SPEC_ZEALOT},
    {RACE_GOBLIN, CLASS_CLERIC, SPEC_HEALER}, //Goblin Allowed: Healer, Zealot
    {RACE_GOBLIN, CLASS_CLERIC, SPEC_ZEALOT},
    {RACE_GREY, CLASS_CLERIC, SPEC_HEALER}, //Grey Allowed: Healer
    {RACE_GREY, CLASS_CLERIC, SPEC_HOLYMAN},
    {RACE_MOUNTAIN, CLASS_CLERIC, SPEC_HOLYMAN}, //Dwarf Allowed: Holyman, Zealot
    {RACE_MOUNTAIN, CLASS_CLERIC, SPEC_ZEALOT},
    {RACE_GNOME, CLASS_CLERIC, SPEC_ALL},
    {RACE_HALFLING, CLASS_CLERIC, SPEC_ZEALOT}, //Halfling Allowed: Healer, Zealot
    {RACE_HALFLING, CLASS_CLERIC, SPEC_HEALER},
    {RACE_GITHYANKI, CLASS_CLERIC, SPEC_HOLYMAN},
//  {RACE_OROG, CLASS_CLERIC, SPEC_ZEALOT},
    {RACE_GITHZERAI, CLASS_CLERIC, SPEC_HOLYMAN},
    {RACE_GITHZERAI, CLASS_CLERIC, SPEC_ZEALOT},
    {RACE_TROLL, CLASS_CLERIC, SPEC_ZEALOT},
    {RACE_BARBARIAN, CLASS_CLERIC, SPEC_ZEALOT}, 
    
    /* End Cleric Restrictions */
    /* Third up, Sorcerers -Zion */
    {RACE_GREY, CLASS_SORCERER, SPEC_SHADOW}, //Grey Allowed: Wizard, Shadowmage
    {RACE_GREY, CLASS_SORCERER, SPEC_WIZARD},
    {RACE_HALFLING, CLASS_SORCERER, SPEC_WILDMAGE}, //Halfling Allowed: Wildmage, Shadowmage
    {RACE_HALFLING, CLASS_SORCERER, SPEC_SHADOW},
    {RACE_GNOME, CLASS_SORCERER, SPEC_ALL}, //Gnome Allowed: Wizard
    {RACE_GITHYANKI, CLASS_SORCERER, SPEC_SHADOW}, //Githyanki Allowed: Shadowmage, Wildmage
    {RACE_GITHYANKI, CLASS_SORCERER, SPEC_WILDMAGE},
    {RACE_DROW, CLASS_SORCERER, SPEC_SHADOW}, //Drow Allowed: Wizard, Wildmage
    {RACE_DROW, CLASS_SORCERER, SPEC_WILDMAGE},
    {RACE_MINOTAUR, CLASS_SORCERER, SPEC_WILDMAGE}, //Minotaur Allowed: Wildmage
    // {RACE_HALFELF, CLASS_SORCERER, SPEC_WILDMAGE},
    {RACE_GITHZERAI, CLASS_SORCERER, SPEC_SHADOW},
    {RACE_GITHZERAI, CLASS_SORCERER, SPEC_WIZARD},
    /* End Sorcerer Restrictions */
    /* Lets Tackle Conjurers now */
    {RACE_GNOME, CLASS_CONJURER, SPEC_FIRE}, //Gnome Allowed: Fire, Earth
    {RACE_GNOME, CLASS_CONJURER, SPEC_EARTH},
    {RACE_GREY, CLASS_CONJURER, SPEC_AIR}, // Grey Allowed: Air, Water
    {RACE_GREY, CLASS_CONJURER, SPEC_WATER},
    {RACE_DROW, CLASS_CONJURER, SPEC_FIRE}, // Drow Allowed: Fire, Water
    {RACE_DROW, CLASS_CONJURER, SPEC_WATER},
    {RACE_GITHYANKI, CLASS_CONJURER, SPEC_AIR}, // Githyanki Allowed: Air, Earth
    {RACE_GITHYANKI, CLASS_CONJURER, SPEC_EARTH},
    // {RACE_HALFELF, CLASS_CONJURER, SPEC_WATER}, // Half-Elf Allowed: Water, Earth
    // {RACE_HALFELF, CLASS_CONJURER, SPEC_EARTH},
    {RACE_GITHZERAI, CLASS_CONJURER, SPEC_WATER},
    {RACE_GITHZERAI, CLASS_CONJURER, SPEC_AIR},
    /* End Conjurer Restrictions */
    /* Now for Shamans */
    {RACE_OGRE, CLASS_SHAMAN, SPEC_SPIRITUALIST}, // Ogre Allowed: Spiritualist
    {RACE_TROLL, CLASS_SHAMAN, SPEC_ELEMENTALIST}, // Troll Allowed: Elementalist
    {RACE_GOBLIN, CLASS_SHAMAN, SPEC_ANIMALIST}, // Goblin Allowed: Animalist
    {RACE_BARBARIAN, CLASS_SHAMAN, SPEC_SPIRITUALIST}, // Barbarian Allowed: Spiritualist
    {RACE_CENTAUR, CLASS_SHAMAN, SPEC_ANIMALIST}, // Centaur Allowed: Animalist
    {RACE_GNOME, CLASS_SHAMAN, SPEC_ELEMENTALIST}, // Gnome Allowed: Elementalist
    {RACE_HALFLING, CLASS_SHAMAN, SPEC_ELEMENTALIST}, // Halfling Allowed: Elementalist, Animalist
    {RACE_HALFLING, CLASS_SHAMAN, SPEC_ANIMALIST},
    {RACE_MINOTAUR, CLASS_SHAMAN, SPEC_ELEMENTALIST}, // Minotaur Allowed: Elementalist, Spiritualist
    {RACE_MINOTAUR, CLASS_SHAMAN, SPEC_SPIRITUALIST},
    {RACE_OROG, CLASS_SHAMAN, SPEC_ALL},
    /* End Shaman Restrictions */
    /* Lets Cover Paladins & Anti-Paladins here, the list will be short */
    {RACE_GITHYANKI, CLASS_ANTIPALADIN, SPEC_DEMONIC}, // Allowing the rider spec for the non base races for these
    // {RACE_HALFELF, CLASS_PALADIN, SPEC_CAVALIER},      // classes.
    /* End AP/Paladin Restrictions */
    /* Hoo boy, lets tackle rangers */
    {RACE_GREY, CLASS_RANGER, SPEC_WOODSMAN}, // Grey Allowed: Huntsman
    {RACE_CENTAUR, CLASS_RANGER, SPEC_MARSHALL}, // Centaur Allowed: Marshall
    // {RACE_HALFELF, CLASS_RANGER, SPEC_BLADEMASTER}, // Half-Elf Allowed: Blademaster
    // {RACE_HALFELF, CLASS_RANGER, SPEC_WOODSMAN},
    {RACE_HALFLING, CLASS_RANGER, SPEC_WOODSMAN},
    /* End Ranger Restrictions */
    /* Reaver Time */
    {RACE_DROW, CLASS_REAVER, SPEC_FLAME_REAVER}, // Drow Allowed: Flame Reaver, Shock Reaver
    {RACE_DROW, CLASS_REAVER, SPEC_SHOCK_REAVER},
    {RACE_GITHYANKI, CLASS_REAVER, SPEC_ICE_REAVER}, // Gith Allowed: Ice Reaver
    /* End Reaver Restrictions */
    /* Stop...Bard time! */
    {RACE_GREY, CLASS_BARD, SPEC_MINSTREL},
    {RACE_DROW, CLASS_BARD, SPEC_SCOUNDREL},
    {RACE_HALFLING, CLASS_BARD, SPEC_SCOUNDREL},
    // {RACE_HALFELF, CLASS_BARD, SPEC_DISHARMONIST},
    {RACE_GITHYANKI, CLASS_BARD, SPEC_DISHARMONIST},
    /* End Bard Restrictions */
    /* Lets tackle Druids */
    {RACE_GREY, CLASS_DRUID, SPEC_WOODLAND},
    {RACE_HALFLING, CLASS_DRUID, SPEC_WOODLAND},
    {RACE_CENTAUR, CLASS_DRUID, SPEC_STORM},
    // {RACE_HALFELF, CLASS_DRUID, SPEC_STORM},
    /* End Druid Restrictions */
    /* Rogues, booya */
    {RACE_DUERGAR, CLASS_ROGUE, SPEC_ASSASSIN},
    {RACE_DUERGAR, CLASS_ROGUE, SPEC_THIEF},
    {RACE_DROW, CLASS_ROGUE, SPEC_SHARPSHOOTER},
    {RACE_MOUNTAIN, CLASS_ROGUE, SPEC_ASSASSIN},
    {RACE_HALFLING, CLASS_ROGUE, SPEC_THIEF},
    {RACE_HALFLING, CLASS_ROGUE, SPEC_SHARPSHOOTER},
    {RACE_GNOME, CLASS_ROGUE, SPEC_SHARPSHOOTER},
    {RACE_GNOME, CLASS_ROGUE, SPEC_THIEF},
    {RACE_GREY, CLASS_ROGUE, SPEC_SHARPSHOOTER},
    // {RACE_HALFELF, CLASS_ROGUE, SPEC_SHARPSHOOTER},
    {RACE_GOBLIN, CLASS_ROGUE, SPEC_SHARPSHOOTER},
    {RACE_GOBLIN, CLASS_ROGUE, SPEC_THIEF},
    /* End Rogues, yay */
    /* Necros! Steo is gay! */
    {RACE_DROW, CLASS_NECROMANCER, SPEC_REAPER},
    {RACE_DROW, CLASS_NECROMANCER, SPEC_DIABOLIS},
    {RACE_GITHYANKI, CLASS_NECROMANCER, SPEC_NECROLYTE},
    /* Bye Necros */
    /* Illusionist! */
    {RACE_GNOME, CLASS_ILLUSIONIST, SPEC_DECEIVER},
    {RACE_HALFLING, CLASS_ILLUSIONIST, SPEC_DARK_DREAMER},
    /* End Illusionist */
    /* Mercs FTW! */
    {RACE_BARBARIAN, CLASS_MERCENARY, SPEC_OPPORTUNIST},
    {RACE_MOUNTAIN, CLASS_MERCENARY, SPEC_BOUNTY},
    {RACE_HALFLING, CLASS_MERCENARY, SPEC_OPPORTUNIST},
    {RACE_GNOME, CLASS_MERCENARY, SPEC_BOUNTY},
    {RACE_DUERGAR, CLASS_MERCENARY, SPEC_BOUNTY},
    {RACE_TROLL, CLASS_MERCENARY, SPEC_OPPORTUNIST},
    {RACE_MINOTAUR, CLASS_MERCENARY, SPEC_BOUNTY},
//  {RACE_OROG, CLASS_MERCENARY, SPEC_OPPORTUNIST},
    /* Last but not least (well, mostly least) Ethermancers! */
    {RACE_GOBLIN, CLASS_ETHERMANCER, SPEC_FROST_MAGUS},
    {RACE_GOBLIN, CLASS_ETHERMANCER, SPEC_COSMOMANCER},
    {RACE_GNOME, CLASS_ETHERMANCER, SPEC_WINDTALKER},
    {RACE_GNOME, CLASS_ETHERMANCER, SPEC_COSMOMANCER},
    /* End Ethermancer */
    /* Handling new berserker/monk races here */
    {RACE_OROG, CLASS_BERSERKER, SPEC_MAULER},
    {RACE_GITHZERAI, CLASS_MONK, SPEC_ALL},
    {RACE_MINOTAUR, CLASS_BERSERKER, SPEC_ALL},
    {RACE_MOUNTAIN, CLASS_BERSERKER, SPEC_ALL},
    {RACE_DUERGAR, CLASS_BERSERKER, SPEC_ALL},
    {0}
};

bool is_allowed_race_spec(int race, uint m_class, int spec)
{
  for( int i = 0; allowed_race_specs[i].race; i++ )
  {
    if( allowed_race_specs[i].race == race &&
        allowed_race_specs[i].m_class == m_class &&
        (allowed_race_specs[i].spec == SPEC_ALL ||
         allowed_race_specs[i].spec == spec ) )
    {
      return TRUE;
    }
  }
  return FALSE;
}

bool append_valid_specs(char *buf, P_char ch)
{
  if( !ch )
    return false;
  
  bool found_one = false;
  for (int i = 0; i < MAX_SPEC; i++)
  {
    if ( *GET_SPEC_NAME(ch->player.m_class, i) && 
         is_allowed_race_spec(GET_RACE(ch), ch->player.m_class, i+1) )
    {
      strcat(buf, GET_SPEC_NAME(ch->player.m_class, i));
      strcat(buf, "\r\n");
      found_one = true;
    }
  }
  
  return found_one;  
}

void do_specialize(P_char ch, char *argument, int cmd)
{
  P_char teacher;
  int      i;
  char     buf[MAX_STRING_LENGTH] = "";
  bool found_one;
  
  if (IS_MULTICLASS_PC(ch)) {
    send_to_char("You have already chosen another path!\n", ch);
    return;
  }
  
  if (!*argument)
  {
    strcpy(buf, "You can choose from the following specializations:\n\r");
    
    found_one = append_valid_specs(buf, ch);;
    
    if( !found_one )
    {
      send_to_char("There are no specializations available to you.\r\n", ch);
      return;      
    }

    send_to_char(buf, ch);
    return;
  }
  
  for (teacher = world[ch->in_room].people; teacher; teacher = teacher->next_in_room)
    if (IS_NPC(teacher) && GET_CLASS(teacher, ch->player.m_class) &&
        IS_SET(teacher->specials.act, ACT_SPEC_TEACHER))
      break;
  
  if (!teacher) {
    send_to_char("You need to find a teacher first.\n", ch);
    return;
  }
  
  if (IS_NPC(ch))
  {
    mobsay(teacher, "Please stop trying to crash the game by ordering pets to specialize.");
    return;
  }
  
  if (teacher->player.m_class != ch->player.m_class)
  {
    mobsay(teacher, "I know nothing of your kind. Be gone.");
    return;
  }
  
  if (IS_SPECIALIZED(ch))
  {
    mobsay(teacher, "You are already specialized.");
    return;
  }
  
  char     tmp[512];
  
  while (*argument == ' ')
    argument++;
  
  if (time(NULL) < ch->only.pc->time_unspecced)
  {
    sprintf(buf, "You cannot specialize until %s.\r\n",
            asctime(localtime(&ch->only.pc->time_unspecced)));
    send_to_char(buf, ch);
    return;
  }
  
  if (GET_LEVEL(ch) < 30)
  {
    mobsay(teacher, "You are not yet experienced enough to specialize.");
    return;
  }
  
  for (i = 0; i < MAX_SPEC; i++)
  {
    if( !*GET_SPEC_NAME(ch->player.m_class, i))
      continue;
    
    if( !is_allowed_race_spec(GET_RACE(ch), ch->player.m_class, i+1) )
      continue;
    
    if( is_abbrev( argument, strip_ansi(GET_SPEC_NAME(ch->player.m_class, i)).c_str()) )
    {
      sprintf(buf, "From this day onwards you will follow the path of the %s&n!", GET_SPEC_NAME(ch->player.m_class, i));
      ch->player.spec = i+1;
      update_skills(ch);
      mobsay(teacher, buf);
      return;
    }    
  }
  
  mobsay(teacher, "I'm sorry, but that specialization isn't available to you.");
}

void unspecialize(P_char ch, P_obj obj)
{
  if (!IS_SPECIALIZED(ch)) {
    send_to_char("You pray to the &+bWater Goddess&n but you get no response.",
                 ch);
  } if (epic_skillpoints(ch) < 1) {
    send_to_char("You need one practice point to pay for this.\n", ch);
  } else {
    act("You kneel in front of $p and pray to the \n"
        "&+bWater Goddess&n. As you continue your meditation, you begin\n"
        "to feel your mind is breaking free from the old habits and you\n"
        "feel ready to learn new ways.\n", FALSE, ch, obj, 0, TO_CHAR);
    act("$n kneels before $p and sinks in prayers.\n"
        "After few moments of silence $e smiles and stands up looking reborn.\n", FALSE, ch, obj, 0, TO_ROOM);
    ch->player.spec = 0;
    update_skills(ch);
    epic_gain_skillpoints(ch, -1);
    forget_spells(ch, -1);
  }
}
