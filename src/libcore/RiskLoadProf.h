/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2004 University of Illinois.

   Contributed by  Wei Liu
                 
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
#ifndef _RISK_LOAD_PROF_H
#define _RISK_LOAD_PROF_H

#include <stdlib.h>

#include "Instruction.h"

class RiskLoadProf {
private:
  typedef HASH_MAP<uint32_t, int> HashType;

  FILE *fout;
  HashType riskLoads;

public:
  RiskLoadProf();
  ~RiskLoadProf() {}

  void insert(uint32_t ldInst);
  void finalize();
};

#endif
