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

#ifndef VALUETABLE_H
#define VALUETABLE_H

#include "estl.h"
#include "nanassert.h"
#include "GStats.h"

class ValueTable {
 private:
  static GStatsCntr *putCntr;
  static GStatsCntr *getCntr;

  static GStatsCntr *goodCntr;
  static GStatsCntr *badCntr;

  static GStatsCntr *verifyGoodCntr;
  static GStatsCntr *verifyBadCntr;

  //TODO: move it to configuration file
  class PredEntry {
  protected:
    const int32_t id;

    GStatsCntr goodCntr;
    GStatsCntr badCntr;

    // Use a hash_map because the compiler does not provide the
    // maximum prediction ID generated.
    typedef HASH_MAP<int32_t, PredEntry*> VTable;
    static VTable table;

    PredEntry(int32_t i);

  public:
    static const int32_t MaxValue = (1<<2)-1;
    int32_t curr;
    int32_t pred;
    int32_t stride;
    int32_t confidence;

    static PredEntry *getEntry(int32_t i);

    void good() {
      goodCntr.inc();
      if (confidence < MaxValue) {
	confidence++;
      }
    }
    void bad() {
      badCntr.inc();
      if (confidence > 0)
	confidence--;
    }
    bool isBad() const {
      return confidence < MaxValue;
    }
  };

  ValueTable();
 public:
  static void boot();

  /* Last Value Predictor:
   *   1 entry used: curr, confidence
   */
  static int32_t readLVPredictor(int32_t id);
  static void updateLVPredictor(int32_t id, int32_t value);

  /* Stride Value Predictor:
   *   3 entries used: curr, stride, confidence
   */
  static int32_t readSVPredictor(int32_t id);
  static void updateSVPredictor(int32_t id, int32_t value);

  /* Increment Predictor:
   *   2 entries used: stride, confidence
   */
  static int32_t readIncrPredictor(int32_t id, int32_t lvalue);
  static void updateIncrPredictor(int32_t id, int32_t value);

  static void verifyValue(int32_t rval, int32_t pval);
};

#endif
