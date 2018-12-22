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
  ErrNum_UselessDisp = 0,
  ErrNum_ShortAddrPossible = 10,
  ErrNum_ShortJumpPossible = 20,
  ErrNum_NoShareFile = 30,
  ErrNum_BigDecFloat = 40,
  ErrNum_PrivOrder = 50,
  ErrNum_DistNull = 60,
  ErrNum_WrongSegment = 70,
  ErrNum_InAccSegment = 75,
  ErrNum_PhaseErr = 80,
  ErrNum_Overlap = 90,
  ErrNum_NoCaseHit = 100,
  ErrNum_InAccPage = 110,
  ErrNum_RMustBeEven = 120,
  ErrNum_Obsolete = 130,
  ErrNum_Unpredictable = 140,
  ErrNum_AlphaNoSense = 150,
  ErrNum_Senseless = 160,
  ErrNum_RepassUnknown = 170,
  ErrNum_AddrNotAligned = 180,
  ErrNum_IOAddrNotAllowed = 190,
  ErrNum_Pipeline = 200,
  ErrNum_DoubleAdrRegUse = 210,
  ErrNum_NotBitAddressable = 220,
  ErrNum_StackNotEmpty = 230,
  ErrNum_NULCharacter = 240,
  ErrNum_PageCrossing = 250,
  ErrNum_WOverRange = 260,
  ErrNum_NegDUP = 270,
  ErrNum_ConvIndX = 280,
  ErrNum_NullResMem = 290,
  ErrNum_BitNumberTruncated = 300,
  ErrNum_InvRegisterPointer = 310,
  ErrNum_MacArgRedef = 320,
  ErrNum_Deprecated = 330,
  ErrNum_SrcLEThanDest = 340,
  ErrNum_TrapValidInstruction = 350,
  ErrNum_DoubleDef = 1000,
  ErrNum_SymbolUndef = 1010,
  ErrNum_InvSymName = 1020,
  ErrNum_InvFormat = 1090,
  ErrNum_UseLessAttr = 1100,
  ErrNum_TooLongAttr = 1105,
  ErrNum_UndefAttr = 1107,
  ErrNum_WrongArgCnt = 1110,
  ErrNum_CannotSplitArg = 1112,
  ErrNum_WrongOptCnt = 1115,
  ErrNum_OnlyImmAddr = 1120,
  ErrNum_InvOpsize = 1130,
  ErrNum_ConfOpSizes = 1131,
  ErrNum_UndefOpSizes = 1132,
  ErrNum_InvOpSize = 1130,
  ErrNum_InvOpType = 1135,
  ErrNum_TooManyArgs = 1140,
  ErrNum_NoRelocs = 1150,
  ErrNum_UnresRelocs = 1155,
  ErrNum_Unexportable = 1156,
  ErrNum_UnknownInstruction = 1200,
  ErrNum_BrackErr = 1300,
  ErrNum_DivByZero = 1310,
  ErrNum_UnderRange = 1315,
  ErrNum_OverRange = 1320,
  ErrNum_NotAligned = 1325,
  ErrNum_DistTooBig = 1330,
  ErrNum_InAccReg = 1335,
  ErrNum_NoShortAddr = 1340,
  ErrNum_InvAddrMode = 1350,
  ErrNum_MustBeEven = 1351,
  ErrNum_InvParAddrMode = 1355,
  ErrNum_UndefCond = 1360,
  ErrNum_IncompCond = 1365,
  ErrNum_JmpDistTooBig = 1370,
  ErrNum_DistIsOdd = 1375,
  ErrNum_InvShiftArg = 1380,
  ErrNum_Range18 = 1390,
  ErrNum_ShiftCntTooBig = 1400,
  ErrNum_InvRegList = 1410,
  ErrNum_InvCmpMode = 1420,
  ErrNum_InvCPUType = 1430,
  ErrNum_InvCtrlReg = 1440,
  ErrNum_InvReg = 1445,
  ErrNum_DoubleReg = 1446,
  ErrNum_NoSaveFrame = 1450,
  ErrNum_NoRestoreFrame = 1460,
  ErrNum_UnknownMacArg = 1465,
  ErrNum_MissEndif = 1470,
  ErrNum_InvIfConst = 1480,
  ErrNum_DoubleSection = 1483,
  ErrNum_InvSection = 1484,
  ErrNum_MissingEndSect = 1485,
  ErrNum_WrongEndSect = 1486,
  ErrNum_NotInSection = 1487,
  ErrNum_UndefdForward = 1488,
  ErrNum_ContForward = 1489,
  ErrNum_InvFuncArgCnt = 1490,
  ErrNum_MsgMissingLTORG = 1495,
  ErrNum_InstructionNotSupported = 1500,
  ErrNum_FPUNotEnabled = 1501,
  ErrNum_PMMUNotEnabled = 1502,
  ErrNum_FullPMMUNotEnabled = 1503, 
  ErrNum_Z80SyntaxNotEnabled = 1504,
  ErrNum_AddrModeNotSupported = 1505,
  ErrNum_InvBitPos = 1510,
  ErrNum_OnlyOnOff = 1520,
  ErrNum_StackEmpty = 1530,
  ErrNum_NotOneBit = 1540,
  ErrNum_MissingStruct = 1550,
  ErrNum_OpenStruct = 1551,
  ErrNum_WrongStruct = 1552,
  ErrNum_PhaseDisallowed = 1553,
  ErrNum_InvStructDir = 1554,
  ErrNum_DoubleStruct = 1555,
  ErrNum_UnresolvedStructRef = 1556,
  ErrNum_NotRepeatable = 1560,
  ErrNum_ShortRead = 1600,
  ErrNum_UnknownCodepage = 1610,
  ErrNum_RomOffs063 = 1700,
  ErrNum_InvFCode = 1710,
  ErrNum_InvFMask = 1720,
  ErrNum_InvMMUReg = 1730,
  ErrNum_Level07 = 1740,
  ErrNum_InvBitMask = 1750,
  ErrNum_InvRegPair = 1760,
  ErrNum_OpenMacro = 1800,
  ErrNum_OpenIRP = 1801,
  ErrNum_OpenIRPC = 1802,
  ErrNum_OpenREPT = 1803,
  ErrNum_OpenWHILE = 1804,
  ErrNum_EXITMOutsideMacro = 1805,
  ErrNum_TooManyMacParams = 1810,
  ErrNum_UndefKeyArg = 1811,
  ErrNum_NoPosArg = 1812,
  ErrNum_DoubleMacro = 1815,
  ErrNum_FirstPassCalc = 1820,
  ErrNum_TooManyNestedIfs = 1830,
  ErrNum_MissingIf = 1840,
  ErrNum_RekMacro = 1850,
  ErrNum_UnknownFunc = 1860,
  ErrNum_InvFuncArg = 1870,
  ErrNum_FloatOverflow = 1880,
  ErrNum_InvArgPair = 1890,
  ErrNum_NotOnThisAddress = 1900,
  ErrNum_NotFromThisAddress = 1905,
  ErrNum_TargOnDiffPage = 1910,
  ErrNum_CodeOverflow = 1920,
  ErrNum_AdrOverflow = 1925,
  ErrNum_MixDBDS = 1930,
  ErrNum_NotInStruct = 1940,
  ErrNum_ParNotPossible = 1950,
  ErrNum_InvSegment = 1960,
  ErrNum_UnknownSegment = 1961,
  ErrNum_UnknownSegReg = 1962,
  ErrNum_InvString = 1970,
  ErrNum_InvRegName = 1980,
  ErrNum_InvArg = 1985,
  ErrNum_NoIndir = 1990,
  ErrNum_NotInThisSegment = 1995,
  ErrNum_NotInMaxmode = 1996,
  ErrNum_OnlyInMaxmode = 1997,
  ErrNum_PackCrossBoundary = 2000,
  ErrNum_UnitMultipleUsed = 2001,
  ErrNum_MultipleLongRead = 2002,
  ErrNum_MultipleLongWrite = 2003,
  ErrNum_LongReadWithStore = 2004,
  ErrNum_TooManyRegisterReads = 2005,
  ErrNum_OverlapDests = 2006,
  ErrNum_TooManyBranchesInExPacket = 2008,
  ErrNum_CannotUseUnit = 2009,
  ErrNum_InvEscSequence = 2010,
  ErrNum_InvPrefixCombination = 2020,
  ErrNum_ConstantRedefinedAsVariable = 2030,
  ErrNum_VariableRedefinedAsConstant = 2035,
  ErrNum_StructNameMissing = 2040,
  ErrNum_EmptyArgument = 2050,
  ErrNum_Unimplemented = 2060,
  ErrNum_FreestandingUnnamedStruct = 2070,
  ErrNum_STRUCTEndedByENDUNION = 2080,
  ErrNum_AddrOnDifferentPage = 2090,
  ErrNum_UnknownMacExpMod = 2100,
  ErrNum_ConflictingMacExpMod = 2110,
  ErrNum_InvalidPrepDir = 2120,
  ErrNum_InternalError = 10000,
  ErrNum_OpeningFile = 10001,
  ErrNum_ListWrError = 10002,
  ErrNum_FileReadError = 10003,
  ErrNum_FileWriteError = 10004,
  ErrNum_HeapOvfl = 10006,
  ErrNum_StackOvfl = 10007,
} tErrorNum;

struct sLineComp;

extern Boolean ChkRange(LargeInt Value, LargeInt Min, LargeInt Max);

extern Boolean ChkArgCntExtPos(int ThisCnt, int MinCnt, int MaxCnt, const struct sLineComp *pComp);
#define ChkArgCnt(MinCnt, MaxCnt) ChkArgCntExtPos(ArgCnt, MinCnt, MaxCnt, NULL)
#define ChkArgCntExt(ThisCnt, MinCnt, MaxCnt) ChkArgCntExtPos(ThisCnt, MinCnt, MaxCnt, NULL)
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

extern Boolean ChkSamePage(LargeWord Addr1, LargeWord Addr2, unsigned PageBits);

#endif /* _ERRMSG_H */
