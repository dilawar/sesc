/*
* ========== Copyright Header Begin ==========================================
* 
* OpenSPARC T1 Processor File: rstzip2if.cpp
* Copyright (c) 2006 Sun Microsystems, Inc.  All Rights Reserved.
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES.
* 
* The above named program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public
* License version 2 as published by the Free Software Foundation.
* 
* The above named program is distributed in the hope that it will be 
* useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* General Public License for more details.
* 
* You should have received a copy of the GNU General Public
* License along with this work; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
* 
* ========== Copyright Header End ============================================
*/

#include <assert.h>

#include "Rstzipi.h"
#include "rstzip2if.h"
#include "rstzipif.h"

// C++ rstzip2 (de)compression class.

Rstzip2if::Rstzip2if() {
  rstzip = new RstzipIF;
  assert(rstzip != NULL);
}

Rstzip2if::~Rstzip2if() {
  delete rstzip;
}

// ==== Version routines ====

int32_t Rstzip2if::getMajorVersion() {
  return RSTZIP_MAJOR_VERSION;
}

int32_t Rstzip2if::getMinorVersion() {
  return RSTZIP_MINOR_VERSION;
}

// ==== Compression routines ====

int32_t Rstzip2if::openRstzip(const char* outfile, int32_t buffersize, int32_t gzip, int32_t stats, int32_t numcpus) {
  return rstzip->openRstzip(outfile, buffersize, gzip, stats, numcpus);
}

int32_t Rstzip2if::compress(rstf_unionT* rstbuf, int32_t nrecs) {
  return rstzip->compress(rstbuf, nrecs);
}

void Rstzip2if::closeRstzip() {
  rstzip->closeRstzip();
}

// ==== Decompression routines ====

int32_t Rstzip2if::openRstunzip(const char* infile, int32_t buffersize, int32_t gzip, int32_t stats) {
  return rstzip->openRstunzip(infile, buffersize, gzip, stats);
}

int32_t Rstzip2if::decompress(rstf_unionT* rstbuf, int32_t nrecs) {
  return rstzip->decompress(rstbuf, nrecs);
}

void Rstzip2if::closeRstunzip() {
  rstzip->closeRstunzip();
}

// C wrapper prototypes.

int32_t rz2_getMajorVersion(Rstzip2if* rstzip) {
  return rstzip->getMajorVersion();
}

int32_t rz2_getMinorVersion(Rstzip2if* rstzip) {
  return rstzip->getMinorVersion();
}

Rstzip2if* rz2_openRstzip(char* outfile, int32_t buffersize, int32_t gzip, int32_t stats, int32_t numcpus) {
  Rstzip2if* rstzip = new Rstzip2if;

  assert(rstzip != NULL);
  rstzip->openRstzip(outfile, buffersize, gzip, stats, numcpus);

  return rstzip;
}

int32_t rz2_compress(Rstzip2if* rstzip, rstf_unionT* rstbuf, int32_t nrecs) {
  return rstzip->compress(rstbuf, nrecs);
}

void rz2_closeRstzip(Rstzip2if* rstzip) {
  rstzip->closeRstzip();
  delete rstzip;
}

Rstzip2if* rz2_openRstunzip(char* infile, int32_t buffersize, int32_t gzip, int32_t stats) {
  Rstzip2if* rstunzip = new Rstzip2if;

  assert(rstunzip != NULL);
  rstunzip->openRstunzip(infile, buffersize, gzip, stats);

  return rstunzip;
}

int32_t rz2_decompress(Rstzip2if* rstunzip, rstf_unionT* rstbuf, int32_t nrecs) {
  return rstunzip->decompress(rstbuf, nrecs);
}

void rz2_closeRstunzip(Rstzip2if* rstunzip) {
  rstunzip->closeRstunzip();
  delete rstunzip;
}
