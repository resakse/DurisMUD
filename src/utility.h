/*
 *  utility.h
 *  Duris
 *
 *  Created by Torgal on 1/29/10.
 *
 */

#ifndef _UTILITY_H_
#define _UTILITY_H_

#include "structs.h"

int GET_LVL_FOR_SKILL(P_char ch, int skill);
bool is_ansi_char(char collor_char);

void connect_rooms(int, int, int, int);
void connect_rooms(int, int, int);

void disconnect_exit(int v1, int dir);
void disconnect_rooms(int v1, int v2);

P_char get_char_online(char *name);

void logit(const char *, const char *,...);

int cmd_from_dir(int dir);
int direction_tag(P_char ch);

const char *condition_str(P_char ch);

#endif // _UTILITY_H_