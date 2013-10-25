/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2004 University of Illinois.

   Contributed by Luis Ceze
		  Pablo Montesinos Ortego
		  Paul Sack

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

/* this is the basic interface for trace readers in Sesc */
/* for an example of a reader, look TT6Reader */

#ifndef TRACEREADER_H
#define TRACEREADER_H

#include "TraceEntry.h"

class TraceReader {
 private:
 public:
  TraceReader() {}
  virtual ~TraceReader() {}

  virtual void openTrace(const char* basename) = 0;
  virtual void closeTrace() = 0;

  virtual void fillTraceEntry(TraceEntry *te, int32_t id) = 0;

  virtual bool hasBufferedEntries(int32_t id=-1) { return false; }
};

#endif
