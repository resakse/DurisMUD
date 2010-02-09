#ifndef __AUCTION_HOUSES_H_
#define __AUCTION_HOUSES_H_

/*
 auction_houses.h
 -- torgal april 2006 (torgal@durismud.com)
*/

#include "files.h"
#include "db.h"
#include "structs.h"
#include <string>
#include <vector>
using namespace std;

void init_auction_houses();
void shutdown_auction_houses();
void auction_houses_activity();
int auction_house_room_proc(int room_num, P_char ch, int cmd, char *arg);

string format_time(long);
bool finalize_auction(int auction_id, P_char to_ch);
bool insert_money_pickup(int pid, int money);

bool auction_offer(P_char ch, char *arg);
bool auction_list(P_char ch, char *arg);
bool auction_info(P_char ch, char *arg);
bool auction_bid(P_char ch, char *arg);
bool auction_pickup(P_char ch, char *arg);
bool auction_help(P_char ch, char *arg);
bool auction_remove(P_char ch, char *arg);
bool auction_resort(P_char ch, char *arg);

class EqSortFlag {
  public:
	EqSortFlag(const char *_keyword, const char *_desc) : keyword(_keyword), desc(_desc) {}

	virtual bool match(P_obj obj) = 0;
		
	string keyword;
	string desc;
};

class EqSlotFlag : public EqSortFlag {
  public:
	EqSlotFlag(const char *_keyword, const char *_desc, unsigned int _eq_bit) :
		eq_bit(_eq_bit), EqSortFlag(_keyword, _desc)
		{}

	bool match(P_obj obj) {
		return (obj->wear_flags & eq_bit) ? true : false;
	}
	
	unsigned int eq_bit;
};

class EqClassFlag : public EqSortFlag {
  public:
	EqClassFlag(const char *_keyword, const char *_desc, unsigned _wear_class) :
		wear_class(_wear_class), EqSortFlag(_keyword, _desc)
		{}

	bool match(P_obj obj) {
		return IS_SET(obj->extra_flags, ITEM_ALLOWED_CLASSES) ?  
	             IS_SET(obj->anti_flags, wear_class ) :  
	             !IS_SET(obj->anti_flags, wear_class);		
	}

  unsigned wear_class;
};

class EqTypeFlag : public EqSortFlag {
  public:
	EqTypeFlag(const char *_keyword, const char *_desc, byte _type) :
		type(_type), EqSortFlag(_keyword, _desc)
		{}

	bool match(P_obj obj) {
		return (obj->type == type) ? true : false;
	}
	
	byte type;
};

class EqApplyFlag : public EqSortFlag {
  public:
	EqApplyFlag(const char *_keyword, const char *_desc, byte _apply) :
		apply(_apply), EqSortFlag(_keyword, _desc)
		{}

	bool match(P_obj obj) {
	  for (int i = 0; i < MAX_OBJ_AFFECT; i++)
	  {
	    if ((obj->affected[i].location == apply ) &&
	        (obj->affected[i].modifier != 0))
	      return true;
	  }
	  return false;		
	}
	
	byte apply;
};

class EqSort {
  public:
	EqSort();
	
	~EqSort() {
		for( vector<EqSortFlag*>::iterator it = flags.begin(); it != flags.end(); it++ ) {
			if( *it ) {
				delete *it;
				*it = NULL;
			}
		}
		
	}
	
	string getSortFlagsString(P_obj obj);

	string getDescString(const char *keyword);

	bool isKeyword(const char *_keyword);

  private:
	vector<EqSortFlag*> flags;
	
};

#endif // __AUCTION_HOUSES_H_
