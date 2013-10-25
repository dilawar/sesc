

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>

#include "TraceGen.h"

TraceGen::IDMap TraceGen::idMap;

bool TraceGen::tracing = true;

// TOFIX: implement a fast/smarter memory allocation

char *TraceGen::getText(const char *format, va_list ap)
{
  char strid[4096];

  vsprintf(strid, format, ap);

  return strdup(strid);
}

void TraceGen::add(int32_t id, const char *format, ...)
{
  if (!tracing)
    return;

  char *str;
  va_list ap;

  va_start(ap, format);
  str = getText(format, ap);
  va_end(ap);

  IDMap::iterator it = idMap.find(id);
  if (it == idMap.end()) {
    idMap[id] = str;
  }else{
    char *join = (char *)malloc(strlen(str) + strlen(it->second)+5);
    sprintf(join, "%s:%s",it->second, str);
    free(str);
    free(it->second);
    it->second = join;
  }
}

void TraceGen::dump(int32_t id)
{
  IDMap::iterator it = idMap.find(id);
  if (it == idMap.end())
    return;
  
  if (tracing)
    fprintf(stderr,"TraceGen:id=%d:%s\n", id, it->second);
  
  free(it->second);
  idMap.erase(it);
}
