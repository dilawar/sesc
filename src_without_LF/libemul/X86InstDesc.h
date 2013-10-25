#include "InstDesc.h"

namespace X86{
  void decodeInstSize(ThreadContext *context, VAddr funcAddr, VAddr &curAddr, VAddr endAddr, size_t &tsize, bool domap);
  void decodeInst(ThreadContext *context, VAddr funcAddr, VAddr &curAddr, VAddr endAddr, InstDesc *&trace, bool domap);
}
