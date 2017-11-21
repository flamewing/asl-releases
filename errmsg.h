#ifndef _ERRMSG_H
#define _ERRMSG_H
/* errmsg.h */
/*****************************************************************************
 * Cross Assembler
 *
 * Error message definition & associated checking
 *
 *****************************************************************************/

#include "cpulist.h"
#include "datatypes.h"

typedef enum
{
  ErrNum_WrongArgCnt = 1110,
  ErrNum_CannotSplitArg = 1112,
  ErrNum_InvOpSize = 1130,
  ErrNum_InstructionNotSupported = 1500,
  ErrNum_FPUNotEnabled = 1501,
  ErrNum_PMMUNotEnabled = 1502,
  ErrNum_FullPMMUNotEnabled = 1503, 
  ErrNum_Z80SyntaxNotEnabled = 1504,
  ErrNum_AddrModeNotSupported = 1505,
  ErrNum_OpenMacro = 1800,
  ErrNum_OpenIRP = 1801,
  ErrNum_OpenIRPC = 1802,
  ErrNum_OpenREPT = 1803,
  ErrNum_OpenWHILE = 1804,
} tErrorNum;

extern Boolean ChkRange(LargeInt Value, LargeInt Min, LargeInt Max);

extern Boolean ChkArgCntExt(int ThisCnt, int MinCnt, int MaxCnt);
#define ChkArgCnt(MinCnt, MaxCnt) ChkArgCntExt(ArgCnt, MinCnt, MaxCnt)
extern Boolean ChkArgCntExtEitherOr(int ThisCnt, int EitherCnt, int OrCnt);

extern Boolean ChkMinCPUExt(CPUVar MinCPU, tErrorNum ErrorNum);
#define ChkMinCPU(MinCPU) ChkMinCPUExt(MinCPU, ErrNum_InstructionNotSupported)

extern Boolean ChkMaxCPUExt(CPUVar MaxCPU, tErrorNum ErrorNum);
#define ChkMaxCPU(MaxCPU) ChkMaxCPUExt(MaxCPU, ErrNum_InstructionNotSupported)

extern Boolean ChkExactCPUExt(CPUVar CheckCPU, tErrorNum ErrorNum);
#define ChkExactCPU(CheckCPU) ChkExactCPUExt(CheckCPU, ErrNum_InstructionNotSupported)

extern Boolean ChkRangeCPUExt(CPUVar MinCPU, CPUVar MaxCPU, tErrorNum ErrorNum);
#define ChkRangeCPU(MinCPU, MaxCPU) ChkRangeCPUExt(MinCPU, MaxCPU, ErrNum_InstructionNotSupported)

extern Boolean ChkExcludeCPUExt(CPUVar CheckCPU, tErrorNum ErrorNum);
#define ChkExcludeCPU(CheckCPU) ChkExcludeCPUExt(CheckCPU, ErrNum_InstructionNotSupported)

extern int ChkExactCPUList(tErrorNum ErrorNum, ...);
extern int ChkExcludeCPUList(tErrorNum ErrorNum, ...);

extern int ChkExactCPUMaskExt(Word CPUMask, CPUVar FirstCPU, tErrorNum ErrorNum);
#define ChkExactCPUMask(CPUMask, FirstCPU) ChkExactCPUMaskExt(CPUMask, FirstCPU, ErrNum_InstructionNotSupported)

#endif /* _ERRMSG_H */
