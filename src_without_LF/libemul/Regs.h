#if !(defined REGS_H)
#define REGS_H

#include <stdint.h>
#include <stddef.h>

enum RegNameEnum{
  RegNumMask  = 0x0FF, // Mask for register number

  RegTypeMask = 0xF00, // Mask for determining the type of a register
  RegTypeGpr  = 0x000, // General-purpose (integer) registers
  RegTypeFpr  = 0x100, // Floating point registers
  RegTypeCtl  = 0x200, // Control registers
  RegTypeSpc  = 0x300, // Special registers
  // Not a register name, it is the maximum number of all registers
  NumOfRegs   = RegTypeSpc+RegNumMask+1,
  // Placeholder for when the instruction has no register operand
  // Note that there is no actual register with this name
  RegNone,
  // Used when we don't know the type of the register and must check
  // Note that there are no actual registers of this type
  RegDyn
};
typedef uint16_t RegName;

inline bool isGprName(RegName reg){ return (reg&RegTypeMask)==RegTypeGpr; }
inline bool isFprName(RegName reg){ return (reg&RegTypeMask)==RegTypeFpr; }
inline bool isCtlName(RegName reg){ return (reg&RegTypeMask)==RegTypeCtl; }
inline bool isSpcName(RegName reg){ return (reg&RegTypeMask)==RegTypeSpc; }
inline bool isNotName(RegName reg){ return (reg&RegTypeMask)>=NumOfRegs;  }

inline size_t  getRegNum(RegName reg){ return (reg&RegNumMask); }
inline RegName getRegType(RegName reg){ return (RegName)(reg&RegTypeMask); }

typedef int64_t RegVal;

#endif // !(defined REGS_H)
