#ifndef WORLD_QUEST_H
#define WORLD_QUEST_H

#include <vector>

int calc_zone_mob_level();
int suggestQuestMob(int zone_num, P_char ch, int questtype);
int newbie_quest(P_char, P_char, int, char *);
void quest_kill(P_char ch, P_char vict);
void quest_ask(P_char ch, P_char quest_mob);
int quest_buy_map(P_char ch);
void resetQuest(P_char ch);
int getItemFromZone(int zone);
void show_map_at(P_char ch, int room);
void quest_full_reward(P_char ch, P_char quest_mob, int type);
bool createQuest(P_char ch, P_char giver);
void getQuestZoneList(P_char ch, vector<int>&);
bool isInvalidQuestZone(int zoneID);

#endif // WORLD_QUEST_H
