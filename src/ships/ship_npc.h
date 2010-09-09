#ifndef _SHIP_NPC_H_
#define _SHIP_NPC_H_

#include "ships.h"
#include "ship_npc_ai.h"

struct NPCShipSetup
{
    int m_class;
    int level;
    int crew_size;
    void (*setup)(P_ship);
};

struct NPCShipCrewData
{
    int level;
    int captain_mob;
    int firstmate_mob;
    /*int quartermaster_mob;
    int gunmaster_mob;
    int helmsman_mob;
    int doctor_mob;
    int lookout_mob;*/
    int spec_mobs[10];
    int inner_grunts[10];
    int outer_grunts[10];
    int treasure_chest;
    int treasure_chest_key;
    int ship_crew_index;
};

P_ship try_load_pirate_ship(P_ship target);
P_ship try_load_npc_ship(P_ship target, NPC_AI_Type type, int level, P_char dbg_ch);
P_ship try_load_npc_ship(P_ship target, NPC_AI_Type type, int level, int location, P_char dgb_ch);
P_ship load_npc_ship(int level, NPC_AI_Type type, int min_speed, int m_class, int room, P_char dgb_ch);
bool try_unload_npc_ship(P_ship ship);



extern NPCShipCrewData npcShipCrewData[];
P_char load_npc_ship_crew_member(P_ship ship, int room_no, int vnum, int load_eq);
bool load_npc_ship_crew(P_ship ship, int crew_size, int ship_level);
P_obj load_treasure_chest(P_ship ship, P_char captain, NPCShipCrewData* crew_data);

int npc_ship_crew_member_func(P_char ch, P_char player, int cmd, char *arg);
int npc_ship_crew_captain_func(P_char ch, P_char player, int cmd, char *arg);
int npc_ship_crew_board_func(P_char ch, P_char player, int cmd, char *arg);

#define CYRICS_REVENGE_CREW 4
#define CYRICS_REVENGE_NAME "&+RCy&+rri&+Lc's Rev&+ren&+Rge"
#define CYRICS_REVENGE_NEXUS_STONE NEXUS_STONE_ENLIL
bool load_cyrics_revenge();
bool load_cyrics_revenge_crew(P_ship ship);
int get_cyrics_revenge_nexus_rvnum(P_ship cyrics_revenge);
extern P_ship cyrics_revenge;
extern bool nexus_to_cyrics_revenge;


#endif // _SHIP_NPC_H_
