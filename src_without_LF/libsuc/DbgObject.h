#if !(defined _DBGOBJECT_H_)
#define _DBGOBJECT_H_
#include "nanassert.h"

typedef void _magic_type(void);

template<class T>
class DbgObject{
 private:
  static void _magic(void){
  }
#if (defined DEBUG)
  _magic_type *magic;
#endif
 protected:
  DbgObject(void)
#if (defined DEBUG)
  : magic(&_magic)
#endif
  {
  }
  ~DbgObject(void){
    I(magic==&_magic);
#if (defined DEBUG)
    magic=0;
#endif
  }
 public:
  void debug() const{
    I(magic==&_magic);
  }
};
#endif // !(defined _DBGOBJECT_H_)
