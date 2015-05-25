/* as.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Hauptmodul                                                                */
/*                                                                           */
/* Historie:  4. 5.1996 Grundsteinlegung                                     */
/*           24. 6.1998 Zeichenuebersetzungstabellen                         */
/*           30. 6.1998 Ausgabe in MacPro-File auch wenn Zeile nur aus       */
/*                      Kommentar oder Label besteht                         */
/*           18. 7.1998 IRPC-Statement                                       */
/*           24. 7.1998 Debug-Modus NoICE                                    */
/*           25. 7.1998 Formate glattgezogen                                 */
/*           16. 8.1998 Datei-Adressbereiche zuruecksetzen                   */
/*           17. 8.1998 InMacroFlag nach asmdef verschoben                   */
/*           19. 8.1998 BranchExt-Initialisierung                            */
/*           25. 8.1998 i960-Initialisierung                                 */
/*           28. 8.1998 32-Bit-Listen gehen auch korrekt mit                 */
/*                      Codelaengen != 4*n um                                */
/*           30. 8.1998 uPD7720-Initialisierung                              */
/*                      Einrueckung fuer 'R' in Retracted-Zeilen im Listing  */
/*                      war nicht korrekt                                    */
/*           13. 9.1998 uPD77230-Initialisierung                             */
/*           30. 9.1998 SYM53C8xx-Initialisierung                            */
/*            3.12.1998 8008-Initialisierung                                 */
/*            9. 1.1999 PCs erst nach Schreiben des Codes hochzaehlen        */
/*                      ChkPC mit Adresse als Parameter                      */
/*           30. 1.1999 Formate maschinenunabhaengig gemacht                 */
/*           12. 2.1999 Compilerwarnungen beseitigt                          */
/*           25. 3.1999 SC14xxx-Initialisierung                              */
/*           17. 4.1999 CPU per Kommandozeile setzen                         */
/*           18. 4.1999 Ausgabeliste Sharefiles                              */
/*            4. 7.1999 F2MC8-Initialisierung                                */
/*            8. 8.1999 Externliste immer am  Ende einer Zeile loeschen      */
/*           14. 8.1999 Initialisierung ACE                                  */
/*            5.11.1999 ExtendErrors, 2. Stufe                               */
/*           19.11.1999 F2MC16-Initialisierung                               */
/*            4.12.1999 IRP/REPT-Zeilenangabe in Klammern                    */
/*            9. 1.2000 Zeilenzaehler mit plattformabhaengigem Formatstring  */
/*           13. 2.2000 Ausgabename fuer Listing setzen                      */
/*            8. 3.2000 'ambigious else'-Warnungen beseitigt                 */
/*           20. 5.2000 added ArgCName, expansion of macro argument count    */
/*           21. 5.2000 added TmpSymCounter initialization                   */
/*            1. 6.2000 REPT/WHILE/IRP(C) not expanded without IfAsm         */
/*                      added maximum nesting level for macros               */
/*           24.12.2000 added -noicemask option                              */
/*           14. 1.2001 silenced warnings about unused parameters            */
/*                      set 'segment used' flag once code is generated       */
/*           27. 1.2001 added 1802 initialization                            */
/*           24. 3.2001 correctly handle predefined symbols when operating   */
/*                      in case-sensitive mode                               */
/*            9. 6.2001 moved initialization of DoPadding before CPU-specific*/
/*                      initialization, to allow CPU-specific override       */
/*           2001-07-07 added intiialization of C54x generator               */
/*           2001-10-20 GNU error style possible                             */
/*           2001-12-31 added IntLabel directive                             */
/*           2002-01-26 changed end behaviour of while statement             */
/*           2002-03-03 use FromFile, LineRun fields in input tag            */
/*                                                                           */
/*****************************************************************************/
/* $Id: as.c,v 1.48 2015/04/20 18:40:29 alfred Exp $                          */
/*****************************************************************************
 * $Log: as.c,v $
 * Revision 1.48  2015/04/20 18:40:29  alfred
 * - add TMS1000 support (no docs yet)
 *
 * Revision 1.47  2015/01/04 20:33:42  alfred
 * - avoid double build in parallel make
 * - begun with disassembler
 *
 * Revision 1.46  2014/12/14 17:58:46  alfred
 * - remove static variables in strutil.c
 *
 * Revision 1.45  2014/12/02 13:33:19  alfred
 * - do not use strncpy()
 *
 * Revision 1.44  2014/11/23 18:52:36  alfred
 * - use common allocator for OUTProcessor
 *
 * Revision 1.43  2014/11/23 18:27:09  alfred
 * - remove trailing blanks
 *
 * Revision 1.42  2014/11/17 23:51:31  alfred
 * - begun with TLCS-870/C
 *
 * Revision 1.41  2014/11/17 21:20:24  alfred
 * - rework to current style
 *
 * Revision 1.40  2014/11/16 18:52:07  alfred
 * - first step of rework
 *
 * Revision 1.39  2014/11/06 11:22:01  alfred
 * - replace hook chain for ClearUp, document new mechanism
 *
 * Revision 1.38  2014/11/05 15:47:13  alfred
 * - replace InitPass callchain with registry
 *
 * Revision 1.37  2014/11/05 09:19:39  alfred
 * - remove static string
 *
 * Revision 1.36  2014/10/26 20:52:32  alfred
 * - cleanup tag only right before destruction because contents may be needed for GetPos()
 *
 * Revision 1.35  2014/10/26 20:43:11  alfred
 * - correct usage of parameter counter in IRP get position function
 *
 * Revision 1.34  2014/10/06 17:54:56  alfred
 * - display filename if include failed
 * - some valgrind workaraounds
 *
 * Revision 1.33  2014/09/21 13:15:16  alfred
 * - assure structure is initialized
 *
 * Revision 1.32  2014/09/14 13:22:32  alfred
 * - ass keyword arguments
 *
 * Revision 1.31  2014/06/15 09:17:08  alfred
 * - optional Memo profiling
 *
 * Revision 1.30  2014/05/29 10:59:05  alfred
 * - some const cleanups
 *
 * Revision 1.29  2013/12/21 19:46:50  alfred
 * - dynamically resize code buffer
 *
 * Revision 1.28  2013/12/17 18:54:17  alfred
 * - correct local symbol handle processing in IRPC
 *
 * Revision 1.27  2013-03-09 16:15:07  alfred
 * - add NEC 75xx
 *
 * Revision 1.26  2013-03-09 07:54:38  alfred
 * - add GLOBALSYMBOLS option
 *
 * Revision 1.25  2012-07-22 11:51:45  alfred
 * - begun with XCore target
 *
 * Revision 1.24  2012-05-26 13:49:19  alfred
 * - MSP additions, make implicit macro parameters always case-insensitive
 *
 * Revision 1.23  2012-01-14 14:34:58  alfred
 * - add some platforms
 *
 * Revision 1.22  2010/04/17 13:14:18  alfred
 * - address overlapping strcpy()
 *
 * Revision 1.21  2010/02/27 14:17:26  alfred
 * - correct increment/decrement of macro nesting level
 *
 * Revision 1.20  2009/06/07 09:32:25  alfred
 * - add named temporary symbols
 *
 * Revision 1.19  2009/05/10 10:48:45  alfred
 * - display macro nesting in listing
 *
 * Revision 1.18  2008/04/13 20:23:46  alfred
 * - add Atari Vecor Processor target
 *
 * Revision 1.17  2007/11/24 22:48:02  alfred
 * - some NetBSD changes
 *
 * Revision 1.16  2007/04/30 10:19:19  alfred
 * - make default nesting level consistent
 *
 * Revision 1.15  2006/12/19 17:26:00  alfred
 * - allow full list mask range
 *
 * Revision 1.14  2006/10/10 10:41:12  alfred
 * - allocate FileMask dynamically
 *
 * Revision 1.13  2006/07/08 10:32:55  alfred
 * - added RS08
 *
 * Revision 1.12  2006/06/15 21:15:24  alfred
 * - cleanups in listing output
 *
 * Revision 1.11  2006/04/06 20:26:53  alfred
 * - add COP4
 *
 * Revision 1.10  2005/12/09 14:48:06  alfred
 * - added 2650
 *
 * Revision 1.9  2005/10/02 10:00:43  alfred
 * - ConstLongInt gets default base, correct length check on KCPSM3 registers
 *
 * Revision 1.8  2005/09/11 18:10:50  alfred
 * - added XGATE
 *
 * Revision 1.7  2005/07/30 13:57:02  alfred
 * - add LatticeMico8
 *
 * Revision 1.6  2005/03/21 19:48:16  alfred
 * - shortened name to 8+3 (again...)
 *
 * Revision 1.5  2005/02/19 18:05:59  alfred
 * - use shorter name for 8+3 filesystems, correct bugs
 *
 * Revision 1.4  2005/02/19 14:10:14  alfred
 * - added KCPSM3
 *
 * Revision 1.3  2004/10/03 12:52:31  alfred
 * - MinGW adaptions
 *
 * Revision 1.2  2004/05/29 12:28:13  alfred
 * - remove unneccessary dummy fcn
 *
 * Revision 1.1  2003/11/06 02:49:18  alfred
 * - recreated
 *
 * Revision 1.24  2003/10/12 19:28:52  alfred
 * - created 78K/2
 *
 * Revision 1.23  2003/10/04 15:38:46  alfred
 * - differentiate constant/variable messages
 *
 * Revision 1.22  2003/05/02 21:23:08  alfred
 * - strlen() updates
 *
 * Revision 1.21  2003/03/29 18:45:50  alfred
 * - allow source file spec in key files
 *
 * Revision 1.20  2003/03/26 20:31:51  alfred
 * - some Win32 path fixes
 *
 * Revision 1.19  2003/03/16 18:53:42  alfred
 * - created 807x
 *
 * Revision 1.18  2003/03/09 10:28:27  alfred
 * - added KCPSM
 *
 * Revision 1.17  2003/02/02 13:00:05  alfred
 * - use ReadLnCont()
 *
 * Revision 1.16  2003/01/29 21:25:41  alfred
 * - do not convert IRP args when in case-sensitive mode
 *
 * Revision 1.15  2002/11/20 20:25:04  alfred
 * - added unions
 *
 * Revision 1.14  2002/11/16 20:53:11  alfred
 * - additions for structures
 *
 * Revision 1.13  2002/11/15 23:30:53  alfred
 * - relocated EnterLebel
 *
 * Revision 1.12  2002/11/11 21:56:57  alfred
 * - store/display struct elements
 *
 * Revision 1.11  2002/11/11 21:13:54  alfred
 * - basic structure handling
 *
 * Revision 1.10  2002/11/11 19:24:57  alfred
 * - new module for structs
 *
 * Revision 1.9  2002/11/10 16:27:32  alfred
 * - use free fcns for macros
 *
 * Revision 1.8  2002/11/04 19:19:37  alfred
 * - use struct separation character
 *
 * Revision 1.7  2002/10/07 20:25:01  alfred
 * - added '/' nameless temporary symbols
 *
 * Revision 1.6  2002/09/30 17:12:20  alfred
 * - added nameless symbol counter intialization
 *
 * Revision 1.5  2002/05/19 13:45:32  alfred
 * - clear section usage before starting new pass
 *
 * Revision 1.4  2002/05/01 15:35:46  alfred
 * - removed umlaut
 *
 * Revision 1.3  2002/03/10 10:47:20  alfred
 * - add CVS log
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>
#include <setjmp.h>

#include "version.h"
#include "endian.h"
#include "bpemu.h"

#include "stdhandl.h"
#include "nls.h"
#include "nlmessages.h"
#include "as.rsc"
#include "ioerrs.h"
#include "strutil.h"
#include "stringlists.h"
#include "cmdarg.h"
#include "asmitree.h"
#include "chunks.h"
#include "asminclist.h"
#include "asmfnums.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmmac.h"
#include "asmstructs.h"
#include "asmif.h"
#include "asmcode.h"
#include "asmdebug.h"
#include "asmrelocs.h"
#include "asmallg.h"
#include "codepseudo.h"
#include "as.h"

#include "code68k.h"
#include "code56k.h"
#include "code601.h"
#include "codemcore.h"
#include "codexgate.h"
#include "code68.h"
#include "code6805.h"
#include "code6809.h"
#include "code6812.h"
#include "code6816.h"
#include "code68rs08.h"
#include "codeh8_3.h"
#include "codeh8_5.h"
#include "code7000.h"
#include "code65.h"
#include "code7700.h"
#include "code4500.h"
#include "codem16.h"
#include "codem16c.h"
#include "code4004.h"
#include "code8008.h"
#include "code48.h"
#include "code51.h"
#include "code96.h"
#include "code85.h"
#include "code86.h"
#include "code960.h"
#include "code8x30x.h"
#include "code2650.h"
#include "codexa.h"
#include "codeavr.h"
#include "code29k.h"
#include "code166.h"
#include "codez80.h"
#include "codez8.h"
#include "codekcpsm.h"
#include "codekcp3.h"
#include "codemic8.h"
#include "code96c141.h"
#include "code90c141.h"
#include "code87c800.h"
#include "code870c.h"
#include "code47c00.h"
#include "code97c241.h"
#include "code16c5x.h"
#include "code16c8x.h"
#include "code17c4x.h"
#include "codest6.h"
#include "codest7.h"
#include "codest9.h"
#include "code6804.h"
#include "code3201x.h"
#include "code3202x.h"
#include "code3203x.h"
#include "code3205x.h"
#include "code3254x.h"
#include "code3206x.h"
#include "code9900.h"
#include "codetms7.h"
#include "code370.h"
#include "codemsp.h"
#include "codetms1.h"
#include "codescmp.h"
#include "code807x.h"
#include "codecop4.h"
#include "codecop8.h"
#include "codesc14xxx.h"
#include "codeace.h"
#include "code78c10.h"
#include "code75xx.h"
#include "code75k0.h"
#include "code78k0.h"
#include "code78k2.h"
#include "code7720.h"
#include "code77230.h"
#include "code53c8xx.h"
#include "codefmc8.h"
#include "codefmc16.h"
#include "code1802.h"
#include "codevector.h"
#include "codexcore.h"
#include "as1750.h"
/**          Code21xx};**/

static char *FileMask;
static long StartTime, StopTime;
static Boolean GlobErrFlag;
static Boolean MasterFile;
static unsigned MacroNestLevel = 0;

/*=== Zeilen einlesen ======================================================*/


#if 0
# define dbgentry(str) printf("***enter %s\n", str);
# define dbgexit(str) printf("***exit %s\n", str);
#else
# define dbgentry(str) {}
# define dbgexit(str) {}
#endif

static void NULL_Restorer(PInputTag PInp)
{
  UNUSED(PInp);
}

static Boolean NULL_GetPos(PInputTag PInp, char *dest)
{
  UNUSED(PInp);

  *dest = '\0';
  return False;
}

static Boolean INCLUDE_Processor(PInputTag PInp, char *Erg);

static PInputTag GenerateProcessor(void)
{
  PInputTag PInp = malloc(sizeof(TInputTag));

  PInp->IsMacro = False;
  PInp->Next = NULL;
  PInp->First = True;
  PInp->OrigDoLst = DoLst;
  PInp->StartLine = CurrLine;
  PInp->ParCnt = 0; PInp->ParZ = 0;
  InitStringList(&(PInp->Params));
  PInp->LineCnt = 0; PInp->LineZ = 1;
  PInp->Lines = PInp->LineRun = NULL;
  PInp->SpecName[0] = '\0';
  PInp->AllArgs[0] = '\0';
  PInp->NumArgs[0] = '\0';
  PInp->IsEmpty = False;
  PInp->Buffer = NULL;
  PInp->Datei = NULL;
  PInp->IfLevel = SaveIFs();
  PInp->Restorer = NULL_Restorer;
  PInp->GetPos = NULL_GetPos;
  PInp->Macro = NULL;
  PInp->SaveAttr[0] = '\0';
  PInp->SaveLabel[0] = '\0';
  PInp->GlobalSymbols = False;

  /* in case the input tag chain is empty, this must be the master file */

  PInp->FromFile = (!FirstInputTag) || (FirstInputTag->Processor == INCLUDE_Processor);

  return PInp;
}

static POutputTag GenerateOUTProcessor(SimpProc Processor)
{
  POutputTag POut;

  POut = (POutputTag) malloc(sizeof(TOutputTag));
  POut->Processor = Processor;
  POut->NestLevel = 0;
  POut->Tag = NULL;
  POut->Mac = NULL;
  POut->ParamNames = NULL;
  POut->ParamDefVals = NULL;
  POut->PubSect = 0;
  POut->GlobSect = 0;
  POut->DoExport = False;
  POut->DoGlobCopy= False;
  *POut->GName = '\0';

  return POut;
}

/*=========================================================================*/
/* Listing erzeugen */

static void MakeList_Gen2Line(char *h, Word EffLen, Word *n)
{
  int z, Rest;
  char Str[20];

  Rest = EffLen - (*n);
  if (Rest > 8)
    Rest = 8;
  if (DontPrint)
    Rest = 0;
  for (z = 0; z < (Rest >> 1); z++)
  {
    HexString(Str, sizeof(Str), WAsmCode[(*n) >> 1], 4);
    strmaxcat(h, Str, 255);
    strmaxcat(h, " ", 255);
    (*n) += 2;
  }
  if (Rest & 1)
  {
    HexString(Str, sizeof(Str), BAsmCode[*n], 2);
    strmaxcat(h, Str, 255);
    strmaxcat(h, "   ", 255);
    (*n)++;
  }
  for (z = 1; z <= (8 - Rest) >> 1; z++)
    strmaxcat(h, "     ", 255);
}

static void MakeList_Gen4Line(char *h, Word EffLen, Word *n)
{
  int z, Rest, wr = 0;
  char Str[20];

  Rest = EffLen - (*n);
  if (Rest > 8)
    Rest = 8;
  if (DontPrint)
    Rest = 0;
  for (z = 0; z < (Rest >> 2); z++)
  {
    HexString(Str, sizeof(Str), DAsmCode[(*n) >> 2], 8);
    strmaxcat(h, Str, 255);
    strmaxcat(h, " ", 255);
    *n += 4;
    wr += 9;
  }
  for (z = 0; z < (Rest&3); z++)
  {
    HexString(Str, sizeof(Str), BAsmCode[(*n)++], 2);
    strmaxcat(h, Str, 255);
    strmaxcat(h, " ", 255);
    wr += 3;
  }
  strmaxcat(h, Blanks(20 - wr), 255);
}

static void MakeList(void)
{
  String h, h2, h3, Tmp;
  Word i, k;
  Word n, EffLen;

  EffLen = CodeLen * Granularity();

  if ((!ListToNull) && (DoLst) && ((ListMask&1) != 0) && (!IFListMask()))
  {
    /* Zeilennummer / Programmzaehleradresse: */

    if (IncDepth == 0)
      strmaxcpy(h2, "   ", 255);
    else
    {
      sprintf(Tmp, IntegerFormat, IncDepth);
      sprintf(h2, "(%s)", Tmp);
    }
    if (ListMask & ListMask_LineNums)
    {
      sprintf(h3, Integ32Format, CurrLine);
      sprintf(h, "%5s/", h3);
      strmaxcat(h2, h, 255);
    }
    strmaxcpy(h, h2, 255);
    HexBlankString(h2, sizeof(h2), EProgCounter() - CodeLen, 8);
    strmaxcat(h, h2, 255);
    strmaxcat(h, Retracted?" R ":" : ", 255);

    /* Extrawurst in Listing ? */

    if (*ListLine != '\0')
    {
      strmaxcat(h, ListLine, 255);
      strmaxcat(h, Blanks(20 - strlen(ListLine)), 255);
      strmaxcat(h, OneLine, 255);
      WrLstLine(h);
      *ListLine = '\0';
    }

    /* Code ausgeben */

    else
    {
      switch (ActListGran)
      {
        case 4:
          n = 0;
          MakeList_Gen4Line(h, EffLen, &n);
          strmaxcat(h, OneLine, 255); WrLstLine(h);
          if (!DontPrint)
          {
            while (n < EffLen)
            {
              strmaxcpy(h, "                    ", 255);
              MakeList_Gen4Line(h, EffLen, &n);
              WrLstLine(h);
            }
          }
          break;
        case 2:
          n = 0;
          MakeList_Gen2Line(h, EffLen, &n);
          strmaxcat(h, OneLine, 255); WrLstLine(h);
          if (!DontPrint)
          {
            while (n < EffLen)
            {
              strmaxcpy(h, "                    ", 255);
              MakeList_Gen2Line(h, EffLen, &n);
              WrLstLine(h);
            }
          }
          break;
        default:
        {
          char Str[20];

          if ((TurnWords) && (Granularity() != ActListGran))
            DreheCodes();
          for (i = 0; i < 6; i++)
            if ((!DontPrint) && (EffLen > i))
            {
              HexString(Str, sizeof(Str), BAsmCode[i], 2);
              strmaxcat(h, Str, 255);
              strmaxcat(h, " ", 255);
            }
            else
              strmaxcat(h, "   ", 255);
          strmaxcat(h, "  ", 255);
          strmaxcat(h, OneLine, 255);
          WrLstLine(h);
          if ((EffLen > 6) && (!DontPrint))
          {
            EffLen -= 6;
            n = EffLen / 6;
            if ((EffLen % 6) == 0)
              n--;
            for (i = 0; i <= n; i++)
            {
              strmaxcpy(h, "              ", 255);
              if (ListMask & ListMask_LineNums)
                strmaxcat(h, "      ", 255);
              for (k = 0; k < 6; k++)
                if (EffLen > i * 6 + k)
                {
                  HexString(Str, sizeof(Str), BAsmCode[i * 6 + k + 6], 2);
                  strmaxcat(h, Str, 255);
                  strmaxcat(h, " ", 255);
                }
               WrLstLine(h);
            }
          }
          if ((TurnWords) && (Granularity() != ActListGran))
            DreheCodes();
        }
      }
    }
  }
}

/*=========================================================================*/
/* Makroprozessor */

/*-------------------------------------------------------------------------*/
/* allgemein gebrauchte Subfunktionen */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* werden gebraucht, um festzustellen, ob innerhalb eines Makrorumpfes weitere
   Makroschachtelungen auftreten */

static Boolean MacroStart(void)
{
  return ((Memo("MACRO")) || (Memo("IRP")) || (Memo("IRPC")) || (Memo("REPT")) || (Memo("WHILE")));
}

static Boolean MacroEnd(void)
{
  return (Memo("ENDM"));
}

typedef void (*tMacroArgCallback)(Boolean CtrlArg, char *pArg, void *pUser);

static void ProcessMacroArgs(tMacroArgCallback Callback, void *pUser)
{
  int z1, l;

  for (z1 = 1; z1 <= ArgCnt; z1++)
  {
    l = strlen(ArgStr[z1]);
    if ((l >= 2) && (ArgStr[z1][0] == '{') && (ArgStr[z1][l - 1] == '}'))
    {
      ArgStr[z1][l - 1] = '\0';
      Callback(TRUE, ArgStr[z1] + 1, pUser);
    }
    else
    {
      Callback(FALSE, ArgStr[z1], pUser);
    }
  }
}

/*-------------------------------------------------------------------------*/
/* Dieser Einleseprozessor dient nur dazu, eine fehlerhafte Makrodefinition
  bis zum Ende zu ueberlesen */

static void WaitENDM_Processor(void)
{
  POutputTag Tmp;

  if (MacroStart())
    FirstOutputTag->NestLevel++;
  else if (MacroEnd())
    FirstOutputTag->NestLevel--;
  if (FirstOutputTag->NestLevel <= -1)
  {
    Tmp = FirstOutputTag;
    FirstOutputTag = Tmp->Next;
    free(Tmp);
  }
}

static void AddWaitENDM_Processor(void)
{
  POutputTag Neu;

  Neu = GenerateOUTProcessor(WaitENDM_Processor);
  Neu->Next = FirstOutputTag;
  FirstOutputTag = Neu;
}

/*-------------------------------------------------------------------------*/
/* normale Makros */

static void ComputeMacroStrings(PInputTag Tag)
{
  StringRecPtr Lauf;

  /* recompute # of params */

  sprintf(Tag->NumArgs, "%d", Tag->ParCnt);

  /* recompute 'all string' parameter */

  Tag->AllArgs[0] = '\0';
  Lauf = Tag->Params;
  while (Lauf)
  {
    if (Tag->AllArgs[0] != '\0')
      strmaxcat(Tag->AllArgs, ",", 255);
    strmaxcat(Tag->AllArgs, Lauf->Content, 255);
    Lauf = Lauf->Next;
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Diese Routine leitet die Quellcodezeilen bei der Makrodefinition in den
   Makro-Record um */

static void MACRO_OutProcessor(void)
{
  POutputTag Tmp;
  int z;
  StringRecPtr l;
  PMacroRec GMacro;
  String s;

  /* write preprocessed output to file ? */

  if ((MacroOutput) && (FirstOutputTag->DoExport))
  {
    errno = 0;
    fprintf(MacroFile, "%s\n", OneLine);
    ChkIO(10004);
  }

  /* check for additional nested macros resp. end of definition */

  if (MacroStart())
    FirstOutputTag->NestLevel++;
  else if (MacroEnd())
    FirstOutputTag->NestLevel--;

  /* still lines to put into the macro body ? */

  if (FirstOutputTag->NestLevel != -1)
  {
    strmaxcpy(s, OneLine, 255);
    KillCtrl(s);

    /* compress into tokens */

    l = FirstOutputTag->ParamNames;
    for (z = 1; z <= FirstOutputTag->Mac->ParamCount; z++)
      CompressLine(GetStringListNext(&l), z, s, CaseSensitive);

    /* reserved argument names are never case-sensitive */

    if (HasAttrs)
      CompressLine(AttrName, ParMax + 1, s, FALSE);
    CompressLine(ArgCName, ParMax + 2, s, FALSE);
    CompressLine(AllArgName, ParMax + 3, s, FALSE);
    if (FirstOutputTag->Mac->LocIntLabel)
      CompressLine(LabelName, ParMax + 4, s, FALSE);

    AddStringListLast(&(FirstOutputTag->Mac->FirstLine), s);
  }

  /* otherwise, finish definition */

  if (FirstOutputTag->NestLevel == -1)
  {
    if (IfAsm)
    {
      FirstOutputTag->Mac->ParamNames = FirstOutputTag->ParamNames;
      FirstOutputTag->ParamNames = NULL;
      FirstOutputTag->Mac->ParamDefVals = FirstOutputTag->ParamDefVals;
      FirstOutputTag->ParamDefVals = NULL;
      AddMacro(FirstOutputTag->Mac, FirstOutputTag->PubSect, True);
      if ((FirstOutputTag->DoGlobCopy) && (SectionStack))
      {
        GMacro = (PMacroRec) malloc(sizeof(MacroRec));
        GMacro->Name = strdup(FirstOutputTag->GName);
        GMacro->ParamCount = FirstOutputTag->Mac->ParamCount;
        GMacro->FirstLine = DuplicateStringList(FirstOutputTag->Mac->FirstLine);
        GMacro->ParamNames = DuplicateStringList(FirstOutputTag->Mac->ParamNames);
        GMacro->ParamDefVals = DuplicateStringList(FirstOutputTag->Mac->ParamDefVals);
        AddMacro(GMacro, FirstOutputTag->GlobSect, False);
      }
    }
    else
    {
      ClearMacroRec(&(FirstOutputTag->Mac), TRUE);
    }

    Tmp = FirstOutputTag;
    FirstOutputTag = Tmp->Next;
    ClearStringList(&(Tmp->ParamNames));
    ClearStringList(&(Tmp->ParamDefVals));
    free(Tmp);
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Hierher kommen bei einem Makroaufruf die expandierten Zeilen */

Boolean MACRO_Processor(PInputTag PInp, char *erg)
{
  StringRecPtr Lauf;
  int z;
  Boolean Result;

  Result = True;

  /* run to current line */

  Lauf = PInp->Lines;
  for (z = 1; z <= PInp->LineZ - 1; z++)
    Lauf = Lauf->Next;
  strcpy(erg, Lauf->Content);

  /* process parameters */

  Lauf = PInp->Params;
  for (z = 1; z <= PInp->ParCnt; z++)
  {
    ExpandLine(Lauf->Content, z, erg);
    Lauf = Lauf->Next;
  }

  /* process special parameters */

  if (HasAttrs)
    ExpandLine(PInp->SaveAttr, ParMax + 1, erg);
  ExpandLine(PInp->NumArgs, ParMax + 2, erg);
  ExpandLine(PInp->AllArgs, ParMax + 3, erg);
  if (PInp->Macro->LocIntLabel)
    ExpandLine(PInp->SaveLabel, ParMax + 4, erg);

  CurrLine = PInp->StartLine;
  InMacroFlag = True;

  /* before the first line, start a new local symbol space */

  if ((PInp->LineZ == 1) && (!PInp->GlobalSymbols))
    PushLocHandle(GetLocHandle());

  /* signal the end of the macro */

  if (++(PInp->LineZ) > PInp->LineCnt)
    Result = False;

  return Result;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Initialisierung des Makro-Einleseprozesses */

static Boolean ReadMacro_SearchArg(const char *pTest, const char *pComp, Boolean *pErg)
{
  if (!strcasecmp(pTest, pComp))
  {
    *pErg = True;
    return True;
  }
  else if ((strlen(pTest) > 2) && (!strncasecmp(pTest, "NO", 2)) && (!strcasecmp(pTest + 2, pComp)))
  {
    *pErg = False;
    return True;
  }
  else
    return False;
}

static Boolean ReadMacro_SearchSect(char *Test_O, char *Comp, Boolean *Erg, LongInt *Section)
{
  char *p;
  String Test, Sect;

  strmaxcpy(Test, Test_O, 255); KillBlanks(Test);
  p = strchr(Test, ':');
  if (!p)
    *Sect = '\0';
  else
  {
    strmaxcpy(Sect, p + 1, 255);
    *p = '\0';
  }
  if ((strlen(Test) > 2) && (!strncasecmp(Test, "NO", 2)) && (!strcasecmp(Test + 2, Comp)))
  {
    *Erg = False;
    return True;
  }
  else if (!strcasecmp(Test, Comp))
  {
    *Erg = True;
    return (IdentifySection(Sect, Section));
  }
  else
    return False;
}

typedef struct
{
  String PList;
  POutputTag pOutputTag;
  Boolean DoMacExp, DoPublic, DoIntLabel, GlobalSymbols;
  Boolean ErrFlag;
  int ParamCount;
} tReadMacroContext;

static void ExpandPList(String PList, const char *pArg, Boolean CtrlArg)
{
  if (!*PList)
    strmaxcat(PList, ",", 255);
  if (CtrlArg)
    strmaxcat(PList, "{", 255);
  strmaxcat(PList, pArg, 255);
  if (CtrlArg)
    strmaxcat(PList, "}", 255);
}

static void ProcessMACROArgs(Boolean CtrlArg, char *pArg, void *pUser)
{
  tReadMacroContext *pContext = (tReadMacroContext*)pUser;

  if (CtrlArg)
  {
    if (ReadMacro_SearchArg(pArg, "EXPORT", &(pContext->pOutputTag->DoExport)));
    else if (ReadMacro_SearchArg(pArg, "GLOBALSYMBOLS", &pContext->GlobalSymbols));
    else if (ReadMacro_SearchArg(pArg, "EXPAND", &pContext->DoMacExp))
    {
      ExpandPList(pContext->PList, pArg, CtrlArg);
    }
    else if (ReadMacro_SearchArg(pArg, "INTLABEL", &pContext->DoIntLabel))
    {
      ExpandPList(pContext->PList, pArg, CtrlArg);
    }
    else if (ReadMacro_SearchSect(pArg, "GLOBAL", &(pContext->pOutputTag->DoGlobCopy), &(pContext->pOutputTag->GlobSect)));
    else if (ReadMacro_SearchSect(pArg, "PUBLIC", &pContext->DoPublic, &(pContext->pOutputTag->PubSect)));
    else
    {
      WrXError(1465, pArg);
      pContext->ErrFlag = True;
    }
  }
  else
  {
    char *pDefault;

    ExpandPList(pContext->PList, pArg, CtrlArg);
    pDefault = QuotPos(pArg, '=');
    if (pDefault)
    {
      *pDefault++ = '\0';
      KillPostBlanks(pArg);
      KillPrefBlanks(pArg);
    }
    if (!ChkMacSymbName(pArg))
    {
      WrXError(1020, pArg);
      pContext->ErrFlag = True;
    }
    if (!CaseSensitive)
      UpString(pArg);
    AddStringListLast(&(pContext->pOutputTag->ParamNames), pArg);
    AddStringListLast(&(pContext->pOutputTag->ParamDefVals), pDefault ? pDefault : "");
    pContext->ParamCount++;
  }
}

static void ReadMacro(void)
{
  PSaveSection RunSection;
  PMacroRec OneMacro;
  tReadMacroContext Context;
  LongInt HSect;

  CodeLen = 0;
  Context.ErrFlag = False;

  /* Makronamen pruefen */
  /* Definition nur im ersten Pass */

  if (PassNo != 1)
    Context.ErrFlag = True;
  else if (!ExpandSymbol(LabPart))
    Context.ErrFlag = True;
  else if (!ChkSymbName(LabPart))
  {
    WrXError(1020, LabPart);
    Context.ErrFlag = True;
  }

  /* create tag */

  Context.pOutputTag = GenerateOUTProcessor(MACRO_OutProcessor);
  Context.pOutputTag->Next = FirstOutputTag;

  /* check arguments, sort out control directives */

  Context.DoMacExp = LstMacroEx;
  Context.DoPublic = False;
  Context.DoIntLabel = False;
  Context.GlobalSymbols = False;
  *Context.PList = '\0';
  Context.ParamCount = 0;
  ProcessMacroArgs(ProcessMACROArgs, &Context);

  /* Abbruch bei Fehler */

  if (Context.ErrFlag)
  {
    ClearStringList(&(Context.pOutputTag->ParamNames));
    ClearStringList(&(Context.pOutputTag->ParamDefVals));
    free(Context.pOutputTag);
    AddWaitENDM_Processor();
    return;
  }

  /* Bei Globalisierung Namen des Extramakros ermitteln */

  if (Context.pOutputTag->DoGlobCopy)
  {
    strmaxcpy(Context.pOutputTag->GName, LabPart, 255);
    RunSection = SectionStack;
    HSect = MomSectionHandle;
    while ((HSect != Context.pOutputTag->GlobSect) && (RunSection != NULL))
    {
      strmaxprep(Context.pOutputTag->GName, "_", 255);
      strmaxprep(Context.pOutputTag->GName, GetSectionName(HSect), 255);
      HSect = RunSection->Handle;
      RunSection = RunSection->Next;
    }
  }
  if (!Context.DoPublic)
    Context.pOutputTag->PubSect = MomSectionHandle;

  /* chain in */

  OneMacro = (PMacroRec) calloc(1, sizeof(MacroRec));
  OneMacro->FirstLine =
  OneMacro->ParamNames =
  OneMacro->ParamDefVals = NULL;
  Context.pOutputTag->Mac = OneMacro;

  if ((MacroOutput) && (Context.pOutputTag->DoExport))
  {
    errno = 0;
    fprintf(MacroFile, "%s MACRO %s\n",
            Context.pOutputTag->DoGlobCopy ? Context.pOutputTag->GName : LabPart,
            Context.PList);
    ChkIO(10004);
  }

  OneMacro->UseCounter = 0;
  OneMacro->Name = strdup(LabPart);
  OneMacro->ParamCount = Context.ParamCount;
  OneMacro->FirstLine = NULL;
  OneMacro->LocMacExp = Context.DoMacExp;
  OneMacro->LocIntLabel = Context.DoIntLabel;
  OneMacro->GlobalSymbols = Context.GlobalSymbols;

  FirstOutputTag = Context.pOutputTag;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Beendigung der Expansion eines Makros */

static void MACRO_Cleanup(PInputTag PInp)
{
  ClearStringList(&(PInp->Params));
}

static Boolean MACRO_GetPos(PInputTag PInp, char *dest)
{
  String Tmp;

  sprintf(Tmp, LongIntFormat, PInp->LineZ - 1);
  sprintf(dest, "%s(%s) ", PInp->SpecName, Tmp);
  return False;
}

static void MACRO_Restorer(PInputTag PInp)
{
  /* discard the local symbol space */

  if (!PInp->GlobalSymbols)
    PopLocHandle();

  /* undo the recursion counter by one */

  if ((PInp->Macro) && (PInp->Macro->UseCounter > 0))
    PInp->Macro->UseCounter--;

  /* restore list flag */

  DoLst = PInp->OrigDoLst;

  /* decrement macro nesting counter only if this actually was a macro */

  if (PInp->Processor == MACRO_Processor)
    MacroNestLevel--;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Dies initialisiert eine Makroexpansion */

static void ExpandMacro(PMacroRec OneMacro)
{
  int z1, z2;
  StringRecPtr Lauf, pDefault, pParamName, pArg;
  PInputTag Tag = NULL;
  Boolean NamedArgs;
  char *p;

  CodeLen = 0;

  if ((NestMax > 0) && (OneMacro->UseCounter > NestMax)) WrError(1850);
  else
  {
    OneMacro->UseCounter++;

    /* 1. Tag erzeugen */

    Tag = GenerateProcessor();
    Tag->Processor = MACRO_Processor;
    Tag->Restorer  = MACRO_Restorer;
    Tag->Cleanup   = MACRO_Cleanup;
    Tag->GetPos    = MACRO_GetPos;
    Tag->Macro     = OneMacro;
    Tag->GlobalSymbols = OneMacro->GlobalSymbols;
    strmaxcpy(Tag->SpecName, OneMacro->Name, 255);
    strmaxcpy(Tag->SaveAttr, AttrPart, 255);
    if (OneMacro->LocIntLabel)
      strmaxcpy(Tag->SaveLabel, LabPart, 255);
    Tag->IsMacro   = True;

    /* 2. store special parameters - in the original form */

    sprintf(Tag->NumArgs, "%d", ArgCnt);
    Tag->AllArgs[0] = '\0';
    for (z1 = 1; z1 <= ArgCnt; z1++)
    {
      if (z1 == 1) strmaxcat(Tag->AllArgs, ",", 255);
      strmaxcat(Tag->AllArgs, ArgStr[z1], 255);
    }
    Tag->ParCnt = OneMacro->ParamCount;

    /* 3. generate argument list */

    /* 3a. initialize with empty defaults - order is irrelevant at this point: */

    for (z1 = OneMacro->ParamCount; z1 >= 1; z1--)
      AddStringListFirst(&(Tag->Params), NULL);

    /* 3b. walk over given arguments */

    NamedArgs = False;
    for (z1 = 1; z1 <= ArgCnt; z1++)
    {
      if (!CaseSensitive) UpString(ArgStr[z1]);

      /* explicit name given? */

      p = QuotPos(ArgStr[z1], '=');

      /* if parameter name given... */

      if (p)
      {
        /* split it off */

        *p++ = '\0';
        KillPostBlanks(ArgStr[z1]);
        KillPrefBlanks(p);

        /* search parameter by name */

        for (pParamName = OneMacro->ParamNames, pArg = Tag->Params;
             pParamName; pParamName = pParamName->Next, pArg = pArg->Next)
          if (!strcmp(ArgStr[z1], pParamName->Content))
          {
            if (pArg->Content)
            {
              WrXError(320, pParamName->Content);
              free(pArg->Content);
            }
            pArg->Content = strdup(p);
            break;
          }
        if (!pParamName)
          WrXError(1811, ArgStr[z1]);

        /* set flag that no unnamed args are any longer allowed */

        NamedArgs = True;
      }

      /* unnamed argument: */

      else if (NamedArgs)
        WrError(1812);

      /* empty positional parameters mean using defaults: */

      else if ((z1 <= OneMacro->ParamCount) && (strlen(ArgStr[z1]) > 0))
      {
        pArg = Tag->Params;
        pParamName = OneMacro->ParamNames;
        for (z2 = 0; z2 < z1 - 1; z2++)
        {
          pParamName = pParamName->Next;
          pArg = pArg->Next;
        }
        if (pArg->Content)
        {
          WrXError(320, pParamName->Content);
          free(pArg->Content);
        }
        pArg->Content = strdup(ArgStr[z1]);
      }
    }

    /* 3c. fill in defaults */

    for (pParamName = OneMacro->ParamNames, pArg = Tag->Params, pDefault = OneMacro->ParamDefVals;
             pParamName; pParamName = pParamName->Next, pArg = pArg->Next, pDefault = pDefault->Next)
      if (!pArg->Content)
        pArg->Content = strdup(pDefault->Content);

    /* 4. Zeilenliste anhaengen */

    Tag->Lines = OneMacro->FirstLine;
    Tag->IsEmpty = !OneMacro->FirstLine;
    Lauf = OneMacro->FirstLine;
    while (Lauf)
    {
      Tag->LineCnt++;
      Lauf = Lauf->Next;
    }
  }

  /* 5. anhaengen */

  if (Tag)
  {
    if (IfAsm)
    {
      NextDoLst = (DoLst && OneMacro->LocMacExp);
      Tag->Next = FirstInputTag;
      FirstInputTag = Tag;
      MacroNestLevel++;
    }
    else
    {
      ClearStringList(&(Tag->Params)); free(Tag);
    }
  }
}

/*-------------------------------------------------------------------------*/
/* vorzeitiger Abbruch eines Makros */

static void ExpandEXITM(void)
{
  if (ArgCnt != 0) WrError(1110);
  else if (!FirstInputTag) WrError(1805);
  else if (!FirstInputTag->IsMacro) WrError(1805);
  else if (IfAsm)
  {
    FirstInputTag->Cleanup(FirstInputTag);
    RestoreIFs(FirstInputTag->IfLevel);
    FirstInputTag->IsEmpty = True;
  }
}

/*-------------------------------------------------------------------------*/
/* discard first argument */

static void ExpandSHIFT(void)
{
  PInputTag RunTag;

  if (ArgCnt != 0) WrError(1110);
  else if (!FirstInputTag) WrError(1805);
  else if (!FirstInputTag->IsMacro) WrError(1805);
  else if (IfAsm)
  {
    for (RunTag = FirstInputTag; RunTag; RunTag = RunTag->Next)
      if (RunTag->Processor == MACRO_Processor)
        break;

    if ((RunTag) && (RunTag->Params))
    {
      GetAndCutStringList(&(RunTag->Params));
      RunTag->ParCnt--;
      ComputeMacroStrings(RunTag);
    }
  }
}

/*-------------------------------------------------------------------------*/
/*--- IRP (was das bei MASM auch immer heissen mag...)
      Ach ja: Individual Repeat! Danke Bernhard, jetzt hab'
      ich's gerafft! -----------------------*/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Diese Routine liefert bei der Expansion eines IRP-Statements die expan-
  dierten Zeilen */

Boolean IRP_Processor(PInputTag PInp, char *erg)
{
  StringRecPtr Lauf;
  int z;
  Boolean Result;

  Result = True;

  /* increment line counter only if contents came from a true file */

  CurrLine = PInp->StartLine;
  if (PInp->FromFile)
    CurrLine += PInp->LineZ;

  /* first line? Then open new symbol space and reset line pointer */

  if (PInp->LineZ == 1)
  {
    if (!PInp->GlobalSymbols)
    {
      if (!PInp->First) PopLocHandle();
      PushLocHandle(GetLocHandle());
    }
    PInp->First = False;
    PInp->LineRun = PInp->Lines;
  }

  /* extract line */

  strcpy(erg, PInp->LineRun->Content);
  PInp->LineRun = PInp->LineRun->Next;

  /* expand iteration parameter */

  Lauf = PInp->Params; for (z = 1; z <= PInp->ParZ - 1; z++)
    Lauf = Lauf->Next;
  ExpandLine(Lauf->Content, 1, erg);

  /* end of body? then reset to line 1 and exit if this was the last iteration */

  if (++(PInp->LineZ) > PInp->LineCnt)
  {
    PInp->LineZ = 1;
    if (++(PInp->ParZ) > PInp->ParCnt)
      Result = False;
  }

  return Result;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Aufraeumroutine IRP/IRPC */

static void IRP_Cleanup(PInputTag PInp)
{
  StringRecPtr Lauf;

  /* letzten Parameter sichern, wird evtl. noch fuer GetPos gebraucht!
     ... SaveAttr ist aber frei */
  if (PInp->Processor == IRP_Processor)
  {
    for (Lauf = PInp->Params; Lauf->Next; Lauf = Lauf->Next);
    strmaxcpy(PInp->SaveAttr, Lauf->Content, 255);
  }

  ClearStringList(&(PInp->Lines));
  ClearStringList(&(PInp->Params));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Posisionsangabe im IRP(C) fuer Fehlermeldungen */

static Boolean IRP_GetPos(PInputTag PInp, char *dest)
{
  int z, ParZ = PInp->ParZ, LineZ = PInp->LineZ;
  char *IRPType, *IRPVal, tmp[10];

  /* LineZ/ParZ already hopped to next line - step one back: */

  if (--LineZ <= 0)
  {
    LineZ = PInp->LineCnt;
    ParZ--;
  }

  if (PInp->Processor == IRP_Processor)
  {
    IRPType = "IRP";
    if (*PInp->SaveAttr != '\0')
      IRPVal = PInp->SaveAttr;
    else
    {
      StringRecPtr Lauf = PInp->Params;

      for (z = 1; z <= ParZ - 1; z++)
        Lauf = Lauf->Next;
      IRPVal = Lauf->Content;
    }
  }
  else
  {
    IRPType = "IRPC";
    sprintf(tmp, "'%c'", PInp->SpecName[ParZ - 1]);
    IRPVal = tmp;
  }

  sprintf(dest, "%s:%s(%ld) ", IRPType, IRPVal, (long)LineZ);

  return False;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Diese Routine sammelt waehrend der Definition eines IRP(C)-Statements die
  Quellzeilen ein */

static void IRP_OutProcessor(void)
{
  POutputTag Tmp;
  StringRecPtr Dummy;
  String s;

  /* Schachtelungen mitzaehlen */

  if (MacroStart())
    FirstOutputTag->NestLevel++;
  else if (MacroEnd())
    FirstOutputTag->NestLevel--;

  /* falls noch nicht zuende, weiterzaehlen */

  if (FirstOutputTag->NestLevel > -1)
  {
    strmaxcpy(s, OneLine, 255); KillCtrl(s);
    CompressLine(GetStringListFirst(FirstOutputTag->ParamNames, &Dummy), 1, s, CaseSensitive);
    AddStringListLast(&(FirstOutputTag->Tag->Lines), s);
    FirstOutputTag->Tag->LineCnt++;
  }

  /* alles zusammen? Dann umhaengen */

  if (FirstOutputTag->NestLevel == -1)
  {
    Tmp = FirstOutputTag;
    FirstOutputTag = FirstOutputTag->Next;
    Tmp->Tag->IsEmpty = !Tmp->Tag->Lines;
    if (IfAsm)
    {
      NextDoLst = DoLst && LstMacroEx;
      Tmp->Tag->Next = FirstInputTag;
      FirstInputTag = Tmp->Tag;
    }
    else
    {
      ClearStringList(&(Tmp->Tag->Lines));
      ClearStringList(&(Tmp->Tag->Params));
      free(Tmp->Tag);
    }
    ClearStringList(&(Tmp->ParamNames));
    free(Tmp);
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Initialisierung der IRP-Bearbeitung */

typedef struct
{
  Boolean ErrFlag;
  Boolean GlobalSymbols;
  int ArgCnt;
  POutputTag pOutputTag;
  StringList Params;
} tExpandIRPContext;

static void ProcessIRPArgs(Boolean CtrlArg, char *pArg, void *pUser)
{
  tExpandIRPContext *pContext = (tExpandIRPContext*)pUser;

  if (CtrlArg)
  {
    if (ReadMacro_SearchArg(pArg, "GLOBALSYMBOLS", &pContext->GlobalSymbols));
    else
    {
      WrXError(1465, pArg);
      pContext->ErrFlag = True;
    }
  }
  else
  {
    /* differentiate placeholder & arguments */

    if (0 == pContext->ArgCnt)
    {
      if (!ChkMacSymbName(pArg))
      {
        WrXError(1020, pArg);
        pContext->ErrFlag = True;
      }
      else
        AddStringListFirst(&(pContext->pOutputTag->ParamNames), pArg);
    }
    else
    {
      if (!CaseSensitive)
        UpString(pArg);
      AddStringListLast(&(pContext->Params), pArg);
    }
    pContext->ArgCnt++;
  }
}

static void ExpandIRP(void)
{
  PInputTag Tag;
  tExpandIRPContext Context;

  /* 0. terminate if conditional assembly bites */

  if (!IfAsm)
  {
    AddWaitENDM_Processor();
    return;
  }

  /* 1. Parameter pruefen */

  Context.ErrFlag = False;
  Context.GlobalSymbols = False;
  Context.ArgCnt = 0;
  Context.Params = NULL;

  Context.pOutputTag = GenerateOUTProcessor(IRP_OutProcessor);
  Context.pOutputTag->Next      = FirstOutputTag;
  ProcessMacroArgs(ProcessIRPArgs, &Context);

  /* at least parameter & one arg */

  if (Context.ArgCnt < 2)
  {
    WrError(1110);
    Context.ErrFlag = True;
  }
  if (Context.ErrFlag)
  {
    ClearStringList(&(Context.pOutputTag->ParamNames));
    ClearStringList(&(Context.pOutputTag->ParamDefVals));
    ClearStringList(&(Context.Params));
    free(Context.pOutputTag);
    AddWaitENDM_Processor();
    return;
  }

  /* 2. Tag erzeugen */

  Tag = GenerateProcessor();
  Tag->ParCnt    = Context.ArgCnt - 1;
  Tag->Params    = Context.Params;
  Tag->Processor = IRP_Processor;
  Tag->Restorer  = MACRO_Restorer;
  Tag->Cleanup   = IRP_Cleanup;
  Tag->GetPos    = IRP_GetPos;
  Tag->GlobalSymbols = Context.GlobalSymbols;
  Tag->ParZ      = 1;
  Tag->IsMacro   = True;
  *Tag->SaveAttr = '\0';
  Context.pOutputTag->Tag = Tag;

  /* 4. einbetten */

  FirstOutputTag = Context.pOutputTag;
}

/*--- IRPC: dito fuer Zeichen eines Strings ---------------------------------*/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Diese Routine liefert bei der Expansion eines IRPC-Statements die expan-
  dierten Zeilen */

Boolean IRPC_Processor(PInputTag PInp, char *erg)
{
  Boolean Result;
  char tmp[5];

  Result = True;

  /* increment line counter only if contents came from a true file */

  CurrLine = PInp->StartLine;
  if (PInp->FromFile)
    CurrLine += PInp->LineZ;

  /* first line? Then open new symbol space and reset line pointer */

  if (PInp->LineZ == 1)
  {
    if (!PInp->GlobalSymbols)
    {
      if (!PInp->First) PopLocHandle();
      PushLocHandle(GetLocHandle());
    }
    PInp->First = False;
    PInp->LineRun = PInp->Lines;
  }

  /* extract line */

  strcpy(erg, PInp->LineRun->Content);
  PInp->LineRun = PInp->LineRun->Next;

  /* extract iteration parameter */

  *tmp = PInp->SpecName[PInp->ParZ - 1];
  tmp[1] = '\0';
  ExpandLine(tmp, 1, erg);

  /* end of body? then reset to line 1 and exit if this was the last iteration */

  if (++(PInp->LineZ) > PInp->LineCnt)
  {
    PInp->LineZ = 1;
    if (++(PInp->ParZ) > PInp->ParCnt)
      Result = False;
  }

  return Result;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Initialisierung der IRPC-Bearbeitung */

typedef struct
{
  Boolean ErrFlag;
  Boolean GlobalSymbols;
  int ArgCnt;
  POutputTag pOutputTag;
  String Parameter;
} tExpandIRPCContext;

static void ProcessIRPCArgs(Boolean CtrlArg, char *pArg, void *pUser)
{
  tExpandIRPCContext *pContext = (tExpandIRPCContext*)pUser;

  if (CtrlArg)
  {
    if (ReadMacro_SearchArg(pArg, "GLOBALSYMBOLS", &pContext->GlobalSymbols));
    else
    {
      WrXError(1465, pArg);
      pContext->ErrFlag = True;
    }
  }
  else
  {
    if (0 == pContext->ArgCnt)
    {
      if (!ChkMacSymbName(pArg))
      {
        WrXError(1020, pArg);
        pContext->ErrFlag = True;
      }
      else
        AddStringListFirst(&(pContext->pOutputTag->ParamNames), pArg);
    }
    else
    {
      Boolean OK;

      EvalStringExpression(pArg, &OK, pContext->Parameter);
      if (!OK)
        pContext->ErrFlag = True;
    }
    pContext->ArgCnt++;
  }
}

static void ExpandIRPC(void)
{
  PInputTag Tag;
  tExpandIRPCContext Context;

  /* 0. terminate if conditinal assembly bites */

  if (!IfAsm)
  {
    AddWaitENDM_Processor();
    return;
  }

  /* 1.Parameter pruefen */

  Context.ErrFlag = False;
  Context.GlobalSymbols = False;
  Context.ArgCnt = 0;

  Context.pOutputTag = GenerateOUTProcessor(IRP_OutProcessor);
  Context.pOutputTag->Next = FirstOutputTag;
  ProcessMacroArgs(ProcessIRPCArgs, &Context);

  /* parameter & string */

  if (Context.ArgCnt != 2)
  {
    WrError(1110);
    Context.ErrFlag = True;
  }
  if (Context.ErrFlag)
  {
    ClearStringList(&(Context.pOutputTag->ParamNames));
    AddWaitENDM_Processor();
    return;
  }

  /* 2. Tag erzeugen */

  Tag = GenerateProcessor();
  Tag->ParCnt    = strlen(Context.Parameter);
  Tag->Processor = IRPC_Processor;
  Tag->Restorer  = MACRO_Restorer;
  Tag->Cleanup   = IRP_Cleanup;
  Tag->GetPos    = IRP_GetPos;
  Tag->GlobalSymbols = Context.GlobalSymbols;
  Tag->ParZ      = 1;
  Tag->IsMacro   = True;
  *Tag->SaveAttr = '\0';
  strmaxcpy(Tag->SpecName, Context.Parameter, 255);

  /* 4. einbetten */

  Context.pOutputTag->Tag = Tag;
  FirstOutputTag = Context.pOutputTag;
}

/*--- Repetition -----------------------------------------------------------*/

static void REPT_Cleanup(PInputTag PInp)
{
  ClearStringList(&(PInp->Lines));
}

static Boolean REPT_GetPos(PInputTag PInp, char *dest)
{
  int z1 = PInp->ParZ, z2 = PInp->LineZ;

  if (--z2 <= 0)
  {
    z2 = PInp->LineCnt;
    z1--;
  }
  sprintf(dest, "REPT %ld(%ld)", (long)z1, (long)z2);
  return False;
}

Boolean REPT_Processor(PInputTag PInp, char *erg)
{
  Boolean Result;

  Result = True;

  /* increment line counter only if contents came from a true file */

  CurrLine = PInp->StartLine;
  if (PInp->FromFile)
    CurrLine += PInp->LineZ;

  /* first line? Then open new symbol space and reset line pointer */

  if (PInp->LineZ == 1)
  {
    if (!PInp->GlobalSymbols)
    {
      if (!PInp->First) PopLocHandle();
      PushLocHandle(GetLocHandle());
    }
    PInp->First = False;
    PInp->LineRun = PInp->Lines;
  }

  /* extract line */

  strcpy(erg, PInp->LineRun->Content);
  PInp->LineRun = PInp->LineRun->Next;

  /* last line of body? Then increment count and stop if last iteration */

  if ((++PInp->LineZ) > PInp->LineCnt)
  {
    PInp->LineZ = 1;
    if ((++PInp->ParZ) > PInp->ParCnt)
      Result = False;
  }

  return Result;
}

static void REPT_OutProcessor(void)
{
  POutputTag Tmp;

  /* Schachtelungen mitzaehlen */

  if (MacroStart())
    FirstOutputTag->NestLevel++;
  else if (MacroEnd())
    FirstOutputTag->NestLevel--;

  /* falls noch nicht zuende, weiterzaehlen */

  if (FirstOutputTag->NestLevel > -1)
  {
    AddStringListLast(&(FirstOutputTag->Tag->Lines), OneLine);
    FirstOutputTag->Tag->LineCnt++;
  }

  /* alles zusammen? Dann umhaengen */

  if (FirstOutputTag->NestLevel == -1)
  {
    Tmp = FirstOutputTag;
    FirstOutputTag = FirstOutputTag->Next;
    Tmp->Tag->IsEmpty = !Tmp->Tag->Lines;
    if ((IfAsm) && (Tmp->Tag->ParCnt > 0))
    {
      NextDoLst = (DoLst && LstMacroEx);
      Tmp->Tag->Next = FirstInputTag;
      FirstInputTag = Tmp->Tag;
    }
    else
    {
      ClearStringList(&(Tmp->Tag->Lines));
      free(Tmp->Tag);
    }
    free(Tmp);
  }
}

typedef struct
{
  Boolean ErrFlag;
  Boolean GlobalSymbols;
  int ArgCnt;
  LongInt ReptCount;
} tExpandREPTContext;

static void ProcessREPTArgs(Boolean CtrlArg, char *pArg, void *pUser)
{
  tExpandREPTContext *pContext = (tExpandREPTContext*)pUser;

  if (CtrlArg)
  {
    if (ReadMacro_SearchArg(pArg, "GLOBALSYMBOLS", &pContext->GlobalSymbols));
    else
    {
      WrXError(1465, pArg);
      pContext->ErrFlag = True;
    }
  }
  else
  {
    Boolean ValOK;

    FirstPassUnknown = False;
    pContext->ReptCount = EvalIntExpression(pArg, Int32, &ValOK);
    if (FirstPassUnknown)
      WrError(1820);
    if ((!ValOK) || (FirstPassUnknown))
      pContext->ErrFlag = True;
    pContext->ArgCnt++;
  }
}

static void ExpandREPT(void)
{
  PInputTag Tag;
  POutputTag Neu;
  tExpandREPTContext Context;

  /* 0. skip everything when conditional assembly is off */

  if (!IfAsm)
  {
    AddWaitENDM_Processor();
    return;
  }

  /* 1. Repetitionszahl ermitteln */

  Context.GlobalSymbols = False;
  Context.ReptCount = 0;
  Context.ErrFlag = False;
  Context.ArgCnt = 0;
  ProcessMacroArgs(ProcessREPTArgs, &Context);

  /* rept count must be present only once */

  if (Context.ArgCnt != 1)
  {
    WrError(1110);
    Context.ErrFlag = True;
  }
  if (Context.ErrFlag)
  {
    AddWaitENDM_Processor();
    return;
  }

  /* 2. Tag erzeugen */

  Tag = GenerateProcessor();
  Tag->ParCnt    = Context.ReptCount;
  Tag->Processor = REPT_Processor;
  Tag->Restorer  = MACRO_Restorer;
  Tag->Cleanup   = REPT_Cleanup;
  Tag->GetPos    = REPT_GetPos;
  Tag->GlobalSymbols = Context.GlobalSymbols;
  Tag->IsMacro   = True;
  Tag->ParZ      = 1;

  /* 3. einbetten */

  Neu = GenerateOUTProcessor(REPT_OutProcessor);
  Neu->Next      = FirstOutputTag;
  Neu->Tag       = Tag;
  FirstOutputTag = Neu;
}

/*- bedingte Wiederholung -------------------------------------------------------*/

static void WHILE_Cleanup(PInputTag PInp)
{
  ClearStringList(&(PInp->Lines));
}

static Boolean WHILE_GetPos(PInputTag PInp, char *dest)
{
  int z1 = PInp->ParZ, z2 = PInp->LineZ;

  if (--z2 <= 0)
  {
    z2 = PInp->LineCnt;
    z1--;
  }
  sprintf(dest, "WHILE %ld/%ld", (long)z1, (long)z2);
  return False;
}

Boolean WHILE_Processor(PInputTag PInp, char *erg)
{
  int z;
  Boolean OK, Result;

  /* increment line counter only if this came from a true file */

  CurrLine = PInp->StartLine;
  if (PInp->FromFile)
    CurrLine += PInp->LineZ;

  /* if this is the first line of the loop body, open a new handle
     for macro-local symbols and drop the old one if this was not the
     first pass through the body. */

  if (PInp->LineZ == 1)
  {
    if (!PInp->GlobalSymbols)
    {
      if (!PInp->First)
        PopLocHandle();
      PushLocHandle(GetLocHandle());
    }
    PInp->First = False;
    PInp->LineRun = PInp->Lines;
  }

  /* evaluate condition before first line */

  if (PInp->LineZ == 1)
  {
    z = EvalIntExpression(PInp->SpecName, Int32, &OK);
    Result = (OK && (z != 0));
  }
  else
    Result = True;

  if (Result)
  {
    /* get line of body */

    strcpy(erg, PInp->LineRun->Content);
    PInp->LineRun = PInp->LineRun->Next;

    /* in case this is the last line of the body, reset counters */

    if ((++PInp->LineZ) > PInp->LineCnt)
    {
      PInp->LineZ = 1;
      PInp->ParZ++;
    }
  }

  /* nasty last line... */

  else
    *erg = '\0';

  return Result;
}

static void WHILE_OutProcessor(void)
{
  POutputTag Tmp;
  Boolean OK;
  LongInt Erg;

  /* Schachtelungen mitzaehlen */

  if (MacroStart())
    FirstOutputTag->NestLevel++;
  else if (MacroEnd())
    FirstOutputTag->NestLevel--;

  /* falls noch nicht zuende, weiterzaehlen */

  if (FirstOutputTag->NestLevel > -1)
  {
    AddStringListLast(&(FirstOutputTag->Tag->Lines), OneLine);
    FirstOutputTag->Tag->LineCnt++;
  }

  /* alles zusammen? Dann umhaengen */

  if (FirstOutputTag->NestLevel == -1)
  {
    Tmp = FirstOutputTag;
    FirstOutputTag = FirstOutputTag->Next;
    Tmp->Tag->IsEmpty = !Tmp->Tag->Lines;
    FirstPassUnknown = False;
    Erg = EvalIntExpression(Tmp->Tag->SpecName, Int32, &OK);
    if (FirstPassUnknown)
      WrError(1820);
    OK = (OK && (!FirstPassUnknown) && (Erg != 0));
    if ((IfAsm) && (OK))
    {
      NextDoLst = (DoLst && LstMacroEx);
      Tmp->Tag->Next = FirstInputTag;
      FirstInputTag = Tmp->Tag;
    }
    else
    {
      ClearStringList(&(Tmp->Tag->Lines));
      free(Tmp->Tag);
    }
    free(Tmp);
  }
}

typedef struct
{
  Boolean ErrFlag;
  Boolean GlobalSymbols;
  int ArgCnt;
  String SpecName;
} tExpandWHILEContext;

static void ProcessWHILEArgs(Boolean CtrlArg, char *pArg, void *pUser)
{
  tExpandWHILEContext *pContext = (tExpandWHILEContext*)pUser;

  if (CtrlArg)
  {
    if (ReadMacro_SearchArg(pArg, "GLOBALSYMBOLS", &pContext->GlobalSymbols));
    else
    {
      WrXError(1465, pArg);
      pContext->ErrFlag = True;
    }
  }
  else
  {
    strmaxcpy(pContext->SpecName, pArg, 255);
    pContext->ArgCnt++;
  }
}

static void ExpandWHILE(void)
{
  PInputTag Tag;
  POutputTag Neu;
  tExpandWHILEContext Context;

  /* 0. turned off ? */

  if (!IfAsm)
  {
    AddWaitENDM_Processor();
    return;
  }

  /* 1. Bedingung ermitteln */

  Context.GlobalSymbols = False;
  Context.ErrFlag = False;
  Context.ArgCnt = 0;
  ProcessMacroArgs(ProcessWHILEArgs, &Context);

  /* condition must be present only once */

  if (Context.ArgCnt != 1)
  {
    WrError(1110);
    Context.ErrFlag = True;
  }
  if (Context.ErrFlag)
  {
    AddWaitENDM_Processor();
    return;
  }

  /* 2. Tag erzeugen */

  Tag = GenerateProcessor();
  Tag->Processor = WHILE_Processor;
  Tag->Restorer  = MACRO_Restorer;
  Tag->Cleanup   = WHILE_Cleanup;
  Tag->GetPos    = WHILE_GetPos;
  Tag->GlobalSymbols = Context.GlobalSymbols;
  Tag->IsMacro   = True;
  Tag->ParZ      = 1;
  strmaxcpy(Tag->SpecName, Context.SpecName, 255);

  /* 3. einbetten */

  Neu = GenerateOUTProcessor(WHILE_OutProcessor);
  Neu->Next      = FirstOutputTag;
  Neu->Tag       = Tag;
  FirstOutputTag = Neu;
}

/*--------------------------------------------------------------------------*/
/* Einziehen von Include-Files */

static void INCLUDE_Cleanup(PInputTag PInp)
{
  String Tmp;

  fclose(PInp->Datei);
  free(PInp->Buffer);
  LineSum += MomLineCounter;
  if ((*LstName != '\0') && (!QuietMode))
  {
    sprintf(Tmp, LongIntFormat, CurrLine);
    printf("%s(%s)", NamePart(CurrFileName), Tmp);
    printf("%s\n", ClrEol); fflush(stdout);
  }
  if (MakeIncludeList)
    PopInclude();
}

static Boolean INCLUDE_GetPos(PInputTag PInp, char *dest)
{
  String Tmp;
  UNUSED(PInp);

  sprintf(Tmp, LongIntFormat, PInp->LineZ);
  sprintf(dest, GNUErrors ? "%s:%s" : "%s(%s) ", NamePart(PInp->SpecName), Tmp);
  return !GNUErrors;
}

Boolean INCLUDE_Processor(PInputTag PInp, char *Erg)
{
  Boolean Result;
  int Count = 1;

  Result = True;

  if (feof(PInp->Datei))
    *Erg = '\0';
  else
  {
    Count = ReadLnCont(PInp->Datei, Erg, 256);
    /**ChkIO(10003);**/
  }
  PInp->LineZ = CurrLine = (MomLineCounter += Count);
  if (feof(PInp->Datei))
    Result = False;

  return Result;
}

static void INCLUDE_Restorer(PInputTag PInp)
{
  MomLineCounter = PInp->StartLine;
  strmaxcpy(CurrFileName, PInp->SaveAttr, 255);
  IncDepth--;
}

static void ExpandINCLUDE(Boolean SearchPath)
{
  PInputTag Tag;

  if (!IfAsm)
    return;

  if (ArgCnt != 1)
  {
    WrError(1110);
    return;
  }

  strmaxcpy(ArgPart, (*ArgStr[1] == '"') ? (ArgStr[1] + 1) : ArgStr[1], 255);
  if ((*ArgPart) && (ArgPart[strlen(ArgPart) - 1] == '"'))
    ArgPart[strlen(ArgPart) - 1] = '\0';
  AddSuffix(ArgPart, IncSuffix); strmaxcpy(ArgStr[1], ArgPart, 255);
  if (SearchPath)
  {
    strmaxcpy(ArgPart, FExpand(FSearch(ArgPart, IncludeList)), 255);
    if ((*ArgPart) && (ArgPart[strlen(ArgPart) - 1] == '/'))
      strmaxcat(ArgPart, ArgStr[1], 255);
  }

  /* Tag erzeugen */

  Tag = GenerateProcessor();
  Tag->Processor = INCLUDE_Processor;
  Tag->Restorer  = INCLUDE_Restorer;
  Tag->Cleanup   = INCLUDE_Cleanup;
  Tag->GetPos    = INCLUDE_GetPos;
  Tag->Buffer    = (void *) malloc(BufferArraySize);

  /* Sicherung alter Daten */

  Tag->StartLine = MomLineCounter;
  strmaxcpy(Tag->SpecName, ArgPart, 255);
  strmaxcpy(Tag->SaveAttr, CurrFileName, 255);

  /* Datei oeffnen */

#ifdef __CYGWIN32__
  DeCygwinPath(ArgPart);
#endif
  Tag->Datei = fopen(ArgPart, "r");
  if (!Tag->Datei) ChkXIO(10001, ArgPart);
  setvbuf(Tag->Datei, Tag->Buffer, _IOFBF, BufferArraySize);

  /* neu besetzen */

  strmaxcpy(CurrFileName, ArgPart, 255); Tag->LineZ = MomLineCounter = 0;
  NextIncDepth++; AddFile(ArgPart);
  PushInclude(ArgPart);

  /* einhaengen */

  Tag->Next = FirstInputTag; FirstInputTag = Tag;
}

/*=========================================================================*/
/* Einlieferung von Zeilen */

static void GetNextLine(char *Line)
{
  PInputTag HTag;

  InMacroFlag = False;

  while ((FirstInputTag) && (FirstInputTag->IsEmpty))
  {
    FirstInputTag->Cleanup(FirstInputTag);
    FirstInputTag->Restorer(FirstInputTag);
    HTag = FirstInputTag;
    FirstInputTag = HTag->Next;
    free(HTag);
  }

  if (!FirstInputTag)
  {
    *Line = '\0';
    return;
  }

  if (!FirstInputTag->Processor(FirstInputTag, Line))
  {
    FirstInputTag->IsEmpty = True;
  }

  MacLineSum++;
}

char *GetErrorPos(void)
{
  String ActPos;
  PInputTag RunTag;
  char *ErgPos = strdup(""), *tmppos;
  Boolean Last;

  /* for GNU error message style: */

  if (GNUErrors)
  {
    PInputTag InnerTag = NULL;
    Boolean First = TRUE;
    char *Msg;

    /* we only honor the include positions.  First, print the upper include layers... */

    for (RunTag = FirstInputTag; RunTag; RunTag = RunTag->Next)
      if (RunTag->GetPos == INCLUDE_GetPos)
      {
        if (!InnerTag)
          InnerTag = RunTag;
        else
        {
          Last = RunTag->GetPos(RunTag, ActPos);
          if (First)
          {
            Msg = getmessage(Num_GNUErrorMsg1);
            tmppos = (char *) malloc(strlen(Msg) + 1 + strlen(ActPos) + 1);
            sprintf(tmppos, "%s %s", Msg, ActPos);
          }
          else
          {
            Msg = getmessage(Num_GNUErrorMsgN);
            tmppos = (char *) malloc(strlen(ErgPos) + 2 + strlen(Msg) + 1 + strlen(ActPos) + 1);
            sprintf(tmppos, "%s,\n%s %s", ErgPos, Msg, ActPos);
          }
          First = False;
          free(ErgPos);
          ErgPos = tmppos;
        }
      }

    /* ...append something... */

    if (*ErgPos)
    {
      tmppos = (char *) malloc(strlen(ErgPos) + 3);
      sprintf(tmppos, "%s:\n", ErgPos);
      free(ErgPos);
      ErgPos = tmppos;
    }

    /* ...then the innermost one */

    if (InnerTag)
    {
      InnerTag->GetPos(InnerTag, ActPos);
      tmppos = (char *) malloc(strlen(ErgPos) + strlen(ActPos) + 1);
      sprintf(tmppos, "%s%s", ErgPos, ActPos);
      free(ErgPos);
      ErgPos = tmppos;
    }
  }

  /* otherwise the standard AS position generator: */

  else
  {
    int TotLen = 0, ThisLen;

    for (RunTag = FirstInputTag; RunTag; RunTag = RunTag->Next)
    {
      Last = RunTag->GetPos(RunTag, ActPos);
      ThisLen = strlen(ActPos);
      tmppos = (char *) malloc(TotLen + ThisLen + 1);
      strcpy(tmppos, ActPos);
      strcat(tmppos, ErgPos);
      free(ErgPos);
      ErgPos = tmppos;
      TotLen += ThisLen;
      if (Last)
        break;
    }
  }

  return ErgPos;
}

static Boolean InputEnd(void)
{
  PInputTag Lauf;

  Lauf = FirstInputTag;
  while (Lauf)
  {
    if (!Lauf->IsEmpty)
      return False;
    Lauf = Lauf->Next;
  }

  return True;
}

/*=== Eine Quelldatei ( Haupt-oder Includedatei ) bearbeiten ===============*/

/*--- aus der zerlegten Zeile Code erzeugen --------------------------------*/

Boolean HasLabel(void)
{
  if (!*LabPart)
    return False;
  if (IsDef())
    return False;

  switch (*OpPart)
  {
    case '=':
      return (!Memo("="));
    case ':':
      return (!Memo(":="));
    case 'M':
      return (!Memo("MACRO"));
    case 'F':
      return (!Memo("FUNCTION"));
    case 'L':
      return (!Memo("LABEL"));
    case 'S':
      return ((!Memo("SET")) || (SetIsOccupied)) && (!(Memo("STRUCT") || Memo("STRUC")));
    case 'E':
      return ((!Memo("EVAL")) || (!SetIsOccupied))
           && (!Memo("EQU")) && (!(Memo("ENDSTRUCT") || Memo("ENDS")));
    case 'U':
      return (!Memo("UNION"));
    default:
      return True;
  }
}

void HandleLabel(char *Name, LargeWord Value)
{
  PStructStack ZStruct;
  String tmp, tmp2;

  /* structure element ? */

  if (StructStack)
  {
    AddStructElem(StructStack->StructRec, Name, Value);
    strmaxcpy(tmp, Name, 255);
    for (ZStruct = StructStack; ZStruct; ZStruct = ZStruct->Next)
      if (ZStruct->StructRec->DoExt)
      {
        sprintf(tmp2, "%s%c", ZStruct->Name, ZStruct->StructRec->ExtChar);
        strmaxprep(tmp, tmp2, 255);
      }
    EnterIntSymbol(tmp, Value, SegNone, False);
  }

  /* normal label */

  else if (RelSegs)
    EnterRelSymbol(Name, Value, ActPC, False);
  else
    EnterIntSymbol(Name, Value, ActPC, False);
}

static void Produce_Code(void)
{
  Byte z;
  PMacroRec OneMacro;
  PStructRec OneStruct;
  Boolean SearchMacros, Found, IsMacro, IsStruct;

  ActListGran = ListGran();

  /* Makrosuche unterdruecken ? */

  if (*OpPart == '!')
  {
    SearchMacros = False;
    strmov(OpPart, OpPart + 1);
  }
  else
  {
    SearchMacros = True;
    ExpandSymbol(OpPart);
  }
  strcpy(LOpPart, OpPart);
  NLS_UpString(OpPart);

  /* Prozessor eingehaengt ? */

  if (FirstOutputTag)
  {
    FirstOutputTag->Processor();
    return;
  }

  /* otherwise generate code: check for macro/structs here */

  if (!(IsMacro = (SearchMacros) && (FoundMacro(&OneMacro))))
    IsStruct = FoundStruct(&OneStruct);
  else
    IsStruct = FALSE;

  /* evtl. voranstehendes Label ablegen */

  if ((IfAsm) && ((!IsMacro) || (!OneMacro->LocIntLabel)))
  {
    if (HasLabel())
      HandleLabel(LabPart, EProgCounter());
  }

  Found = False;
  switch (*OpPart)
  {
    case 'I':
      /* Makroliste ? */
      Found = True;
      if (Memo("IRP")) ExpandIRP();
      else if (Memo("IRPC")) ExpandIRPC();
      else Found = False;
      break;
    case 'R':
      /* Repetition ? */
      Found = True;
      if (Memo("REPT")) ExpandREPT();
      else Found = False;
      break;
    case 'W':
      /* bedingte Repetition ? */
      Found = True;
      if (Memo("WHILE")) ExpandWHILE();
      else Found = False;
      break;
  }

  /* bedingte Assemblierung ? */

  if (!Found) Found = CodeIFs();

  if (!Found)
    switch (*OpPart)
    {
      case 'M':
        /* Makrodefinition ? */
        Found = True;
        if (Memo("MACRO")) ReadMacro();
        else Found = False;
        break;
      case 'E':
        /* Abbruch Makroexpansion ? */
        Found = True;
        if (Memo("EXITM")) ExpandEXITM();
        else Found = False;
        break;
      case 'S':
        /* shift macro arguments ? */
        Found = True;
        if (Memo("SHIFT")) ExpandSHIFT();
        else Found = False;
        break;
      case 'I':
        /* Includefile? */
        Found = True;
        if (Memo("INCLUDE"))
        {
          ExpandINCLUDE(True);
          MasterFile = False;
        }
        else Found = False;
        break;
    }

  if (Found);

  /* Makroaufruf ? */

  else if (IsMacro)
  {
    if (IfAsm)
    {
      ExpandMacro(OneMacro);
      if ((MacroNestLevel > 1) && (MacroNestLevel < 100))
        sprintf(ListLine, "%*s(MACRO-%u)", MacroNestLevel - 1, "", MacroNestLevel);
      else
        strmaxcpy(ListLine, "(MACRO)", 255);
    }
  }

  /* structure declaration ? */

  else if (IsStruct)
  {
    if (IfAsm)
    {
      ExpandStruct(OneStruct);
      strmaxcpy(ListLine, OneStruct->IsUnion ? "(UNION)" : "(STRUCT)", 255);
      PCs[ActPC] += CodeLen;
    }
  }

  else
  {
    StopfZahl = 0;
    CodeLen = 0;
    DontPrint = False;

#ifdef PROFILE_MEMO
    NumMemo = 0;
#endif

    if (IfAsm)
    {
      if (!CodeGlobalPseudo())
        MakeCode();
      if ((MacProOutput) && ((*OpPart != '\0') || (*LabPart != '\0') || (*CommPart != '\0')))
      {
        errno = 0;
        fprintf(MacProFile, "%s\n", OneLine);
        ChkIO(10002);
      }
    }

    for (z = 0; z < StopfZahl; z++)
    {
      switch (ActListGran)
      {
        case 4:
          DAsmCode[CodeLen >> 2] = NOPCode;
          break;
        case 2:
          WAsmCode[CodeLen >> 1] = NOPCode;
          break;
        case 1:
          BAsmCode[CodeLen] = NOPCode;
          break;
      }
      CodeLen += ActListGran/Granularity();
    }

#ifdef PROFILE_MEMO
    NumMemoSum += NumMemo;
    NumMemoCnt++;
#endif

    if ((ActPC != StructSeg) && (!ChkPC(PCs[ActPC] + CodeLen - 1)) && (CodeLen != 0))
      WrError(1925);
    else
    {
      if ((!DontPrint) && (ActPC != StructSeg) && (CodeLen > 0))
        BookKeeping();
      if (ActPC == StructSeg)
      {
        if ((CodeLen != 0) && (!DontPrint)) WrError(1940);
        if (StructStack->StructRec->IsUnion)
        {
          BumpStructLength(StructStack->StructRec, CodeLen);
          CodeLen = 0;
        }
      }
      else if (CodeOutput)
      {
        PCsUsed[ActPC] = True;
        if (DontPrint)
          NewRecord(PCs[ActPC] + CodeLen);
        else
          WriteBytes();
      }
      PCs[ActPC] += CodeLen;
    }
  }

  /* dies ueberprueft implizit, ob von der letzten Eval...-Operation noch
     externe Referenzen liegengeblieben sind. */

  SetRelocs(NULL);
}

/*--- Zeile in Listing zerteilen -------------------------------------------*/

static void SplitLine(void)
{
  jmp_buf Retry;
  String h;
  char *i, *k, *p, *div, *run;
  int l;
  Boolean lpos;

  Retracted = False;

  /* Kommentar loeschen */

  strmaxcpy(h, OneLine, 255);
  i = QuotPos(h, ';');
  if (i)
  {
    strcpy(CommPart, i + 1);
    *i = '\0';
  }
  else
    *CommPart = '\0';

  /* alles in Grossbuchstaben wandeln, Praeprozessor laufen lassen */

  ExpandDefines(h);

  /* Label abspalten */

  if ((*h) && (!myisspace(*h)))
  {
    for (i = h; *i; i++)
      if ((myisspace(*i)) || (*i == ':'))
        break;
    if (!*i)
    {
      strcpy(LabPart, h);
      *h = '\0';
    }
    else
    {
      *i = '\0';
      strcpy(LabPart, h);
      strmov(h, i + 1);
    }
    if (LabPart[l = (strlen(LabPart) - 1)] == ':')
      LabPart[l] = '\0';
  }
  else
    *LabPart = '\0';

  /* Opcode & Argument trennen */

  setjmp(Retry);
  KillPrefBlanks(h);
  i = FirstBlank(h);
  SplitString(h, OpPart, ArgPart, i);

  /* Falls noch kein Label da war, kann es auch ein Label sein */

  i = strchr(OpPart, ':');
  if ((*LabPart == '\0') && (i) && (i[1] == '\0'))
  {
    *i = '\0';
    strcpy(LabPart, OpPart);
    strcpy(OpPart, i + 1);
    if (*OpPart == '\0')
    {
      strcpy(h, ArgPart);
      longjmp(Retry, 1);
    }
  }

  /* Attribut abspalten */

  if (HasAttrs)
  {
    k = NULL; AttrSplit = ' ';
    for (run = AttrChars; *run != '\0'; run++)
    {
      p = strchr(OpPart, *run);
      if (p) if ((!k) || (p < k))
        k = p;
    }
    if (k)
    {
      AttrSplit = (*k);
      strmaxcpy(AttrPart, k + 1, 255);
      *k = '\0';
      if ((*OpPart == '\0') && (*AttrPart != '\0'))
      {
        strmaxcpy(OpPart, AttrPart, 255);
        *AttrPart = '\0';
      }
    }
    else
      *AttrPart = '\0';
  }
  else
    *AttrPart = '\0';

  KillPostBlanks(ArgPart);

  /* Argumente zerteilen: Da alles aus einem String kommt und die Teile alle auch
     so lang sind, koennen wir uns Laengenabfragen sparen */

  ArgCnt = 0;
  strcpy(h, ArgPart);
  run = h;
  if (*run != '\0')
    do
    {
      while ((*run != '\0') && (isspace((unsigned char)*run)))
        run++;
      i = NULL;
      for (div = DivideChars; *div != '\0'; div++)
      {
        p = QuotPos(run, *div);
        if (p)
          if ((!i) || (p < i))
            i = p;
      }
      lpos = ((i) && (i[1] == '\0'));
      if (i)
        *i = '\0';
      strcpy(ArgStr[++ArgCnt], run);
      if ((lpos) && (ArgCnt != ParMax))
        *ArgStr[++ArgCnt] = '\0';
      KillPostBlanks(ArgStr[ArgCnt]);
      run = !i ? i : i + 1;
    }
    while ((run) && (ArgCnt != ParMax) && (!lpos));

  if ((run) && (*run != '\0')) WrError(1140);

  Produce_Code();
}

/*------------------------------------------------------------------------*/

static void ProcessFile(String FileName)
{
  long NxtTime, ListTime;
  String Num;
  char *Name, *Run;

  dbgentry("ProcessFile");

  sprintf(OneLine, " INCLUDE \"%s\"", FileName);
  MasterFile = False;
  NextIncDepth = IncDepth;
  SplitLine();
  IncDepth = NextIncDepth;

  ListTime = GTime();

  while ((!InputEnd()) && (!ENDOccured))
  {
    /* Zeile lesen */

    GetNextLine(OneLine);

    /* Ergebnisfelder vorinitialisieren */

    DontPrint = False;
    CodeLen = 0;
    *ListLine = '\0';

    NextDoLst = DoLst;
    NextIncDepth = IncDepth;

    for (Run = OneLine; *Run != '\0'; Run++)
      if (!isspace(((unsigned int) * Run) & 0xff))
        break;
    if (*Run == '#')
      Preprocess();
    else
      SplitLine();

    MakeList();
    DoLst = NextDoLst;
    IncDepth = NextIncDepth;

    /* Zeilenzaehler */

    if (!QuietMode)
    {
      NxtTime = GTime();
      if (((!ListToStdout) || ((ListMask&1) == 0)) && (DTime(ListTime, NxtTime) > 50))
      {
        sprintf(Num, LongIntFormat, MomLineCounter);
        Name = NamePart(CurrFileName);
        printf("%s(%s)%s", Name, Num, ClrEol);
        /*for (z = 0; z < strlen(Name) + strlen(Num) + 2; z++) putchar('\b');*/
        putchar('\r');
        fflush(stdout);
        ListTime = NxtTime;
      }
    }

    /* bei Ende Makroprozessor ausraeumen
      OK - das ist eine Hauruckmethode... */

    if (ENDOccured)
      while (FirstInputTag)
        GetNextLine(OneLine);
  }

  while (FirstInputTag)
    GetNextLine(OneLine);

  /* irgendeine Makrodefinition nicht abgeschlossen ? */

  if (FirstOutputTag)
    WrError(1800);

  dbgexit("ProcessFile");
}

/****************************************************************************/

static char *TWrite_Plur(int n)
{
  return (n != 1) ? getmessage(Num_ListPlurName) : "";
}

static void TWrite_RWrite(char *dest, Double r, Byte Stellen)
{
  String s;
  char *pFirst;

  sprintf(s, "%20.*f", Stellen, r);

  for (pFirst = s; *pFirst; pFirst++)
    if (!isspace(*pFirst))
      break;

  strcat(dest, pFirst);
}

static void TWrite(Double DTime, char *dest)
{
  int h;
  String s;

  *dest = '\0';
  h = (int) floor(DTime/3600.0);
  if (h > 0)
  {
    sprintf(s, "%d", h);
    strcat(dest, s);
    strcat(dest, getmessage(Num_ListHourName));
    strcat(dest, TWrite_Plur(h));
    strcat(dest, ", ");
    DTime -= 3600.0 * h;
  }
  h = (int) floor(DTime/60.0);
  if (h > 0)
  {
    sprintf(s, "%d", h);
    strcat(dest, s);
    strcat(dest, getmessage(Num_ListMinuName));
    strcat(dest, TWrite_Plur(h));
    strcat(dest, ", ");
    DTime -= 60.0 * h;
  }
  TWrite_RWrite(dest, DTime, 2);
  strcat(dest, getmessage(Num_ListSecoName));
  if (DTime != 1)
    strcat(dest, getmessage(Num_ListPlurName));
}

/*--------------------------------------------------------------------------*/

static void AssembleFile_InitPass(void)
{
  static char DateS[31], TimeS[31];
  int z;
  String ArchVal;

  dbgentry("AssembleFile_InitPass");

  FirstInputTag = NULL;
  FirstOutputTag = NULL;

  MomLineCounter = 0;
  MomLocHandle = -1;
  LocHandleCnt = 0;
  SectSymbolCounter = 0;

  SectionStack = NULL;
  FirstIfSave = NULL;
  FirstSaveState = NULL;
  StructStack = NULL;

  InitPass();

  ActPC = SegCode;
  PCs[ActPC] = 0;
  RelSegs = False;
  ENDOccured = False;
  ErrorCount = 0;
  WarnCount = 0;
  LineSum = 0;
  MacLineSum = 0;
  for (z = 1; z <= StructSeg; z++)
  {
    PCsUsed[z] = FALSE;
    Phases[z] = 0;
    InitChunk(SegChunks + z);
  }

  TransTables =
  CurrTransTable = (PTransTable) malloc(sizeof(TTransTable));
  CurrTransTable->Next = NULL;
  CurrTransTable->Name = strdup("STANDARD");
  CurrTransTable->Table = (unsigned char *) malloc(256 * sizeof(char));
  for (z = 0; z < 256; z++)
    CurrTransTable->Table[z] = z;

  strmaxcpy(CurrFileName, "INTERNAL", 255);
  AddFile(CurrFileName);
  CurrLine = 0;

  IncDepth = -1;
  DoLst = True;

  /* Pseudovariablen initialisieren */

  ResetSymbolDefines();
  ResetMacroDefines();
  ResetStructDefines();
  EnterIntSymbol(FlagTrueName, 1, 0, True);
  EnterIntSymbol(FlagFalseName, 0, 0, True);
  EnterFloatSymbol(PiName, 4.0 * atan(1.0), True);
  EnterIntSymbol(VerName, VerNo, 0, True);
  sprintf(ArchVal, "%s-%s", ARCHPRNAME, ARCHSYSNAME);
  EnterStringSymbol(ArchName, ArchVal, True);
#ifdef HAS64
  EnterIntSymbol(Has64Name, 1, 0, True);
#else
  EnterIntSymbol(Has64Name, 0, 0, True);
#endif
  EnterIntSymbol(CaseSensName, Ord(CaseSensitive), 0, True);
  if (PassNo == 0)
  {
    NLS_CurrDateString(DateS);
    NLS_CurrTimeString(False, TimeS);
  }
  EnterStringSymbol(DateName, DateS, True);
  EnterStringSymbol(TimeName, TimeS, True);

  SetFlag(&DoPadding, DoPaddingName, True);

  if (*DefCPU == '\0')
    SetCPU(0, True);
  else if (!SetNCPU(DefCPU, True))
    SetCPU(0, True);

  SetFlag(&SupAllowed, SupAllowedName, False);
  SetFlag(&FPUAvail, FPUAvailName, False);
  SetFlag(&Maximum, MaximumName, False);
  SetFlag(&DoBranchExt, BranchExtName, False);
  EnterIntSymbol(ListOnName, ListOn = 1, SegNone, True);
  SetFlag(&LstMacroEx, LstMacroExName, True);
  SetFlag(&RelaxedMode, RelaxedName, False);
  EnterIntSymbol(NestMaxName, NestMax = DEF_NESTMAX, SegNone, True);
  CopyDefSymbols();

  /* initialize counter for temp symbols here after implicit symbols
     have been defined, so counter starts at a value as low as possible */

  InitTmpSymbols();

  ResetPageCounter();

  StartAdrPresent = False;

  Repass = False;
  PassNo++;

#ifdef PROFILE_MEMO
  NumMemoSum = 0;
  NumMemoCnt = 0;
#endif

  dbgexit("AssembleFile_InitPass");
}

static void AssembleFile_ExitPass(void)
{
  SwitchFrom();
  ClearLocStack();
  ClearStacks();
  TossRegDefs(-1);
  if (FirstIfSave)
    WrError(1470);
  if (FirstSaveState)
    WrError(1460);
  if (SectionStack)
    WrError(1485);
  if (StructStack)
    WrXError(1551, StructStack->Name);
}

static void AssembleFile(char *Name)
{
  String s, Tmp;

  dbgentry("AssembleFile");

  strmaxcpy(SourceFile, Name, 255);
  if (MakeDebug)
    fprintf(Debug, "File %s\n", SourceFile);

  /* Untermodule initialisieren */

  AsmDefInit();
  AsmParsInit();
  AsmIFInit();
  InitFileList();
  ResetStack();

  /* Kommandozeilenoptionen verarbeiten */

  strmaxcpy(OutName, GetFromOutList(), 255);
  if (OutName[0] == '\0')
  {
    strmaxcpy(OutName, SourceFile, 255);
    KillSuffix(OutName);
    AddSuffix(OutName, PrgSuffix);
  }

  if (*ErrorPath == '\0')
  {
    strmaxcpy(ErrorName, SourceFile, 255);
    KillSuffix(ErrorName);
    AddSuffix(ErrorName, LogSuffix);
    unlink(ErrorName);
  }

  switch (ListMode)
  {
    case 0:
      strmaxcpy(LstName, NULLDEV, 255);
      break;
    case 1:
      strmaxcpy(LstName, "!1", 255);
      break;
    case 2:
      strmaxcpy(LstName, GetFromListOutList(), 255);
      if (*LstName == '\0')
      {
        strmaxcpy(LstName, SourceFile, 255);
        KillSuffix(LstName);
        AddSuffix(LstName, LstSuffix);
      }
      break;
  }
  ListToStdout = !strcmp(LstName, "!1");
  ListToNull = !strcmp(LstName, NULLDEV);

  if (ShareMode != 0)
  {
    strmaxcpy(ShareName, GetFromShareOutList(), 255);
    if (*ShareName == '\0')
    {
      strmaxcpy(ShareName, SourceFile, 255);
      KillSuffix(ShareName);
      switch (ShareMode)
      {
        case 1:
          AddSuffix(ShareName, ".inc");
          break;
        case 2:
          AddSuffix(ShareName, ".h");
          break;
        case 3:
          AddSuffix(ShareName, IncSuffix);
          break;
      }
    }
  }

  if (MacProOutput)
  {
    strmaxcpy(MacProName, SourceFile, 255);
    KillSuffix(MacProName);
    AddSuffix(MacProName, PreSuffix);
  }

  if (MacroOutput)
  {
    strmaxcpy(MacroName, SourceFile, 255);
    KillSuffix(MacroName);
    AddSuffix(MacroName, MacSuffix);
  }

  ClearIncludeList();

  if (DebugMode != DebugNone)
    InitLineInfo();

  /* Variablen initialisieren */

  StartTime = GTime();

  PassNo = 0;
  MomLineCounter = 0;

  /* Listdatei eroeffnen */

  if (!QuietMode)
    printf("%s%s\n", getmessage(Num_InfoMessAssembling), SourceFile);

  do
  {
    /* Durchlauf initialisieren */

    AssembleFile_InitPass();
    AsmSubInit();
    if (!QuietMode)
    {
      sprintf(Tmp, IntegerFormat, PassNo);
      printf("%s%s%s\n", getmessage(Num_InfoMessPass), Tmp, ClrEol);
    }

    /* Dateien oeffnen */

    if (CodeOutput)
      OpenFile();

    if (ShareMode != 0)
    {
      ShareFile = fopen(ShareName, "w");
      if (!ShareFile)
        ChkIO(10001);
      errno = 0;
      switch (ShareMode)
      {
        case 1:
          fprintf(ShareFile, "(* %s-Includefile f%sr CONST-Sektion *)\n", SourceFile, CH_ue);
          break;
        case 2:
          fprintf(ShareFile, "/* %s-Includefile f%sr C-Programm */\n", SourceFile, CH_ue);
          break;
        case 3:
          fprintf(ShareFile, "; %s-Includefile f%sr Assembler-Programm\n", SourceFile, CH_ue);
          break;
      }
      ChkIO(10002);
    }

    if (MacProOutput)
    {
      MacProFile = fopen(MacProName, "w");
      if (!MacProFile)
        ChkIO(10001);
    }

    if ((MacroOutput) && (PassNo == 1))
    {
      MacroFile = fopen(MacroName, "w");
      if (!MacroFile)
        ChkIO(10001);
    }

    /* Listdatei oeffnen */

    RewriteStandard(&LstFile, LstName);
    if (!LstFile)
      ChkIO(10001);
    if (!ListToNull)
    {
      errno = 0;
      fprintf(LstFile, "%s", PrtInitString);
      ChkIO(10002);
    }
    if ((ListMask & 1) != 0)
      NewPage(0, False);

    /* assemblieren */

    ProcessFile(SourceFile);
    AssembleFile_ExitPass();

    /* Dateien schliessen */

    if (CodeOutput)
      CloseFile();

    if (ShareMode != 0)
    {
      errno = 0;
      switch (ShareMode)
      {
        case 1:
          fprintf(ShareFile, "(* Ende Includefile f%sr CONST-Sektion *)\n", CH_ue);
          break;
        case 2:
          fprintf(ShareFile, "/* Ende Includefile f%sr C-Programm */\n", CH_ue);
          break;
        case 3:
          fprintf(ShareFile, "; Ende Includefile f%sr Assembler-Programm\n", CH_ue);
          break;
      }
      ChkIO(10002);
      fclose(ShareFile);
    }

    if (MacProOutput)
      fclose(MacProFile);
    if ((MacroOutput) && (PassNo == 1))
      fclose(MacroFile);

    /* evtl. fuer naechsten Durchlauf aufraeumen */

    if ((ErrorCount == 0) && (Repass))
    {
      fclose(LstFile);
      if (CodeOutput)
        unlink(OutName);
      CleanupRegDefs();
      ClearCodepages();
      if (MakeUseList)
        ClearUseList();
      if (MakeCrossList)
        ClearCrossList();
      ClearDefineList();
      if (DebugMode != DebugNone)
        ClearLineInfo();
      ClearIncludeList();
      if (DebugMode != DebugNone)
      {
        ResetAddressRanges();
        ClearSectionUsage();
      }
    }
  }
  while ((ErrorCount == 0) && (Repass));

  /* bei Fehlern loeschen */

  if (ErrorCount != 0)
  {
    if (CodeOutput)
      unlink(OutName);
    if (MacProOutput)
      unlink(MacProName);
    if ((MacroOutput) && (PassNo == 1))
      unlink(MacroName);
    if (ShareMode != 0)
      unlink(ShareName);
    GlobErrFlag = True;
  }

  /* Debug-Ausgabe muss VOR die Symbollistenausgabe, weil letztere die
     Symbolliste loescht */

  if (DebugMode != DebugNone)
  {
    if (ErrorCount == 0)
      DumpDebugInfo();
    ClearLineInfo();
  }

  /* Listdatei abschliessen */

  if  (strcmp(LstName, NULLDEV))
  {
    if (ListMask & 2)
      PrintSymbolList();

    if (ListMask & 64)
      PrintRegDefs();

    if (ListMask & 4)
      PrintMacroList();

    if (ListMask & 256)
      PrintStructList();

    if (ListMask & 8)
      PrintFunctionList();

    if (ListMask & 32)
      PrintDefineList();

    if (ListMask & 128)
      PrintCodepages();

    if (MakeUseList)
    {
      NewPage(ChapDepth, True);
      PrintUseList();
    }

    if (MakeCrossList)
    {
      NewPage(ChapDepth, True);
      PrintCrossList();
    }

    if (MakeSectionList)
      PrintSectionList();

    if (MakeIncludeList)
      PrintIncludeList();

    if (!ListToNull)
    {
      errno = 0;
      fprintf(LstFile, "%s", PrtExitString);
      ChkIO(10002);
    }
  }

  if (MakeUseList)
    ClearUseList();

  if (MakeCrossList)
    ClearCrossList();

  ClearSectionList();

  ClearIncludeList();

  if ((*ErrorPath == '\0') && (IsErrorOpen))
  {
    fclose(ErrorFile);
    IsErrorOpen = False;
  }

  ClearUp();

  /* Statistik ausgeben */

  StopTime = GTime();
  TWrite(DTime(StartTime, StopTime)/100.0, s);
  if (!QuietMode)
    printf("\n%s%s%s\n\n", s, getmessage(Num_InfoMessAssTime), ClrEol);
  if (ListMode == 2)
  {
    WrLstLine("");
    strmaxcat(s, getmessage(Num_InfoMessAssTime), 255);
    WrLstLine(s);
    WrLstLine("");
  }

  Dec32BlankString(s, LineSum, 7);
  strmaxcat(s, getmessage((LineSum == 1) ? Num_InfoMessAssLine : Num_InfoMessAssLines), 255);
  if (!QuietMode)
    printf("%s%s\n", s, ClrEol);
  if (ListMode == 2)
    WrLstLine(s);

  if (LineSum != MacLineSum)
  {
    Dec32BlankString(s, MacLineSum, 7);
    strmaxcat(s, getmessage((MacLineSum == 1) ? Num_InfoMessMacAssLine : Num_InfoMessMacAssLines), 255);
    if (!QuietMode)
      printf("%s%s\n", s, ClrEol);
    if (ListMode == 2)
      WrLstLine(s);
  }

  Dec32BlankString(s, PassNo, 7);
  strmaxcat(s, getmessage((PassNo == 1) ? Num_InfoMessPassCnt : Num_InfoMessPPassCnt), 255);
  if (!QuietMode)
    printf("%s%s\n", s, ClrEol);
  if (ListMode == 2)
    WrLstLine(s);

  if ((ErrorCount > 0) && (Repass) && (ListMode != 0))
    WrLstLine(getmessage(Num_InfoMessNoPass));

#ifdef __TURBOC__
  sprintf(s, "%s%s", Dec32BlankString(Tmp, coreleft() >> 10, 7), getmessage(Num_InfoMessRemainMem));
  if (!QuietMode)
    printf("%s%s\n", s, ClrEol);
  if (ListMode == 2)
    WrLstLine(s);

  sprintf(s, "%s%s", Dec32BlankString(Tmp, StackRes(), 7), getmessage(Num_InfoMessRemainStack));
  if (!QuietMode)
    printf("%s%s\n", s, ClrEol);
  if (ListMode == 2)
    WrLstLine(s);
#endif

  sprintf(s, "%s%s", Dec32BlankString(Tmp, ErrorCount, 7), getmessage(Num_InfoMessErrCnt));
  if (ErrorCount != 1)
    strmaxcat(s, getmessage(Num_InfoMessErrPCnt), 255);
  if (!QuietMode)
    printf("%s%s\n", s, ClrEol);
  if (ListMode == 2)
    WrLstLine(s);

  sprintf(s, "%s%s", Dec32BlankString(Tmp, WarnCount, 7), getmessage(Num_InfoMessWarnCnt));
  if (WarnCount != 1)
    strmaxcat(s, getmessage(Num_InfoMessWarnPCnt), 255);
  if (!QuietMode)
    printf("%s%s\n", s, ClrEol);
  if (ListMode == 2)
    WrLstLine(s);

#ifdef PROFILE_MEMO
  sprintf(s, "%7.2f%s", ((double)NumMemoSum) / NumMemoCnt, " oppart compares");
  if (!QuietMode)
    printf("%s%s\n", s, ClrEol);
  if (ListMode == 2)
    WrLstLine(s);
#endif

  fclose(LstFile);

  /* verstecktes */

  if (MakeDebug)
    PrintSymbolDepth();

  /* Speicher freigeben */

  ClearSymbolList();
  ClearRegDefs();
  ClearCodepages();
  ClearMacroList();
  ClearFunctionList();
  ClearDefineList();
  ClearFileList();
  ClearStructList();

  dbgentry("AssembleFile");
}

static void AssembleGroup(void)
{
  AddSuffix(FileMask, SrcSuffix);
  if (!DirScan(FileMask, AssembleFile))
    fprintf(stderr, "%s%s\n", FileMask, getmessage(Num_InfoMessNFilesFound));
}

/*-------------------------------------------------------------------------*/

static CMDResult CMD_SharePascal(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  if (!Negate)
    ShareMode = 1;
  else if (ShareMode == 1)
    ShareMode = 0;
  return CMDOK;
}

static CMDResult CMD_ShareC(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  if (!Negate)
    ShareMode = 2;
  else if (ShareMode == 2)
    ShareMode = 0;
  return CMDOK;
}

static CMDResult CMD_ShareAssembler(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  if (!Negate)
    ShareMode = 3;
  else if (ShareMode == 3)
    ShareMode = 0;
  return CMDOK;
}

static CMDResult CMD_DebugMode(Boolean Negate, const char *pArg)
{
  String Arg;

  strmaxcpy(Arg, pArg, 255);
  UpString(Arg);

  if (Negate)
  {
    if (Arg[0] != '\0')
      return CMDErr;
    else
    {
      DebugMode = DebugNone;
      return CMDOK;
    }
  }
  else if (!strcmp(Arg, ""))
  {
    DebugMode = DebugMAP;
    return CMDOK;
  }
  else if (!strcmp(Arg, "ATMEL"))
  {
    DebugMode = DebugAtmel;
    return CMDArg;
  }
  else if (!strcmp(Arg, "MAP"))
  {
    DebugMode = DebugMAP;
    return CMDArg;
  }
  else if (!strcmp(Arg, "NOICE"))
  {
    DebugMode = DebugNoICE;
    return CMDArg;
  }
#if 0
  else if (!strcmp(Arg, "A.OUT"))
  {
    DebugMode = DebugAOUT;
    return CMDArg;
  }
  else if (!strcmp(Arg, "COFF"))
  {
    DebugMode = DebugCOFF;
    return CMDArg;
  }
  else if (!strcmp(Arg, "ELF"))
  {
    DebugMode = DebugELF;
    return CMDArg;
  }
#endif
  else
    return CMDErr;

#if 0
  if (Negate)
    DebugMode = DebugNone;
  else
    DebugMode = DebugMAP;
  return CMDOK;
#endif
}

static CMDResult CMD_ListConsole(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  if (!Negate)
    ListMode = 1;
  else if (ListMode == 1)
    ListMode = 0;
  return CMDOK;
}

static CMDResult CMD_ListFile(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  if (!Negate)
    ListMode = 2;
  else if (ListMode == 2)
    ListMode = 0;
  return CMDOK;
}

static CMDResult CMD_SuppWarns(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  SuppWarns = !Negate;
  return CMDOK;
}

static CMDResult CMD_UseList(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  MakeUseList = !Negate;
  return CMDOK;
}

static CMDResult CMD_CrossList(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  MakeCrossList = !Negate;
  return CMDOK;
}

static CMDResult CMD_SectionList(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  MakeSectionList = !Negate;
  return CMDOK;
}

static CMDResult CMD_BalanceTree(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  BalanceTree = !Negate;
  return CMDOK;
}

static CMDResult CMD_MakeDebug(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  if (!Negate)
  {
    MakeDebug = True;
    errno = 0;
    Debug = fopen("as.deb", "w");
    if (!Debug)
      ChkIO(10002);
  }
  else if (MakeDebug)
  {
    MakeDebug = False;
    fclose(Debug);
  }
  return CMDOK;
}

static CMDResult CMD_MacProOutput(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  MacProOutput = !Negate;
  return CMDOK;
}

static CMDResult CMD_MacroOutput(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  MacroOutput = !Negate;
  return CMDOK;
}

static CMDResult CMD_MakeIncludeList(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  MakeIncludeList = !Negate;
  return CMDOK;
}

static CMDResult CMD_CodeOutput(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  CodeOutput = !Negate;
  return CMDOK;
}

static CMDResult CMD_MsgIfRepass(Boolean Negate, const char *Arg)
{
  Boolean OK;
  UNUSED(Arg);

  MsgIfRepass = !Negate;
  if (MsgIfRepass)
  {
    if (Arg[0] == '\0')
    {
      PassNoForMessage = 1;
      return CMDOK;
    }
    else
    {
      PassNoForMessage = ConstLongInt(Arg, &OK, 10);
      if (!OK)
      {
        PassNoForMessage = 1;
        return CMDOK;
      }
      else if (PassNoForMessage < 1)
        return CMDErr;
      else
        return CMDArg;
    }
  }
  else
    return CMDOK;
}

static CMDResult CMD_ExtendErrors(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  if ((Negate) && (ExtendErrors > 0))
    ExtendErrors--;
  else if ((!Negate) && (ExtendErrors < 2))
    ExtendErrors++;

  return CMDOK;
}

static CMDResult CMD_NumericErrors(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  NumericErrors = !Negate;
  return CMDOK;
}

static CMDResult CMD_HexLowerCase(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  HexLowerCase = !Negate;
  return CMDOK;
}

static CMDResult CMD_QuietMode(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  QuietMode = !Negate;
  return CMDOK;
}

static CMDResult CMD_ThrowErrors(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  ThrowErrors = !Negate;
  return CMDOK;
}

static CMDResult CMD_CaseSensitive(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  CaseSensitive = !Negate;
  return CMDOK;
}

static CMDResult CMD_GNUErrors(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  GNUErrors  =  !Negate;
  return CMDOK;
}

static CMDResult CMD_IncludeList(Boolean Negate, const char *Arg)
{
  char *p;
  String Copy, part;

  if (*Arg == '\0') return CMDErr;
  else
  {
    strmaxcpy(Copy, Arg, 255);
    do
    {
      p = strrchr(Copy, DIRSEP);
      if (!p)
      {
        strmaxcpy(part, Copy, 255);
        *Copy = '\0';
      }
      else
      {
        *p = '\0';
        strmaxcpy(part, p + 1, 255);
      }
      if (Negate)
        RemoveIncludeList(part);
      else
        AddIncludeList(part);
    }
    while (Copy[0] != '\0');
    return CMDArg;
  }
}

static CMDResult CMD_ListMask(Boolean Negate, const char *Arg)
{
  Word erg;
  Boolean OK;

  if (Arg[0] == '\0')
    return CMDErr;
  else
  {
    erg = ConstLongInt(Arg, &OK, 10);
    if ((!OK) || (erg > 511))
      return CMDErr;
    else
    {
      ListMask = Negate ? (ListMask & ~erg) : (ListMask | erg);
      return CMDArg;
    }
  }
}

static CMDResult CMD_DefSymbol(Boolean Negate, const char *Arg)
{
  String Copy, Part, Name;
  char *p;
  TempResult t;

  if (Arg[0] == '\0')
    return CMDErr;

  strmaxcpy(Copy, Arg, 255);
  do
  {
    p = QuotPos(Copy, ',');
    if (!p)
    {
      strmaxcpy(Part, Copy, 255);
      Copy[0] = '\0';
    }
    else
    {
      *p = '\0';
      strmaxcpy(Part, Copy, 255);
      strcpy(Copy, p + 1);
    }
   if (!CaseSensitive)
     UpString(Part);
   p = QuotPos(Part, '=');
   if (!p)
   {
     strmaxcpy(Name, Part, 255);
     Part[0] = '\0';
   }
   else
   {
     *p = '\0';
     strmaxcpy(Name, Part, 255);
     strcpy(Part, p + 1);
   }
   if (!ChkSymbName(Name))
     return CMDErr;
   if (Negate)
     RemoveDefSymbol(Name);
   else
   {
     AsmParsInit();
     if (Part[0] != '\0')
     {
       FirstPassUnknown = False;
       EvalExpression(Part, &t);
       if ((t.Typ == TempNone) || (FirstPassUnknown))
         return CMDErr;
     }
     else
     {
       t.Typ = TempInt;
       t.Contents.Int = 1;
     }
     AddDefSymbol(Name, &t);
   }
  }
  while (Copy[0] != '\0');

  return CMDArg;
}

static CMDResult CMD_ErrorPath(Boolean Negate, const char *Arg)
{
  if (Negate)
    return CMDErr;
  else if (Arg[0] == '\0')
  {
    ErrorPath[0] = '\0';
    return CMDOK;
  }
  else
  {
    strmaxcpy(ErrorPath, Arg, 255);
    return CMDArg;
  }
}

static CMDResult CMD_HardRanges(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  HardRanges = Negate;
  return CMDOK;
}

static CMDResult CMD_OutFile(Boolean Negate, const char *Arg)
{
  if (Arg[0] == '\0')
  {
    if (Negate)
    {
      ClearOutList();
      return CMDOK;
    }
    else
      return CMDErr;
  }
  else
  {
    if (Negate)
      RemoveFromOutList(Arg);
    else
      AddToOutList(Arg);
    return CMDArg;
  }
}

static CMDResult CMD_ShareOutFile(Boolean Negate, const char *Arg)
{
  if (Arg[0] == '\0')
  {
    if (Negate)
    {
      ClearShareOutList();
      return CMDOK;
    }
    else
      return CMDErr;
  }
  else
  {
    if (Negate)
      RemoveFromShareOutList(Arg);
    else
      AddToShareOutList(Arg);
    return CMDArg;
  }
}

static CMDResult CMD_ListOutFile(Boolean Negate, const char *Arg)
{
  if (Arg[0] == '\0')
  {
    if (Negate)
    {
      ClearListOutList();
      return CMDOK;
    }
    else
      return CMDErr;
  }
  else
  {
    if (Negate)
      RemoveFromListOutList(Arg);
    else
      AddToListOutList(Arg);
    return CMDArg;
  }
}

static Boolean CMD_CPUAlias_ChkCPUName(char *s)
{
  int z;

  for(z = 0; z < (int)strlen(s); z++)
    if (!isalnum((unsigned int) s[z]))
      return False;
  return True;
}

static CMDResult CMD_CPUAlias(Boolean Negate, const char *Arg)
{
  char *p;
  String s1, s2;

  if (Negate)
    return CMDErr;
  else if (Arg[0] == '\0')
    return CMDErr;
  else
  {
    p = strchr(Arg, '=');
    if (!p)
      return CMDErr;
    else
    {
      *p = '\0';
      strmaxcpy(s1, Arg, 255);
      UpString(s1);
      strmaxcpy(s2, p + 1, 255);
      UpString(s2);
      *p = '=';
      if (!(CMD_CPUAlias_ChkCPUName(s1) && CMD_CPUAlias_ChkCPUName(s2)))
        return CMDErr;
      else if (!AddCPUAlias(s2, s1))
        return CMDErr;
      else
        return CMDArg;
    }
  }
}

static CMDResult CMD_SetCPU(Boolean Negate, const char *Arg)
{
  if (Negate)
  {
    *DefCPU = '\0';
    return CMDOK;
  }
  else
  {
    if (*Arg == '\0')
      return CMDErr;
    strmaxcpy(DefCPU, Arg, sizeof(DefCPU) - 1);
    NLS_UpString(DefCPU);
    return CMDArg;
  }
}

static CMDResult CMD_NoICEMask(Boolean Negate, const char *Arg)
{
  Word erg;
  Boolean OK;

  if (Negate)
  {
    NoICEMask = 1 << SegCode;
    return CMDOK;
  }
  else if (Arg[0] == '\0')
    return CMDErr;
  else
  {
    erg = ConstLongInt(Arg, &OK, 10);
    if ((!OK) || (erg > (1 << PCMax)))
      return CMDErr;
    else
    {
      NoICEMask = erg;
      return CMDArg;
    }
  }
}

static void ParamError(Boolean InEnv, char *Arg)
{
  printf("%s%s\n", getmessage((InEnv) ? Num_ErrMsgInvEnvParam : Num_ErrMsgInvParam), Arg);
  exit(4);
}

#define ASParamCnt (sizeof(ASParams) / sizeof(*ASParams))

static CMDRec ASParams[] =
{
  { "A"             , CMD_BalanceTree     },
  { "ALIAS"         , CMD_CPUAlias        },
  { "a"             , CMD_ShareAssembler  },
  { "C"             , CMD_CrossList       },
  { "c"             , CMD_ShareC          },
  { "CPU"           , CMD_SetCPU          },
  { "D"             , CMD_DefSymbol       },
  { "E"             , CMD_ErrorPath       },
  { "g"             , CMD_DebugMode       },
  { "G"             , CMD_CodeOutput      },
  { "GNUERRORS"     , CMD_GNUErrors       },
  { "h"             , CMD_HexLowerCase    },
  { "i"             , CMD_IncludeList     },
  { "I"             , CMD_MakeIncludeList },
  { "L"             , CMD_ListFile        },
  { "l"             , CMD_ListConsole     },
  { "M"             , CMD_MacroOutput     },
  { "n"             , CMD_NumericErrors   },
  { "NOICEMASK"     , CMD_NoICEMask       },
  { "o"             , CMD_OutFile         },
  { "P"             , CMD_MacProOutput    },
  { "p"             , CMD_SharePascal     },
  { "q"             , CMD_QuietMode       },
  { "QUIET"         , CMD_QuietMode       },
  { "r"             , CMD_MsgIfRepass     },
  { "s"             , CMD_SectionList     },
  { "SHAREOUT"      , CMD_ShareOutFile    },
  { "OLIST"         , CMD_ListOutFile     },
  { "t"             , CMD_ListMask        },
  { "u"             , CMD_UseList         },
  { "U"             , CMD_CaseSensitive   },
  { "w"             , CMD_SuppWarns       },
  { "WARNRANGES"    , CMD_HardRanges      },
  { "x"             , CMD_ExtendErrors    },
  { "X"             , CMD_MakeDebug       },
  { "Y"             , CMD_ThrowErrors     }
};

/*--------------------------------------------------------------------------*/

#ifdef __sunos__

extern void on_exit(void (*procp)(int status, caddr_t arg),caddr_t arg);

static void GlobExitProc(int status, caddr_t arg)
{
  if (MakeDebug)
    fclose(Debug);
}

#else

static void GlobExitProc(void)
{
  if (MakeDebug)
    fclose(Debug);
}

#endif

static int LineZ;

static void NxtLine(void)
{
  if (++LineZ == 23)
  {
    LineZ = 0;
    if (Redirected != NoRedir)
      return;
    printf("%s", getmessage(Num_KeyWaitMsg));
    fflush(stdout);
    while (getchar() != '\n');
    printf("%s%s", CursUp, ClrEol);
  }
}

static void WrHead(void)
{
  if (!QuietMode)
  {
    printf("%s%s\n", getmessage(Num_InfoMessMacroAss), Version);
    NxtLine();
    printf("(%s-%s)\n", ARCHPRNAME, ARCHSYSNAME);
    NxtLine();
    printf("%s\n", InfoMessCopyright);
    NxtLine();
    WriteCopyrights(NxtLine);
    printf("\n");
    NxtLine();
  }
}

int main(int argc, char **argv)
{
  char *Env, *ph1, *ph2;
  String Dummy;
  static Boolean First = TRUE;
  CMDProcessed ParUnprocessed;     /* bearbeitete Kommandozeilenparameter */

  FileMask = (char*)malloc(sizeof(char) * STRINGSIZE);

  ParamCount = argc - 1;
  ParamStr = argv;

  if (First)
  {
    endian_init();
    nls_init();
    bpemu_init();
    stdhandl_init();
    strutil_init();
    stringlists_init();
    chunks_init();
    NLS_Initialize();

    nlmessages_init("as.msg", *argv, MsgId1, MsgId2);
    ioerrs_init(*argv);
    cmdarg_init(*argv);

    asmfnums_init();
    asminclist_init();
    asmitree_init();

    asmdef_init();
    asmsub_init();
    asmpars_init();

    asmmac_init();
    asmstruct_init();
    asmif_init();
    asmcode_init();
    asmdebug_init();

    codeallg_init();

    code68k_init();
    code56k_init();
    code601_init();
    codemcore_init();
    codexgate_init();
    code68_init();
    code6805_init();
    code6809_init();
    code6812_init();
    code6816_init();
    code68rs08_init();
    codeh8_3_init();
    codeh8_5_init();
    code7000_init();
    code65_init();
    code7700_init();
    code4500_init();
    codem16_init();
    codem16c_init();
    code4004_init();
    code8008_init();
    code48_init();
    code51_init();
    code96_init();
    code85_init();
    code86_init();
    code960_init();
    code8x30x_init();
    code2650_init();
    codexa_init();
    codeavr_init();
    code29k_init();
    code166_init();
    codez80_init();
    codez8_init();
    codekcpsm_init();
    codekcpsm3_init();
    codemico8_init();
    code96c141_init();
    code90c141_init();
    code87c800_init();
    code870c_init();
    code47c00_init();
    code97c241_init();
    code16c5x_init();
    code16c8x_init();
    code17c4x_init();
    codest6_init();
    codest7_init();
    codest9_init();
    code6804_init();
    code3201x_init();
    code3202x_init();
    code3203x_init();
    code3205x_init();
    code32054x_init();
    code3206x_init();
    code9900_init();
    codetms7_init();
    code370_init();
    codemsp_init();
    codetms1_init();
    code78c10_init();
    code75xx_init();
    code75k0_init();
    code78k0_init();
    code78k2_init();
    code7720_init();
    code77230_init();
    codescmp_init();
    code807x_init();
    codecop4_init();
    codecop8_init();
    codesc14xxx_init();
    codeace_init();
    code53c8xx_init();
    codef2mc8_init();
    codef2mc16_init();
    code1802_init();
    codevector_init();
    codexcore_init();
    /*as1750_init();*/
    First = FALSE;
  }

#ifdef __sunos__
  on_exit(GlobExitProc, (caddr_t) NULL);
#else
# ifndef __MUNIX__
  atexit(GlobExitProc);
# endif
#endif

  *CursUp = '\0';
  *ClrEol = '\0';
  switch (Redirected)
  {
    case NoRedir:
      Env = getenv("USEANSI");
      strmaxcpy(Dummy, Env ? Env : "Y", 255);
      if (mytoupper(Dummy[0]) == 'N')
      {
      }
      else
      {
        strcpy(ClrEol, " [K"); ClrEol[0] = Char_ESC;  /* ANSI-Sequenzen */
        strcpy(CursUp, " [A"); CursUp[0] = Char_ESC;
      }
      break;
    case RedirToDevice:
      /* Basissteuerzeichen fuer Geraete */
      memset(ClrEol, ' ', 20);
      memset(ClrEol + 20, '\b', 20);
      ClrEol[40] = '\0';
      break;
    case RedirToFile:
      strcpy(ClrEol, "\n");  /* CRLF auf Datei */
  }

  ShareMode = 0;
  ListMode = 0;
  IncludeList[0] = '\0';
  SuppWarns = False;
  MakeUseList = False;
  MakeCrossList = False;
  MakeSectionList = False;
  MakeIncludeList = False;
  ListMask = 0x1ff;
  MakeDebug = False;
  ExtendErrors = 0;
  MacroOutput = False;
  MacProOutput = False;
  CodeOutput = True;
  strcpy(ErrorPath,  "!2");
  MsgIfRepass = False;
  QuietMode = False;
  NumericErrors = False;
  DebugMode = DebugNone;
  CaseSensitive = False;
  ThrowErrors = False;
  HardRanges = True;
  NoICEMask = 1 << SegCode;
  GNUErrors = False;

  LineZ = 0;

  if (ParamCount == 0)
  {
    WrHead();
    printf("%s%s%s\n", getmessage(Num_InfoMessHead1), GetEXEName(), getmessage(Num_InfoMessHead2));
    NxtLine();
    for (ph1 = getmessage(Num_InfoMessHelp), ph2 = strchr(ph1, '\n'); ph2; ph1 = ph2 + 1, ph2 = strchr(ph1, '\n'))
    {
      *ph2 = '\0';
      printf("%s\n", ph1);
      NxtLine();
      *ph2 = '\n';
    }
    PrintCPUList(NxtLine);
    ClearCPUList();
    exit(1);
  }

#if defined(STDINCLUDES)
  CMD_IncludeList(False, STDINCLUDES);
#endif
  ProcessCMD(ASParams, ASParamCnt, ParUnprocessed, EnvName, ParamError);

  /* wegen QuietMode dahinter */

  WrHead();

  GlobErrFlag = False;
  if (ErrorPath[0] != '\0')
  {
    strcpy(ErrorName, ErrorPath);
    unlink(ErrorName);
  }
  IsErrorOpen = False;

  if (StringListEmpty(FileArgList))
  {
    printf("%s [%s] ", getmessage(Num_InvMsgSource), SrcSuffix);
    fflush(stdout);
    fgets(FileMask, 255, stdin);
    if ((*FileMask) && (FileMask[strlen(FileMask) - 1] == '\n'))
      FileMask[strlen(FileMask) - 1] = '\0';
    AssembleGroup();
  }
  else
  {
    StringRecPtr Lauf;
    char *pFile;

    pFile = GetStringListFirst(FileArgList, &Lauf);
    while ((pFile) && (*pFile))
    {
      strmaxcpy(FileMask, pFile, 255);
      AssembleGroup();
      pFile = GetStringListNext(&Lauf);
    }
  }

  if ((ErrorPath[0] != '\0') && (IsErrorOpen))
  {
    fclose(ErrorFile);
    IsErrorOpen = False;
  }

  ClearCPUList();

  return GlobErrFlag ? 2 : 0;
}
