/*
 *  guildhall_db.h
 *  Implementation of saving guildhalls/rooms to DB
 *
 *  Created by Torgal on 1/30/10.
 *
 */

#ifndef _GUILDHALLS_DB_H_
#define _GUILDHALLS_DB_H_

#include "guildhall.h"
struct Guildhall;
struct GuildhallRoom;

int next_guildhall_id();
int next_guildhall_room_id();
int next_guildhall_room_vnum();

void load_guildhalls(vector<Guildhall*>&);
void load_guildhall(int id, Guildhall *);
void load_guildhall_rooms(Guildhall*);
vector<GuildhallRoom*> load_guildhall_rooms(int guildhall_id);

bool save_guildhall(Guildhall*);
bool save_guildhall_room(GuildhallRoom*);

bool delete_guildhall(Guildhall*);
bool delete_guildhall_room(GuildhallRoom*);

#endif // _GUILDHALLS_DB_H_