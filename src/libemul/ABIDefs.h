#if !(defined _ABI_DEFS_H_)
#define _ABI_DEFS_H_

template<ExecMode mode>
class ABIDefs{
};

#if (defined SUPPORT_MIPS32)
#include "ABIDefsMips32.h"
#endif
#if (defined SUPPORT_MIPSEL32)
#include "ABIDefsMipsel32.h"
#endif
#if (defined SUPPORT_MIPS64)
#include "ABIDefsMips64.h"
#endif
#if (defined SUPPORT_MIPSEL64)
#include "ABIDefsMipsel64.h"
#endif

#endif // !(defined _ABI_DEFS_H_)
