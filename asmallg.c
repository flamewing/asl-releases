/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* von allen Codegeneratoren benutzte Pseudobefehle                          */
/*                                                                           */
/* Historie:  10. 5.1996 Grundsteinlegung                                    */
/*            24. 6.1998 CODEPAGE-Kommando                                   */
/*            17. 7.1998 CHARSET ohne Argumente                              */
/*            17. 8.1998 BookKeeping-Aufruf bei ALIGN                        */
/*            18. 8.1998 RADIX-Kommando                                      */
/*             2. 1.1999 Standard-ChkPC-Routine                              */
/*             9. 1.1999 BINCLUDE gefixt...                                  */
/*            30. 5.1999 OUTRADIX-Kommando                                   */
/*            12. 7.1999 EXTERN-Kommando                                     */
/*             8. 3.2000 'ambigious else'-Warnungen beseitigt                */
/*             1. 6.2000 reset error flag before checks in BINCLUDE          */
/*                       added NESTMAX instruction                           */
/*            26. 6.2000 added EXPORT instruction                            */
/*             6. 7.2000 unknown symbol in EXPORT triggers repassing         */
/*            30.10.2000 EXTERN also works for symbols without a segment spec*/
/*             1.11.2000 added ASEG/RSEG                                     */
/*            14. 1.2001 silenced warnings about unused parameters           */
/*                       use SegInits also when segment hasn't been used up  */
/*                       to now                                              */
/*                                                                           */
/*****************************************************************************/
/* $Id: asmallg.c,v 1.38 2017/06/28 16:35:15 alfred Exp $                     */
/*****************************************************************************
 * $Log: asmallg.c,v $
 * Revision 1.38  2017/06/28 16:35:15  alfred
 * - correct function call
 *
 * Revision 1.37  2017/06/03 08:25:00  alfred
 * - silence warning about unused argument
 *
 * Revision 1.36  2017/04/02 11:10:36  alfred
 * - allow more fine-grained macro expansion in listing
 *
 * Revision 1.35  2017/02/26 17:11:20  alfred
 * - allow alternate syntax for SET and EQU
 *
 * Revision 1.34  2017/02/26 16:20:46  alfred
 * - silence compiler warnings about unused function results
 *
 * Revision 1.33  2016/11/25 18:12:12  alfred
 * - first version to support OLMS-50
 *
 * Revision 1.32  2016/11/25 16:29:36  alfred
 * - allow SELECT as alternative to SWITCH
 *
 * Revision 1.31  2016/11/24 22:41:45  alfred
 * - add SELECT as alternative to SWITCH
 *
 * Revision 1.30  2016/10/07 20:03:03  alfred
 * - make some arguments const
 *
 * Revision 1.29  2016/09/12 18:44:53  alfred
 * - fix memory leak
 *
 * Revision 1.28  2015/10/28 17:54:33  alfred
 * - allow substructures of same name in different structures
 *
 * Revision 1.27  2015/10/23 08:43:33  alfred
 * - beef up & fix structure handling
 *
 * Revision 1.26  2015/10/20 17:31:25  alfred
 * - put all pseudo instructions into table for init
 *
 * Revision 1.25  2015/10/18 19:02:16  alfred
 * - first reork/fix of nested structure handling
 *
 * Revision 1.24  2015/10/06 16:42:23  alfred
 * - repair SHARED output in C mode
 *
 * Revision 1.23  2015/10/05 21:41:27  alfred
 * - correct C shared symbol output
 *
 * Revision 1.22  2014/12/14 17:58:46  alfred
 * - remove static variables in strutil.c
 *
 * Revision 1.21  2014/12/12 17:16:08  alfred
 * - repainr -cpu comman dline switch and provide test for it
 *
 * Revision 1.20  2014/12/07 19:13:58  alfred
 * - silence a couple of Borland C related warnings and errors
 *
 * Revision 1.19  2014/12/01 10:06:03  alfred
 * - remove trailing blank
 *
 * Revision 1.18  2014/12/01 09:26:14  alfred
 * - rework to current style
 *
 * Revision 1.17  2014/11/23 18:29:29  alfred
 * - correct buffer overflow in MomCPUName
 *
 * Revision 1.16  2014/11/05 09:28:34  alfred
 * - move array from BSS segement to hep
 *
 * Revision 1.15  2014/10/06 17:55:27  alfred
 * - remove debug printf
 *
 * Revision 1.14  2014/03/08 21:06:34  alfred
 * - rework ASSUME framework
 *
 * Revision 1.13  2014/03/08 17:26:14  alfred
 * - print out declaration position for unresolved forwards
 *
 * Revision 1.12  2014/03/08 16:18:46  alfred
 * - add RORG statement
 *
 * Revision 1.11  2012-05-26 13:49:19  alfred
 * - MSP additions, make implicit macro parameters always case-insensitive
 *
 * Revision 1.10  2012-01-16 19:31:18  alfred
 * - regard symbol name expansion in arguments for SHARED
 *
 * Revision 1.9  2010/08/27 14:52:41  alfred
 * - some more overlapping strcpy() cleanups
 *
 * Revision 1.8  2010/04/17 13:14:19  alfred
 * - address overlapping strcpy()
 *
 * Revision 1.7  2008/11/23 10:39:15  alfred
 * - allow strings with NUL characters
 *
 * Revision 1.6  2007/11/24 22:48:02  alfred
 * - some NetBSD changes
 *
 * Revision 1.5  2007/04/30 18:37:51  alfred
 * - add weird integer coding
 *
 * Revision 1.4  2006/08/05 18:26:32  alfred
 * - remove static string
 *
 * Revision 1.3  2005/10/02 10:00:43  alfred
 * - ConstLongInt gets default base, correct length check on KCPSM3 registers
 *
 * Revision 1.2  2005/08/07 10:29:24  alfred
 * - remove mnemonic conflict with MICO8
 *
 * Revision 1.1  2003/11/06 02:49:18  alfred
 * - recreated
 *
 * Revision 1.12  2003/05/02 21:23:08  alfred
 * - strlen() updates
 *
 * Revision 1.11  2003/02/06 07:38:09  alfred
 * - add missing 'else'
 *
 * Revision 1.10  2003/02/02 12:05:01  alfred
 * - limit BINCLUDE transfer size to 256 bytes
 *
 * Revision 1.9  2002/11/23 15:53:27  alfred
 * - SegLimits are unsigned now
 *
 * Revision 1.8  2002/11/20 20:25:04  alfred
 * - added unions
 *
 * Revision 1.7  2002/11/17 16:09:12  alfred
 * - added DottedStructs
 *
 * Revision 1.6  2002/11/16 20:51:15  alfred
 * - adapted to structure changes
 *
 * Revision 1.5  2002/11/11 21:13:37  alfred
 * - add structure storage
 *
 * Revision 1.4  2002/11/11 19:24:57  alfred
 * - new module for structs
 *
 * Revision 1.3  2002/11/04 19:18:00  alfred
 * - add DOTS option for structs
 *
 * Revision 1.2  2002/07/14 18:39:57  alfred
 * - fixed TempAll-related warnings
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "strutil.h"
#include "stringlists.h"
#include "bpemu.h"
#include "cmdarg.h"
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
#include "codepseudo.h"
#include "nlmessages.h"

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

static void SetCPUCore(const tCPUDef *pCPUDef, Boolean NotPrev)
{
  LargeInt HCPU;
  char *z, *dest;
  Boolean ECPU;
  char s[20];

  strmaxcpy(MomCPUIdent, pCPUDef->Name, sizeof(MomCPUIdent));
  MomCPU = pCPUDef->Orig;
  MomVirtCPU = pCPUDef->Number;
  strmaxcpy(s, MomCPUIdent, sizeof(s));
  for (z = dest = s; *z; z++)
    if (isxdigit(*z))
      *(dest++) = (*z);
  *dest = '\0';
  for (z = s; *z != '\0'; z++)
    if (isdigit(*z))
      break;
  if (*z)
    strmov(s, z);
  strprep(s, "$");
  HCPU = ConstLongInt(s, &ECPU, 10);
  if (ParamCount != 0)
  {
    EnterIntSymbol(MomCPUName, HCPU, SegNone, True);
    EnterStringSymbol(MomCPUIdentName, MomCPUIdent, True);
  }

  InternSymbol = Default_InternSymbol;
  SetIsOccupied = SwitchIsOccupied = PageIsOccupied = False;
  ChkPC = DefChkPC;
  ASSUMERecCnt = 0;
  pASSUMERecs = NULL;
  pASSUMEOverride = NULL;
  if (!NotPrev) SwitchFrom();
  pCPUDef->SwitchProc(pCPUDef->pUserData);

  DontPrint = True;
}

void SetCPU(CPUVar NewCPU, Boolean NotPrev)
{
  const tCPUDef *pCPUDef;

  pCPUDef = LookupCPUDefByVar(NewCPU);
  if (pCPUDef)
    SetCPUCore(pCPUDef, NotPrev);
}

Boolean SetNCPU(const char *pName, Boolean NoPrev)
{
  const tCPUDef *pCPUDef;

  pCPUDef = LookupCPUDefByName(pName);
  if (!pCPUDef)
    return False;
  else
  {
    SetCPUCore(pCPUDef, NoPrev);
    return True;
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

static void IntLine(char *pDest, int DestSize, LongInt Inp)
{
  HexString(pDest, DestSize, Inp, 0);
  switch (ConstMode)
  {
    case ConstModeIntel:
      strmaxcat(pDest, "H", DestSize);
      if (*pDest > '9')
        strmaxprep(pDest, "0", DestSize);
      break;
    case ConstModeMoto:
      strmaxprep(pDest, "$", DestSize);
      break;
    case ConstModeC:
      strmaxprep(pDest, "0x", DestSize);
      break;
    case ConstModeWeird:
      strmaxprep(pDest, "x'", DestSize);
      strmaxcat(pDest, "'", DestSize);
      break;
  }
}


static void CodeSECTION(Word Index)
{
  PSaveSection Neu;
  UNUSED(Index);

  if (ChkArgCnt(1, 1)
   && ExpandSymbol(ArgStr[1].Str))
  {
    if (!ChkSymbName(ArgStr[1].Str)) WrStrErrorPos(ErrNum_InvSymName, &ArgStr[1]);
    else if ((PassNo == 1) && (GetSectionHandle(ArgStr[1].Str, False, MomSectionHandle) != -2)) WrError(ErrNum_DoubleSection);
    else
    {
      Neu = (PSaveSection) malloc(sizeof(TSaveSection));
      Neu->Next = SectionStack;
      Neu->Handle = MomSectionHandle;
      Neu->LocSyms = NULL;
      Neu->GlobSyms = NULL;
      Neu->ExportSyms = NULL;
      SetMomSection(GetSectionHandle(ArgStr[1].Str, True, MomSectionHandle));
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
    strmaxcpy(XError, (*Root)->Name, 255);
    strmaxcat(XError, ", ", 255);
    strmaxcat(XError, (*Root)->pErrorPos, 255);
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
  UNUSED(Index);

  if (!ChkArgCnt(0, 1));
  else if (!SectionStack) WrError(ErrNum_NotInSection);
  else if ((ArgCnt == 0) || (ExpandSymbol(ArgStr[1].Str)))
  {
    if ((ArgCnt == 1) && (GetSectionHandle(ArgStr[1].Str, False, SectionStack->Handle) != MomSectionHandle)) WrError(ErrNum_WrongEndSect);
    else
    {
      Tmp = SectionStack;
      SectionStack = Tmp->Next;
      CodeENDSECTION_ChkEmptList(&(Tmp->LocSyms));
      CodeENDSECTION_ChkEmptList(&(Tmp->GlobSyms));
      CodeENDSECTION_ChkEmptList(&(Tmp->ExportSyms));
      TossRegDefs(MomSectionHandle);
      if (ArgCnt == 0)
        sprintf(ListLine, "[%s]", GetSectionName(MomSectionHandle));
      SetMomSection(Tmp->Handle);
      free(Tmp);
    }
  }
}


static void CodeCPU(Word Index)
{
  UNUSED(Index);

  if (!ChkArgCnt(1, 1));
  else if (*AttrPart.Str != '\0') WrError(ErrNum_UseLessAttr);
  else
  {
    NLS_UpString(ArgStr[1].Str);
    if (SetNCPU(ArgStr[1].Str, False))
      SetNSeg(SegCode);
    else
      WrStrErrorPos(ErrNum_InvCPUType, &ArgStr[1]);
  }
}


static void CodeSETEQU(Word MayChange)
{
  TempResult t;
  Integer DestSeg;
  int ValIndex = *LabPart.Str ? 1 : 2;

  FirstPassUnknown = False;
  if (*AttrPart.Str != '\0') WrError(ErrNum_UseLessAttr);
  else if (ChkArgCnt(ValIndex, ValIndex + 1))
  {
    EvalStrExpression(&ArgStr[ValIndex], &t);
    if (!FirstPassUnknown)
    {
      if (ArgCnt == ValIndex)
        DestSeg = SegNone;
      else
      {
        NLS_UpString(ArgStr[ValIndex + 1].Str);
        if (!strcmp(ArgStr[ValIndex + 1].Str, "MOMSEGMENT"))
          DestSeg = ActPC;
        else if (*ArgStr[ValIndex + 1].Str == '\0')
          DestSeg = SegNone;
        else
        {
          for (DestSeg = 0; DestSeg <= PCMax; DestSeg++)
            if (!strcmp(ArgStr[ValIndex + 1].Str, SegNames[DestSeg]))
              break;
        }
      }
      if (DestSeg > PCMax) WrStrErrorPos(ErrNum_UnknownSegment, &ArgStr[ValIndex + 1]);
      else
      {
        const char *pName = *LabPart.Str ? LabPart.Str : ArgStr[1].Str;

        SetListLineVal(&t);
        PushLocHandle(-1);
        switch (t.Typ)
        {
          case TempInt:
            EnterIntSymbol(pName, t.Contents.Int, DestSeg, MayChange);
            break;
          case TempFloat:
            EnterFloatSymbol(pName, t.Contents.Float, MayChange);
            break;
          case TempString:
            EnterDynStringSymbol(pName, &t.Contents.Ascii, MayChange);
            break;
          default:
            break;
        }
        PopLocHandle();
      }
    }
  }
}


static void CodeORG(Word Index)
{
  LargeInt HVal;
  Boolean ValOK;
  UNUSED(Index);

  FirstPassUnknown = False;
  if (*AttrPart.Str != '\0') WrError(ErrNum_UseLessAttr);
  else if (ChkArgCnt(1, 1))
  {
#ifndef HAS64
    HVal = EvalStrIntExpression(&ArgStr[1], UInt32, &ValOK);
#else
    HVal = EvalStrIntExpression(&ArgStr[1], Int64, &ValOK);
#endif
    if (FirstPassUnknown) WrError(ErrNum_FirstPassCalc);
    if ((ValOK) && (!FirstPassUnknown))
    {
      PCs[ActPC] = HVal;
      DontPrint = True;
    }
  }
}

static void CodeRORG(Word Index)
{
  LargeInt HVal;
  Boolean ValOK;
  UNUSED(Index);

  FirstPassUnknown = False;

  if (*AttrPart.Str != '\0') WrError(ErrNum_UseLessAttr);
  else if (ChkArgCnt(1, 1))
  {
#ifndef HAS64
    HVal = EvalStrIntExpression(&ArgStr[1], SInt32, &ValOK);
#else
    HVal = EvalStrIntExpression(&ArgStr[1], Int64, &ValOK);
#endif
    if (FirstPassUnknown) WrError(ErrNum_FirstPassCalc);
    else if (ValOK)
    {
      PCs[ActPC] += HVal;
      DontPrint = True;
    }
  }
}

static void CodeSHARED_BuildComment(char *c)
{
  switch (ShareMode)
  {
    case 1:
      sprintf(c, "(* %s *)", CommPart.Str);
      break;
    case 2:
      sprintf(c, "/* %s */", CommPart.Str);
      break;
    case 3:
      sprintf(c, "; %s", CommPart.Str);
      break;
  }
}

static void CodeSHARED(Word Index)
{
  tStrComp *pArg;
  Boolean ValOK;
  LargeInt HVal;
  Double FVal;
  String s, c;
  UNUSED(Index);

  if (ShareMode == 0) WrError(ErrNum_NoShareFile);
  else if ((ArgCnt == 0) && (*CommPart.Str != '\0'))
  {
    CodeSHARED_BuildComment(c);
    errno = 0;
    fprintf(ShareFile, "%s\n", c); ChkIO(10004);
  }
  else
   forallargs (pArg, True)
   {
     if (!ExpandSymbol(pArg->Str))
       continue;
     if (!ChkSymbName(pArg->Str))
     {
       WrStrErrorPos(ErrNum_InvSymName, pArg);
       continue;
     }
     if (IsSymbolString(pArg->Str))
     {
       ValOK = GetStringSymbol(pArg->Str, s);
       if (ShareMode == 1)
       {
         strmaxprep(s, "\'", 255);
         strmaxcat(s, "\'", 255);
       }
       else
       {
         strmaxprep(s, "\"", 255);
         strmaxcat(s, "\"", 255);
       }
     }
     else if (IsSymbolFloat(pArg->Str))
     {
       ValOK = GetFloatSymbol(pArg->Str, &FVal);
       sprintf(s, "%0.17g", FVal);
     }
     else
     {
       ValOK = GetIntSymbol(pArg->Str, &HVal, NULL);
       switch (ShareMode)
       {
         case 1:
           s[0] = '$';
           HexString(s + 1, sizeof(s) - 1, HVal, 0);
           break;
         case 2:
           s[0] = '0';
           s[1] = 'x';
           HexString(s + 2, sizeof(s) - 2, HVal, 0);
           break;
         case 3:
           IntLine(s, sizeof(s), HVal);
           break;
       }
     }
     if (ValOK)
     {
       if ((pArg == ArgStr + 1) && (*CommPart.Str != '\0'))
       {
         CodeSHARED_BuildComment(c);
         strmaxprep(c, " ", 255);
       }
       else
         *c = '\0';
       errno = 0;
       switch (ShareMode)
       {
         case 1:
           fprintf(ShareFile, "%s = %s;%s\n", pArg->Str, s, c);
           break;
         case 2:
           fprintf(ShareFile, "#define %s %s%s\n", pArg->Str, s, c);
           break;
         case 3:
           strmaxprep(s, IsSymbolChangeable(pArg->Str) ? "set " : "equ ", 255);
           fprintf(ShareFile, "%s %s%s\n", pArg->Str, s, c);
           break;
       }
       ChkIO(10004);
     }
     else if (PassNo == 1)
     {
       Repass = True;
       if ((MsgIfRepass) && (PassNo >= PassNoForMessage))
         WrStrErrorPos(ErrNum_RepassUnknown, pArg);
     }
   }
}

static void CodeEXPORT(Word Index)
{
  tStrComp *pArg;
  LargeInt Value;
  PRelocEntry Relocs;
  UNUSED(Index);

  forallargs (pArg, True)
  {
    FirstPassUnknown = True;
    if (GetIntSymbol(pArg->Str, &Value, &Relocs))
    {
      if (Relocs == NULL)
        AddExport(pArg->Str, Value, 0);
      else if ((Relocs->Next != NULL) || (strcmp(Relocs->Ref, RelName_SegStart)))
        WrStrErrorPos(ErrNum_Unexportable, pArg);
      else
        AddExport(pArg->Str, Value, RelFlag_Relative);
    }
    else
    {
      Repass = True;
      if ((MsgIfRepass) && (PassNo >= PassNoForMessage))
        WrStrErrorPos(ErrNum_RepassUnknown, pArg);
    }
  }
}

static void CodePAGE(Word Index)
{
  Integer LVal, WVal;
  Boolean ValOK;
  UNUSED(Index);

  if (!ChkArgCnt(1, 2));
  else if (*AttrPart.Str != '\0') WrError(ErrNum_UseLessAttr);
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
  else if (*AttrPart.Str != '\0') WrError(ErrNum_UseLessAttr);
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
          strmaxcpy(PrtInitString, tmp, 255);
          break;
        case 1:
          strmaxcpy(PrtExitString, tmp, 255);
          break;
        case 2:
          strmaxcpy(PrtTitleString, tmp, 255);
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
      printf("%s%s\n", mess, ClrEol);
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
  TempResult t;
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
    FirstPassUnknown = False;
    EvalStrExpression(&ArgStr[1], &t);
    switch (t.Typ)
    {
      case TempInt:
        if (FirstPassUnknown)
          t.Contents.Int &= 255;
        if (ChkRange(t.Contents.Int, 0, 255))
        {
          if (ChkArgCnt(2, 3))
          {
            Start = t.Contents.Int;
            FirstPassUnknown = False;
            EvalStrExpression(&ArgStr[2], &t);
            switch (t.Typ)
            {
              case TempInt: /* Übersetzungsbereich als Character-Angabe */
                if (FirstPassUnknown)
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
                l = t.Contents.Ascii.Length; /* Uebersetzungsstring ab Start */
                if (Start + l > 256) WrError(ErrNum_OverRange);
                else
                  for (z = 0; z < l; z++)
                    CharTransTable[Start + z] = t.Contents.Ascii.Contents[z];
                break;
              case TempFloat:
                WrError(ErrNum_InvOpType);
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

          DynString2CString(Tmp, &t.Contents.Ascii, sizeof(Tmp));
          f = fopen(Tmp, OPENRDMODE);
          if (!f) ChkIO(10001);
          if (fread(tfield, sizeof(char), 256, f) != 256) ChkIO(10003);
          fclose(f);
          memcpy(CharTransTable, tfield, sizeof(char) * 256);
        }
        break;
      case TempFloat:
        WrError(ErrNum_InvOpType);
        break;
      default:
        break;
    }
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
  else if (!ChkSymbName(ArgStr[1].Str)) WrStrErrorPos(ErrNum_InvSymName, &ArgStr[1]);
  else
  {
    if (!CaseSensitive)
    {
      UpString(ArgStr[1].Str);
      if (ArgCnt == 2)
        UpString(ArgStr[2].Str);
    }

    if (ArgCnt == 1)
      Source = CurrTransTable;
    else
    {
      for (Source = TransTables; Source; Source = Source->Next)
        if (!strcmp(Source->Name, ArgStr[2].Str))
          break;
    }

    if (!Source) WrStrErrorPos(ErrNum_UnknownCodepage, &ArgStr[2]);
    else
    {
      for (Prev = NULL, Run = TransTables; Run; Prev = Run, Run = Run->Next)
        if ((erg = strcmp(ArgStr[1].Str, Run->Name)) <= 0)
          break;

      if ((!Run) || (erg < 0))
      {
        New = (PTransTable) malloc(sizeof(TTransTable));
        New->Next = Run;
        New->Name = as_strdup(ArgStr[1].Str);
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
  String FName;
  Boolean OK;
  int z;
  UNUSED(Index);

  if (ChkArgCnt(2, ArgCntMax))
  {
    OK = True;
    z = 1;
    do
    {
      OK = (OK && ChkMacSymbName(ArgStr[z].Str));
      if (!OK)
        WrStrErrorPos(ErrNum_InvSymName, &ArgStr[z]);
      z++;
    }
    while ((z < ArgCnt) && (OK));
    if (OK)
    {
      strmaxcpy(FName, ArgStr[ArgCnt].Str, 255);
      for (z = 1; z < ArgCnt; z++)
        CompressLine(ArgStr[z].Str, z, FName, STRINGSIZE, CaseSensitive);
      EnterFunction(LabPart.Str, FName, ArgCnt - 1);
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
    Neu->SavePC = ActPC;
    Neu->SaveListOn = ListOn;
    Neu->SaveLstMacroExp = LstMacroExp;
    Neu->SaveLstMacroExpOverride = LstMacroExpOverride;
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
    Old = FirstSaveState; FirstSaveState = Old->Next;
    if (Old->SavePC != ActPC)
    {
      ActPC = Old->SavePC;
      DontPrint = True;
    }
    if (Old->SaveCPU != MomCPU)
      SetCPU(Old->SaveCPU, False);
    EnterIntSymbol(ListOnName, ListOn = Old->SaveListOn, 0, True);
    SetLstMacroExp(Old->SaveLstMacroExp);
    LstMacroExpOverride = Old->SaveLstMacroExpOverride;
    CurrTransTable = Old->SaveTransTable;
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

    sprintf(Msg, getmessage(Num_ErrMsgDeprecated_Instead), "MACEXP_DFT");
    WrXError(ErrNum_Deprecated, Msg);
  }
#endif

  /* allow zero arguments for MACEXP_OVR, to remove all overrides */

  if (!ChkArgCnt((Index & 0x0f) ? 0 : 1, ArgCntMax));
  else if (*AttrPart.Str != '\0') WrError(ErrNum_UseLessAttr);
  else
  {
    tStrComp *pArg;
    tLstMacroExpMod LstMacroExpMod;
    Boolean OK = True;

    InitLstMacroExpMod(&LstMacroExpMod);
    forallargs (pArg, True)
    {
      if (!strcasecmp(pArg->Str, "ON")) LstMacroExpMod.SetAll = True;
      else if (!strcasecmp(pArg->Str, "OFF")) LstMacroExpMod.ClrAll = True;
      else if (!strcasecmp(pArg->Str, "NOIF")) LstMacroExpMod.ANDMask |= eLstMacroExpIf;
      else if (!strcasecmp(pArg->Str, "NOMACRO")) LstMacroExpMod.ANDMask |= eLstMacroExpMacro;
      else if (!strcasecmp(pArg->Str, "IF")) LstMacroExpMod.ORMask |= eLstMacroExpIf;
      else if (!strcasecmp(pArg->Str, "MACRO")) LstMacroExpMod.ORMask |= eLstMacroExpMacro;
      else
        OK = False;
      if (!OK)
      {
        WrStrErrorPos(ErrNum_UnknownMacExpMod, pArg);
        break;
      }
    }
    if (OK)
    {
      if (!ChkLstMacroExpMod(&LstMacroExpMod)) WrError(ErrNum_ConflictingMacExpMod);
      else if (Index) /* Override */
        LstMacroExpOverride = LstMacroExpMod;
      else
        SetLstMacroExp(ApplyLstMacroExpMod(LstMacroExp, &LstMacroExpMod));
    }
  }
}

static Boolean DecodeSegment(char *pArg, Integer StartSeg, Integer *pResult)
{
  Integer SegZ;
  Word Mask;

  NLS_UpString(pArg);
  for (SegZ = StartSeg, Mask = 1 << StartSeg; SegZ <= PCMax; SegZ++, Mask <<= 1)
    if ((ValidSegs & Mask) && !strcmp(pArg, SegNames[SegZ]))
    {
      *pResult = SegZ;
      return True;
    }
  WrXError(ErrNum_UnknownSegment, pArg);
  return False;
}

static void CodeSEGMENT(Word Index)
{
  Integer NewSegment;
  UNUSED(Index);

  if (ChkArgCnt(1, 1)
   && DecodeSegment(ArgStr[1].Str, SegCode, &NewSegment))
    SetNSeg(NewSegment);
}


static void CodeLABEL(Word Index)
{
  LongInt Erg;
  Boolean OK;
  UNUSED(Index);

  FirstPassUnknown = False;
  if (ChkArgCnt(1, 1))
  {
    Erg = EvalStrIntExpression(&ArgStr[1], Int32, &OK);
    if ((OK) && (!FirstPassUnknown))
    {
      char s[40];

      PushLocHandle(-1);
      EnterIntSymbol(LabPart.Str, Erg, SegCode, False);
      IntLine(s, sizeof(s), Erg);
      sprintf(ListLine, "=%s", s);
      PopLocHandle();
    }
  }
}


static void CodeREAD(Word Index)
{
  String ExpStr;
  tStrComp Exp;
  TempResult Erg;
  Boolean OK;
  LongInt SaveLocHandle;
  UNUSED(Index);

  StrCompMkTemp(&Exp, ExpStr);
  if (ChkArgCnt(1, 2))
  {
    if (ArgCnt == 2) EvalStrStringExpression(&ArgStr[1], &OK, Exp.Str);
    else
    {
      sprintf(Exp.Str, "Read %s ? ", ArgStr[1].Str);
      OK = True;
    }
    if (OK)
    {
      printf("%s", Exp.Str);
      fflush(stdout);
      if (!fgets(Exp.Str, 255, stdin))
        OK = False;
      else
      {
        UpString(Exp.Str);
        FirstPassUnknown = False;
        EvalStrExpression(&Exp, &Erg);
      }
      if (OK)
      {
        SetListLineVal(&Erg);
        SaveLocHandle = MomLocHandle;
        MomLocHandle = -1;
        if (FirstPassUnknown) WrError(ErrNum_FirstPassCalc);
        else switch (Erg.Typ)
        {
          case TempInt:
            EnterIntSymbol(ArgStr[ArgCnt].Str, Erg.Contents.Int, SegNone, True);
            break;
          case TempFloat:
            EnterFloatSymbol(ArgStr[ArgCnt].Str, Erg.Contents.Float, True);
            break;
          case TempString:
          {
            String Tmp;

            DynString2CString(Tmp, &Erg.Contents.Ascii, sizeof(Tmp));
            EnterStringSymbol(ArgStr[ArgCnt].Str, Tmp, True);
            break;
          }
          default:
            break;
        }
        MomLocHandle = SaveLocHandle;
      }
    }
  }
}

static void CodeRADIX(Word Index)
{
  Boolean OK;
  LargeWord tmp;

  if (ChkArgCnt(1, 1))
  {
    tmp = ConstLongInt(ArgStr[1].Str, &OK, 10);
    if (!OK) WrError(ErrNum_InvOpType);
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
    LongInt NewPC;

    FirstPassUnknown = False;
    if (2 == ArgCnt)
      AlignFill = EvalStrIntExpression(&ArgStr[2], Int8, &OK);
    FirstPassUnknown = False;
    if (OK)
      AlignValue = EvalStrIntExpression(&ArgStr[1], Int16, &OK);
    if (OK)
    {
      if (FirstPassUnknown) WrError(ErrNum_FirstPassCalc);
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
  LongInt HVal;
  tStrComp RegPart, ValPart;
  char *pSep;

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
      pSep = QuotPos(ArgStr[z1].Str, ':');
      if (pSep)
        StrCompSplitRef(&RegPart, &ValPart, &ArgStr[z1], pSep);
      else
      {
        RegPart = ArgStr[z1];
        ValPart.Str = "";
        LineCompReset(&ValPart.Pos);
      }
      z2 = 0;
      NLS_UpString(RegPart.Str);
      while ((z2 < ASSUMERecCnt) && (strcmp(pASSUMERecs[z2].Name, RegPart.Str)))
        z2++;
      OK = (z2 < ASSUMERecCnt);
      if (!OK) WrStrErrorPos(ErrNum_InvRegName, &RegPart);
      else
      {
        if (!strcasecmp(ValPart.Str, "NOTHING"))
        {
          if (pASSUMERecs[z2].NothingVal == -1) WrError(ErrNum_InvAddrMode);
          else
            *(pASSUMERecs[z2].Dest) = pASSUMERecs[z2].NothingVal;
        }
        else
        {
          FirstPassUnknown = False;
          HVal = EvalStrIntExpression(&ValPart, Int32, &OK);
          if (OK)
          {
            if (FirstPassUnknown)
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
  LongInt  First = 0, Last = 0;
  String ListSymPart;
  tStrComp SymPart;

  if (!IsNext)
    EnumCurrentValue = 0;
  if (ChkArgCnt(1, ArgCntMax))
  {
    for (z = 1; z <= ArgCnt; z++)
    {
      p = QuotPos(ArgStr[z].Str, '=');
      if (p)
      {
        StrCompSplitRef(&ArgStr[z], &SymPart, &ArgStr[z], p);
        FirstPassUnknown = False;
        EnumCurrentValue = EvalStrIntExpression(&SymPart, Int32, &OK);
        if (!OK)
          return;
        if (FirstPassUnknown)
        {
          WrStrErrorPos(ErrNum_FirstPassCalc, &SymPart);
          return;
        }
        *p = '\0';
      }
      EnterIntSymbol(ArgStr[z].Str, EnumCurrentValue, EnumSegment, False);
      if (z == 1)
        First = EnumCurrentValue;
      else if (z == ArgCnt)
        Last = EnumCurrentValue;
      EnumCurrentValue += EnumIncrement;
    }
  }
  IntLine(ListSymPart, sizeof(ListSymPart), First);
  sprintf(ListLine, "=%s", ListSymPart);
  if (ArgCnt != 1)
  {
    strmaxcat(ListLine, "..", 255);
    IntLine(ListSymPart, sizeof(ListSymPart), Last);
    strmaxcat(ListLine, ListSymPart, 255);
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

      if (DecodeSegment(ArgStr[2].Str, SegNone, &NewSegment))
        EnumSegment = NewSegment;
    }
  }
}

static void CodeEND(Word Index)
{
  LongInt HVal;
  Boolean OK;
  UNUSED(Index);

  if (ChkArgCnt(0, 1))
  {
    if (ArgCnt == 1)
    {
      FirstPassUnknown = False;
      HVal = EvalStrIntExpression(&ArgStr[1], Int32, &OK);
      if ((OK) && (!FirstPassUnknown))
      {
        ChkSpace(SegCode);
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
  else if (*AttrPart.Str != '\0') WrError(ErrNum_UseLessAttr);
  else
  {
    OK = True;
    NLS_UpString(ArgStr[1].Str);
    if (!strcmp(ArgStr[1].Str, "OFF"))
      Value = 0;
    else if (!strcmp(ArgStr[1].Str, "ON"))
      Value = 1;
    else if (!strcmp(ArgStr[1].Str, "NOSKIPPED"))
      Value = 2;
    else if (!strcmp(ArgStr[1].Str, "PURECODE"))
      Value = 3;
    else
      OK = False;
    if (!OK) WrStrErrorPos(ErrNum_OnlyOnOff, &ArgStr[1]);
    else
      EnterIntSymbol(ListOnName, ListOn = Value, 0, True);
  }
}

static void CodeBINCLUDE(Word Index)
{
  FILE *F;
  LongInt Len = -1;
  LongWord Ofs = 0, Curr, Rest, FSize;
  Word RLen;
  Boolean OK, SaveTurnWords;
  LargeWord OldPC;
  String Name;
  UNUSED(Index);

  if (!ChkArgCnt(1, 3));
  else if (ActPC == StructSeg) WrError(ErrNum_NotInStruct);
  else
  {
    if (ArgCnt == 1)
      OK = True;
    else
    {
      FirstPassUnknown = False;
      Ofs = EvalStrIntExpression(&ArgStr[2], Int32, &OK);
      if (FirstPassUnknown)
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
          Len = EvalStrIntExpression(&ArgStr[3], Int32, &OK);
          if (FirstPassUnknown)
          {
            WrError(ErrNum_FirstPassCalc);
            OK = False;
          }
        }
      }
    }
    if (OK)
    {
      strmaxcpy(Name, ArgStr[1].Str, 255);
      if (*Name == '"')
        strmov(Name, Name + 1);
      if ((*Name) && (Name[strlen(Name) - 1] == '"'))
        Name[strlen(Name) - 1] = '\0';
      strmaxcpy(ArgStr[1].Str, Name, 255);
      strmaxcpy(Name, FExpand(FSearch(Name, IncludeList)), 255);
      if ((*Name) && (Name[strlen(Name) - 1] == '/'))
        strmaxcat(Name, ArgStr[1].Str, 255);
      F = fopen(Name, OPENRDMODE);
      if (F == NULL) ChkIO(10001);
      errno = 0; FSize = FileSize(F); ChkIO(10003);
      if (Len == -1)
      {
        if ((Len = FSize - Ofs) < 0)
        {
          fclose(F); WrError(ErrNum_ShortRead); return;
        }
      }
      if (!ChkPC(PCs[ActPC] + Len - 1)) WrError(ErrNum_AdrOverflow);
      else
      {
        errno = 0; fseek(F, Ofs, SEEK_SET); ChkIO(10003);
        Rest = Len;
        SaveTurnWords = TurnWords;
        TurnWords = False;
        OldPC = ProgCounter();
        do
        {
          Curr = (Rest <= 256) ? Rest : 256;
          errno = 0; RLen = fread(BAsmCode, 1, Curr, F); ChkIO(10003);
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
      NLS_UpString(ArgStr[1].Str);
    for (z = 2; z <= ArgCnt; z++)
      PushSymbol(ArgStr[z].Str, ArgStr[1].Str);
  }
}

static void CodePOPV(Word Index)
{
  int z;
  UNUSED(Index);

  if (ChkArgCnt(2, ArgCntMax))
  {
    if (!CaseSensitive)
      NLS_UpString(ArgStr[1].Str);
    for (z = 2; z <= ArgCnt; z++)
      PopSymbol(ArgStr[z].Str, ArgStr[1].Str);
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

  if (!ChkArgCnt(0, 1))
    return;

  /* unnamed struct/union only allowed if embedded into at least one named struct/union */

  if (!*LabPart.Str)
  {
    if (!pInnermostNamedStruct)
    {
      WrError(ErrNum_FreestandingUnnamedStruct);
      return;
    }
  }
  else
  {
    if (!ChkSymbName(LabPart.Str))
    {
      WrXError(ErrNum_InvSymName, LabPart.Str);
      return;
    }
    if (!CaseSensitive)
      NLS_UpString(LabPart.Str);
  }

  /* compose name of nested structures */

  if (*LabPart.Str)
    BuildStructName(StructName, sizeof(StructName), LabPart.Str);
  else
    *StructName = '\0';

  /* If named and embedded into another struct, add as element to innermost named parent struct.
     Add up all offsets of unnamed structs in between. */

  if (StructStack && (*LabPart.Str))
  {
    PStructStack pRun;
    LargeWord Offset = ProgCounter();
    PStructElem pElement = CreateStructElem(LabPart.Str);

    for (pRun = StructStack; pRun && pRun != pInnermostNamedStruct; pRun = pRun->Next)
      Offset += pRun->SaveCurrPC;
    pElement->Offset = Offset;
    pElement->IsStruct = True;
    AddStructElem(pInnermostNamedStruct->StructRec, pElement);
    AddStructSymbol(LabPart.Str, ProgCounter());
  }

  NStruct = (PStructStack) malloc(sizeof(TStructStack));
  NStruct->Name = as_strdup(StructName);
  NStruct->pBaseName = NStruct->Name + strlen(NStruct->Name) - strlen(LabPart.Str); /* NULL -> complain too long */
  NStruct->SaveCurrPC = ProgCounter();
  DoExt = True;
  ExtChar = DottedStructs ? '.' : '_';
  NStruct->Next = StructStack;
  OK = True;
  forallargs (pArg, True)
    if (OK)
    {
      if (!strcasecmp(pArg->Str, "EXTNAMES"))
        DoExt = True;
      else if (!strcasecmp(pArg->Str, "NOEXTNAMES"))
        DoExt = False;
      else if (!strcasecmp(pArg->Str, "DOTS"))
        ExtChar = '.';
      else if (!strcasecmp(pArg->Str, "NODOTS"))
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
  TempResult t;

  if (!ChkArgCnt(0, 1));
  else if (!StructStack) WrError(ErrNum_MissingStruct);
  else
  {
    if (IsUnion && !StructStack->StructRec->IsUnion)
      WrXError(ErrNum_STRUCTEndedByENDUNION, StructStack->Name);

    if (*LabPart.Str == '\0') OK = True;
    else
    {
      if (!CaseSensitive)
        NLS_UpString(LabPart.Str);
      OK = !strcmp(LabPart.Str, StructStack->pBaseName);
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

          sprintf(tmp2, "%s%clen", OStruct->Name, OStruct->StructRec->ExtChar);
          EnterIntSymbol(tmp2, TotLen, SegNone, False);
        }
      }
      else
        EnterIntSymbol(ArgStr[1].Str, TotLen, SegNone, False);

      t.Typ = TempInt;
      t.Contents.Int = TotLen;
      SetListLineVal(&t);

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
  Byte Type;
  UNUSED(Index);

  if (ChkArgCnt(1, ArgCntMax))
  {
    i = 1;
    OK = True;
    while ((OK) && (i <= ArgCnt))
    {
      Split = strrchr(ArgStr[i].Str, ':');
      if (Split == NULL)
        Type = SegNone;
      else
      {
        *Split = '\0';
        for (Type = SegNone + 1; Type <= PCMax; Type++)
          if (!strcasecmp(Split + 1, SegNames[Type]))
            break;
      }
      if (Type > PCMax) WrXError(ErrNum_UnknownSegment, Split + 1);
      else
      {
        EnterExtSymbol(ArgStr[i].Str, 0, Type, FALSE);
      }
      i++;
    }
  }
}

static void CodeNESTMAX(Word Index)
{
  LongInt Temp;
  Boolean OK;
  UNUSED(Index);

  if (ChkArgCnt(1, 1))
  {
    FirstPassUnknown = False;
    Temp = EvalStrIntExpression(&ArgStr[1], UInt32, &OK);
    if (OK)
    {
      if (FirstPassUnknown) WrError(ErrNum_FirstPassCalc);
      else NestMax = Temp;
    }
  }
}

static void CodeSEGTYPE(Word Index)
{
  UNUSED(Index);

  if (ChkArgCnt(0, 0))
    RelSegs = (mytoupper(*OpPart.Str) == 'R');
}

static void CodePPSyms(PForwardSymbol *Orig,
                       PForwardSymbol *Alt1,
                       PForwardSymbol *Alt2)
{
  PForwardSymbol Lauf;
  tStrComp *pArg;
  String Sym, Section;

  if (ChkArgCnt(1, ArgCntMax))
    forallargs (pArg, True)
    {
      SplitString(pArg->Str, Sym, Section, QuotPos(pArg->Str, ':'));
      if (!ExpandSymbol(Sym)) return;
      if (!ExpandSymbol(Section)) return;
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
          IdentifySection(Section, &(Lauf->DestSection));
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

static void DecodeONOFF(Word Index)
{
  ONOFFTab *Tab = ONOFFList + Index;

  if (ChkArgCnt(1, 1))
  {
    NLS_UpString(ArgStr[1].Str);
    if (*AttrPart.Str != '\0') WrError(ErrNum_UseLessAttr);
    {
      Boolean IsON = !strcasecmp(ArgStr[1].Str, "ON");
      if ((!IsON) && (strcasecmp(ArgStr[1].Str, "OFF"))) WrStrErrorPos(ErrNum_OnlyOnOff, &ArgStr[1]);
      else
        SetFlag(Tab->FlagAddr, Tab->FlagName, IsON);
    }
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

/*------------------------------------------------------------------------*/

typedef struct
{
  char *Name;
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
  {"ENDS",       CodeENDSTRUCT  , 0 },
  {"ENDSECTION", CodeENDSECTION , 0 },
  {"ENDSTRUC",   CodeENDSTRUCT  , 0 },
  {"ENDSTRUCT",  CodeENDSTRUCT  , 0 },
  {"ENDUNION",   CodeENDSTRUCT  , 1 },
  {"ENUM",       CodeENUM       , 0 },
  {"ENUMCONF",   CodeENUMCONF   , 0 },
  {"EQU",        CodeSETEQU     , 0 },
  {"ERROR",      CodeERROR      , 0 },
  {"EXPORT_SYM", CodeEXPORT     , 0 },
  {"EXTERN_SYM", CodeEXTERN     , 0 },
  {"FATAL",      CodeFATAL      , 0 },
  {"FUNCTION",   CodeFUNCTION   , 0 },
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
  {"RESTORE",    CodeRESTORE    , 0 },
  {"MACEXP",     CodeMACEXP     , 0x10 },
  {"MACEXP_DFT", CodeMACEXP     , 0 },
  {"MACEXP_OVR", CodeMACEXP     , 1 },
  {"RORG",       CodeRORG       , 0 },
  {"RSEG",       CodeSEGTYPE    , 0 },
  {"SAVE",       CodeSAVE       , 0 },
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
  switch (*OpPart.Str)
  {
    case 'S':
      if ((!SetIsOccupied) && (Memo("SET")))
      {
        CodeSETEQU(True);
        return True;
      }
      break;
    case 'E':
      if ((SetIsOccupied) && (Memo("EVAL")))
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
  }

  if (LookupInstTable(ONOFFTable, OpPart.Str))
    return True;

  if (LookupInstTable(PseudoTable, OpPart.Str))
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
  AddONOFF("RELAXED", &RelaxedMode, RelaxedName, True);
  AddONOFF("DOTTEDSTRUCTS", &DottedStructs, DottedStructsName, True);
}
