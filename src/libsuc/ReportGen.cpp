/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Basilio Fraguela

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


#include <alloca.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>

#include "nanassert.h"
#include "ReportGen.h"

FILE *Report::rfd[MAXREPORTSTACK];
int32_t Report::tos=0;

Report::Report()
{
  rfd[0]=stdout;
  tos=1;
}

void Report::openFile(char *name)
{
  I(tos<MAXREPORTSTACK);

  FILE *ffd;
  
  if(strstr(name, "XXXXXX")) {
    int32_t fd;
    
    fd = mkstemp(name);
    
    ffd = fdopen(fd, "a");
  }else{
    ffd = fopen(name, "a");
  }

  if(ffd == 0) {
    fprintf(stderr, "NANASSERT::REPORT could not open temporal file [%s]\n", name);
    exit(-3);
  }

  rfd[tos++]=ffd;
}

void Report::close()
{
  if( tos ) {
    tos--;
    fclose(rfd[tos]);
  }
}

void Report::field(int32_t fn, const char *format,...)
{
  va_list ap;

  I( fn < tos );
  FILE *ffd = rfd[fn];
  
  va_start(ap, format);

  vfprintf(ffd, format, ap);

  va_end(ap);

  fprintf(ffd, "\n");
}

void Report::field(const char *format, ...)
{
  va_list ap;

  I( tos );
  FILE *ffd = rfd[tos-1];
  
  va_start(ap, format);

  vfprintf(ffd, format, ap);

  va_end(ap);

  fprintf(ffd, "\n");
}

void Report::flush()
{
  if( tos == 0 )
    return;
  
  fflush(rfd[tos-1]);
}


