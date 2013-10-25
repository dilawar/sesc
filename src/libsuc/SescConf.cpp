/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Basilio Fraguela
                  Luis Ceze
                  Smruti Sarangi
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

#include <ctype.h>
#include <string.h>

#include "Events.h"
#include "SescConf.h"

SConfig *SescConf=0;

#define isalnum_or_(p) ((p)=='_' || isalnum(p))
#define jump_spaces(p) {while (*(p)==' ' || *(p)=='\t') (p)++;}

/* Aux func */

static char *auxstrndup(const char *source, int32_t len)
{ 
  char *q;

  q=new char [len + 1];
  strncpy(q, source, len);
  q[len]=0;
  return q;
}


/* END Aux func */


SConfig::SConfig(const char *name)
  :Config(name ? name : (getenv("SESCCONF") ? getenv("SESCCONF") : "sesc.conf"), "SESC")
{
}

const char *SConfig::getEnvVar(const char *block,
                               const char *name)
{
  const char *val = Config::getEnvVar("", name);

  if(val)
    return val;

  return Config::getEnvVar(block, name);
}

const SConfig::Record * SConfig::getRecord(const char *block,
                                           const char *name,
					   int32_t vectorPos)
{
  const Record *rec = Config::getRecord(block, name, vectorPos);
  if(rec)
    return rec;
  
  // Use Indirection when neither of [block]name or []name exists. Indirection
  // can not handle vector inside a block.

  rec = Config::getRecord("", block, vectorPos);

  if(rec == 0)
    return Config::getRecord(block, name, vectorPos);

  const char *secName = rec->getCharPtr();

  if(secName == 0) {
    return rec;
  }

  return Config::getRecord(secName, name, 0);
}

std::vector<char *> SConfig::getSplitCharPtr(const char *block,
                                             const char *name,
                                             int32_t vectorPos)
{ 
  std::vector<char *> vRes;
  const char *q;

  const char *source=getCharPtr(block, name, vectorPos);

  jump_spaces(source);
  while (*source) {
    for(q=source; isalnum_or_(*q); q++);
    if (source==q) break; // May be an error could be yielded
    vRes.push_back(auxstrndup(source, q-source));
    jump_spaces(q);
    source=q;
  }

  return vRes;
}

