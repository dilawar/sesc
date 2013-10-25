/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Luis Ceze
                  Liu Wei

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

#include "ValueTable.h"

ValueTable::PredEntry::VTable ValueTable::PredEntry::table;

GStatsCntr *ValueTable::putCntr=0;
GStatsCntr *ValueTable::getCntr=0;

GStatsCntr *ValueTable::goodCntr=0;
GStatsCntr *ValueTable::badCntr =0;

GStatsCntr *ValueTable::verifyGoodCntr=0;
GStatsCntr *ValueTable::verifyBadCntr =0;


ValueTable::PredEntry::PredEntry(int32_t i) 
  : id(i)
  ,goodCntr("ValueTable:good_%d",i)
  ,badCntr("ValueTable:bad_%d" ,i)
{
  pred = 0;
  curr = 0;
  stride = 0;
  confidence = 0;
}

ValueTable::PredEntry *ValueTable::PredEntry::getEntry(int32_t i)
{
  VTable::iterator it = table.find(i);

  if (it != table.end())
    return it->second;

  PredEntry *pe = new PredEntry(i);
  table[i] = pe;

  return pe;
}

ValueTable::ValueTable()
{
  I(0);
}

// TODO: move good./bad to PredEntry. Report per id

void ValueTable::boot() 
{
  putCntr = new GStatsCntr("ValueTable:put");
  getCntr = new GStatsCntr("ValueTable:get");

  goodCntr = new GStatsCntr("ValueTable:good");
  badCntr  = new GStatsCntr("ValueTable:bad");

  verifyGoodCntr = new GStatsCntr("ValueTable:verifyGood");
  verifyBadCntr  = new GStatsCntr("ValueTable:verifyBad");
}

int32_t ValueTable::readLVPredictor(int32_t id) 
{
  getCntr->inc();

  PredEntry *e = PredEntry::getEntry(id);
  I(e);

  return e->curr;
}

void ValueTable::updateLVPredictor(int32_t id, int32_t value) 
{
  putCntr->inc();

  PredEntry *pe = PredEntry::getEntry(id);

  // update confidence
  if (pe->curr == value) {
    
    //    MSG("LV Good %d:%d::",id, value);

    pe->good();
    goodCntr->inc();
  } else {

    //    MSG("LV bad %d:%d:%d:",id, value, pe->curr);

    pe->bad();
    badCntr->inc();

    pe->curr = value;
  }
}

int32_t ValueTable::readSVPredictor(int32_t id) 
{
  getCntr->inc();

  PredEntry *pe = PredEntry::getEntry(id);

  pe->pred += pe->stride;
  return pe->pred;
}

void ValueTable::updateSVPredictor(int32_t id, int32_t value)
{
  putCntr->inc();

  PredEntry *pe = PredEntry::getEntry(id);

  if (value - pe->curr == pe->stride) {
    // correct stride prediction

    //    MSG("ST Good %d:%d:",id, value);

    pe->curr = value;

    pe->good();
    goodCntr->inc();
  } else {
    // wrong stride prediction

    //    MSG("ST Bad %d:%d:%d:",id, value, pe->curr + pe->stride);

    if (pe->isBad())
      pe->stride = value - pe->curr;
    pe->curr = value;
    pe->pred = value;
    pe->bad();
    badCntr->inc();
  }
}

int32_t ValueTable::readIncrPredictor(int32_t id, int32_t lvalue) 
{
  getCntr->inc();

  PredEntry *pe = PredEntry::getEntry(id);

  return pe->stride + lvalue;
}

void ValueTable::updateIncrPredictor(int32_t id, int32_t value) 
{
  putCntr->inc();

  PredEntry *pe = PredEntry::getEntry(id);
    
  // update confidence
  if (pe->stride == value) {

    //    MSG("INC Good %d:%d:",id, value);

    pe->good();
    goodCntr->inc();
  } else {

    //    MSG("INC Bad %d:%d:%d:",id, value, pe->stride);

    pe->bad();
    badCntr->inc(); 
 }
    
  // update value
  if (pe->isBad())
    pe->stride = value;
}

void ValueTable::verifyValue(int32_t rval, int32_t pval)
{
  if (rval == pval)
    goodCntr->inc();
  else
    badCntr->inc();
}
