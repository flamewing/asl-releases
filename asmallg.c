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
/* $Id: asmallg.c,v 1.28 2015/10/28 17:54:33 alfred Exp $                     */
/*****************************************************************************
 * $Log: asmallg.c,v $
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
#include "as.h"
#include "asmpars.h"
#include "asmmac.h"
#include "asmstructs.h"
#include "asmcode.h"
#include "asmrelocs.h"
#include "asmitree.h"
#include "codepseudo.h"

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

void SetCPU(CPUVar NewCPU, Boolean NotPrev)
{
  LongInt HCPU;
  char *z, *dest;
  Boolean ECPU;
  char s[20];
  PCPUDef Lauf;

  Lauf = FirstCPUDef;
  while ((Lauf) && (Lauf->Number != NewCPU))
    Lauf = Lauf->Next;
  if (!Lauf)
    return;

  strmaxcpy(MomCPUIdent, Lauf->Name, sizeof(MomCPUIdent));
  MomCPU = Lauf->Orig;
  MomVirtCPU = Lauf->Number;
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
  ChkPC = DefChkPC;
  ASSUMERecCnt = 0;
  pASSUMERecs = NULL;
  pASSUMEOverride = NULL;
  if (!NotPrev) SwitchFrom();
  Lauf->SwitchProc();

  DontPrint = True;
}

Boolean SetNCPU(char *Name, Boolean NoPrev)
{
  PCPUDef Lauf;

  Lauf = FirstCPUDef;
  while ((Lauf) && (strcmp(Name, Lauf->Name)))
    Lauf = Lauf->Next;
  if (!Lauf)
  {
    WrXError(1430, Name);
    return False;
  }
  else
  {
    SetCPU(Lauf->Number, NoPrev);
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

  if (ArgCnt != 1) WrError(1110);
  else if (ExpandSymbol(ArgStr[1]))
  {
    if (!ChkSymbName(ArgStr[1])) WrXError(1020, ArgStr[1]);
    else if ((PassNo == 1) && (GetSectionHandle(ArgStr[1], False, MomSectionHandle) != -2)) WrError(1483);
    else
    {
      Neu = (PSaveSection) malloc(sizeof(TSaveSection));
      Neu->Next = SectionStack;
      Neu->Handle = MomSectionHandle;
      Neu->LocSyms = NULL;
      Neu->GlobSyms = NULL;
      Neu->ExportSyms = NULL;
      SetMomSection(GetSectionHandle(ArgStr[1], True, MomSectionHandle));
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
    WrXError(1488, XError);
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

  if (ArgCnt > 1) WrError(1110);
  else if (!SectionStack) WrError(1487);
  else if ((ArgCnt == 0) || (ExpandSymbol(ArgStr[1])))
  {
    if ((ArgCnt == 1) && (GetSectionHandle(ArgStr[1], False, SectionStack->Handle) != MomSectionHandle)) WrError(1486);
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

  if (ArgCnt != 1) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else
  {
    NLS_UpString(ArgStr[1]);
    if (SetNCPU(ArgStr[1], False))
      SetNSeg(SegCode);
  }
}


static void CodeSETEQU(Word MayChange)
{
  TempResult t;
  Integer DestSeg;

  FirstPassUnknown = False;
  if (*AttrPart != '\0') WrError(1100);
  else if ((ArgCnt < 1) || (ArgCnt > 2)) WrError(1110);
  else
  {
    EvalExpression(ArgStr[1], &t);
    if (!FirstPassUnknown)
    {
      if (ArgCnt == 1)
        DestSeg = SegNone;
      else
      {
        NLS_UpString(ArgStr[2]);
        if (!strcmp(ArgStr[2], "MOMSEGMENT"))
          DestSeg = ActPC;
        else if (*ArgStr[2] == '\0')
          DestSeg = SegNone;
        else
        {
          for (DestSeg = 0; DestSeg <= PCMax; DestSeg++)
            if (!strcmp(ArgStr[2], SegNames[DestSeg]))
              break;
        }
      }
      if (DestSeg > PCMax) WrXError(1961, ArgStr[2]);
      else
      {
        SetListLineVal(&t);
        PushLocHandle(-1);
        switch (t.Typ)
        {
          case TempInt:
            EnterIntSymbol(LabPart, t.Contents.Int, DestSeg, MayChange);
            break;
          case TempFloat:
            EnterFloatSymbol(LabPart, t.Contents.Float, MayChange);
            break;
          case TempString:
            EnterDynStringSymbol(LabPart, &t.Contents.Ascii, MayChange);
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
  if (*AttrPart != '\0') WrError(1100);
  else if (ArgCnt != 1) WrError(1110);
  else
  {
#ifndef HAS64
    HVal = EvalIntExpression(ArgStr[1], UInt32, &ValOK);
#else
    HVal = EvalIntExpression(ArgStr[1], Int64, &ValOK);
#endif
    if (FirstPassUnknown) WrError(1820);
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

  if (*AttrPart != '\0') WrError(1100);
  else if (ArgCnt != 1) WrError(1110);
  else
  {
#ifndef HAS64
    HVal = EvalIntExpression(ArgStr[1], SInt32, &ValOK);
#else
    HVal = EvalIntExpression(ArgStr[1], Int64, &ValOK);
#endif
    if (FirstPassUnknown) WrError(1820);
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
      sprintf(c, "(* %s *)", CommPart);
      break;
    case 2:
      sprintf(c, "/* %s */", CommPart);
      break;
    case 3:
      sprintf(c, "; %s", CommPart);
      break;
  }
}

static void CodeSHARED(Word Index)
{
  int z;
  Boolean ValOK;
  LargeInt HVal;
  Double FVal;
  String s, c;
  UNUSED(Index);

  if (ShareMode == 0) WrError(30);
  else if ((ArgCnt == 0) && (*CommPart != '\0'))
  {
    CodeSHARED_BuildComment(c);
    errno = 0;
    fprintf(ShareFile, "%s\n", c); ChkIO(10004);
  }
  else
   for (z = 1; z <= ArgCnt; z++)
   {
     if (!ExpandSymbol(ArgStr[z]))
       continue;
     if (!ChkSymbName(ArgStr[z]))
     {
       WrXError(1020, ArgStr[z]);
       continue;
     }
     if (IsSymbolString(ArgStr[z]))
     {
       ValOK = GetStringSymbol(ArgStr[z], s);
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
     else if (IsSymbolFloat(ArgStr[z]))
     {
       ValOK = GetFloatSymbol(ArgStr[z], &FVal);
       sprintf(s, "%0.17g", FVal);
     }
     else
     {
       ValOK = GetIntSymbol(ArgStr[z], &HVal, NULL);
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
       if ((z == 1) && (*CommPart != '\0'))
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
           fprintf(ShareFile, "%s = %s;%s\n", ArgStr[z], s, c);
           break;
         case 2:
           fprintf(ShareFile, "#define %s %s%s\n", ArgStr[z], s, c);
           break;
         case 3:
           strmaxprep(s, IsSymbolChangeable(ArgStr[z]) ? "set " : "equ ", 255);
           fprintf(ShareFile, "%s %s%s\n", ArgStr[z], s, c);
           break;
       }
       ChkIO(10004);
     }
     else if (PassNo == 1)
     {
       Repass = True;
       if ((MsgIfRepass) && (PassNo >= PassNoForMessage))
         WrXError(170, ArgStr[z]);
     }
   }
}

static void CodeEXPORT(Word Index)
{
  int z;
  LargeInt Value;
  PRelocEntry Relocs;
  UNUSED(Index);

  for (z = 1; z <= ArgCnt; z++)
  {
    FirstPassUnknown = True;
    if (GetIntSymbol(ArgStr[z], &Value, &Relocs))
    {
      if (Relocs == NULL)
        AddExport(ArgStr[z], Value, 0);
      else if ((Relocs->Next != NULL) || (strcmp(Relocs->Ref, RelName_SegStart)))
        WrXError(1156, ArgStr[z]);
      else
        AddExport(ArgStr[z], Value, RelFlag_Relative);
    }
    else
    {
      Repass = True;
      if ((MsgIfRepass) && (PassNo >= PassNoForMessage))
        WrXError(170,ArgStr[z]);
    }
  }
}

static void CodePAGE(Word Index)
{
  Integer LVal, WVal;
  Boolean ValOK;
  UNUSED(Index);

  if ((ArgCnt != 1) && (ArgCnt != 2)) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else
  {
    LVal = EvalIntExpression(ArgStr[1], UInt8, &ValOK);
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
        WVal = EvalIntExpression(ArgStr[2], UInt8, &ValOK);
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

  if (ArgCnt > 1) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else
  {
    if (ArgCnt == 0)
    {
      HVal8 = 0;
      ValOK = True;
    }
    else
      HVal8 = EvalIntExpression(ArgStr[1], Int8, &ValOK);
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

  if (ArgCnt != 1) WrError(1110);
  else
  {
    EvalStringExpression(ArgStr[1], &OK, tmp);
    if (!OK) WrError(1970);
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

  if (ArgCnt != 1) WrError(1110);
  else if (ActPC == StructSeg) WrError(1553);
  else
  {
    HVal = EvalIntExpression(ArgStr[1], Int32, &OK);
    if (OK)
      Phases[ActPC] = HVal - ProgCounter();
  }
}


static void CodeDEPHASE(Word Index)
{
  UNUSED(Index);

  if (ArgCnt != 0) WrError(1110);
  else if (ActPC == StructSeg) WrError(1553);
  else
    Phases[ActPC] = 0;
}


static void CodeWARNING(Word Index)
{
  String mess;
  Boolean OK;
  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    EvalStringExpression(ArgStr[1], &OK, mess);
    if (!OK) WrError(1970);
    else
      WrErrorString(mess, "", True, False);
  }
}


static void CodeMESSAGE(Word Index)
{
  String mess;
  Boolean OK;
  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    EvalStringExpression(ArgStr[1], &OK, mess);
    if (!OK) WrError(1970);
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

  if (ArgCnt != 1) WrError(1110);
  else
  {
    EvalStringExpression(ArgStr[1], &OK, mess);
    if (!OK) WrError(1970);
    else
      WrErrorString(mess, "", False, False);
  }
}


static void CodeFATAL(Word Index)
{
  String mess;
  Boolean OK;
  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    EvalStringExpression(ArgStr[1], &OK, mess);
    if (!OK) WrError(1970);
    else
      WrErrorString(mess, "", False, True);
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

  if (ArgCnt > 3) WrError(1110);
  else if (ArgCnt == 0)
  {
    for (z = 0; z < 256; z++)
      CharTransTable[z] = z;
  }
  else
  {
    FirstPassUnknown = False;
    EvalExpression(ArgStr[1], &t);
    switch (t.Typ)
    {
      case TempInt:
        if (FirstPassUnknown)
          t.Contents.Int &= 255;
        if (ChkRange(t.Contents.Int, 0, 255))
        {
          if (ArgCnt < 2) WrError(1110);
          else
          {
            Start = t.Contents.Int;
            FirstPassUnknown = False;
            EvalExpression(ArgStr[2], &t);
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
                    TStart = EvalIntExpression(ArgStr[3], UInt8, &OK);
                  else
                    TStart = 0;
                }
                if (OK)
                  for (z = Start; z <= Stop; z++)
                    CharTransTable[z] = TStart + (z - Start);
                break;
              case TempString:
                l = t.Contents.Ascii.Length; /* Uebersetzungsstring ab Start */
                if (Start + l > 256) WrError(1320);
                else
                  for (z = 0; z < l; z++)
                    CharTransTable[Start + z] = t.Contents.Ascii.Contents[z];
                break;
              case TempFloat:
                WrError(1135);
                break;
              default:
                break;
            }
          }
        }
        break;
      case TempString:
        if (ArgCnt != 1) WrError(1110); /* Tabelle von Datei lesen */
        else
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
        WrError(1135);
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

  if ((ArgCnt != 1) && (ArgCnt != 2)) WrError(1110);
  else if (!ChkSymbName(ArgStr[1])) WrXError(1020, ArgStr[1]);
  else
  {
    if (!CaseSensitive)
    {
      UpString(ArgStr[1]);
      if (ArgCnt == 2)
        UpString(ArgStr[2]);
    }

    if (ArgCnt == 1)
      Source = CurrTransTable;
    else
    {
      for (Source = TransTables; Source; Source = Source->Next)
        if (!strcmp(Source->Name, ArgStr[2]))
          break;
    }

    if (!Source) WrXError(1610, ArgStr[2]);
    else
    {
      for (Prev = NULL, Run = TransTables; Run; Prev = Run, Run = Run->Next)
        if ((erg = strcmp(ArgStr[1], Run->Name)) <= 0)
          break;

      if ((!Run) || (erg < 0))
      {
        New = (PTransTable) malloc(sizeof(TTransTable));
        New->Next = Run;
        New->Name = strdup(ArgStr[1]);
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

  if (ArgCnt < 2) WrError(1110);
  else
  {
    OK = True;
    z = 1;
    do
    {
      OK = (OK && ChkMacSymbName(ArgStr[z]));
      if (!OK)
        WrXError(1020, ArgStr[z]);
      z++;
    }
    while ((z < ArgCnt) && (OK));
    if (OK)
    {
      strmaxcpy(FName, ArgStr[ArgCnt], 255);
      for (z = 1; z < ArgCnt; z++)
        CompressLine(ArgStr[z], z, FName, CaseSensitive);
      EnterFunction(LabPart, FName, ArgCnt - 1);
    }
  }
}


static void CodeSAVE(Word Index)
{
  PSaveState Neu;
  UNUSED(Index);

  if (ArgCnt != 0) WrError(1110);
  else
  {
    Neu = (PSaveState) malloc(sizeof(TSaveState));
    Neu->Next = FirstSaveState;
    Neu->SaveCPU = MomCPU;
    Neu->SavePC = ActPC;
    Neu->SaveListOn = ListOn;
    Neu->SaveLstMacroEx = LstMacroEx;
    Neu->SaveTransTable = CurrTransTable;
    FirstSaveState = Neu;
  }
}


static void CodeRESTORE(Word Index)
{
  PSaveState Old;
  UNUSED(Index);

  if (ArgCnt != 0) WrError(1110);
  else if (!FirstSaveState) WrError(1450);
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
    SetFlag(&LstMacroEx, LstMacroExName, Old->SaveLstMacroEx);
    CurrTransTable = Old->SaveTransTable;
    free(Old);
  }
}


static void CodeSEGMENT(Word Index)
{
  Byte SegZ;
  Word Mask;
  Boolean Found;
  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    Found = False;
    NLS_UpString(ArgStr[1]);
    for (SegZ = 1, Mask = 2; SegZ <= PCMax; SegZ++, Mask <<= 1)
      if (((ValidSegs&Mask) != 0) && (!strcmp(ArgStr[1], SegNames[SegZ])))
      {
        Found = True;
        SetNSeg(SegZ);
      }
    if (!Found)
      WrXError(1961, ArgStr[1]);
  }
}


static void CodeLABEL(Word Index)
{
  LongInt Erg;
  Boolean OK;
  UNUSED(Index);

  FirstPassUnknown = False;
  if (ArgCnt != 1) WrError(1110);
  else
  {
    Erg = EvalIntExpression(ArgStr[1], Int32, &OK);
    if ((OK) && (!FirstPassUnknown))
    {
      char s[40];

      PushLocHandle(-1);
      EnterIntSymbol(LabPart, Erg, SegCode, False);
      IntLine(s, sizeof(s), Erg);
      sprintf(ListLine, "=%s", s);
      PopLocHandle();
    }
  }
}


static void CodeREAD(Word Index)
{
  String Exp;
  TempResult Erg;
  Boolean OK;
  LongInt SaveLocHandle;
  UNUSED(Index);

  if ((ArgCnt != 1) && (ArgCnt != 2)) WrError(1110);
  else
  {
    if (ArgCnt == 2) EvalStringExpression(ArgStr[1], &OK, Exp);
    else
    {
      sprintf(Exp, "Read %s ? ", ArgStr[1]);
      OK = True;
    }
    if (OK)
    {
      setbuf(stdout, NULL);
      printf("%s", Exp);
      fgets(Exp, 255, stdin);
      UpString(Exp);
      FirstPassUnknown = False;
      EvalExpression(Exp, &Erg);
      if (OK)
      {
        SetListLineVal(&Erg);
        SaveLocHandle = MomLocHandle;
        MomLocHandle = -1;
        if (FirstPassUnknown) WrError(1820);
        else switch (Erg.Typ)
        {
          case TempInt:
            EnterIntSymbol(ArgStr[ArgCnt], Erg.Contents.Int, SegNone, True);
            break;
          case TempFloat:
            EnterFloatSymbol(ArgStr[ArgCnt], Erg.Contents.Float, True);
            break;
          case TempString:
          {
            String Tmp;

            DynString2CString(Tmp, &Erg.Contents.Ascii, sizeof(Tmp));
            EnterStringSymbol(ArgStr[ArgCnt], Tmp, True);
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

  if (ArgCnt != 1) WrError(1110);
  else
  {
    tmp = ConstLongInt(ArgStr[1], &OK, 10);
    if (!OK) WrError(1135);
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
  Word Dummy;
  Boolean OK;
  LongInt NewPC;
  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    FirstPassUnknown = False;
    Dummy = EvalIntExpression(ArgStr[1], Int16, &OK);
    if (OK)
    {
      if (FirstPassUnknown) WrError(1820);
      else
      {
        NewPC = ProgCounter() + Dummy - 1;
        NewPC -= NewPC%Dummy;
        CodeLen = NewPC - ProgCounter();
        DontPrint = (CodeLen != 0);
        if (DontPrint)
          BookKeeping();
      }
    }
  }
}

static void CodeASSUME(Word Index)
{
  int z1, z2;
  Boolean OK;
  LongInt HVal;
  String RegPart, ValPart;

  UNUSED(Index);

  /* CPU-specific override? */

  if (pASSUMEOverride)
  {
    pASSUMEOverride();
    return;
  }

  if (ArgCnt == 0) WrError(1110);
  else
  {
    z1 = 1;
    OK = True;
    while ((z1 <= ArgCnt) && (OK))
    {
      SplitString(ArgStr[z1], RegPart, ValPart, QuotPos(ArgStr[z1], ':'));
      z2 = 0;
      NLS_UpString(RegPart);
      while ((z2 < ASSUMERecCnt) && (strcmp(pASSUMERecs[z2].Name, RegPart)))
        z2++;
      OK = (z2 < ASSUMERecCnt);
      if (!OK) WrXError(1980, RegPart);
      else
      {
        if (!strcasecmp(ValPart, "NOTHING"))
        {
          if (pASSUMERecs[z2].NothingVal == -1) WrError(1350);
          else
            *(pASSUMERecs[z2].Dest) = pASSUMERecs[z2].NothingVal;
        }
        else
        {
          FirstPassUnknown = False;
          HVal = EvalIntExpression(ValPart, Int32, &OK);
          if (OK)
          {
            if (FirstPassUnknown)
            {
              WrError(1820);
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

static void CodeENUM(Word Index)
{
  int z;
  char *p = NULL;
  Boolean OK;
  LongInt Counter, First = 0, Last = 0;
  String SymPart;
  UNUSED(Index);

  Counter = 0;
  if (ArgCnt == 0) WrError(1110);
  else
  {
    for (z = 1; z <= ArgCnt; z++)
    {
      p = QuotPos(ArgStr[z], '=');
      if (p)
      {
        strmaxcpy(SymPart, p + 1, 255);
        FirstPassUnknown = False;
        Counter = EvalIntExpression(SymPart, Int32, &OK);
        if (!OK)
          return;
        if (FirstPassUnknown)
        {
          WrXError(1820, SymPart);
          return;
        }
        *p = '\0';
      }
      EnterIntSymbol(ArgStr[z], Counter, SegNone, False);
      if (z == 1)
        First = Counter;
      else if (z == ArgCnt)
        Last = Counter;
      Counter++;
    }
  }
  IntLine(SymPart, sizeof(SymPart), First);
  sprintf(ListLine, "=%s", SymPart);
  if (ArgCnt != 1)
  {
    strmaxcat(ListLine, "..", 255);
    IntLine(SymPart, sizeof(SymPart), Last);
    strmaxcat(ListLine, SymPart, 255);
  }
}


static void CodeEND(Word Index)
{
  LongInt HVal;
  Boolean OK;
  UNUSED(Index);

  if (ArgCnt > 1) WrError(1110);
  else
  {
    if (ArgCnt == 1)
    {
      FirstPassUnknown = False;
      HVal = EvalIntExpression(ArgStr[1], Int32, &OK);
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

  if (ArgCnt != 1) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else
  {
    OK = True;
    NLS_UpString(ArgStr[1]);
    if (!strcmp(ArgStr[1], "OFF"))
      Value = 0;
    else if (!strcmp(ArgStr[1], "ON"))
      Value = 1;
    else if (!strcmp(ArgStr[1], "NOSKIPPED"))
      Value = 2;
    else if (!strcmp(ArgStr[1], "PURECODE"))
      Value = 3;
    else
      OK = False;
    if (!OK) WrError(1520);
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

  if ((ArgCnt < 1) || (ArgCnt > 3)) WrError(1110);
  else if (ActPC == StructSeg) WrError(1940);
  else
  {
    if (ArgCnt == 1)
      OK = True;
    else
    {
      FirstPassUnknown = False;
      Ofs = EvalIntExpression(ArgStr[2], Int32, &OK);
      if (FirstPassUnknown)
      {
        WrError(1820);
        OK = False;
      }
      if (OK)
      {
        if (ArgCnt == 2)
          Len = -1;
        else
        {
          Len = EvalIntExpression(ArgStr[3], Int32, &OK);
          if (FirstPassUnknown)
          {
            WrError(1820);
            OK = False;
          }
        }
      }
    }
    if (OK)
    {
      strmaxcpy(Name, ArgStr[1], 255);
      if (*Name == '"')
        strmov(Name, Name + 1);
      if ((*Name) && (Name[strlen(Name) - 1] == '"'))
        Name[strlen(Name) - 1] = '\0';
      strmaxcpy(ArgStr[1], Name, 255);
      strmaxcpy(Name, FExpand(FSearch(Name, IncludeList)), 255);
      if ((*Name) && (Name[strlen(Name) - 1] == '/'))
        strmaxcat(Name, ArgStr[1], 255);
      F = fopen(Name, OPENRDMODE);
      if (F == NULL) ChkIO(10001);
      errno = 0; FSize = FileSize(F); ChkIO(10003);
      if (Len == -1)
      {
        if ((Len = FSize - Ofs) < 0)
        {
          fclose(F); WrError(1600); return;
        }
      }
      if (!ChkPC(PCs[ActPC] + Len - 1)) WrError(1925);
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
        if (Rest != 0) WrError(1600);
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

  if (ArgCnt < 2) WrError(1110);
  else
  {
    if (!CaseSensitive)
      NLS_UpString(ArgStr[1]);
    for (z = 2; z <= ArgCnt; z++)
      PushSymbol(ArgStr[z], ArgStr[1]);
  }
}

static void CodePOPV(Word Index)
{
  int z;
  UNUSED(Index);

  if (ArgCnt < 2) WrError(1110);
  else
  {
    if (!CaseSensitive)
      NLS_UpString(ArgStr[1]);
    for (z = 2; z <= ArgCnt; z++)
      PopSymbol(ArgStr[z], ArgStr[1]);
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
  int z;
  Boolean OK, DoExt;
  char ExtChar;
  String StructName;

  if (ArgCnt > 1)
  {
    WrError(1110);
    return;
  }

  /* unnamed struct/union only allowed if embedded into at least one named struct/union */

  if (!*LabPart)
  {
    if (!pInnermostNamedStruct)
    {
      WrError(2070);
      return;
    }
  }
  else
  {
    if (!ChkSymbName(LabPart))
    {
      WrXError(1020, LabPart);
      return;
    }
    if (!CaseSensitive)
      NLS_UpString(LabPart);
  }

  /* compose name of nested structures */

  if (*LabPart)
    BuildStructName(StructName, sizeof(StructName), LabPart);
  else
    *StructName = '\0';

  /* If named and embedded into another struct, add as element to innermost named parent struct.
     Add up all offsets of unnamed structs in between. */

  if (StructStack && (*LabPart))
  {
    PStructStack pRun;
    LargeWord Offset = ProgCounter();

    for (pRun = StructStack; pRun && pRun != pInnermostNamedStruct; pRun = pRun->Next)
      Offset += pRun->SaveCurrPC;
    AddStructElem(pInnermostNamedStruct->StructRec, LabPart, True, Offset);
    AddStructSymbol(LabPart, ProgCounter());
  }

  NStruct = (PStructStack) malloc(sizeof(TStructStack));
  NStruct->Name = strdup(StructName);
  NStruct->pBaseName = NStruct->Name + strlen(NStruct->Name) - strlen(LabPart); /* NULL -> complain too long */
  NStruct->SaveCurrPC = ProgCounter();
  DoExt = True;
  ExtChar = DottedStructs ? '.' : '_';
  NStruct->Next = StructStack;
  OK = True;
  for (z = 1; z <= ArgCnt; z++)
    if (OK)
    {
      if (!strcasecmp(ArgStr[z], "EXTNAMES"))
        DoExt = True;
      else if (!strcasecmp(ArgStr[z], "NOEXTNAMES"))
        DoExt = False;
      else if (!strcasecmp(ArgStr[z], "DOTS"))
        ExtChar = '.';
      else if (!strcasecmp(ArgStr[z], "NODOTS"))
        ExtChar = '_';
      else
      {
        WrXError(1554, ArgStr[z]);
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

  if (ArgCnt > 1) WrError(1110);
  else if (!StructStack) WrError(1550);
  else
  {
    if (IsUnion && !StructStack->StructRec->IsUnion)
      WrXError(2080, StructStack->Name);

    if (*LabPart == '\0') OK = True;
    else
    {
      if (!CaseSensitive)
        NLS_UpString(LabPart);
      OK = !strcmp(LabPart, StructStack->pBaseName);
      if (!OK) WrError(1552);
    }
    if (OK)
    {
      LargeWord TotLen;

      /* unchain current struct from stack */

      OStruct = StructStack;
      StructStack = OStruct->Next;

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
        EnterIntSymbol(ArgStr[1], TotLen, SegNone, False);

      t.Typ = TempInt;
      t.Contents.Int = TotLen;
      SetListLineVal(&t);

      /* If named, store completed structure.
         Otherwise, discard temporary struct. */

      if (OStruct->Name[0])
        AddStruct(OStruct->StructRec, OStruct->Name, True);

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

  if (ArgCnt < 1) WrError(1110);
  else
  {
    i = 1;
    OK = True;
    while ((OK) && (i <= ArgCnt))
    {
      Split = strrchr(ArgStr[i], ':');
      if (Split == NULL)
        Type = SegNone;
      else
      {
        *Split = '\0';
        for (Type = SegNone + 1; Type <= PCMax; Type++)
          if (!strcasecmp(Split + 1, SegNames[Type]))
            break;
      }
      if (Type > PCMax) WrXError(1961, Split + 1);
      else
      {
        EnterExtSymbol(ArgStr[i], 0, Type, FALSE);
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

  if (ArgCnt != 1) WrError(1110);
  else
  {
    FirstPassUnknown = False;
    Temp = EvalIntExpression(ArgStr[1], UInt32, &OK);
    if (OK)
    {
      if (FirstPassUnknown) WrError(1820);
      else NestMax = Temp;
    }
  }
}

static void CodeSEGTYPE(Word Index)
{
  UNUSED(Index);

  if (ArgCnt != 0) WrError(1110);
  else
    RelSegs = (mytoupper(*OpPart) == 'R');
}

static void CodePPSyms(PForwardSymbol *Orig,
                       PForwardSymbol *Alt1,
                       PForwardSymbol *Alt2)
{
  PForwardSymbol Lauf;
  int z;
  String Sym, Section;

  if (ArgCnt == 0) WrError(1110);
  else
   for (z = 1; z <= ArgCnt; z++)
   {
     SplitString(ArgStr[z], Sym, Section, QuotPos(ArgStr[z], ':'));
     if (!ExpandSymbol(Sym)) return;
     if (!ExpandSymbol(Section)) return;
     if (!CaseSensitive)
       NLS_UpString(Sym);
     Lauf = CodePPSyms_SearchSym(*Alt1, Sym);
     if (Lauf) WrXError(1489, ArgStr[z]);
     else
     {
       Lauf = CodePPSyms_SearchSym(*Alt2, Sym);
       if (Lauf) WrXError(1489, ArgStr[z]);
       else
       {
         Lauf = CodePPSyms_SearchSym(*Orig, Sym);
         if (!Lauf)
         {
           Lauf = (PForwardSymbol) malloc(sizeof(TForwardSymbol));
           Lauf->Next = (*Orig); *Orig = Lauf;
           Lauf->Name = strdup(Sym);
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
  char *FlagName, *InstName;
} ONOFFTab;
static ONOFFTab *ONOFFList;

static void DecodeONOFF(Word Index)
{
  ONOFFTab *Tab = ONOFFList + Index;
  Boolean OK;

  if (ArgCnt != 1) WrError(1110);
  else
  {
    NLS_UpString(ArgStr[1]);
    if (*AttrPart != '\0') WrError(1100);
    {
      Boolean IsON = !strcasecmp(ArgStr[1], "ON");
      if ((!IsON) && (strcasecmp(ArgStr[1], "OFF"))) WrError(1520);
      else
      {
        OK = !strcasecmp(ArgStr[1], "ON");
        SetFlag(Tab->FlagAddr, Tab->FlagName, OK);
      }
    }
  }
}

void AddONOFF(char *InstName, Boolean *Flag, char *FlagName, Boolean Persist)
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
  {"ORG",        CodeORG        , 0 },
  {"OUTRADIX",   CodeRADIX      , 1 },
  {"PAGE",       CodePAGE       , 0 },
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
  switch (*OpPart)
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
  }

  if (LookupInstTable(ONOFFTable, OpPart))
    return True;

  if (LookupInstTable(PseudoTable, OpPart))
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
  AddONOFF("MACEXP", &LstMacroEx, LstMacroExName, True);
  AddONOFF("RELAXED", &RelaxedMode, RelaxedName, True);
  AddONOFF("DOTTEDSTRUCTS", &DottedStructs, DottedStructsName, True);
}
