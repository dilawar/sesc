#ifndef TRACEGEN_H
#define TRACEGEN_H

#include "estl.h"
#include "nanassert.h"

class TraceGen {
 private:
  typedef HASH_MAP<int32_t, char *> IDMap;

  static IDMap idMap;

  static bool tracing;
 protected:

  static char *getText(const char *format, va_list ap);

 public:

  static void add(int32_t id, const char *format, ...);
  
  static void dump(int32_t id);
};

#endif // TRACEGEN_H
