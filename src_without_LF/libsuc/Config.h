/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Basilio Fraguela
                  Smruti Sarangi
		  Milos Prvulovic

This file is part of SESC.

SESC is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2, or (at your option) any later version.

SESC is    distributed in the  hope that  it will  be  useful, but  WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should  have received a copy of  the GNU General  Public License along with
SESC; see the file COPYING.  If not, write to the  Free Software Foundation, 59
Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef CONFIGCLASS_H
#define CONFIGCLASS_H

#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdio.h>
#include <vector>
#include <string>
using std::string;
#include "nanassert.h"
#include "estl.h"

using std::string;

class Config {
protected:

  class KeyIndex {
  public:
    string s1;
    string s2;
    KeyIndex(void) : s1(), s2(){
    }
    KeyIndex(const char *cstr1, const char *cstr2) : s1(cstr1), s2(cstr2){
    }
    bool operator==(const KeyIndex &a) const {
      if(a.s1.size()!=s1.size())
        return false;
      if(strncasecmp(a.s1.data(),s1.data(),s1.size()))
        return false;
      if(a.s2.size()!=s2.size())
        return false;
      if(strncasecmp(a.s2.data(),s2.data(),s2.size()))
        return false;
      return true;
    }
  };

  // Class for each one of the fields stored in Config. It's similar
  // to the concept of record in DB.
  class Record {
  private:
    enum RCType {
      RCDouble = 0,
      RCInt,
      RCBool,
      RCCharPtr
    };

    bool env;
    bool used;
    bool printed;
    RCType type;

    union {
      bool Bool;
      int32_t Int;
      double Double;
      char *CharPtr;
    } v;
    Record & operator = (const Record & rec) {
      I(type!=RCCharPtr);
      I(rec.type!=RCCharPtr);
      type = rec.type;
      used = rec.used;
      printed = rec.printed;
      v = rec.v;
      X = rec.X;
      Y = rec.Y;
      return *this;
    }

    int32_t X;
    int32_t Y;

  public:
    Record(const Record & rec) {
      I(type!=RCCharPtr);
      I(rec.type!=RCCharPtr);
      type = rec.type;
      used = rec.used;
      printed = rec.printed;
      v = rec.v;
      X = 0;
      Y = 0;
    }
    ~Record(void);
    Record(bool val);
    Record(int32_t val);
    Record(double val);
    Record(const char *val);

    Record(bool val,int32_t x, int32_t y);
    Record(int32_t val,int32_t x, int32_t y);
    Record(double val,int32_t x, int32_t y);
    Record(const char *val,int32_t x, int32_t y);

    bool isEnv() const {
      return env;
    }
    void setEnv() {
      env = true;
    }

    bool isUsed() const {
      return used;
    }
    void setUsed() {
      used = true;
    }
    void setUnUsed() {
      used = false;
    }

    bool isPrinted() const {
      return printed;
    }
    void setPrinted() {
      printed = true;
    }
    void setUnPrinted() {
      printed = false;
    }

    bool isBool() const {
      if(type == RCCharPtr) {
        if((strcmp(v.CharPtr, "true") == 0) ||
           (strcmp(v.CharPtr, "false") == 0)) {
          return true;
        }
      }

      return type == RCBool;
    }
    bool getBool() const {
      if(type == RCCharPtr && (strcmp(v.CharPtr, "true") == 0))
        return true;
      if(type == RCCharPtr && (strcmp(v.CharPtr, "false") == 0))
        return false;
      
      return v.Bool;
    }

    bool isInt() const {
      return type == RCInt;
    }
	 int32_t getInt() const {
      return v.Int;
    }

    bool isDouble() const {
      return type == RCDouble;
    }
    double getDouble() const {
      return v.Double;
    }
    void setDouble(double val) {
      v.Double = val ;
    }

    bool isCharPtr() const {
      return type == RCCharPtr;
    }
    const char *getCharPtr() const {
      return v.CharPtr;
    }
    void setCharPtr(const char *val) {
      free(v.CharPtr);
      v.CharPtr = strdup(val);
    }

	 int32_t getVectorFirst() const { return X; }
	 int32_t getVectorLast() const { return Y; }
                
    void dump(const char *pre,
              const char *post);
  };

  // Instead of using the default hash function I prefer to use this
  // one. The reason is that STL use tables with prime number
  // length. It's a pretty good one. I got it from Dr.Dobbs Journal
  // December 1996. I use a slightly modified Colin Plumb hash
  // function.
  class HashColin {
  private:
    size_t hash(const string &s, size_t hval) const{
      for(size_t i=0;i<s.size();i++){
        hval += tolower(s[i]);
        hval += (hval << 10);
        hval ^= (hval >> 6);
      }
      hval += (hval << 3);
      hval ^= (hval >> 11);
      hval += (hval << 15);
      return hval;
    }
  public:
    size_t operator()(const KeyIndex & k)const {
      return hash(k.s2,hash(k.s1,0));
    }
  };

  typedef HASH_MULTIMAP< KeyIndex, Record *, HashColin > hashRecord_t;

  const char *envstart;       // Match to add for environment variables

  bool errorReading;
  bool errorFound;
  bool locked;

  FILE *fp;
  const char *fpname;

  hashRecord_t hashRecord;

  bool read_config(void);
  void getSvalue();
  bool getSimplifiedLine(char *line);

  virtual const char *getEnvVar(const char *block,
                                const char *name);

  /* Overwritten by SescConf */
  virtual const Record *getRecord(const char *block,
                                  const char *name,
											 int32_t vectorPos);

  void addRecord(const char *block,
                 const char *name,
                 Record * rec);

public:
  Config(const char *name,
         const char *envstr);
  virtual ~ Config(void);

  void notCorrect();

  void addRecord(const char *block,
                 const char *name,
                 const char *val);
  void addVRecord(const char *block,
                  const char *name,
                  const char *val,
		  int32_t X,
		  int32_t Y);

  void addVRecord(const char *block,
                  const char *name,
                  double val,
		  int32_t X,
		  int32_t Y);

  void addVRecord(const char *block,
                  const char *name,
                  int32_t val,
		  int32_t X,
		  int32_t Y);

  void addRecord(const char *block,
                 const char *name,
                 bool val);
  void addRecord(const char *block,
                 const char *name,
		 int32_t val);
  void addRecord(const char *block,
                 const char *name,
                 double val);

  void copyVariable(const char *block,
                    const char *name,
                    const char *val);

  
  bool getBool(const char *block,
               const char *name,
					int32_t vectorPos=0);
  double getDouble(const char *block,
                   const char *name,
						 int32_t vectorPos=0);
  int32_t getInt(const char *block,
               const char *name,
					int32_t vectorPos=0);
  const char *getCharPtr(const char *block,
                         const char *name,
								 int32_t vectorPos=0);

  // checking functions
  bool checkBool(const char *block,
               const char *name,
					int32_t vectorPos=0);
  bool checkDouble(const char *block,
                   const char *name,
						 int32_t vectorPos=0);
  bool checkInt(const char *block,
                   const char *name,
						 int32_t vectorPos=0);
  bool checkCharPtr(const char *block,
                   const char *name,
						 int32_t vectorPos=0);

  virtual ssize_t getRecordMin(const char *block, const char *name) const;
  virtual ssize_t getRecordMax(const char *block, const char *name) const;

  int32_t getRecordSize(const char *block, const char *name) const {
    return getRecordMax(block,name)-getRecordMin(block,name)+1;
  }

  void updateRecord(const char *block, const char *name, double v, int32_t vpos=0);
  void updateRecord(const char *block, const char *name, const char *val, int32_t vpos=0);
  void getAllSections(std::vector<char *>& sections);

  void lock();

  // Restrictions enforced if the record exists. When a record does
  // not exists the restriction always returns true. Unless the
  // resitriction is isRequired(...)

  bool isPower2(const char *block,
                const char *name,
					 int32_t vectorPos=0);
  bool isBetween(const char *block,
                 const char *name,
                 double llim,
                 double ulim,
					  int32_t vectorPos=0); // llim <= v <= ulim; or v in [llim, ulim]
  bool isGT(const char *block,
            const char *name,
            double val,
				int32_t vectorPos=0);      // Greater Than
  bool isLT(const char *block,
            const char *name,
            double val,
				int32_t vectorPos=0);      // Less Than
  bool isBool(const char *block,
              const char *name,
				  int32_t vectorPos=0);
  bool isInt(const char *block,
              const char *name,
				  int32_t vectorPos=0);
  bool isDouble(const char *block,
                const char *name,
					 int32_t vectorPos=0);
  bool isCharPtr(const char *block,
                 const char *name,
					  int32_t vectorPos=0);


  bool isInList(const char *block,
                const char *name,
                const char *l1,
                const char *l2,
                const char *l3,
                const char *l4,
                const char *l5,
                const char *l6,
                const char *l7,
					 const int32_t vectorPos=0);

  bool isInList(const char *block,
                const char *name,
                const char *l1,
                const char *l2,
                const char *l3,
                const char *l4,
                const char *l5,
                const char *l6,
					 const int32_t vectorPos=0) {
    return isInList(block,name,l1,l2,l3,l4,l5,l6,0,vectorPos);
  }
    
  bool isInList(const char *block,
                const char *name,
                const char *l1,
					 int32_t vectorPos=0) {
    return isInList(block,name,l1,0,0,0,0,0,0,vectorPos);
  }
  bool isInList(const char *block,
                const char *name,
                const char *l1,
                const char *l2,
					 int32_t vectorPos=0) {
    return isInList(block,name,l1,l2,0,0,0,0,0,vectorPos);
  }
  bool isInList(const char *block,
                const char *name,
                const char *l1,
                const char *l2,
                const char *l3,
					 int32_t vectorPos=0) {
    return isInList(block,name,l1,l2,l3,0,0,0,0,vectorPos);
  }
  bool isInList(const char *block,
                const char *name,
                const char *l1,
                const char *l2,
                const char *l3,
                const char *l4,
					 int32_t vectorPos=0) {
    return isInList(block,name,l1,l2,l3,l4,0,0,0,vectorPos);
  }
  bool isInList(const char *block,
                const char *name,
                const char *l1,
                const char *l2,
                const char *l3,
                const char *l4,
                const char *l5,
					 int32_t vectorPos=0) {
    return isInList(block,name,l1,l2,l3,l4,l5,0,0,vectorPos);
  }
  void dump(bool showAll = false);
};


extern Config *Conf;            // Defined in interface.cpp

#endif   // CONFIGCLASS_H
