/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2006 University California, Santa Cruz.

   Contributed by Saangetha
                  Keertika
                  Jose Renau

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


#ifndef RST_READER_H
#define RST_READER_H

#include "DInst.h"
#include "rstf.h"
#include "Rstzip.h"

class RSTReader {
 private:
  const int32_t Max_Num_Recs;
  const int32_t Max_Head_Size;

  Rstzip *rz;
  
  int32_t nFlows;
  DInst *head;
  char *head_size;

  // Decompressed buffer
  bool end_of_trace;
  int32_t  buf_pos;
  int32_t  buf_end;
  rstf_unionT *buf;

  void addInstruction(const rstf_unionT *rp);

  void advancePC(int32_t fid);

 public:
  RSTReader();

  void openTrace(const char* basename);
  void closeTrace();

  DInst *executePC(int32_t fid);
  VAddr currentPC(int32_t fid) const {
    return head_size[fid] ? 
      head[fid].getFirstPending()->getInst()->getAddr(): 
      0xffffffff ;
  }

  bool hasWork(int32_t fid) const { return head_size[fid] != 0; }
};

#endif
