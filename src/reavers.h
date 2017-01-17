#ifndef __REAVERS_H__
#define __REAVERS_H__

void spell_baladors_protection(int, P_char, char *, int, P_char, P_obj);
void spell_ferrix_precision(int, P_char, char *, int, P_char, P_obj);
void spell_eshabalas_vitality(int, P_char, char *, int, P_char, P_obj);
void spell_kanchelsis_fury(int, P_char, char *, int, P_char, P_obj);

void spell_blood_alliance(int level, P_char ch, char *arg, int type, P_char victim, P_obj obj);
void check_blood_alliance(P_char ch, int dam);
void event_blood_alliance(P_char ch, P_char victim, P_obj obj, void *data);

void spell_cegilunes_searing_blade(int, P_char, char *, int, P_char, P_obj);
bool cegilune_blade(P_char ch, P_char victim, P_obj wpn);
void event_cegilune_searing(P_char ch, P_char vict, P_obj obj, void *data);

void spell_stormcallers_fury(int, P_char, char *, int, P_char, P_obj);
bool stormcallers_fury(P_char ch, P_char victim, P_obj wpn);

bool kostchtchies_implosion(P_char ch, P_char victim, P_obj wpn);
void spell_kostchtchies_implosion(int level, P_char ch, char *arg, int type, P_char victim, P_obj obj);

void spell_ilienzes_flame_sword(int, P_char, char *, int, P_char, P_obj);
bool ilienze_sword(P_char ch, P_char victim, P_obj wpn);
void ilienze_sword_proc_messages(struct damage_messages *messages, const char *sub);

void spell_girilals_granite_hammer(int, P_char, char *, int, P_char, P_obj);
void spell_ileshs_smashing_fury(int, P_char, char *, int, P_char, P_obj);
bool ileshs_fury(P_char ch, P_char victim, P_obj wpn);
void event_ileshs_fury(P_char ch, P_char vict, P_obj obj, void *data);

void spell_thryms_icerazor(int, P_char, char *, int, P_char, P_obj);
bool thryms_icerazor(P_char ch, P_char victim, P_obj wpn);

void spell_lliendils_stormshock(int, P_char, char *, int, P_char, P_obj);
bool lliendils_stormshock(P_char ch, P_char victim, P_obj wpn);

bool reaver_hit_proc(P_char ch, P_char victim, P_obj weapon);

int required_weapon_skill(P_obj wpn); // defined in fight.c
void apply_reaver_mods(P_char ch);


#endif // __REAVERS_H__

