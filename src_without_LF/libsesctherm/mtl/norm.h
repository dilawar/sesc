//===========================================================================
//---------------------------------------------------------------------------
//
// DESCRIPTION
//
//---------------------------------------------------------------------------
//
// LICENSE AGREEMENT
// Software License for MTL
//
// Copyright (c) 2001-2005 The Trustees of Indiana University. All rights reserved.
// Copyright (c) 1998-2001 University of Notre Dame. All rights reserved.
// Authors: Andrew Lumsdaine, Jeremy G. Siek, Lie-Quan Lee
//
// This file is part of the Matrix Template Library
//
// See also license.mtl.txt in the distribution.
//
// REVISION HISTORY:
//
// $Log: norm.h,v $
// Revision 1.2  2008/01/30 05:49:55  jrenau
// Sync for OS X and sesctherm
//
// Revision 1.1  2008/01/30 05:33:21  jrenau
// Sync for OS X and sesctherm
//
// Revision 1.1  2000/07/12 21:51:46  jsiek
// Initial revision
//
// Revision 1.4  1999/08/30 17:49:12  jsiek
// changed config.h to mtl_config.h
//
// Revision 1.3  1999/06/10 15:29:04  jsiek
// bunch of changes
//
// Revision 1.1.1.1  1999/06/04 16:58:31  jeremys
// Import MTL
//
// Revision 1.2  1999/03/05 16:13:36  jsiek
// config
//
// Revision 1.1  1999/01/16 14:33:51  jsiek
// *** empty log message ***
//
// Revision 1.4  1998/10/07 16:13:17  jsiek
// asdf
//
// Revision 1.3  1998/08/21 19:51:21  llee1
// it was horrible that I forget to make it inlining
//
// Revision 1.2  1998/08/13 00:35:35  jsiek
// use MTL_NAMESPACE
//
// Revision 1.1  1998/08/12 20:55:54  llee1
// for real type
//
//
//===========================================================================
#ifndef REAL_NORM_H
#define REAL_NORM_H

#include "mtl_config.h"

namespace std {

  template <class T>
  inline T norm(const T& a)
  {
    return a * a;
  }

} /* namespace std */

#endif
