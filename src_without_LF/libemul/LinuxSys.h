#if !(defined LINUXSYS_H)
#define LINUXSYS_H

#include <stddef.h>
#include "Addressing.h"
#include "SignalHandling.h"
//To get ExecMode
#include "ExecMode.h"

class ThreadContext;
class InstDesc;

class LinuxSys{
 public:
  virtual InstDesc *sysCall(ThreadContext *context, InstDesc *inst) = 0;
  // Creates an object of an architecture-specific LinuxSys subclass
  static LinuxSys *create(ExecMode mode);
  // Destroys a Linuxsys object
  virtual ~LinuxSys(void){
  }
  virtual SignalAction handleSignal(ThreadContext *context, SigInfo *sigInfo) const = 0;
  bool handleSignals(ThreadContext *context) const;
  // Initialize the exception handler trampoline code
  virtual void initSystem(ThreadContext *context) const = 0;
  virtual void createStack(ThreadContext *context) const = 0;
  virtual void setProgArgs(ThreadContext *context, int argc, char **argv, int envc, char **envp) const = 0;
  virtual void exitRobustList(ThreadContext *context, VAddr robust_list) = 0;
  virtual void clearChildTid(ThreadContext *context, VAddr &clear_child_tid) = 0;
};

#endif // !(defined LINUXSYS_H)
