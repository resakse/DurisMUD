//
// C++ Interface: storage_lockers
//
// Description: 
//
//
// Author: Gary Dezern <gdezern@xp3000>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __STORAGE_LOCKERS_H__
#define __STORAGE_LOCKERS_H__

#include <string.h>

int guild_locker_room_hook(int room, P_char ch, int cmd, char *arg);
void remove_all_locker_access( P_char ch );

class LockerChest;
class ComboChest;

class StorageLocker
{
public:
  StorageLocker(int rroom, P_char chLocker, P_char chUser);
  ~StorageLocker(void);

  // basically, deletes everything IN the list without deleting the list
  //  object itself.
  void NukeLockerChests(void);

  // parses 'args' and creates needed chests
  bool MakeChests(P_char ch, char *args);

  // figures out where 'obj' belongs and puts it there.  If
  //  it doesn't belong anyplace, return false;
  bool PutInProperChest(P_obj obj);

  P_char GetLockerChar(void) {return m_chLocker;};
  P_char GetLockerUser(void) {return m_chUser;};

  void LockerToPFile(void);
  void PFileToLocker(void);
  void SortIValues(void);

  static void event_resortLocker(P_char chLocker, P_char ch, P_obj obj, void *data);
   int m_itemCount;

protected:

  LockerChest *AddLockerChest(LockerChest *p);
  LockerChest *m_pChestList;
  int m_realRoom;
  P_char m_chLocker;
  P_char m_chUser;
//  int m_itemCount;
  bool m_bIValue;
};


class LockerChest
{
friend class StorageLocker;
friend class ComboChest;

public:
  // destructor...  should drop all contents of check into room,
  //   remove check_obj from room, and extract check_obj.
  virtual ~LockerChest(void);

  // returns the chest object assoicated with this c++ class
  virtual P_obj GetChestObj(void)
    { return m_pChestObject; };
  
  // does an object fit in 'this' locker?
  virtual bool ItemFits(P_obj obj) 
    {return true;};

  virtual void FillExtraDescBuf(char *GBuf1);
  
  void BeautifyDesc(const char *srcDesc, char *destDesc);
        
  const char *m_chestKeyword;
  const char *m_chestDescText;

  static const unsigned m_chestVnum;    
  
protected:  
  LockerChest(const char *keyword, const char *prettyDesc) :
            m_chestKeyword(keyword), 
            m_chestDescText(prettyDesc),
            m_pChestObject(NULL),
            m_pNextInChain(NULL)
  {};
  
  P_obj CreateChestObject(void);
  
  LockerChest *m_pNextInChain;  // init to 0
  P_obj m_pChestObject;         // init to 0
  
  static const char *m_keywords;
  static const char *m_PrettyDesc;  

private:
    
};

class UnsortedChest : public LockerChest
{
  friend class StorageLocker;
public:
  
  virtual bool ItemFits(P_obj obj)
  {  return ((obj->type == ITEM_CONTAINER)||(obj->type == ITEM_CORPSE)) ? false : true; };
  
protected:
  UnsortedChest(void) :
            LockerChest("unsorted", "that are unsorted")
  {};
};

class EqSlotChest : public LockerChest
{
  friend class StorageLocker;
  
public:

  virtual bool ItemFits(P_obj obj)
    { return (obj->wear_flags & m_eqBit) ? true : false; };
  
protected:  
  EqSlotChest(unsigned int eqSlotBit, const char *keyword, const char *prettyDesc) :
            LockerChest(keyword, prettyDesc), m_eqBit(eqSlotBit)
  {};
  
private:
  unsigned int m_eqBit;
};

class EqWearChest : public LockerChest
{
  friend class StorageLocker;
public:
  virtual bool ItemFits(P_obj obj)
    { return IS_SET(obj->extra_flags, ITEM_ALLOWED_CLASSES) ?  
             IS_SET(obj->anti_flags, m_wearClass) :  
             !IS_SET(obj->anti_flags, m_wearClass); };
protected:
  EqWearChest(unsigned wearClass, const char *keyword, const char *prettyDesc) :
            LockerChest(keyword, prettyDesc), m_wearClass(wearClass) 
  {};
  
private:
  unsigned m_wearClass;
};


class EqTypeChest : public LockerChest
{
  friend class StorageLocker;
  
public:    
  
  virtual bool ItemFits(P_obj obj)
    {  return (obj->type == m_eqType) ? true : false;  };
  
protected:
  EqTypeChest(byte eqType, const char *keyword, const char *prettyDesc) :
            LockerChest(keyword, prettyDesc), m_eqType(eqType)
  {};
    
private:
  byte m_eqType;
};

class EqApplyChest : public LockerChest
{
  friend class StorageLocker;
public:
  
  virtual bool ItemFits(P_obj obj);

protected:
  EqApplyChest(byte applyType, const char *keyword, const char *prettyDesc) :
            LockerChest(keyword, prettyDesc), m_applyType(applyType)
  {};
    
private:
  byte m_applyType;
};

class EqAffectChest : public LockerChest
{
  friend class StorageLocker;
public:
  
  virtual bool ItemFits(P_obj obj);

protected:
  EqAffectChest(int b, int bv, const char *keyword, const char *prettyDesc) :
            LockerChest(keyword, prettyDesc), m_bitVector(bv), m_bit(b)
  {};
    
private:
  int m_bitVector;
  int m_bit;
};


class OreChest : public LockerChest
{
  friend class StorageLocker;
public:

  virtual bool ItemFits(P_obj obj)
  { return 
      (strstr(obj->name, "_ore_" ) && 
       (!m_oreType[0] || strstr(obj->name, m_oreType))) ? true : false; };

protected:
  OreChest(const char *oreType, const char *keyword, const char *prettyDesc) :
            LockerChest(keyword, prettyDesc)
  { if (oreType && oreType[0]) 
      strcpy(m_oreType, oreType); else m_oreType[0] = '\0'; };
    
private:
  char m_oreType[1024];
};


class ComboChest : public LockerChest
{
  friend class StorageLocker;
public:
  virtual bool ItemFits(P_obj obj);

  virtual void FillExtraDescBuf(char *GBuf1);
        
protected:
  ComboChest(LockerChest *pChestList) : LockerChest("custom" , "that you custom specified"), m_LockerChests(pChestList) {};
  ~ComboChest(void);
  
  LockerChest *m_LockerChests;
};


#endif // __STORAGE_LOCKERS_H__

