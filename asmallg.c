/* asmallg.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* von allen Codegeneratoren benutzte Pseudobefehle                          */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "strutil.h"
#include "stringlists.h"
#include "bpemu.h"
#include "console.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "errmsg.h"
#include "as.h"
#include "as.rsc"
#include "asmpars.h"
#include "asmmac.h"
#include "asmstructs.h"
#include "asmcode.h"
#include "asmrelocs.h"
#include "asmitree.h"
#include "operator.h"
#include "codepseudo.h"
#include "nlmessages.h"
#include "asmallg.h"

#define LEAVE goto func_exit

/*--------------------------------------------------------------------------*/


static PInstTable PseudoTable = NULL, ONOFFTable;

/*--------------------------------------------------------------------------*/

static Boolean DefChkPC(LargeWord Addr)
{
  if (!((1 << ActPC) & ValidSegs))
    return False;
  else
    return (Addr <= SegLimits[ActPC]);
}

/*!------------------------------------------------------------------------
 * \fn     ParseCPUArgs(tStrComp *pArgs, const tCPUArg *pCPUArgs)
 * \brief  parse possible arguments of CPU
 * \param  pArgs arguments set by user (may be NULL)
 * \param  pCPUArgs arguments provided by target (may be NULL)
 * ------------------------------------------------------------------------ */

static void ParseCPUArgs(const tStrComp *pArgs, const tCPUArg *pCPUArgs)
{
  const tCPUArg *pCPUArg;
  char *pNext, *pSep;
  tStrComp Args, Remainder, NameComp, ValueComp;
  LongInt VarValue;
  Boolean OK;
  String ArgStr;

  /* always reset to defaults, also when no user arguments are given */

  if (!pCPUArgs)
    return;
  for (pCPUArg = pCPUArgs; pCPUArg->pName; pCPUArg++)
    *pCPUArg->pValue = pCPUArg->DefValue;

  if (!pArgs || !*pArgs->str.p_str)
    return;
  StrCompMkTemp(&Args, ArgStr, sizeof(ArgStr));
  StrCompCopy(&Args, pArgs);
  do
  {
    pNext = strchr(Args.str.p_str, ':');
    if (pNext)
      StrCompSplitRef(&Args, &Remainder, &Args, pNext);
    pSep = strchr(Args.str.p_str, '=');
    if (!pSep) WrStrErrorPos(ErrNum_ArgValueMissing, &Args);
    else
    {
      StrCompSplitRef(&NameComp, &ValueComp, &Args, pSep);
      KillPrefBlanksStrCompRef(&NameComp); KillPostBlanksStrComp(&NameComp);
      KillPrefBlanksStrCompRef(&ValueComp); KillPostBlanksStrComp(&ValueComp);

      VarValue = EvalStrIntExpression(&ValueComp, Int32, &OK);
      if (OK)
      {
        for (pCPUArg = pCPUArgs; pCPUArg->pName; pCPUArg++)
          if (!as_strcasecmp(NameComp.str.p_str, pCPUArg->pName))
            break;
        if (!pCPUArg->pName) WrStrErrorPos(ErrNum_UnknownArg, &NameComp);
        else if (ChkRange(VarValue, pCPUArg->Min, pCPUArg->Max))
          *pCPUArg->pValue = VarValue;
      }
    }
    if (pNext)
      Args = Remainder;
  }
  while (pNext);
}

static void SetCPUCore(const tCPUDef *pCPUDef, const tStrComp *pCPUArgs)
{
  LargeInt HCPU;
  int Digit, Base;
  const char *pRun;

  tStrComp TmpComp;
  static const char Default_CommentLeadIn[] = { ';', '\0', '\0' };
  String TmpCompStr;
  StrCompMkTemp(&TmpComp, TmpCompStr, sizeof(TmpCompStr));

  strmaxcpy(MomCPUIdent, pCPUDef->Name, sizeof(MomCPUIdent));
  MomCPU = pCPUDef->Orig;
  MomVirtCPU = pCPUDef->Number;
  HCPU = 0;
  Base = 10;
  for (pRun = MomCPUIdent; *pRun; pRun++)
  {
    if (isdigit(*pRun))
      Base = 16;
    Digit = DigitVal(*pRun, Base);
    if (Digit >= 0)
      HCPU = (HCPU << 4) + Digit;
  }

  strmaxcpy(TmpCompStr, MomCPUName, sizeof(TmpCompStr)); EnterIntSymbol(&TmpComp, HCPU, SegNone, True);
  strmaxcpy(TmpCompStr, MomCPUIdentName, sizeof(TmpCompStr)); EnterStringSymbol(&TmpComp, MomCPUIdent, True);

  InternSymbol = Default_InternSymbol;
  IntConstModeIBMNoTerm = False;
  DissectBit = Default_DissectBit;
  DissectReg = NULL;
  QualifyQuote = NULL;
  pPotMonadicOperator = NULL;
  SetIsOccupiedFnc =
  SaveIsOccupiedFnc =
  RestoreIsOccupiedFnc = NULL;
  DecodeAttrPart = NULL;
  SwitchIsOccupied =
  PageIsOccupied =
  ShiftIsOccupied = False;
  ChkPC = DefChkPC;
  ASSUMERecCnt = 0;
  pASSUMERecs = NULL;
  pASSUMEOverride = NULL;
  pCommentLeadIn = Default_CommentLeadIn;
  UnsetCPU();
  strmaxcpy(MomCPUArgs, pCPUArgs ? pCPUArgs->str.p_str : "", STRINGSIZE);

  ParseCPUArgs(pCPUArgs, pCPUDef->pArgs);
  pCPUDef->SwitchProc(pCPUDef->pUserData);

  DontPrint = True;
}

void SetCPUByType(CPUVar NewCPU, const tStrComp *pCPUArgs)
{
  const tCPUDef *pCPUDef;

  pCPUDef = LookupCPUDefByVar(NewCPU);
  if (pCPUDef)
    SetCPUCore(pCPUDef, pCPUArgs);
}

Boolean SetCPUByName(const tStrComp *pName)
{
  const tCPUDef *pCPUDef;

  pCPUDef = LookupCPUDefByName(pName->str.p_str);
  if (!pCPUDef)
    return False;
  else
  {
    int l = strlen(pCPUDef->Name);

    if (pName->str.p_str[l] == ':')
    {
      tStrComp ArgComp;

      StrCompRefRight(&ArgComp, pName, l + 1);
      SetCPUCore(pCPUDef, &ArgComp);
    }
    else
      SetCPUCore(pCPUDef, NULL);
    return True;
  }
}

/*!------------------------------------------------------------------------
 * \fn     UnsetCPU(void)
 * \brief  Cleanups when switching away from a target
 * ------------------------------------------------------------------------ */

void UnsetCPU(void)
{
  if (SwitchFrom)
  {
    ClearONOFF();
    SwitchFrom();
    SwitchFrom = NULL;
  }
}

static void SetNSeg(Byte NSeg)
{
  if ((ActPC != NSeg) || (!PCsUsed[ActPC]))
  {
    ActPC = NSeg;
    if (!PCsUsed[ActPC])
      PCs[ActPC] = SegInits[ActPC];
    PCsUsed[ActPC] = True;
    DontPrint = True;
  }
}

static void IntLine(char *pDest, size_t DestSize, LargeWord Inp, tIntConstMode ThisConstMode)
{
  switch (ThisConstMode)
  {
    case eIntConstModeIntel:
      as_snprintf(pDest, DestSize, "%" PRIx64 "%s", Inp, GetIntConstIntelSuffix(16));
      if (*pDest > '9')
        strmaxprep(pDest, "0", DestSize);
      break;
    case eIntConstModeMoto:
      as_snprintf(pDest, DestSize, "%s%" PRIx64, GetIntConstMotoPrefix(16), Inp);
      break;
    case eIntConstModeC:
      as_snprintf(pDest, DestSize, "0x%" PRIx64, Inp);
      break;
    case eIntConstModeIBM:
      as_snprintf(pDest, DestSize, "x'%" PRIx64 "'", Inp);
      break;
  }
}


static void CodeSECTION(Word Index)
{
  PSaveSection Neu;
  String ExpName;
  UNUSED(Index);

  if (ChkArgCnt(1, 1)
   && ExpandStrSymbol(ExpName, sizeof(ExpName), &ArgStr[1]))
  {
    if (!ChkSymbName(ExpName)) WrStrErrorPos(ErrNum_InvSymName, &ArgStr[1]);
    else if ((PassNo == 1) && (GetSectionHandle(ExpName, False, MomSectionHandle) != -2)) WrError(ErrNum_DoubleSection);
    else
    {
      Neu = (PSaveSection) malloc(sizeof(TSaveSection));
      Neu->Next = SectionStack;
      Neu->Handle = MomSectionHandle;
      Neu->LocSyms = NULL;
      Neu->GlobSyms = NULL;
      Neu->ExportSyms = NULL;
      SetMomSection(GetSectionHandle(ExpName, True, MomSectionHandle));
      SectionStack = Neu;
    }
  }
}


static void CodeENDSECTION_ChkEmptList(PForwardSymbol *Root)
{
  PForwardSymbol Tmp;
  String XError;

  while (*Root)
  {
    strmaxcpy(XError, (*Root)->Name, STRINGSIZE);
    strmaxcat(XError, ", ", STRINGSIZE);
    strmaxcat(XError, (*Root)->pErrorPos, STRINGSIZE);
    WrXError(ErrNum_UndefdForward, XError);
    free((*Root)->Name);
    free((*Root)->pErrorPos);
    Tmp = (*Root);
    *Root = Tmp->Next; free(Tmp);
  }
}

static void CodeENDSECTION(Word Index)
{
  PSaveSection Tmp;
  String ExpName;
  UNUSED(Index);

  if (!ChkArgCnt(0, 1));
  else if (!SectionStack) WrError(ErrNum_NotInSection);
  else if ((ArgCnt == 0) || (ExpandStrSymbol(ExpName, sizeof(ExpName), &ArgStr[1])))
  {
    if ((ArgCnt == 1) && (GetSectionHandle(ExpName, False, SectionStack->Handle) != MomSectionHandle)) WrStrErrorPos(ErrNum_WrongEndSect, &ArgStr[1]);
    else
    {
      Tmp = SectionStack;
      SectionStack = Tmp->Next;
      CodeENDSECTION_ChkEmptList(&(Tmp->LocSyms));
      CodeENDSECTION_ChkEmptList(&(Tmp->GlobSyms));
      CodeENDSECTION_ChkEmptList(&(Tmp->ExportSyms));
      if (ArgCnt == 0)
        as_snprintf(ListLine, STRINGSIZE, "[%s]", GetSectionName(MomSectionHandle));
      SetMomSection(Tmp->Handle);
      free(Tmp);
    }
  }
}


static void CodeCPU(Word Index)
{
  UNUSED(Index);

  if (!ChkArgCnt(1, 1));
  else if (*AttrPart.str.p_str != '\0') WrError(ErrNum_UseLessAttr);
  else
  {
    NLS_UpString(ArgStr[1].str.p_str);
    if (SetCPUByName(&ArgStr[1]))
      SetNSeg(SegCode);
    else
      WrStrErrorPos(ErrNum_InvCPUType, &ArgStr[1]);
  }
}

/*!------------------------------------------------------------------------
 * \fn     CodeORG_Core(const tStrComp *pArg)
 * \brief  core function of ORG statement
 * \param  pArg source argument holding new address
 * ------------------------------------------------------------------------ */

static void CodeORG_Core(const tStrComp *pArg)
{
  LargeWord HVal;
  Boolean ValOK;
  tSymbolFlags Flags;

  HVal = EvalStrIntExpressionWithFlags(pArg, LargeUIntType, &ValOK, &Flags);
  if (ValOK)
  {
    if (mFirstPassUnknown(Flags)) WrStrErrorPos(ErrNum_FirstPassCalc, pArg);
    else if (EProgCounter() != HVal)
    {
      PCs[ActPC] = HVal - Phases[ActPC];
      DontPrint = True;
    }
  }
}

/*!------------------------------------------------------------------------
 * \fn     CodeORG(Word Index)
 * \brief  handle ORG statement
 * ------------------------------------------------------------------------ */

static void CodeORG(Word Index)
{
  UNUSED(Index);

  if (*AttrPart.str.p_str != '\0') WrError(ErrNum_UseLessAttr);
  else if (ChkArgCnt(1, 1))
    CodeORG_Core(&ArgStr[1]);
}

/*!------------------------------------------------------------------------
 * \fn     CodeSETEQU(Word MayChange)
 * \brief  handle EQU/SET/EVAL statements
 * \param  MayChange 0 for EQU, 1 for SET/EVAL
 * ------------------------------------------------------------------------ */

static void CodeSETEQU(Word MayChange)
{
  const tStrComp *pName = *LabPart.str.p_str ? &LabPart : &ArgStr[1];
  int ValIndex = *LabPart.str.p_str ? 1 : 2;

  if ((ArgCnt == ValIndex) && !strcmp(pName->str.p_str, PCSymbol))
    CodeORG_Core(&ArgStr[ValIndex]);
  else if (ChkArgCnt(ValIndex, ValIndex + 1))
  {
    TempResult t;
    as_addrspace_t DestSeg;

    as_tempres_ini(&t);
    EvalStrExpression(&ArgStr[ValIndex], &t);
    if (!mFirstPassUnknown(t.Flags))
    {
      if (ArgCnt == ValIndex)
        DestSeg = SegNone;
      else
      {
        NLS_UpString(ArgStr[ValIndex + 1].str.p_str);
        if (!strcmp(ArgStr[ValIndex + 1].str.p_str, "MOMSEGMENT"))
          DestSeg = (as_addrspace_t)ActPC;
        else if (*ArgStr[ValIndex + 1].str.p_str == '\0')
          DestSeg = SegNone;
        else
          DestSeg = addrspace_lookup(ArgStr[ValIndex + 1].str.p_str);
      }
      if (DestSeg >= SegCount) WrStrErrorPos(ErrNum_UnknownSegment, &ArgStr[ValIndex + 1]);
      else
      {
        SetListLineVal(&t);
        PushLocHandle(-1);
        switch (t.Typ)
        {
          case TempInt:
            EnterIntSymbol(pName, t.Contents.Int, DestSeg, MayChange);
            if (AttrPartOpSize != eSymbolSizeUnknown)
              SetSymbolOrStructElemSize(pName, AttrPartOpSize);
            break;
          case TempFloat:
            EnterFloatSymbol(pName, t.Contents.Float, MayChange);
            break;
          case TempString:
            EnterNonZStringSymbolWithFlags(pName, &t.Contents.str, MayChange, t.Flags);
            break;
          case TempReg:
            EnterRegSymbol(pName, &t.Contents.RegDescr, t.DataSize, MayChange, False);
            break;
          default:
            break;
        }
        PopLocHandle();
      }
    }
    as_tempres_free(&t);
  }
}

static void CodeREGCore(const tStrComp *pNameArg, const tStrComp *pValueArg)
{
  TempResult t;

  as_tempres_ini(&t);
  if (InternSymbol)
    InternSymbol(pValueArg->str.p_str, &t);

  switch (t.Typ)
  {
    case TempReg:
      EnterRegSymbol(pNameArg, &t.Contents.RegDescr, t.DataSize, False, True);
      break;
    case TempNone:
    {
      tEvalResult EvalResult;
      tErrorNum ErrorNum;
      tRegDescr RegDescr;

      ErrorNum = EvalStrRegExpressionWithResult(pValueArg, &RegDescr, &EvalResult);

      switch (ErrorNum)
      {
        case ErrNum_SymbolUndef:
          /* ignore undefined symbols in first pass */
          if (PassNo <= MaxSymPass)
          {
            Repass = True;
            LEAVE;
          }
          break;
        case ErrNum_RegWrongTarget:
          /* REG is architecture-agnostic */
          EvalResult.OK = True;
          break;
        default:
          break;
      }

      if (EvalResult.OK)
        EnterRegSymbol(pNameArg, &RegDescr, EvalResult.DataSize, False, True);
      else
        WrStrErrorPos(ErrorNum, pValueArg);
      break;
    }
    default:
      WrStrErrorPos(ErrNum_ExpectReg, pValueArg);
  }

func_exit:
  as_tempres_free(&t);
}

void CodeREG(Word Index)
{
  UNUSED(Index);

  const tStrComp *pName = *LabPart.str.p_str ? &LabPart : &ArgStr[1];
  int ValIndex = *LabPart.str.p_str ? 1 : 2;

  if (ChkArgCnt(ValIndex, ValIndex))
    CodeREGCore(pName, &ArgStr[ValIndex]);
}

void CodeNAMEREG(Word Index)
{
  UNUSED(Index);

  if (ChkArgCnt(2, 2))
    CodeREGCore(&ArgStr[2], &ArgStr[1]);
}

static void CodeRORG(Word Index)
{
  LargeInt HVal;
  Boolean ValOK;
  tSymbolFlags Flags;
  UNUSED(Index);

  if (*AttrPart.str.p_str != '\0') WrError(ErrNum_UseLessAttr);
  else if (ChkArgCnt(1, 1))
  {
#ifndef HAS64
    HVal = EvalStrIntExpressionWithFlags(&ArgStr[1], SInt32, &ValOK, &Flags);
#else
    HVal = EvalStrIntExpressionWithFlags(&ArgStr[1], Int64, &ValOK, &Flags);
#endif
    if (mFirstPassUnknown(Flags)) WrError(ErrNum_FirstPassCalc);
    else if (ValOK)
    {
      PCs[ActPC] += HVal;
      DontPrint = True;
    }
  }
}

static void CodeSHARED_BuildComment(char *c, size_t DestSize)
{
  switch (ShareMode)
  {
    case 1:
      as_snprintf(c, DestSize, "(* %s *)", CommPart.str.p_str);
      break;
    case 2:
      as_snprintf(c, DestSize, "/* %s */", CommPart.str.p_str);
      break;
    case 3:
      as_snprintf(c, DestSize, "; %s", CommPart.str.p_str);
      break;
  }
}

static void CodeSHARED(Word Index)
{
  tStrComp *pArg;
  String s, c;
  TempResult t;

  UNUSED(Index);
  as_tempres_ini(&t);

  if (ShareMode == 0) WrError(ErrNum_NoShareFile);
  else if ((ArgCnt == 0) && (*CommPart.str.p_str != '\0'))
  {
    CodeSHARED_BuildComment(c, sizeof(c));
    errno = 0;
    fprintf(ShareFile, "%s\n", c); ChkIO(ErrNum_FileWriteError);
  }
  else
   forallargs (pArg, True)
   {
     LookupSymbol(pArg, &t, False, TempAll);

     switch (t.Typ)
     {
       case TempInt:
         switch (ShareMode)
         {
           case 1:
             IntLine(s, sizeof(s), t.Contents.Int, eIntConstModeMoto);
             break;
           case 2:
             IntLine(s, sizeof(s), t.Contents.Int, eIntConstModeC);
             break;
           case 3:
             IntLine(s, sizeof(s), t.Contents.Int, IntConstMode);
             break;
         }
         break;
       case TempFloat:
         as_snprintf(s, sizeof(s), "%0.17g", t.Contents.Float);
         break;
       case TempString:
         as_nonz_dynstr_to_c_str(s + 1, &t.Contents.str, sizeof(s) - 1);
         if (ShareMode == 1)
         {
           *s = '\'';
           strmaxcat(s, "\'", STRINGSIZE);
         }
         else
         {
           *s = '\"';
           strmaxcat(s, "\"", STRINGSIZE);
         }
         break;
       default:
         continue;
     }

     if ((pArg == ArgStr + 1) && (*CommPart.str.p_str != '\0'))
     {
       CodeSHARED_BuildComment(c, sizeof(c));
       strmaxprep(c, " ", STRINGSIZE);
     }
     else
       *c = '\0';
     errno = 0;
     switch (ShareMode)
     {
       case 1:
         fprintf(ShareFile, "%s = %s;%s\n", pArg->str.p_str, s, c);
         break;
       case 2:
         fprintf(ShareFile, "#define %s %s%s\n", pArg->str.p_str, s, c);
         break;
       case 3:
         strmaxprep(s, IsSymbolChangeable(pArg) ? "set " : "equ ", STRINGSIZE);
         fprintf(ShareFile, "%s %s%s\n", pArg->str.p_str, s, c);
         break;
     }
     ChkIO(ErrNum_FileWriteError);
   }
  as_tempres_free(&t);
}

static void CodeEXPORT(Word Index)
{
  tStrComp *pArg;
  TempResult t;

  UNUSED(Index);
  as_tempres_ini(&t);

  forallargs (pArg, True)
  {
    LookupSymbol(pArg, &t, True, TempInt);
    if (TempNone == t.Typ)
      continue;
    if (t.Relocs == NULL)
      AddExport(pArg->str.p_str, t.Contents.Int, 0);
    else if ((t.Relocs->Next != NULL) || (strcmp(t.Relocs->Ref, RelName_SegStart)))
      WrStrErrorPos(ErrNum_Unexportable, pArg);
    else
      AddExport(pArg->str.p_str, t.Contents.Int, RelFlag_Relative);
    if (t.Relocs)
      FreeRelocs(&t.Relocs);
  }
  as_tempres_free(&t);
}

static void CodePAGE(Word Index)
{
  Integer LVal, WVal;
  Boolean ValOK;
  UNUSED(Index);

  if (!ChkArgCnt(1, 2));
  else if (*AttrPart.str.p_str != '\0') WrError(ErrNum_UseLessAttr);
  else
  {
    LVal = EvalStrIntExpression(&ArgStr[1], UInt8, &ValOK);
    if (ValOK)
    {
      if ((LVal < 5) && (LVal != 0))
        LVal = 5;
      if (ArgCnt == 1)
      {
        WVal = 0;
        ValOK = True;
      }
      else
        WVal = EvalStrIntExpression(&ArgStr[2], UInt8, &ValOK);
      if (ValOK)
      {
        if ((WVal < 5) && (WVal != 0))
          WVal = 5;
        PageLength = LVal;
        PageWidth = WVal;
      }
    }
  }
}


static void CodeNEWPAGE(Word Index)
{
  ShortInt HVal8;
  Boolean ValOK;
  UNUSED(Index);

  if (!ChkArgCnt(0, 1));
  else if (*AttrPart.str.p_str != '\0') WrError(ErrNum_UseLessAttr);
  else
  {
    if (ArgCnt == 0)
    {
      HVal8 = 0;
      ValOK = True;
    }
    else
      HVal8 = EvalStrIntExpression(&ArgStr[1], Int8, &ValOK);
    if ((ValOK) || (ArgCnt == 0))
    {
      if (HVal8 > ChapMax)
        HVal8 = ChapMax;
      else if (HVal8 < 0)
        HVal8 = 0;
      NewPage(HVal8, True);
    }
  }
}


static void CodeString(Word Index)
{
  String tmp;
  Boolean OK;

  if (ChkArgCnt(1, 1))
  {
    EvalStrStringExpression(&ArgStr[1], &OK, tmp);
    if (!OK) WrError(ErrNum_InvString);
    else
    {
      switch (Index)
      {
        case 0:
          strmaxcpy(PrtInitString, tmp, STRINGSIZE);
          break;
        case 1:
          strmaxcpy(PrtExitString, tmp, STRINGSIZE);
          break;
        case 2:
          strmaxcpy(PrtTitleString, tmp, STRINGSIZE);
          break;
      }
    }
  }
}


static void CodePHASE(Word Index)
{
  Boolean OK;
  LongInt HVal;
  UNUSED(Index);

  if (!ChkArgCnt(1, 1));
  else if (ActPC == StructSeg) WrError(ErrNum_PhaseDisallowed);
  else
  {
    HVal = EvalStrIntExpression(&ArgStr[1], Int32, &OK);
    if (OK)
    {
      tSavePhase *pSavePhase;

      pSavePhase = (tSavePhase*)calloc(1, sizeof (*pSavePhase));
      pSavePhase->SaveValue = Phases[ActPC];
      pSavePhase->pNext = pPhaseStacks[ActPC];
      pPhaseStacks[ActPC] = pSavePhase;
      Phases[ActPC] = HVal - ProgCounter();
    }
  }
}


static void CodeDEPHASE(Word Index)
{
  UNUSED(Index);

  if (!ChkArgCnt(0, 0));
  else if (ActPC == StructSeg) WrError(ErrNum_PhaseDisallowed);
  else if (pPhaseStacks[ActPC])
  {
    tSavePhase *pSavePhase;

    pSavePhase = pPhaseStacks[ActPC];
    pPhaseStacks[ActPC] = pSavePhase->pNext;
    Phases[ActPC] = pSavePhase->SaveValue;
    free(pSavePhase);
  }
  else
    Phases[ActPC] = 0;
}


static void CodeWARNING(Word Index)
{
  String mess;
  Boolean OK;
  UNUSED(Index);

  if (ChkArgCnt(1, 1))
  {
    EvalStrStringExpression(&ArgStr[1], &OK, mess);
    if (!OK) WrError(ErrNum_InvString);
    else
      WrErrorString(mess, "", True, False, NULL, NULL);
  }
}


static void CodeMESSAGE(Word Index)
{
  String mess;
  Boolean OK;
  UNUSED(Index);

  if (ChkArgCnt(1, 1))
  {
    EvalStrStringExpression(&ArgStr[1], &OK, mess);
    if (!OK) WrError(ErrNum_InvString);
    else
    {
      if (QuietMode < 2)
        WrConsoleLine(mess, True);
      if (strcmp(LstName, "/dev/null"))
        WrLstLine(mess);
    }
  }
}


static void CodeERROR(Word Index)
{
  String mess;
  Boolean OK;
  UNUSED(Index);

  if (ChkArgCnt(1, 1))
  {
    EvalStrStringExpression(&ArgStr[1], &OK, mess);
    if (!OK) WrError(ErrNum_InvString);
    else
      WrErrorString(mess, "", False, False, NULL, NULL);
  }
}


static void CodeFATAL(Word Index)
{
  String mess;
  Boolean OK;
  UNUSED(Index);

  if (ChkArgCnt(1, 1))
  {
    EvalStrStringExpression(&ArgStr[1], &OK, mess);
    if (!OK) WrError(ErrNum_InvString);
    else
      WrErrorString(mess, "", False, True, NULL, NULL);
  }
}

static void CodeCHARSET(Word Index)
{
  FILE *f;
  unsigned char tfield[256];
  LongWord Start, l, TStart, Stop, z;
  Boolean OK;
  UNUSED(Index);

  if (!ChkArgCnt(0, 3));
  else if (ArgCnt == 0)
  {
    for (z = 0; z < 256; z++)
      CharTransTable[z] = z;
  }
  else
  {
    TempResult t;

    as_tempres_ini(&t);
    EvalStrExpression(&ArgStr[1], &t);
    if ((t.Typ == TempString) && (t.Flags & eSymbolFlag_StringSingleQuoted))
      TempResultToInt(&t);
    switch (t.Typ)
    {
      case TempInt:
        if (mFirstPassUnknown(t.Flags))
          t.Contents.Int &= 255;
        if (ChkRange(t.Contents.Int, 0, 255))
        {
          if (ChkArgCnt(2, 3))
          {
            Start = t.Contents.Int;
            EvalStrExpression(&ArgStr[2], &t);
            if ((t.Typ == TempString) && (t.Flags & eSymbolFlag_StringSingleQuoted))
              TempResultToInt(&t);
            switch (t.Typ)
            {
              case TempInt: /* �bersetzungsbereich als Character-Angabe */
                if (mFirstPassUnknown(t.Flags))
                  t.Contents.Int &= 255;
                if (ArgCnt == 2)
                {
                  Stop = Start;
                  TStart = t.Contents.Int;
                  OK = ChkRange(TStart, 0, 255);
                }
                else
                {
                  Stop = t.Contents.Int;
                  OK = ChkRange(Stop, Start, 255);
                  if (OK)
                    TStart = EvalStrIntExpression(&ArgStr[3], UInt8, &OK);
                  else
                    TStart = 0;
                }
                if (OK)
                  for (z = Start; z <= Stop; z++)
                    CharTransTable[z] = TStart + (z - Start);
                break;
              case TempString:
                l = t.Contents.str.len; /* Uebersetzungsstring ab Start */
                if (Start + l > 256) WrError(ErrNum_OverRange);
                else
                  for (z = 0; z < l; z++)
                    CharTransTable[Start + z] = t.Contents.str.p_str[z];
                break;
              case TempFloat:
                WrStrErrorPos(ErrNum_StringOrIntButFloat, &ArgStr[2]);
                break;
              default:
                break;
            }
          }
        }
        break;
      case TempString:
        if (ChkArgCnt(1, 1)) /* Tabelle von Datei lesen */
        {
          String Tmp;

          as_nonz_dynstr_to_c_str(Tmp, &t.Contents.str, sizeof(Tmp));
          f = fopen(Tmp, OPENRDMODE);
          if (!f) ChkIO(ErrNum_OpeningFile);
          if (fread(tfield, sizeof(char), 256, f) != 256) ChkIO(ErrNum_FileReadError);
          fclose(f);
          memcpy(CharTransTable, tfield, sizeof(char) * 256);
        }
        break;
      case TempFloat:
        WrStrErrorPos(ErrNum_StringOrIntButFloat, &ArgStr[1]);
        break;
      default:
        break;
    }
    as_tempres_free(&t);
  }
}

static void CodePRSET(Word Index)
{
  int z, z2;
  UNUSED(Index);

  for (z = 0; z < 16; z++)
  {
    for (z2 = 0; z2 < 16; z2++)
      printf(" %02x", CharTransTable[z*16 + z2]);
    printf("  ");
    for (z2 = 0; z2 < 16; z2++)
      printf("%c", CharTransTable[z * 16 + z2] > ' ' ? CharTransTable[z*16 + z2] : '.');
    putchar('\n');
  }
}

static void CodeCODEPAGE(Word Index)
{
  PTransTable Prev, Run, New, Source;
  int erg = 0;
  UNUSED(Index);

  if (!ChkArgCnt(1, 2));
  else if (!ChkSymbName(ArgStr[1].str.p_str)) WrStrErrorPos(ErrNum_InvSymName, &ArgStr[1]);
  else
  {
    if (!CaseSensitive)
    {
      UpString(ArgStr[1].str.p_str);
      if (ArgCnt == 2)
        UpString(ArgStr[2].str.p_str);
    }

    if (ArgCnt == 1)
      Source = CurrTransTable;
    else
    {
      for (Source = TransTables; Source; Source = Source->Next)
        if (!strcmp(Source->Name, ArgStr[2].str.p_str))
          break;
    }

    if (!Source) WrStrErrorPos(ErrNum_UnknownCodepage, &ArgStr[2]);
    else
    {
      for (Prev = NULL, Run = TransTables; Run; Prev = Run, Run = Run->Next)
        if ((erg = strcmp(ArgStr[1].str.p_str, Run->Name)) <= 0)
          break;

      if ((!Run) || (erg < 0))
      {
        New = (PTransTable) malloc(sizeof(TTransTable));
        New->Next = Run;
        New->Name = as_strdup(ArgStr[1].str.p_str);
        New->Table = (unsigned char *) malloc(256 * sizeof(char));
        memcpy(New->Table, Source->Table, 256 * sizeof(char));
        if (!Prev)
          TransTables = New;
        else
          Prev->Next = New;
        CurrTransTable = New;
      }
      else
        CurrTransTable = Run;
    }
  }
}


static void CodeFUNCTION(Word Index)
{
  Boolean OK;
  int z;
  UNUSED(Index);

  if (ChkArgCnt(2, ArgCntMax))
  {
    OK = True;
    z = 1;
    do
    {
      OK = (OK && ChkMacSymbName(ArgStr[z].str.p_str));
      if (!OK)
        WrStrErrorPos(ErrNum_InvSymName, &ArgStr[z]);
      z++;
    }
    while ((z < ArgCnt) && (OK));
    if (OK)
    {
      as_dynstr_t FName;

      as_dynstr_ini_c_str(&FName, ArgStr[ArgCnt].str.p_str);
      for (z = 1; z < ArgCnt; z++)
        CompressLine(ArgStr[z].str.p_str, z, &FName, CaseSensitive);
      EnterFunction(&LabPart, FName.p_str, ArgCnt - 1);
      as_dynstr_free(&FName);
    }
  }
}


static void CodeSAVE(Word Index)
{
  PSaveState Neu;
  UNUSED(Index);

  if (ChkArgCnt(0, 0))
  {
    Neu = (PSaveState) malloc(sizeof(TSaveState));
    Neu->Next = FirstSaveState;
    Neu->SaveCPU = MomCPU;
    Neu->pSaveCPUArgs = as_strdup(MomCPUArgs);
    Neu->SavePC = ActPC;
    Neu->SaveListOn = ListOn;
    Neu->SaveLstMacroExp = GetLstMacroExp();
    Neu->SaveLstMacroExpModDefault = LstMacroExpModDefault;
    Neu->SaveLstMacroExpModOverride = LstMacroExpModOverride;
    Neu->SaveTransTable = CurrTransTable;
    Neu->SaveEnumSegment = EnumSegment;
    Neu->SaveEnumIncrement = EnumIncrement;
    Neu->SaveEnumCurrentValue = EnumCurrentValue;
    FirstSaveState = Neu;
  }
}


static void CodeRESTORE(Word Index)
{
  PSaveState Old;
  UNUSED(Index);

  if (!ChkArgCnt(0, 0));
  else if (!FirstSaveState) WrError(ErrNum_NoSaveFrame);
  else
  {
    tStrComp TmpComp;
    String TmpCompStr;

    Old = FirstSaveState; FirstSaveState = Old->Next;
    if (Old->SavePC != ActPC)
    {
      ActPC = Old->SavePC;
      DontPrint = True;
    }
    if (Old->SaveCPU != MomCPU)
    {
      StrCompMkTemp(&TmpComp, Old->pSaveCPUArgs, 0);
      SetCPUByType(Old->SaveCPU, &TmpComp);
    }
    StrCompMkTemp(&TmpComp, TmpCompStr, sizeof(TmpCompStr));
    strmaxcpy(TmpCompStr, ListOnName, sizeof(TmpCompStr)); EnterIntSymbol(&TmpComp, ListOn = Old->SaveListOn, SegNone, True);
    SetLstMacroExp(Old->SaveLstMacroExp);
    LstMacroExpModDefault = Old->SaveLstMacroExpModDefault;
    LstMacroExpModOverride = Old->SaveLstMacroExpModOverride;
    CurrTransTable = Old->SaveTransTable;
    free(Old->pSaveCPUArgs);
    free(Old);
  }
}


static void CodeMACEXP(Word Index)
{
  /* will deprecate this in 1..2 years, 2018-01-21 */

#if 0
  if (Index & 0x10)
  {
    char Msg[70];

    as_snprintf(Msg, sizeof(Msg), getmessage(Num_ErrMsgDeprecated_Instead), "MACEXP_DFT");
    WrXError(ErrNum_Deprecated, Msg);
  }
#endif

  /* allow zero arguments for MACEXP_OVR, to remove all overrides */

  if (!ChkArgCnt((Index & 0x0f) ? 0 : 1, ArgCntMax));
  else if (*AttrPart.str.p_str != '\0') WrError(ErrNum_UseLessAttr);
  else
  {
    tStrComp *pArg;
    tLstMacroExpMod LstMacroExpMod;
    tLstMacroExp Mod;
    Boolean Set;
    Boolean OK = True;

    InitLstMacroExpMod(&LstMacroExpMod);
    forallargs (pArg, True)
    {
      if (!as_strcasecmp(pArg->str.p_str, "ON"))
      {
        Mod = eLstMacroExpAll; Set = True;
      }
      else if (!as_strcasecmp(pArg->str.p_str, "OFF"))
      {
        Mod = eLstMacroExpAll; Set = False;
      }
      else if (!as_strcasecmp(pArg->str.p_str, "NOIF"))
      {
        Mod= eLstMacroExpIf; Set = False;
      }
      else if (!as_strcasecmp(pArg->str.p_str, "NOMACRO"))
      {
        Mod = eLstMacroExpMacro; Set = False;
      }
      else if (!as_strcasecmp(pArg->str.p_str, "NOREST"))
      {
        Mod = eLstMacroExpRest; Set = False;
      }
      else if (!as_strcasecmp(pArg->str.p_str, "IF"))
      {
        Mod = eLstMacroExpIf; Set = True;
      }
      else if (!as_strcasecmp(pArg->str.p_str, "MACRO"))
      {
        Mod = eLstMacroExpMacro; Set = True;
      }
      else if (!as_strcasecmp(pArg->str.p_str, "REST"))
      {
        Mod = eLstMacroExpRest; Set = True;
      }
      else
        OK = False;
      if (!OK)
      {
        WrStrErrorPos(ErrNum_TooManyMacExpMod, pArg);
        break;
      }
      else if (!AddLstMacroExpMod(&LstMacroExpMod, Set, Mod))
      {
        WrStrErrorPos(ErrNum_TooManyArgs, pArg);
        break;
      }
    }
    if (OK)
    {
      if (!ChkLstMacroExpMod(&LstMacroExpMod)) WrError(ErrNum_ConflictingMacExpMod);
      else if (Index) /* Override */
        LstMacroExpModOverride = LstMacroExpMod;
      else
      {
        /* keep LstMacroExp and LstMacroExpModDefault in sync! */
        LstMacroExpModDefault = LstMacroExpMod;
        SetLstMacroExp(ApplyLstMacroExpMod(eLstMacroExpAll, &LstMacroExpModDefault));
      }
    }
  }
}

static Boolean DecodeSegment(const tStrComp *pArg, Integer StartSeg, Integer *pResult)
{
  Integer SegZ;
  Word Mask;

  for (SegZ = StartSeg, Mask = 1 << StartSeg; SegZ < SegCount; SegZ++, Mask <<= 1)
    if ((ValidSegs & Mask) && !as_strcasecmp(pArg->str.p_str, SegNames[SegZ]))
    {
      *pResult = SegZ;
      return True;
    }
  WrStrErrorPos(ErrNum_UnknownSegment, pArg);
  return False;
}

static void CodeSEGMENT(Word Index)
{
  Integer NewSegment;
  UNUSED(Index);

  if (ChkArgCnt(1, 1)
   && DecodeSegment(&ArgStr[1], SegCode, &NewSegment))
    SetNSeg(NewSegment);
}


static void CodeLABEL(Word Index)
{
  LongInt Erg;
  Boolean OK;
  tSymbolFlags Flags;
  UNUSED(Index);
  const tStrComp *pName = *LabPart.str.p_str ? &LabPart : &ArgStr[1];
  int ValIndex = *LabPart.str.p_str ? 1 : 2;

  if (ChkArgCnt(ValIndex, ValIndex))
  {
    Erg = EvalStrIntExpressionWithFlags(&ArgStr[ValIndex], Int32, &OK, &Flags);
    if (OK && !mFirstPassUnknown(Flags))
    {
      PushLocHandle(-1);
      EnterIntSymbol(pName, Erg, SegCode, False);
      *ListLine = '=';
      IntLine(ListLine + 1, STRINGSIZE - 1, Erg, IntConstMode);
      PopLocHandle();
    }
  }
}


static void CodeREAD(Word Index)
{
  String ExpStr;
  tStrComp Exp;
  Boolean OK;
  LongInt SaveLocHandle;
  UNUSED(Index);

  StrCompMkTemp(&Exp, ExpStr, sizeof(ExpStr));
  if (ChkArgCnt(1, 2))
  {
    if (ArgCnt == 2) EvalStrStringExpression(&ArgStr[1], &OK, Exp.str.p_str);
    else
    {
      as_snprintf(Exp.str.p_str, sizeof(ExpStr), "Read %s ? ", ArgStr[1].str.p_str);
      OK = True;
    }
    if (OK)
    {
      TempResult Erg;

      as_tempres_ini(&Erg);
      printf("%s", Exp.str.p_str);
      fflush(stdout);
      if (!fgets(Exp.str.p_str, STRINGSIZE, stdin))
        OK = False;
      else
      {
        UpString(Exp.str.p_str);
        EvalStrExpression(&Exp, &Erg);
      }
      if (OK)
      {
        SetListLineVal(&Erg);
        SaveLocHandle = MomLocHandle;
        MomLocHandle = -1;
        if (mFirstPassUnknown(Erg.Flags)) WrError(ErrNum_FirstPassCalc);
        else switch (Erg.Typ)
        {
          case TempInt:
            EnterIntSymbol(&ArgStr[ArgCnt], Erg.Contents.Int, SegNone, True);
            break;
          case TempFloat:
            EnterFloatSymbol(&ArgStr[ArgCnt], Erg.Contents.Float, True);
            break;
          case TempString:
            EnterNonZStringSymbol(&ArgStr[ArgCnt], &Erg.Contents.str, True);
            break;
          default:
            break;
        }
        MomLocHandle = SaveLocHandle;
      }
      as_tempres_free(&Erg);
    }
  }
}

static void CodeRADIX(Word Index)
{
  Boolean OK;
  LargeWord tmp;

  if (ChkArgCnt(1, 1))
  {
    tmp = ConstLongInt(ArgStr[1].str.p_str, &OK, 10);
    if (!OK) WrError(ErrNum_ExpectInt);
    else if (ChkRange(tmp, 2, 36))
    {
      if (Index == 1)
        OutRadixBase = tmp;
      else
        RadixBase = tmp;
    }
  }
}

static void CodeALIGN(Word Index)
{
  UNUSED(Index);

  if (ChkArgCnt(1, 2))
  {
    Word AlignValue;
    Byte AlignFill = 0;
    Boolean OK = True;
    tSymbolFlags Flags = eSymbolFlag_None;
    LongInt NewPC;

    if (2 == ArgCnt)
      AlignFill = EvalStrIntExpressionWithFlags(&ArgStr[2], Int8, &OK, &Flags);
    if (OK)
      AlignValue = EvalStrIntExpression(&ArgStr[1], Int16, &OK);
    if (OK)
    {
      if (mFirstPassUnknown(Flags)) WrError(ErrNum_FirstPassCalc);
      else
      {
        NewPC = EProgCounter() + AlignValue - 1;
        NewPC -= NewPC % AlignValue;
        CodeLen = NewPC - EProgCounter();
        if (1 == ArgCnt)
        {
          DontPrint = !!CodeLen;
          BookKeeping();
        }
        else if (CodeLen > (LongInt)MaxCodeLen) WrError(ErrNum_CodeOverflow);
        else
        {
          memset(BAsmCode, AlignFill, CodeLen);
          DontPrint = False;
        }
      }
    }
  }
}

static void CodeASSUME(Word Index)
{
  int z1;
  unsigned z2;
  Boolean OK;
  tSymbolFlags Flags;
  LongInt HVal;
  tStrComp RegPart, ValPart;
  char *pSep, EmptyStr[] = "";

  UNUSED(Index);

  /* CPU-specific override? */

  if (pASSUMEOverride)
  {
    pASSUMEOverride();
    return;
  }

  if (ChkArgCnt(1, ArgCntMax))
  {
    z1 = 1;
    OK = True;
    while ((z1 <= ArgCnt) && (OK))
    {
      pSep = QuotPos(ArgStr[z1].str.p_str, ':');
      if (pSep)
        StrCompSplitRef(&RegPart, &ValPart, &ArgStr[z1], pSep);
      else
      {
        RegPart = ArgStr[z1];
        StrCompMkTemp(&ValPart, EmptyStr, 0);
      }
      z2 = 0;
      NLS_UpString(RegPart.str.p_str);
      while ((z2 < ASSUMERecCnt) && (strcmp(pASSUMERecs[z2].Name, RegPart.str.p_str)))
        z2++;
      OK = (z2 < ASSUMERecCnt);
      if (!OK) WrStrErrorPos(ErrNum_InvRegName, &RegPart);
      else
      {
        if (!as_strcasecmp(ValPart.str.p_str, "NOTHING"))
        {
          if (pASSUMERecs[z2].NothingVal == -1) WrError(ErrNum_InvAddrMode);
          else
            *(pASSUMERecs[z2].Dest) = pASSUMERecs[z2].NothingVal;
        }
        else
        {
          HVal = EvalStrIntExpressionWithFlags(&ValPart, Int32, &OK, &Flags);
          if (OK)
          {
            if (mFirstPassUnknown(Flags))
            {
              WrError(ErrNum_FirstPassCalc);
              OK = False;
            }
            else if (ChkRange(HVal, pASSUMERecs[z2].Min, pASSUMERecs[z2].Max))
              *(pASSUMERecs[z2].Dest) = HVal;
          }
        }
        if (pASSUMERecs[z2].pPostProc)
          pASSUMERecs[z2].pPostProc();
      }
      z1++;
    }
  }
}

static void CodeENUM(Word IsNext)
{
  int z;
  char *p = NULL;
  Boolean OK;
  tSymbolFlags Flags;
  LongInt  First = 0, Last = 0;
  tStrComp SymPart;

  if (!IsNext)
    EnumCurrentValue = 0;
  if (ChkArgCnt(1, ArgCntMax))
  {
    for (z = 1; z <= ArgCnt; z++)
    {
      p = QuotPos(ArgStr[z].str.p_str, '=');
      if (p)
      {
        StrCompSplitRef(&ArgStr[z], &SymPart, &ArgStr[z], p);
        EnumCurrentValue = EvalStrIntExpressionWithFlags(&SymPart, Int32, &OK, &Flags);
        if (!OK)
          return;
        if (mFirstPassUnknown(Flags))
        {
          WrStrErrorPos(ErrNum_FirstPassCalc, &SymPart);
          return;
        }
        *p = '\0';
      }
      EnterIntSymbol(&ArgStr[z], EnumCurrentValue, (as_addrspace_t)EnumSegment, False);
      if (z == 1)
        First = EnumCurrentValue;
      else if (z == ArgCnt)
        Last = EnumCurrentValue;
      EnumCurrentValue += EnumIncrement;
    }
  }
  *ListLine = '=';
  IntLine(ListLine + 1, STRINGSIZE - 1, First, IntConstMode);
  if (ArgCnt != 1)
  {
    int l;

    strmaxcat(ListLine, "..", STRINGSIZE);
    l = strlen(ListLine);
    IntLine(ListLine + l, STRINGSIZE - l, Last, IntConstMode);
  }
}

static void CodeENUMCONF(Word Index)
{
  UNUSED(Index);

  if (ChkArgCnt(1, 2))
  {
    Boolean OK;
    LongInt NewIncrement;

    NewIncrement = EvalStrIntExpression(&ArgStr[1], Int32, &OK);
    if (!OK)
      return;
    EnumIncrement = NewIncrement;

    if (ArgCnt >= 2)
    {
      Integer NewSegment;

      if (DecodeSegment(&ArgStr[2], SegNone, &NewSegment))
        EnumSegment = NewSegment;
    }
  }
}

static void CodeEND(Word Index)
{
  UNUSED(Index);

  if (ChkArgCnt(0, 1))
  {
    if (ArgCnt == 1)
    {
      LongInt HVal;
      tEvalResult EvalResult;

      HVal = EvalStrIntExpressionWithResult(&ArgStr[1], Int32, &EvalResult);
      if (EvalResult.OK)
      {
        ChkSpace(SegCode, EvalResult.AddrSpaceMask);
        StartAdr = HVal;
        StartAdrPresent = True;
      }
    }
   ENDOccured = True;
  }
}


static void CodeLISTING(Word Index)
{
  Byte Value = 0xff;
  Boolean OK;
  UNUSED(Index);

  if (!ChkArgCnt(1, 1));
  else if (*AttrPart.str.p_str != '\0') WrError(ErrNum_UseLessAttr);
  else
  {
    OK = True;
    NLS_UpString(ArgStr[1].str.p_str);
    if (!strcmp(ArgStr[1].str.p_str, "OFF"))
      Value = 0;
    else if (!strcmp(ArgStr[1].str.p_str, "ON"))
      Value = 1;
    else if (!strcmp(ArgStr[1].str.p_str, "NOSKIPPED"))
      Value = 2;
    else if (!strcmp(ArgStr[1].str.p_str, "PURECODE"))
      Value = 3;
    else
      OK = False;
    if (!OK) WrStrErrorPos(ErrNum_OnlyOnOff, &ArgStr[1]);
    else
    {
      tStrComp TmpComp;
      String TmpCompStr;

      StrCompMkTemp(&TmpComp, TmpCompStr, sizeof(TmpCompStr));
      strmaxcpy(TmpCompStr, ListOnName, sizeof(TmpCompStr));
      EnterIntSymbol(&TmpComp, ListOn = Value, SegNone, True);
    }
  }
}

void INCLUDE_SearchCore(tStrComp *pDest, const tStrComp *pArg, Boolean SearchPath)
{
  size_t l = strlen(pArg->str.p_str), offs = 0;

  if (pArg->str.p_str[0] == '"')
  {
    offs++;
    if ((l > 1) && (pArg->str.p_str[l - 1]  == '"'))
      l -= 2;
    else
    {
      WrStrErrorPos(ErrNum_BrackErr, pArg);
      return;
    }
  }
  StrCompCopySub(pDest, pArg, offs, l);

  AddSuffix(pDest->str.p_str, IncSuffix);

  if (SearchPath)
  {
    String FoundFileName;

    if (FSearch(FoundFileName, sizeof(FoundFileName), pDest->str.p_str, CurrFileName, SearchPath ? IncludeList : ""))
      ChkStrIO(ErrNum_OpeningFile, pArg);
    strmaxcpy(pDest->str.p_str, FExpand(FoundFileName), STRINGSIZE - 1);
  }
}

static void CodeBINCLUDE(Word Index)
{
  FILE *F;
  LongInt Len = -1;
  LongWord Ofs = 0, Curr, Rest, FSize;
  Word RLen;
  Boolean OK, SaveTurnWords;
  tSymbolFlags Flags;
  LargeWord OldPC;
  UNUSED(Index);

  if (!ChkArgCnt(1, 3));
  else if (ActPC == StructSeg) WrError(ErrNum_NotInStruct);
  else
  {
    if (ArgCnt == 1)
      OK = True;
    else
    {
      Ofs = EvalStrIntExpressionWithFlags(&ArgStr[2], Int32, &OK, &Flags);
      if (mFirstPassUnknown(Flags))
      {
        WrError(ErrNum_FirstPassCalc);
        OK = False;
      }
      if (OK)
      {
        if (ArgCnt == 2)
          Len = -1;
        else
        {
          Len = EvalStrIntExpressionWithFlags(&ArgStr[3], Int32, &OK, &Flags);
          if (mFirstPassUnknown(Flags))
          {
            WrError(ErrNum_FirstPassCalc);
            OK = False;
          }
        }
      }
    }
    if (OK)
    {
      tStrComp FNameArg;
      String FNameArgStr;

      StrCompMkTemp(&FNameArg, FNameArgStr, sizeof(FNameArgStr));
      INCLUDE_SearchCore(&FNameArg, &ArgStr[1], True);

      F = fopen(FNameArg.str.p_str, OPENRDMODE);
      if (F == NULL) ChkXIO(ErrNum_OpeningFile, FNameArg.str.p_str);
      errno = 0; FSize = FileSize(F); ChkIO(ErrNum_FileReadError);
      if (Len == -1)
      {
        if ((Len = FSize - Ofs) < 0)
        {
          fclose(F); WrError(ErrNum_ShortRead); return;
        }
      }
      if (!ChkPC(EProgCounter() + Len - 1)) WrError(ErrNum_AdrOverflow);
      else
      {
        errno = 0; fseek(F, Ofs, SEEK_SET); ChkIO(ErrNum_FileReadError);
        Rest = Len;
        SaveTurnWords = TurnWords;
        TurnWords = False;
        OldPC = ProgCounter();
        do
        {
          Curr = (Rest <= 256) ? Rest : 256;
          errno = 0; RLen = fread(BAsmCode, 1, Curr, F); ChkIO(ErrNum_FileReadError);
          CodeLen = RLen;
          WriteBytes();
          PCs[ActPC] += CodeLen;
          Rest -= RLen;
        }
        while ((Rest != 0) && (RLen == Curr));
        if (Rest != 0) WrError(ErrNum_ShortRead);
        TurnWords = SaveTurnWords;
        DontPrint = True;
        CodeLen = ProgCounter() - OldPC;
        PCs[ActPC] = OldPC;
      }
      fclose(F);
    }
  }
}

static void CodePUSHV(Word Index)
{
  int z;
  UNUSED(Index);

  if (ChkArgCnt(2, ArgCntMax))
  {
    if (!CaseSensitive)
      NLS_UpString(ArgStr[1].str.p_str);
    for (z = 2; z <= ArgCnt; z++)
      PushSymbol(&ArgStr[z], &ArgStr[1]);
  }
}

static void CodePOPV(Word Index)
{
  int z;
  UNUSED(Index);

  if (ChkArgCnt(2, ArgCntMax))
  {
    if (!CaseSensitive)
      NLS_UpString(ArgStr[1].str.p_str);
    for (z = 2; z <= ArgCnt; z++)
      PopSymbol(&ArgStr[z], &ArgStr[1]);
  }
}

static PForwardSymbol CodePPSyms_SearchSym(PForwardSymbol Root, char *Comp)
{
  PForwardSymbol Lauf = Root;
  UNUSED(Comp);

  while (Lauf && strcmp(Lauf->Name, Comp))
    Lauf = Lauf->Next;
  return Lauf;
}


static void CodeSTRUCT(Word IsUnion)
{
  PStructStack NStruct;
  tStrComp *pArg;
  Boolean OK, DoExt;
  char ExtChar;
  String StructName;

  if (!ChkArgCnt(0, ArgCntMax))
    return;

  /* unnamed struct/union only allowed if embedded into at least one named struct/union */

  if (!*LabPart.str.p_str)
  {
    if (!pInnermostNamedStruct)
    {
      WrError(ErrNum_FreestandingUnnamedStruct);
      return;
    }
  }
  else
  {
    if (!ChkSymbName(LabPart.str.p_str))
    {
      WrXError(ErrNum_InvSymName, LabPart.str.p_str);
      return;
    }
    if (!CaseSensitive)
      NLS_UpString(LabPart.str.p_str);
  }

  /* compose name of nested structures */

  if (*LabPart.str.p_str)
    BuildStructName(StructName, sizeof(StructName), LabPart.str.p_str);
  else
    *StructName = '\0';

  /* If named and embedded into another struct, add as element to innermost named parent struct.
     Add up all offsets of unnamed structs in between. */

  if (StructStack && (*LabPart.str.p_str))
  {
    PStructStack pRun;
    LargeWord Offset = ProgCounter();
    PStructElem pElement = CreateStructElem(&LabPart);
    if (!pElement)
      return;

    for (pRun = StructStack; pRun && pRun != pInnermostNamedStruct; pRun = pRun->Next)
      Offset += pRun->SaveCurrPC;
    pElement->Offset = Offset;
    pElement->IsStruct = True;
    AddStructElem(pInnermostNamedStruct->StructRec, pElement);
    AddStructSymbol(LabPart.str.p_str, ProgCounter());
  }

  NStruct = (PStructStack) malloc(sizeof(TStructStack));
  NStruct->Name = as_strdup(StructName);
  NStruct->pBaseName = NStruct->Name + strlen(NStruct->Name) - strlen(LabPart.str.p_str); /* NULL -> complain too long */
  NStruct->SaveCurrPC = ProgCounter();
  DoExt = True;
  ExtChar = DottedStructs ? '.' : '_';
  NStruct->Next = StructStack;
  OK = True;
  forallargs (pArg, True)
    if (OK)
    {
      if (!as_strcasecmp(pArg->str.p_str, "EXTNAMES"))
        DoExt = True;
      else if (!as_strcasecmp(pArg->str.p_str, "NOEXTNAMES"))
        DoExt = False;
      else if (!as_strcasecmp(pArg->str.p_str, "DOTS"))
        ExtChar = '.';
      else if (!as_strcasecmp(pArg->str.p_str, "NODOTS"))
        ExtChar = '_';
      else
      {
        WrStrErrorPos(ErrNum_InvStructDir, pArg);
        OK = False;
      }
    }
  if (OK)
  {
    NStruct->StructRec = CreateStructRec();
    NStruct->StructRec->ExtChar = ExtChar;
    NStruct->StructRec->DoExt = DoExt;
    NStruct->StructRec->IsUnion = IsUnion;
    StructStack = NStruct;
    if (ActPC != StructSeg)
      StructSaveSeg = ActPC;
    if (NStruct->Name[0])
      pInnermostNamedStruct = NStruct;
    ActPC = StructSeg;
    PCs[ActPC] = 0;
    Phases[ActPC] = 0;
    Grans[ActPC] = Grans[SegCode];
    ListGrans[ActPC] = ListGrans[SegCode];
    ClearChunk(SegChunks + StructSeg);
    CodeLen = 0;
    DontPrint = True;
  }
  else
  {
    free(NStruct->Name);
    free(NStruct);
  }
}

static void CodeENDSTRUCT(Word IsUnion)
{
  Boolean OK;
  PStructStack OStruct;

  if (!ChkArgCnt(0, 1));
  else if (!StructStack) WrError(ErrNum_MissingStruct);
  else
  {
    if (IsUnion && !StructStack->StructRec->IsUnion)
      WrXError(ErrNum_STRUCTEndedByENDUNION, StructStack->Name);

    if (*LabPart.str.p_str == '\0') OK = True;
    else
    {
      if (!CaseSensitive)
        NLS_UpString(LabPart.str.p_str);
      OK = !strcmp(LabPart.str.p_str, StructStack->pBaseName);
      if (!OK) WrError(ErrNum_WrongStruct);
    }
    if (OK)
    {
      LargeWord TotLen;

      /* unchain current struct from stack */

      OStruct = StructStack;
      StructStack = OStruct->Next;

      /* resolve referenced symbols in structure */

      ResolveStructReferences(OStruct->StructRec);

      /* find new innermost named struct */

      for (pInnermostNamedStruct = StructStack;
           pInnermostNamedStruct;
           pInnermostNamedStruct = pInnermostNamedStruct->Next)
      {
        if (pInnermostNamedStruct->Name[0])
          break;
      }

      BumpStructLength(OStruct->StructRec, ProgCounter());
      TotLen = OStruct->StructRec->TotLen;

      /* add symbol for struct length if not nameless */

      if (ArgCnt == 0)
      {
        if (OStruct->Name[0])
        {
          String tmp2;
          tStrComp TmpComp;

          as_snprintf(tmp2, sizeof(tmp2), "%s%clen", OStruct->Name, OStruct->StructRec->ExtChar);
          StrCompMkTemp(&TmpComp, tmp2, sizeof(tmp2));
          EnterIntSymbol(&TmpComp, TotLen, SegNone, False);
        }
      }
      else
        EnterIntSymbol(&ArgStr[1], TotLen, SegNone, False);

      {
        TempResult t;

        as_tempres_ini(&t);
        as_tempres_set_int(&t, TotLen);
        SetListLineVal(&t);
        as_tempres_free(&t);
      }

      /* If named, store completed structure.
         Otherwise, discard temporary struct. */

      if (OStruct->Name[0])
        AddStruct(OStruct->StructRec, OStruct->Name, True);
      else
        DestroyStructRec(OStruct->StructRec);
      OStruct->StructRec = NULL;

      /* set PC back to outer's struct value, plus size of
         just completed struct, or non-struct value: */

      PCs[ActPC] = OStruct->SaveCurrPC;
      if (!StructStack)
      {
        ActPC = StructSaveSeg;
        CodeLen = 0;
      }
      else
      {
        CodeLen = TotLen;
      }

      /* free struct stack elements no longer needed */

      free(OStruct->Name);
      free(OStruct);
      ClearChunk(SegChunks + StructSeg);
      DontPrint = True;
    }
  }
}

static void CodeEXTERN(Word Index)
{
  char *Split;
  int i;
  Boolean OK;
  as_addrspace_t Type;
  UNUSED(Index);

  if (ChkArgCnt(1, ArgCntMax))
  {
    i = 1;
    OK = True;
    while ((OK) && (i <= ArgCnt))
    {
      Split = strrchr(ArgStr[i].str.p_str, ':');
      if (Split == NULL)
        Type = SegNone;
      else
      {
        *Split = '\0';
        for (Type = SegNone + 1; Type < SegCount; Type++)
          if (!as_strcasecmp(Split + 1, SegNames[Type]))
            break;
      }
      if (Type >= SegCount) WrXError(ErrNum_UnknownSegment, Split + 1);
      else
      {
        EnterExtSymbol(&ArgStr[i], 0, Type, FALSE);
      }
      i++;
    }
  }
}

static void CodeNESTMAX(Word Index)
{
  LongInt Temp;
  Boolean OK;
  tSymbolFlags Flags;
  UNUSED(Index);

  if (ChkArgCnt(1, 1))
  {
    Temp = EvalStrIntExpressionWithFlags(&ArgStr[1], UInt32, &OK, &Flags);
    if (OK)
    {
      if (mFirstPassUnknown(Flags)) WrError(ErrNum_FirstPassCalc);
      else NestMax = Temp;
    }
  }
}

static void CodeSEGTYPE(Word Index)
{
  UNUSED(Index);

  if (ChkArgCnt(0, 0))
    RelSegs = (as_toupper(*OpPart.str.p_str) == 'R');
}

static void CodePPSyms(PForwardSymbol *Orig,
                       PForwardSymbol *Alt1,
                       PForwardSymbol *Alt2)
{
  PForwardSymbol Lauf;
  tStrComp *pArg, SymArg, SectionArg;
  String Sym, Section;
  char *pSplit;

  if (ChkArgCnt(1, ArgCntMax))
    forallargs (pArg, True)
    {
      pSplit = QuotPos(pArg->str.p_str, ':');
      if (pSplit)
      {
        StrCompSplitRef(&SymArg, &SectionArg, pArg, pSplit);
        if (!ExpandStrSymbol(Sym, sizeof(Sym), &SymArg))
          return;
      }
      else
      {
        if (!ExpandStrSymbol(Sym, sizeof(Sym), pArg))
          return;
        *Section = '\0';
        StrCompMkTemp(&SectionArg, Section, sizeof(Section));
      }
      if (!CaseSensitive)
        NLS_UpString(Sym);
      Lauf = CodePPSyms_SearchSym(*Alt1, Sym);
      if (Lauf) WrStrErrorPos(ErrNum_ContForward, pArg);
      else
      {
        Lauf = CodePPSyms_SearchSym(*Alt2, Sym);
        if (Lauf) WrStrErrorPos(ErrNum_ContForward, pArg);
        else
        {
          Lauf = CodePPSyms_SearchSym(*Orig, Sym);
          if (!Lauf)
          {
            Lauf = (PForwardSymbol) malloc(sizeof(TForwardSymbol));
            Lauf->Next = (*Orig); *Orig = Lauf;
            Lauf->Name = as_strdup(Sym);
            Lauf->pErrorPos = GetErrorPos();
          }
          IdentifySection(&SectionArg, &Lauf->DestSection);
        }
      }
    }
}

/*------------------------------------------------------------------------*/

#define ONOFFMax 32
static int ONOFFCnt = 0;
typedef struct
{
  Boolean Persist, *FlagAddr;
  const char *FlagName;
  const char *InstName;
} ONOFFTab;
static ONOFFTab *ONOFFList;

Boolean CheckONOFFArg(const tStrComp *pArg, Boolean *pResult)
{
  *pResult = !as_strcasecmp(ArgStr[1].str.p_str, "ON");
  if (!*pResult && as_strcasecmp(ArgStr[1].str.p_str, "OFF"))
  {
    WrStrErrorPos(ErrNum_OnlyOnOff, pArg);
    return False;
  }
  return True;
}

static void DecodeONOFF(Word Index)
{
  ONOFFTab *Tab = ONOFFList + Index;

  if (ChkArgCnt(1, 1))
  {
    Boolean IsON;

    if (*AttrPart.str.p_str != '\0') WrError(ErrNum_UseLessAttr);
    else if (CheckONOFFArg(&ArgStr[1], &IsON))
      SetFlag(Tab->FlagAddr, Tab->FlagName, IsON);
  }
}

void AddONOFF(const char *InstName, Boolean *Flag, const char *FlagName, Boolean Persist)
{
  if (ONOFFCnt == ONOFFMax) exit(255);
  ONOFFList[ONOFFCnt].Persist = Persist;
  ONOFFList[ONOFFCnt].FlagAddr = Flag;
  ONOFFList[ONOFFCnt].FlagName = FlagName;
  ONOFFList[ONOFFCnt].InstName = InstName;
  AddInstTable(ONOFFTable, InstName, ONOFFCnt++, DecodeONOFF);
}

void ClearONOFF(void)
{
  int z, z2;

  for (z = 0; z < ONOFFCnt; z++)
    if (!ONOFFList[z].Persist)
      break;

  for (z2 = ONOFFCnt - 1; z2 >= z; z2--)
    RemoveInstTable(ONOFFTable, ONOFFList[z2].InstName);

  ONOFFCnt = z;
}

static void CodeRELAXED(Word Index)
{
  UNUSED(Index);

  if (ChkArgCnt(1, 1))
  {
    Boolean NewRelaxed;

    NLS_UpString(ArgStr[1].str.p_str);
    if (*AttrPart.str.p_str != '\0') WrError(ErrNum_UseLessAttr);
    else if (CheckONOFFArg(&ArgStr[1], &NewRelaxed))
    {
      SetFlag(&RelaxedMode, RelaxedName, NewRelaxed);
      SetIntConstRelaxedMode(NewRelaxed);
    }
  }
}

/*!------------------------------------------------------------------------
 * \fn     CodeINTSYNTAX(Word Index)
 * \brief  process INTSYNTAX statement
 * ------------------------------------------------------------------------ */

static void CodeINTSYNTAX(Word Index)
{
  UNUSED(Index);

  if (ChkArgCnt(1, ArgCntMax))
  {
    LongWord ANDMask = 0, ORMask = 0;
    tStrComp Ident, *pArg;
    tIntFormatId Id;
    Boolean OK = True;

    forallargs(pArg, OK)
    {
      StrCompRefRight(&Ident, pArg, 1);
      Id = GetIntFormatId(Ident.str.p_str);
      if (!Id)
      {
        WrStrErrorPos(ErrNum_InvIntFormat, &Ident);
        OK = False;
      }
      else switch (pArg->str.p_str[0])
      {
        case '+':
          ORMask |= 1ul << Id;
          break;
        case '-':
          ANDMask |= 1ul << Id;
          break;
        default:
          WrStrErrorPos(ErrNum_InvIntFormat, pArg);
        OK = False;
      }
    }
    if (OK)
    {
      if (!ModifyIntConstModeByMask(ANDMask, ORMask))
        WrError(ErrNum_InvIntFormatList);
    }
  }
}

/*------------------------------------------------------------------------*/

typedef struct
{
  const char *Name;
  InstProc Proc;
  Word Index;
} PseudoOrder;
static const PseudoOrder Pseudos[] =
{
  {"ALIGN",      CodeALIGN      , 0 },
  {"ASEG",       CodeSEGTYPE    , 0 },
  {"ASSUME",     CodeASSUME     , 0 },
  {"BINCLUDE",   CodeBINCLUDE   , 0 },
  {"CHARSET",    CodeCHARSET    , 0 },
  {"CODEPAGE",   CodeCODEPAGE   , 0 },
  {"CPU",        CodeCPU        , 0 },
  {"DEPHASE",    CodeDEPHASE    , 0 },
  {"END",        CodeEND        , 0 },
  {"ENDEXPECT",  CodeENDEXPECT  , 0 },
  {"ENDS",       CodeENDSTRUCT  , 0 },
  {"ENDSECTION", CodeENDSECTION , 0 },
  {"ENDSTRUC",   CodeENDSTRUCT  , 0 },
  {"ENDSTRUCT",  CodeENDSTRUCT  , 0 },
  {"ENDUNION",   CodeENDSTRUCT  , 1 },
  {"ENUM",       CodeENUM       , 0 },
  {"ENUMCONF",   CodeENUMCONF   , 0 },
  {"EQU",        CodeSETEQU     , 0 },
  {"ERROR",      CodeERROR      , 0 },
  {"EXPECT",     CodeEXPECT     , 0 },
  {"EXPORT_SYM", CodeEXPORT     , 0 },
  {"EXTERN_SYM", CodeEXTERN     , 0 },
  {"FATAL",      CodeFATAL      , 0 },
  {"FUNCTION",   CodeFUNCTION   , 0 },
  {"INTSYNTAX",  CodeINTSYNTAX  , 0 },
  {"LABEL",      CodeLABEL      , 0 },
  {"LISTING",    CodeLISTING    , 0 },
  {"MESSAGE",    CodeMESSAGE    , 0 },
  {"NEWPAGE",    CodeNEWPAGE    , 0 },
  {"NESTMAX",    CodeNESTMAX    , 0 },
  {"NEXTENUM",   CodeENUM       , 1 },
  {"ORG",        CodeORG        , 0 },
  {"OUTRADIX",   CodeRADIX      , 1 },
  {"PHASE",      CodePHASE      , 0 },
  {"POPV",       CodePOPV       , 0 },
  {"PRSET",      CodePRSET      , 0 },
  {"PRTINIT",    CodeString     , 0 },
  {"PRTEXIT",    CodeString     , 1 },
  {"TITLE",      CodeString     , 2 },
  {"PUSHV",      CodePUSHV      , 0 },
  {"RADIX",      CodeRADIX      , 0 },
  {"READ",       CodeREAD       , 0 },
  {"RELAXED",    CodeRELAXED    , 0 },
  {"MACEXP",     CodeMACEXP     , 0x10 },
  {"MACEXP_DFT", CodeMACEXP     , 0 },
  {"MACEXP_OVR", CodeMACEXP     , 1 },
  {"RORG",       CodeRORG       , 0 },
  {"RSEG",       CodeSEGTYPE    , 0 },
  {"SECTION",    CodeSECTION    , 0 },
  {"SEGMENT",    CodeSEGMENT    , 0 },
  {"SHARED",     CodeSHARED     , 0 },
  {"STRUC",      CodeSTRUCT     , 0 },
  {"STRUCT",     CodeSTRUCT     , 0 },
  {"UNION",      CodeSTRUCT     , 1 },
  {"WARNING",    CodeWARNING    , 0 },
  {"=",          CodeSETEQU     , 0 },
  {":=",         CodeSETEQU     , 1 },
  {""       ,    NULL           , 0 }
};

Boolean CodeGlobalPseudo(void)
{
  switch (*OpPart.str.p_str)
  {
    case 'S':
      if (!SetIsOccupied() && Memo("SET"))
      {
        CodeSETEQU(True);
        return True;
      }
      else if (!SaveIsOccupied() && Memo("SAVE"))
      {
        CodeSAVE(0);
        return True;
      }
      break;
    case 'E':
      if (Memo("EVAL"))
      {
        CodeSETEQU(True);
        return True;
      }
      break;
    case 'P':
      if ((!PageIsOccupied && Memo("PAGE"))
       || (PageIsOccupied && Memo("PAGESIZE")))
      {
        CodePAGE(0);
        return True;
      }
      break;
    case 'R':
      if (!RestoreIsOccupied() && Memo("RESTORE"))
      {
        CodeRESTORE(0);
        return True;
      }
      break;
  }

  if (LookupInstTable(ONOFFTable, OpPart.str.p_str))
    return True;

  if (LookupInstTable(PseudoTable, OpPart.str.p_str))
    return True;

  if (SectionStack)
  {
    if (Memo("FORWARD"))
    {
      if (PassNo <= MaxSymPass)
        CodePPSyms(&(SectionStack->LocSyms),
                   &(SectionStack->GlobSyms),
                   &(SectionStack->ExportSyms));
      return True;
    }
    if (Memo("PUBLIC"))
    {
      CodePPSyms(&(SectionStack->GlobSyms),
                 &(SectionStack->LocSyms),
                 &(SectionStack->ExportSyms));
      return True;
    }
    if (Memo("GLOBAL"))
    {
      CodePPSyms(&(SectionStack->ExportSyms),
                 &(SectionStack->LocSyms),
                 &(SectionStack->GlobSyms));
      return True;
    }
  }

  return False;
}


void codeallg_init(void)
{
  const PseudoOrder *POrder;

  ONOFFList = (ONOFFTab*)calloc(ONOFFMax, sizeof(*ONOFFList));

  PseudoTable = CreateInstTable(201);
  for (POrder = Pseudos; POrder->Proc; POrder++)
    AddInstTable(PseudoTable, POrder->Name, POrder->Index, POrder->Proc);
  ONOFFTable = CreateInstTable(47);
  AddONOFF("DOTTEDSTRUCTS", &DottedStructs, DottedStructsName, True);
}
