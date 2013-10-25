/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau

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

#include "VMemState.h"

ulong  VMemState::lineMask=0;

void VMemState::initialize()
{
  if (lineMask == 0) {
    lineMask = SescConf->getInt("TaskScalar", "bsize");
    lineMask = lineMask-1;
  }
}

void VMemState::promote() 
{
  // Only caches non-dirty can be promoted
  I(wrmask==0);
  xrdmask = 0;
  // mostSpecLine keeps the same value
}

void VMemState::clearState()
{
  xrdmask = 0;
  wrmask  = 0;
  leastSpecLine = false;
  mostSpecLine  = false;
  //msgSerialNumber = 0;
}

void VMemState::copyStateFrom(const VMemState *st)
{
  I(st);

  xrdmask = st->xrdmask;
  wrmask  = st->wrmask;
  mostSpecLine  = st->mostSpecLine;
  leastSpecLine = st->leastSpecLine;
}

void VMemState::combineStateFrom(const VMemState *st)
{
  I(st);
  
  xrdmask |= st->xrdmask;
  wrmask  |= st->wrmask;
  mostSpecLine  |= st->mostSpecLine;
  leastSpecLine |= st->leastSpecLine;
}

void VMemState::forwardStateTo(VMemState *state)
{
  state->mostSpecLine = mostSpecLine;
  mostSpecLine = false; // It can not be the most spec if it forwarded data
}

