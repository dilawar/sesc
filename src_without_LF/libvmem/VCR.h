/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Karin Strauss
		  Luis Ceze

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

#ifndef VCR_H
#define VCR_H

// Version Combine Register: The name is inspired on Milos paper about
// TLS scalability. Nevertheless, the implementation is slightly
// different.
//
// This class enforces dependences

#include <set>

#include "estl.h"
#include "nanassert.h"
#include "pool.h"
#include "VMemState.h"

class VMemWriteReq;

class VCREntry {
private:
  static pool<VCREntry> rPool;
  friend class pool<VCREntry>;

protected:
  VMemState state;
  HVersion  *version;

public:

  static VCREntry *create(const VMemState *state, HVersion *version);
  void destroy();
  
  VMemState *getState() { return &state; }
  const HVersion *getVersionRef() const { I(version); return version; }

  bool operator()(const VCREntry *x, const VCREntry *y) const {
    return *(x->version) > *(y->version);
  }
};

class VCR {
 private:
  class VMemWriteReqHash {
  public: 
    size_t operator()(const VMemWriteReq *v) const {
      size_t val = (size_t)v;
      return val>>2;
    }
  };

  typedef std::set<VCREntry *, VCREntry> VCRType;
  typedef HASH_MAP<const VMemWriteReq *, VCRType *, VMemWriteReqHash> VCRMap;
  
  static pool<VCRType> vcrPool;
  friend class pool<VCRType>;

  VCRMap vcrMap;

  void clearVCRMap(VCRType *versionList);
 protected:
 public:
  void createCheck(VMemWriteReq *req);
  void addCheck(const VMemWriteReq *oreq, const VMemWriteReq *vreq);
  bool performCheck(const VMemWriteReq *oreq);
};



#endif // VCR_H
