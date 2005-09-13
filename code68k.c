/* code68k.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator 680x0-Familie                                               */
/*                                                                           */
/* Historie:  9. 9.1996 Grundsteinlegung                                     */
/*           14.11.1997 Coldfire-Erweiterungen                               */
/*           31. 5.1998 68040-Erweiterungen                                  */
/*            7. 7.1998 Fix Zugriffe auf CharTransTable wg. signed chars     */
/*            3. 1.1999 ChkPC-Anpassung                                      */
/*           17. 1.1999 automatische Laengenanpassung OutDisp                */
/*           23. 1.1999 Einen String an sich selber anzuhaengen, ist keine   */
/*                      gute Idee gewesen :-)                                */
/*           25. 1.1999 falscher Code fuer SBCD korrigiert                   */
/*            5. 7.1999 bei FMOVE FPreg, <ea> war die Modusmaske Humbug...   */
/*                      FSMOVE/FDMOVE fuer 68040 fehlten noch                */
/*            9. 7.1999 In der Bitfeld-Dekodierung war bei der Portierung    */
/*                      ein call-by-reference verlorengegangen               */
/*            3.11.1999 ...in SplitBitField auch!                            */
/*            4.11.1999 FSMOVE/DMOVE auch mit FPn als Quelle                 */
/*                      F(S/D)(ADD/SUB/MUL/DIV)                              */
/*                      FMOVEM statt FMOVE fpcr<->ea erlaubt                 */
/*           21. 1.2000 ADDX/SUBX vertauscht                                 */
/*            9. 3.2000 'ambigious else'-Warnungen beseitigt                 */
/*                      EXG korrigiert                                       */
/*            1.10.2000 added missing chk.l                                  */
/*                      differ add(i) sub(i) cmp(i) #imm,dn                  */
/*            3.10.2000 fixed coding of register lists with start > stop     */
/*                      better auto-scaling of outer displacements           */
/*                      fix extension word for 32-bit PC-rel. displacements  */
/*                      allow PC-rel. addressing for CMP                     */
/*                      register names must be 2 chars long                  */
/*           15.10.2000 added handling of outer displacement in ()           */
/*           12.11.2000 RelPos must be 4 for MOVEM                           */
/*           2001-12-02 fixed problems with forward refs of shift arguments  */
/*                                                                           */
/*****************************************************************************/
/* $Id: code68k.c,v 1.3 2005/09/08 16:53:41 alfred Exp $                     */
/*****************************************************************************
 * $Log: code68k.c,v $
 * Revision 1.3  2005/09/08 16:53:41  alfred
 * - use common PInstTable
 *
 * Revision 1.2  2004/05/29 12:04:47  alfred
 * - relocated DecodeMot(16)Pseudo into separate module
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "bpemu.h"
#include "endian.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmallg.h"
#include "motpseudo.h"
#include "asmitree.h"
#include "codevars.h"


typedef struct 
         {
          Word Code;
          Boolean MustSup;
          Word CPUMask;
         } FixedOrder;

typedef struct 
         {
          char *Name;
          Word Code;
          CPUVar FirstCPU,LastCPU;
         } CtReg;

typedef struct
         {
          char *Name;
          Byte Code;
          Boolean Dya;
          CPUVar MinCPU;
         } FPUOp;

typedef struct
         {
          char *Name;
          Byte Code;
         } FPUCond;

#define FixedOrderCnt 10
#define CtRegCnt 29
#define CondCnt 20
#define FPUOpCnt 43
#define FPUCondCnt 26
#define PMMUCondCnt 16
#define PMMURegCnt 13

#define PMMUAvailName  "HASPMMU"     /* PMMU-Befehle erlaubt */
#define FullPMMUName   "FULLPMMU"    /* voller PMMU-Befehlssatz */

#define Mdata 1                      /* Adressierungsmasken */
#define Madr 2
#define Madri 4
#define Mpost 8
#define Mpre 16       
#define Mdadri 32
#define Maix 64
#define Mpc 128
#define Mpcidx 256
#define Mabs 512
#define Mimm 1024
#define Mfpn 2048
#define Mfpcr 4096

static Byte OpSize;
static ShortInt RelPos;
static Boolean PMMUAvail;               /* PMMU-Befehle erlaubt? */
static Boolean FullPMMU;                /* voller PMMU-Befehlssatz? */
static Byte AdrNum;                     /* Adressierungsnummer */
static Word AdrMode;                    /* Adressierungsmodus */
static Word AdrVals[10];                /* die Worte selber */

static FixedOrder *FixedOrders;
static CtReg *CtRegs;
static char **CondNams;
static Byte *CondVals;
static FPUOp *FPUOps;
static FPUCond *FPUConds;
static char **PMMUConds;
static char **PMMURegNames;
static Byte *PMMURegSizes;
static Word *PMMURegCodes;
static PInstTable FInstTable,CInstTable;

static SimpProc SaveInitProc;

static CPUVar CPU68008,CPU68000,CPU68010,CPU68012,
              CPUCOLD,
              CPU68332,CPU68340,CPU68360,
              CPU68020,CPU68030,CPU68040;

static Word Masks[14]={0,1,2,4,8,16,32,64,128,256,512,1024,2048,4096};
static Byte FSizeCodes[8]={6,4,0,7,1,5,2,3};

/*-------------------------------------------------------------------------*/

#ifdef DEBSTRCNT
static int strcnt=0;

        static int strccmp(const char *s1, const char *s2)
BEGIN
   strcnt++;
   return strcmp(s1,s2);
END

#undef Memo
#define strcmp(s1,s2) strccmp(s1,s2)
#define Memo(s1) (strccmp(s1,OpPart)==0)
#endif

/*-------------------------------------------------------------------------*/
/* Unterroutinen */

#define CopyAdrVals(Dest) memcpy(Dest,AdrVals,AdrCnt)

        static void ACheckCPU(CPUVar MinCPU)
BEGIN
   if (MomCPU<MinCPU) 
    BEGIN
     WrError(1505); AdrNum=0; AdrCnt=0;
    END
END

        static void CheckCPU(CPUVar Level)
BEGIN
   if (MomCPU<Level) 
    BEGIN
     WrError(1500); CodeLen=0;
    END
END

        static void Check020(void)
BEGIN
   if (MomCPU!=CPU68020)
    BEGIN
     WrError(1500); CodeLen=0;
    END
END

        static void Check32(void)
BEGIN
   if ((MomCPU<CPU68332) OR (MomCPU>CPU68360))
    BEGIN
     WrError(1500); CodeLen=0;
    END
END

        static void CheckSup(void)
BEGIN
   if (NOT SupAllowed) WrError(50);
END

        static Boolean CheckColdSize(void)
BEGIN
   if ((OpSize>2) OR ((MomCPU==CPUCOLD) AND (OpSize<2)))
    BEGIN
     WrError(1130); return False;
    END
   else return True;
END

/*-------------------------------------------------------------------------*/
/* Adressparser */

typedef enum {PC,AReg,Index,indir,Disp,None} CompType;
typedef struct 
         {
          String Name;
          CompType Art;
          Word ANummer,INummer;
          Boolean Long;
          Word Scale;
          Word Size;
          LongInt Wert;
         } AdrComp;

        static Boolean ValReg(char Ch)
BEGIN
   return ((Ch>='0') AND (Ch<='7'));
END

        static Boolean CodeReg(char *s, Word *Erg)
BEGIN
   Boolean Result = True;

   if (strlen(s) != 2) Result = False;
   else if (strcasecmp(s,"SP") == 0) *Erg = 15;
   else if (ValReg(s[1]))
    if (toupper(*s) == 'D') *Erg = s[1] - '0';
    else if (toupper(*s) == 'A') *Erg = s[1] - '0' + 8;
    else Result = False;
   else Result = False;

   return Result;
END

        static Boolean CodeRegPair(char *Asc, Word *Erg1, Word *Erg2)
BEGIN
   if (strlen(Asc)!=5) return False;
   if (toupper(*Asc)!='D') return False;
   if (Asc[2]!=':') return False;
   if (toupper(Asc[3])!='D') return False;
   if (NOT (ValReg(Asc[1]) AND ValReg(Asc[4]))) return False;

   *Erg1=Asc[1]-'0'; *Erg2=Asc[4]-'0';

   return True;
END

        static Boolean CodeIndRegPair(char *Asc, Word *Erg1, Word *Erg2)
BEGIN
   if (strlen(Asc)!=9) return False;
   if (*Asc!='(') return False;
   if ((toupper(Asc[1])!='D') AND (toupper(Asc[1])!='A')) return False;
   if (Asc[3]!=')') return False;
   if (Asc[4]!=':') return False;
   if (Asc[5]!='(') return False;
   if ((toupper(Asc[6])!='D') AND (toupper(Asc[6])!='A')) return False;
   if (Asc[8]!=')') return False;
   if (NOT (ValReg(Asc[2]) AND ValReg(Asc[7]))) return False;

   *Erg1=Asc[2]-'0'; if (toupper(Asc[1])=='A') *Erg1+=8;
   *Erg2=Asc[7]-'0'; if (toupper(Asc[6])=='A') *Erg2+=8;

   return True;
END

        static Boolean CodeCache(char *Asc, Word *Erg)
BEGIN
   if (strcasecmp(Asc,"IC")==0) *Erg=2;
   else if (strcasecmp(Asc,"DC")==0) *Erg=1;
   else if (strcasecmp(Asc,"IC/DC")==0) *Erg=3;
   else if (strcasecmp(Asc,"DC/IC")==0) *Erg=3;
   else return False;
   return True;
END     

        static Boolean DecodeCtrlReg(char *Asc, Word *Erg)
BEGIN
   Byte z;
   String Asc_N;
   CtReg *Reg;

   strmaxcpy(Asc_N,Asc,255); NLS_UpString(Asc_N); Asc=Asc_N;

   for (z=0,Reg=CtRegs; z<CtRegCnt; z++,Reg++)
    if (strcmp(Reg->Name,Asc)==0) break;
   if (z>=CtRegCnt) return False;
   if ((MomCPU<Reg->FirstCPU) OR (MomCPU>Reg->LastCPU)) return False;
   *Erg=Reg->Code; return True;
END

        static int FindICondition(char *Name)
BEGIN
   int i;

   for (i=0; i<CondCnt; i++)
    if (strcmp(Name,CondNams[i])==0) break;

   return i;
END

        static Boolean OneField(char *Asc, Word *Erg, Boolean Ab1)
BEGIN
   Boolean ValOK;

   if ((strlen(Asc)==2) AND (toupper(*Asc)=='D') AND (ValReg(Asc[1])))
    BEGIN
     *Erg=0x20+(Asc[1]-'0'); return True;
    END
   else
    BEGIN
     *Erg=EvalIntExpression(Asc,Int8,&ValOK);
     if ((Ab1) AND (*Erg==32)) *Erg=0;
     return ((ValOK) AND (*Erg<32));
    END
END

        static Boolean SplitBitField(char *Arg, Word *Erg)
BEGIN
   char *p;
   Word OfsVal;
   String Desc;

   p = strchr(Arg, '{');
   if (p == Nil) return False;
   *p = '\0'; strcpy(Desc, p + 1);
   if ((!*Desc) || (Desc[strlen(Desc) - 1] != '}')) return False;
   Desc[strlen(Desc) - 1] = '\0';

   p = strchr(Desc, ':');
   if (p == Nil) return False;
   *p = '\0';
   if (NOT OneField(Desc, &OfsVal, False)) return False;
   if (NOT OneField(p + 1, Erg, True)) return False;
   *Erg += OfsVal << 6;
   return True;
END

	static Boolean SplitSize(char *Asc, ShortInt *DispLen)
BEGIN
   ShortInt NewLen = (-1);
   int l = strlen(Asc);

   if ((l > 2) AND (Asc[l - 2] == '.')) 
    BEGIN
     switch (toupper(Asc[l - 1]))
      BEGIN
       case 'B': NewLen = 0; break;
       case 'W': NewLen = 1; break;
       case 'L': NewLen = 2; break;
       default:
        WrError(1130); return False;
      END
     if ((*DispLen != -1) AND (*DispLen != NewLen))
      BEGIN
       WrError(1131); return False;
      END
     *DispLen = NewLen;
     Asc[l - 2] = '\0';
    END

   return True;
END

        static Boolean ClassComp(AdrComp *C)
BEGIN
   char sh[10];

   if ((*C->Name=='[') AND (C->Name[strlen(C->Name)-1]==']'))
    BEGIN
     C->Art=indir; return True;
    END

   if (strcasecmp(C->Name,"PC")==0) 
    BEGIN
     C->Art=PC; return True;
    END

   sh[0]=C->Name[0]; sh[1]=C->Name[1]; sh[2]='\0';
   if (CodeReg(sh,&C->ANummer)) 
    BEGIN
     if ((C->ANummer>7) AND (strlen(C->Name)==2)) 
      BEGIN
       C->Art=AReg; C->ANummer-=8; return True;
      END
     else
      BEGIN
       if ((strlen(C->Name)>3) AND (C->Name[2]=='.')) 
        BEGIN
         switch (toupper(C->Name[3]))
          BEGIN
           case 'L': C->Long=True; break;
           case 'W': C->Long=False; break;
           default: return False;
          END
         strcpy(C->Name+2,C->Name+4);
        END
       else C->Long=(MomCPU==CPUCOLD);
       if ((strlen(C->Name)>3) AND (C->Name[2]=='*')) 
        BEGIN
         switch (C->Name[3])
          BEGIN
           case '1': C->Scale=0; break;
           case '2': C->Scale=1; break;
           case '4': C->Scale=2; break;
           case '8': if (MomCPU==CPUCOLD) return False;
                     C->Scale=3; break;
           default: return False;
          END
         strcpy(C->Name+2,C->Name+4);
        END
       else C->Scale=0;
       C->INummer=C->ANummer; C->Art=Index; return True;
      END
    END

   C->Art=Disp;
   if (C->Name[strlen(C->Name)-2]=='.') 
    BEGIN
     switch (toupper(C->Name[strlen(C->Name)-1]))
      BEGIN
       case 'L': C->Size=2; break;
       case 'W': C->Size=1; break;
       default: return False;
      END
     C->Name[strlen(C->Name)-2]='\0';
    END
   else C->Size=1;
   C->Art=Disp;
   return True;
END

        static void ChkAdr(Word Erl)
BEGIN
   if ((Erl & Masks[AdrNum])==0) 
    BEGIN
     WrError(1350); AdrNum=0;
    END
END

        static Boolean IsShortAdr(LongInt Adr)
BEGIN
   Word WHi = (Adr >> 16) & 0xffff,
        WLo =  Adr        & 0xffff;
 
   return ((WHi == 0     ) AND (WLo <= 0x7fff))
       OR ((WHi == 0xffff) AND (WLo >= 0x8000));
END

        static Boolean IsDisp8(LongInt Disp)
BEGIN
   return ((Disp>=-128) AND (Disp<=127));
END

        static Boolean IsDisp16(LongInt Disp)
BEGIN
   if (Disp<-32768) return False;
   if (Disp>32767) return False;
   return True;
END

        static void ChkEven(LongInt Adr)
BEGIN
   if ((MomCPU<=CPU68340) AND (Odd(Adr))) WrError(180);
END

        static void DecodeAdr(char *Asc_O, Word Erl)
BEGIN
   Byte l,i;
   char *p;
   Word rerg;
   Byte lklamm,rklamm,lastrklamm;
   Boolean doklamm;

   AdrComp AdrComps[3],OneComp;
   Byte CompCnt;
   String OutDisp;
   ShortInt OutDispLen;
   Boolean PreInd;

#ifdef HAS64
   QuadInt QVal;
#endif
   LongInt HVal;
   Integer HVal16;
   ShortInt HVal8;
   Double DVal;
   Boolean ValOK;
   Word SwapField[6];
   String Asc;
   char CReg[10];

   strmaxcpy(Asc,Asc_O,255); KillBlanks(Asc);
   l=strlen(Asc);
   AdrNum=0; AdrCnt=0;

   /* immediate : */

   if (*Asc=='#') 
    BEGIN
     strcpy(Asc,Asc+1);
     AdrNum=11;
     AdrMode=0x3c;
     switch (OpSize)
      BEGIN
       case 0:
        AdrCnt=2;
        HVal8=EvalIntExpression(Asc,Int8,&ValOK);
        if (ValOK) AdrVals[0]=(Word)((Byte) HVal8);
        break;
       case 1:
        AdrCnt=2;
        HVal16=EvalIntExpression(Asc,Int16,&ValOK);
        if (ValOK) AdrVals[0]=(Word) HVal16;
        break;
       case 2:
        AdrCnt=4;
        HVal=EvalIntExpression(Asc,Int32,&ValOK);
        if (ValOK) 
         BEGIN
          AdrVals[0]=HVal >> 16;
          AdrVals[1]=HVal & 0xffff;
         END
        break;
#ifdef HAS64
       case 3:
        AdrCnt=8;
        QVal=EvalIntExpression(Asc,Int64,&ValOK);
        if (ValOK) 
         BEGIN
          AdrVals[0]=(QVal >> 48) & 0xffff;
          AdrVals[1]=(QVal >> 32) & 0xffff;
          AdrVals[2]=(QVal >> 16) & 0xffff;
          AdrVals[3]=(QVal      ) & 0xffff;
         END
        break;
#endif
       case 4:
        AdrCnt=4;
        DVal=EvalFloatExpression(Asc,Float32,&ValOK);
        if (ValOK) 
         BEGIN
          Double_2_ieee4(DVal,(Byte *) SwapField,BigEndian);
          if (BigEndian) DWSwap((Byte *) SwapField,4);
          AdrVals[0]=SwapField[1];
          AdrVals[1]=SwapField[0];
         END
        break;
       case 5:
        AdrCnt=8;
        DVal=EvalFloatExpression(Asc,Float64,&ValOK);
        if (ValOK) 
         BEGIN
          Double_2_ieee8(DVal,(Byte *) SwapField,BigEndian);
          if (BigEndian) QWSwap((Byte *) SwapField,8);
          AdrVals[0]=SwapField[3];
          AdrVals[1]=SwapField[2];
          AdrVals[2]=SwapField[1];
          AdrVals[3]=SwapField[0];
         END
        break;
       case 6:
        AdrCnt=12;
        DVal=EvalFloatExpression(Asc,Float64,&ValOK);
        if (ValOK) 
         BEGIN
          Double_2_ieee10(DVal,(Byte *) SwapField,False);
          if (BigEndian) WSwap((Byte *) SwapField,10);
          AdrVals[0]=SwapField[4];
          AdrVals[1]=0;
          AdrVals[2]=SwapField[3];
          AdrVals[3]=SwapField[2];
          AdrVals[4]=SwapField[1];
          AdrVals[5]=SwapField[0];
         END
        break;
       case 7:
        AdrCnt=12;
        DVal=EvalFloatExpression(Asc,Float64,&ValOK);
        if (ValOK) 
         BEGIN
          ConvertMotoFloatDec(DVal,SwapField);
          AdrVals[0]=SwapField[5];
          AdrVals[1]=SwapField[4];
          AdrVals[2]=SwapField[3];
          AdrVals[3]=SwapField[2];
          AdrVals[4]=SwapField[1];
          AdrVals[5]=SwapField[0];
         END
        break;
       case 8: /* special arg 1..8 */
        AdrCnt = 2;
        FirstPassUnknown = False;
        HVal8 = EvalIntExpression(Asc, UInt4, &ValOK);
        if (ValOK)
         BEGIN
          if (FirstPassUnknown)
           HVal8 = 1;
          ValOK = ChkRange(HVal8, 1, 8);
         END
        if (ValOK) AdrVals[0]=(Word)((Byte) HVal8);
        break;
      END
     ChkAdr(Erl); return;
    END

   /* CPU-Register direkt: */

   if (CodeReg(Asc,&AdrMode)) 
    BEGIN
     AdrCnt=0; AdrNum=(AdrMode >> 3)+1; ChkAdr(Erl); return;
    END

   /* Gleitkommaregister direkt: */

   if (strncasecmp(Asc,"FP",2)==0) 
    BEGIN
     if ((strlen(Asc)==3) AND (ValReg(Asc[2]))) 
      BEGIN
       AdrMode=Asc[2]-'0'; AdrCnt=0; AdrNum=12; ChkAdr(Erl); return;
      END;
     if (strcasecmp(Asc,"FPCR")==0) 
      BEGIN
       AdrMode=4; AdrNum=13; ChkAdr(Erl); return;
      END
     if (strcasecmp(Asc,"FPSR")==0) 
      BEGIN
       AdrMode=2; AdrNum=13; ChkAdr(Erl); return;
      END
     if (strcasecmp(Asc,"FPIAR")==0) 
      BEGIN
       AdrMode=1; AdrNum=13; ChkAdr(Erl); return;
      END
    END

   /* Adressregister indirekt mit Predekrement: */

   if ((l==5) AND (*Asc=='-') AND (Asc[1]=='(') AND (Asc[4]==')'))
    BEGIN
     strcpy(CReg,Asc+2); CReg[2]='\0';
     if (CodeReg(CReg,&rerg)) 
      if (rerg>7) 
       BEGIN
        AdrMode=rerg+24; AdrCnt=0; AdrNum=5; ChkAdr(Erl); return;
       END
    END

   /* Adressregister indirekt mit Postinkrement */

   if ((l==5) AND (*Asc=='(') AND (Asc[3]==')') AND (Asc[4]=='+')) 
    BEGIN
     strcpy(CReg,Asc+1); CReg[2]='\0';
     if (CodeReg(CReg,&rerg)) 
      if (rerg>7) 
       BEGIN
        AdrMode=rerg+16; AdrCnt=0; AdrNum=4; ChkAdr(Erl); return;
       END
    END

   /* Unterscheidung direkt<->indirekt: */

   lklamm=0; rklamm=0; lastrklamm=0; doklamm=True;
   for (p=Asc; *p!='\0'; p++)
    BEGIN
     if (*p=='[') doklamm=False;
     if (*p==']') doklamm=True;
     if (doklamm) 
      BEGIN
       if (*p=='(') lklamm++;
       else if (*p==')') 
        BEGIN
         rklamm++; lastrklamm=p-Asc;
        END
      END
    END

   if ((lklamm==1) AND (rklamm==1) AND (lastrklamm==strlen(Asc)-1)) 
    BEGIN

     /* aeusseres Displacement abspalten, Klammern loeschen: */

     p = strchr(Asc,'('); *p = '\0';
     strmaxcpy(OutDisp, Asc, 255); strcpy(Asc, p + 1);
     OutDispLen = (-1);
     if (NOT SplitSize(OutDisp, &OutDispLen))
      return;
     Asc[strlen(Asc) - 1] = '\0';

     /* in Komponenten zerteilen: */

     CompCnt = 0;
     do
      BEGIN
       doklamm = True;
       p = Asc;
       do
        BEGIN
         if (*p == '[') doklamm = False;
         else if (*p == ']') doklamm = True;
         p++;
        END
       while (((NOT doklamm) OR (*p != ',')) AND (*p != '\0'));
       if (*p == '\0')
        BEGIN
         strcpy(AdrComps[CompCnt].Name, Asc); *Asc = '\0';
        END
       else
        BEGIN
         *p = '\0'; strcpy(AdrComps[CompCnt].Name, Asc); strcpy(Asc, p + 1);
        END
       if (NOT ClassComp(AdrComps + CompCnt)) 
        BEGIN
         WrError(1350); return;
        END

       /* when the base register is already occupied, we have to move a
          second address register to the index position */

       if ((CompCnt==1) AND (AdrComps[CompCnt].Art==AReg)) 
        BEGIN
         AdrComps[CompCnt].Art=Index; 
         AdrComps[CompCnt].INummer=AdrComps[CompCnt].ANummer+8; 
         AdrComps[CompCnt].Long=False; 
         AdrComps[CompCnt].Scale=0;
         CompCnt++;
        END

       /* a displacement found inside (...), but outside [...].  Explicit
          sizes must be consistent, implicitly checked by SplitSize(). */

       else if (AdrComps[CompCnt].Art == Disp)
        BEGIN
         strcpy(OutDisp, AdrComps[CompCnt].Name);

         if (NOT SplitSize(OutDisp, &OutDispLen))
          return;
        END

       /* no second index */

       else if ((AdrComps[CompCnt].Art!=Index) AND (CompCnt!=0))
        BEGIN
         WrError(1350); return;
        END

       else 
        CompCnt++;
      END
     while (*Asc!='\0');
     if ((CompCnt>2) OR ((AdrComps[0].Art==Index) AND (CompCnt!=1))) 
      BEGIN
       WrError(1350); return;
      END

     /* 1. Variante (An....), d(An....) */

     if (AdrComps[0].Art==AReg) 
      BEGIN

       /* 1.1. Variante (An), d(An) */

       if (CompCnt==1) 
        BEGIN

         /* 1.1.1. Variante (An) */

         if ((*OutDisp=='\0') AND ((Madri & Erl)!=0)) 
          BEGIN
           AdrMode=0x10+AdrComps[0].ANummer; AdrNum=3; AdrCnt=0;
           ChkAdr(Erl); return;
          END

         /* 1.1.2. Variante d(An) */

         else
          BEGIN
           if ((OutDispLen < 0) OR (OutDispLen >= 2))
            HVal = EvalIntExpression(OutDisp, SInt32, &ValOK);
           else
            HVal = EvalIntExpression(OutDisp, SInt16, &ValOK);
           if (NOT ValOK) 
            BEGIN
             WrError(1350); return;
            END
           if ((ValOK) AND (HVal==0) AND ((Madri & Erl)!=0) AND (OutDispLen==-1)) 
            BEGIN
             AdrMode=0x10+AdrComps[0].ANummer; AdrNum=3; AdrCnt=0;
             ChkAdr(Erl); return;
            END
           if (OutDispLen == -1)
            OutDispLen = (IsDisp16(HVal)) ? 1 : 2;
           switch (OutDispLen)
            BEGIN
             case 1:                   /* d16(An) */
              AdrMode=0x28+AdrComps[0].ANummer; AdrNum=6;
              AdrCnt=2; AdrVals[0]=HVal&0xffff;
              ChkAdr(Erl); return;
             case 2:                   /* d32(An) */
              AdrMode=0x30+AdrComps[0].ANummer; AdrNum=7;
              AdrCnt=6; AdrVals[0]=0x0170;
              AdrVals[1]=(HVal >> 16) & 0xffff; AdrVals[2]=HVal & 0xffff;
              ACheckCPU(CPU68332); ChkAdr(Erl); return;
            END
          END
        END

       /* 1.2. Variante d(An,Xi) */

       else
        BEGIN
         AdrVals[0] = (AdrComps[1].INummer << 12) + (Ord(AdrComps[1].Long) << 11) + (AdrComps[1].Scale << 9);
         AdrMode = 0x30 + AdrComps[0].ANummer;
         HVal = EvalIntExpression(OutDisp, Int32, &ValOK);
         if (ValOK)
          switch (OutDispLen)
           BEGIN
            case 0:
             if (NOT IsDisp8(HVal))
              BEGIN
               WrError(1320); ValOK = FALSE;
              END
            break;
            case 1:
             if (NOT IsDisp16(HVal))
              BEGIN
               WrError(1320); ValOK = FALSE;
              END
            break;
           END
         if (ValOK)
          BEGIN
           if (OutDispLen == -1)
            BEGIN
             if (IsDisp8(HVal)) OutDispLen = 0;
             else if (IsDisp16(HVal)) OutDispLen = 1;
             else OutDispLen = 2;
            END
           switch (OutDispLen)
            BEGIN
             case 0:
              AdrNum=7; AdrCnt=2; AdrVals[0]+=(HVal & 0xff);
              if (AdrComps[1].Scale!=0) ACheckCPU(CPUCOLD);
              ChkAdr(Erl); return;
             case 1:
              AdrNum=7; AdrCnt=4;
              AdrVals[0]+=0x120; AdrVals[1]=HVal & 0xffff;
              ACheckCPU(CPU68332);
              ChkAdr(Erl); return;
             case 2:
              AdrNum=7; AdrCnt=6; AdrVals[0]+=0x130;
              AdrVals[1]=HVal >> 16; AdrVals[2]=HVal & 0xffff;
              ACheckCPU(CPU68332);
              ChkAdr(Erl); return;
            END
          END
        END
      END

     /* 2. Variante d(PC....) */

     else if (AdrComps[0].Art==PC) 
      BEGIN

       /* 2.1. Variante d(PC) */

       if (CompCnt==1) 
        BEGIN
         HVal=EvalIntExpression(OutDisp,Int32,&ValOK)-(EProgCounter()+RelPos);
         if (NOT ValOK) 
          BEGIN
           WrError(1350); return;
          END
         if (OutDispLen < 0)
           OutDispLen = (IsDisp16(HVal)) ? 1 : 2;
         switch (OutDispLen)
          BEGIN
           case 1:
            AdrMode=0x3a;
            if (NOT IsDisp16(HVal)) 
             BEGIN
              WrError(1330); return;
             END
            AdrNum=8; AdrCnt=2; AdrVals[0]=HVal & 0xffff;
            ChkAdr(Erl); return;
           case 2:
            AdrMode=0x3b;
            AdrNum=9; AdrCnt=6; AdrVals[0]=0x170;
            AdrVals[1]=HVal >> 16; AdrVals[2]=HVal & 0xffff;
            ACheckCPU(CPU68332); ChkAdr(Erl); return;
          END
        END

       /* 2.2. Variante d(PC,Xi) */

       else
        BEGIN
         AdrVals[0]=(AdrComps[1].INummer << 12)+(Ord(AdrComps[1].Long) << 11)+(AdrComps[1].Scale << 9);
         HVal=EvalIntExpression(OutDisp,Int32,&ValOK)-(EProgCounter()+RelPos);
         if (NOT ValOK) 
          BEGIN
           WrError(1350); return;
          END;
         if (OutDispLen < 0)
          BEGIN
           if (IsDisp8(HVal)) OutDispLen = 0;
           else if (IsDisp16(HVal)) OutDispLen = 1;
           else OutDispLen = 2;
          END
         AdrMode=0x3b;
         switch (OutDispLen)
          BEGIN
           case 0:
            if (NOT IsDisp8(HVal)) 
             BEGIN
              WrError(1330); return;
             END
            AdrVals[0]+=(HVal & 0xff); AdrCnt=2; AdrNum=9;
            if (AdrComps[1].Scale!=0) ACheckCPU(CPUCOLD);
            ChkAdr(Erl); return;
           case 1:
            if (NOT IsDisp16(HVal)) 
             BEGIN
              WrError(1330); return;
             END
            AdrVals[0]+=0x120; AdrCnt=4; AdrNum=9;
            AdrVals[1]=HVal & 0xffff;
            ACheckCPU(CPU68332);
            ChkAdr(Erl); return;
           case 2:
            AdrVals[0]+=0x130; AdrCnt=6; AdrNum=9;
            AdrVals[1]=HVal >> 16; AdrVals[2]=HVal & 0xffff;
            ACheckCPU(CPU68332);
            ChkAdr(Erl); return;
          END
        END
      END

     /* 3. Variante (Xi), d(Xi) */

     else if (AdrComps[0].Art==Index) 
      BEGIN
       AdrVals[0]=(AdrComps[0].INummer << 12)+(Ord(AdrComps[0].Long) << 11)+(AdrComps[0].Scale << 9)+0x180;
       AdrMode=0x30;
       if (*OutDisp=='\0') 
        BEGIN
         AdrVals[0]=AdrVals[0]+0x0010; AdrCnt=2;
         AdrNum=7; ACheckCPU(CPU68332); ChkAdr(Erl); return;
        END
       else
        BEGIN
         if (OutDispLen != 1)
          HVal = EvalIntExpression(OutDisp,SInt32,&ValOK);
         else
          HVal = EvalIntExpression(OutDisp,SInt16,&ValOK);
         if (ValOK)
          BEGIN
           if (OutDispLen == -1)
            BEGIN
             if (IsDisp16(HVal)) OutDispLen = 1;
             else OutDispLen = 2;
            END
           switch (OutDispLen)
            BEGIN
             case 0:
             case 1:
              AdrVals[0]=AdrVals[0]+0x0020; AdrVals[1]=HVal & 0xffff;
              AdrNum=7; AdrCnt=4; ACheckCPU(CPU68332);
              ChkAdr(Erl); return;
             case 2:
              AdrVals[0]=AdrVals[0]+0x0030; AdrNum=7; AdrCnt=6;
              AdrVals[1]=HVal >> 16; AdrVals[2]=HVal & 0xffff;
              ACheckCPU(CPU68332);
              ChkAdr(Erl); return;
            END
          END
        END
      END

     /* 4. Variante indirekt: */

     else if (AdrComps[0].Art==indir) 
      BEGIN

       /* erst ab 68020 erlaubt */

       if (MomCPU<CPU68020) 
        BEGIN
         WrError(1505); return;
        END

       /* Unterscheidung Vor- <---> Nachindizierung: */

       if (CompCnt==2) 
        BEGIN
         PreInd=False;
         AdrComps[2]=AdrComps[1];
        END
       else
        BEGIN
         PreInd=True;
         AdrComps[2].Art=None;
        END

       /* indirektes Argument herauskopieren: */

       strcpy(Asc,AdrComps[0].Name+1);
       Asc[strlen(Asc)-1]='\0';

       /* Felder loeschen: */

       for (i=0; i<2; AdrComps[i++].Art=None);

       /* indirekten Ausdruck auseinanderfieseln: */

       do
        BEGIN

         /* abschneiden & klassifizieren: */

         p=strchr(Asc,',');
         if (p==Nil)
          BEGIN
           strcpy(OneComp.Name,Asc); *Asc='\0';
          END
         else
          BEGIN
           *p='\0'; strcpy(OneComp.Name,Asc); strcpy(Asc,p+1);
          END
         if (NOT ClassComp(&OneComp)) 
          BEGIN
           WrError(1350); return;
          END

        /* passend einsortieren: */

         if ((AdrComps[1].Art!=None) AND (OneComp.Art==AReg)) 
          BEGIN
           OneComp.Art=Index; OneComp.INummer=OneComp.ANummer+8; 
           OneComp.Long=False; OneComp.Scale=0;
          END
         switch (OneComp.Art)
          BEGIN
           case Disp  : i=0; break;
           case AReg  :
           case PC    : i=1; break;
           case Index : i=2; break;
           default    : i=(-1);
          END
         if (AdrComps[i].Art!=None) 
          BEGIN
           WrError(1350); return;
          END
         else AdrComps[i]=OneComp;
        END
       while (*Asc!='\0');

       /* Vor-oder Nachindizierung? */

       AdrVals[0]=0x100+(Ord(PreInd) << 2);

       /* Indexregister eintragen */

       if (AdrComps[2].Art==None) AdrVals[0]+=0x40;
       else AdrVals[0]+=(AdrComps[2].INummer << 12)+(Ord(AdrComps[2].Long) << 11)+(AdrComps[2].Scale << 9);

       /* 4.1 Variante d([...PC...]...) */

       if (AdrComps[1].Art==PC) 
        BEGIN
         if (AdrComps[0].Art==None) HVal=0;
         else HVal=EvalIntExpression(AdrComps[0].Name,Int32,&ValOK);
         HVal-=EProgCounter()+RelPos;
         if (NOT ValOK) return;
         AdrMode=0x3b;
         switch (AdrComps[0].Size)
          BEGIN
           case 1:
            if (NOT IsDisp16(HVal)) 
             BEGIN
              WrError(1330); return;
             END
            AdrVals[1]=HVal & 0xffff; AdrVals[0]+=0x20; AdrNum=7; AdrCnt=4;
            break;
           case 2:
            AdrVals[1]=HVal >> 16; AdrVals[2]=HVal & 0xffff;
            AdrVals[0]+=0x30; AdrNum=7; AdrCnt=6;
            break;
          END
        END

       /* 4.2 Variante d([...An...]...) */

       else
        BEGIN
         if (AdrComps[1].Art==None)
          BEGIN
           AdrMode=0x30; AdrVals[0]+=0x80;
          END
         else AdrMode=0x30+AdrComps[1].ANummer;

         if (AdrComps[0].Art==None) 
          BEGIN
           AdrNum=7; AdrCnt=2; AdrVals[0]+=0x10;
          END
         else switch (AdrComps[0].Size)
          BEGIN
           case 1:
            HVal16=EvalIntExpression(AdrComps[0].Name,Int16,&ValOK);
            if (NOT ValOK) return;
            AdrNum=7; AdrVals[1]=HVal16; AdrCnt=4; AdrVals[0]+=0x20;
            break;
           case 2:
            HVal=EvalIntExpression(AdrComps[0].Name,Int32,&ValOK);
            if (NOT ValOK) return;
            AdrNum=7; AdrCnt=6; AdrVals[0]+=0x30;
            AdrVals[1]=HVal >> 16; AdrVals[2]=HVal & 0xffff;
            break;
          END
        END

       /* aeusseres Displacement: */

       if (OutDispLen == 1)
        HVal = EvalIntExpression(OutDisp, SInt16, &ValOK);
       else
        HVal = EvalIntExpression(OutDisp, SInt32, &ValOK);
       if (NOT ValOK)
        BEGIN
         AdrNum=0; AdrCnt=0; return;
        END;
       if (OutDispLen == -1)
        BEGIN
         if (IsDisp16(HVal)) OutDispLen = 1;
         else OutDispLen = 2;
        END
       if (*OutDisp=='\0')
        BEGIN
         AdrVals[0]++; ChkAdr(Erl); return;
        END
       else switch (OutDispLen)
        BEGIN
         case 0:
         case 1:
          AdrVals[AdrCnt >> 1]=HVal & 0xffff; AdrCnt+=2; AdrVals[0]+=2;
          break;
         case 2:
          AdrVals[(AdrCnt >> 1)  ]=HVal >> 16;
          AdrVals[(AdrCnt >> 1)+1]=HVal & 0xffff;
          AdrCnt+=4; AdrVals[0]+=3;
          break;
        END

       ChkAdr(Erl); return;

      END

    END

   /* absolut: */

   else
    BEGIN
     AdrCnt=0;
     if (strcasecmp(Asc+strlen(Asc)-2,".W")==0) 
      BEGIN
       AdrCnt=2; Asc[strlen(Asc)-2]='\0';
      END
     else if (strcasecmp(Asc+strlen(Asc)-2,".L")==0) 
      BEGIN
       AdrCnt=4; Asc[strlen(Asc)-2]='\0';
      END

     FirstPassUnknown=False;
     HVal=EvalIntExpression(Asc,Int32,&ValOK);
     if ((NOT FirstPassUnknown) AND (OpSize>0)) ChkEven(HVal);
     HVal16=HVal;

     if (ValOK) 
      BEGIN
       if (AdrCnt==0) AdrCnt=(IsShortAdr(HVal))?2:4; 
       AdrNum=10;

       if (AdrCnt==2) 
        BEGIN
         if (NOT IsShortAdr(HVal)) 
          BEGIN
           WrError(1340); AdrNum=0;
          END
         else
          BEGIN
           AdrMode=0x38; AdrVals[0]=HVal16;
          END
        END
       else
        BEGIN
         AdrMode=0x39; AdrVals[0]=HVal >> 16; AdrVals[1]=HVal & 0xffff;
        END
      END
    END

   ChkAdr(Erl);
END

        static Byte OneReg(char *Asc)
BEGIN
   if (strlen(Asc)!=2) return 16;
   if ((toupper(*Asc)!='A') AND (toupper(*Asc)!='D')) return 16;
   if (NOT ValReg(Asc[1])) return 16;
   return Asc[1]-'0'+((toupper(*Asc)=='D')?0:8);
END

        static Boolean DecodeRegList(char *Asc_o, Word *Erg)
BEGIN
   static Word Masks[16]={1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768};
   Byte h,h2,z;
   char *p;
   String Asc,s;

   *Erg=0; strmaxcpy(Asc,Asc_o,255);
   do
    BEGIN
     p=strchr(Asc,'/');
     if (p==Nil)
      BEGIN
       strcpy(s,Asc); *Asc='\0';
      END
     else
      BEGIN
       *p='\0'; strcpy(s,Asc); strcpy(Asc,p+1);
      END
     if (*Asc=='/') strcpy(Asc,Asc+1);
     p=strchr(s,'-');
     if (p==Nil) 
      BEGIN
       if ((h=OneReg(s))==16) return False;
       *Erg|=Masks[h];
      END
     else
      BEGIN
       *p='\0';
       if ((h = OneReg(s)) == 16) return False;
       if ((h2 = OneReg(p + 1)) == 16) return False;
       if (h <= h2)
        for (z = h; z <= h2; *Erg |= Masks[z++]);
       else
        BEGIN
         for (z = h; z <= 15; *Erg |= Masks[z++]);
         for (z = 0; z <= h2; *Erg |= Masks[z++]);
        END
      END
    END    
   while (*Asc!='\0');
   return True;
END

/*-------------------------------------------------------------------------*/
/* Dekodierroutinen: Integer-Einheit */

/* 0=MOVE 1=MOVEA */

        static void DecodeMOVE(Word Index)
BEGIN
   int z;
   UNUSED(Index);

   if (ArgCnt!=2) WrError(1110);
   else if (strcasecmp(ArgStr[1],"USP")==0) 
    BEGIN
     if ((*AttrPart!='\0') AND (OpSize!=2)) WrError(1130);
     else if (MomCPU==CPUCOLD) WrError(1500);
     else
      BEGIN
       DecodeAdr(ArgStr[2],Madr);
       if (AdrNum!=0) 
        BEGIN
         CodeLen=2; WAsmCode[0]=0x4e68 | (AdrMode & 7); CheckSup();
        END
      END
    END
   else if (strcasecmp(ArgStr[2],"USP")==0) 
    BEGIN
     if ((*AttrPart!='\0') AND (OpSize!=2)) WrError(1130);
     else if (MomCPU==CPUCOLD) WrError(1500);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Madr);
       if (AdrNum!=0) 
        BEGIN
         CodeLen=2; WAsmCode[0]=0x4e60 | (AdrMode & 7); CheckSup();
        END
      END
    END
   else if (strcasecmp(ArgStr[1],"SR")==0) 
    BEGIN
     if (OpSize!=1) WrError(1130);
     else
      BEGIN
       if (MomCPU==CPUCOLD) DecodeAdr(ArgStr[2],Mdata);
       else DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
       if (AdrNum!=0) 
        BEGIN
         CodeLen=2+AdrCnt; WAsmCode[0]=0x40c0 | AdrMode;
         CopyAdrVals(WAsmCode+1);
         if (MomCPU>=CPU68010) CheckSup();
        END
      END
    END
   else if (strcasecmp(ArgStr[1],"CCR")==0) 
    BEGIN
     if ((*AttrPart!='\0') AND (OpSize>1)) WrError(1130);
     else
      BEGIN
       OpSize=0;
       if (MomCPU==CPUCOLD) DecodeAdr(ArgStr[2],Mdata);
       else DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
       if (AdrNum!=0) 
        BEGIN
         CodeLen=2+AdrCnt; WAsmCode[0]=0x42c0 | AdrMode;
         CopyAdrVals(WAsmCode+1); CheckCPU(CPU68010);
        END
      END
    END
   else if (strcasecmp(ArgStr[2],"SR")==0) 
    BEGIN
     if (OpSize!=1) WrError(1130);
     else
      BEGIN
       if (MomCPU==CPUCOLD) DecodeAdr(ArgStr[1],Mdata+Mimm);
       else DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
       if (AdrNum!=0) 
        BEGIN
         CodeLen=2+AdrCnt; WAsmCode[0]=0x46c0 | AdrMode;
         CopyAdrVals(WAsmCode+1); CheckSup();
        END
      END
    END
   else if (strcasecmp(ArgStr[2],"CCR")==0) 
    BEGIN
     if ((*AttrPart!='\0') AND (OpSize>1)) WrError(1130);
     else
      BEGIN
       OpSize=0;
       if (MomCPU==CPUCOLD) DecodeAdr(ArgStr[1],Mdata+Mimm);
       else DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
       if (AdrNum!=0) 
        BEGIN
         CodeLen=2+AdrCnt; WAsmCode[0]=0x44c0 | AdrMode;
         CopyAdrVals(WAsmCode+1);
        END
      END
    END
   else
    BEGIN
     if (OpSize>2) WrError(1130);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
       if (AdrNum!=0) 
        BEGIN
         z=AdrCnt; CodeLen=2+z; CopyAdrVals(WAsmCode+1);
         if (OpSize==0) WAsmCode[0]=0x1000;
         else if (OpSize==1) WAsmCode[0]=0x3000;
         else WAsmCode[0]=0x2000;
         WAsmCode[0]|=AdrMode;
         DecodeAdr(ArgStr[2],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
         if (AdrMode!=0)
          BEGIN
           if ((MomCPU==CPUCOLD) AND (z>0) AND (AdrCnt>0)) WrError(1350);
           else
            BEGIN
             AdrMode=((AdrMode & 7) << 3) | (AdrMode >> 3);
             WAsmCode[0]|=AdrMode << 6;
             CopyAdrVals(WAsmCode+(CodeLen >> 1));
             CodeLen+=AdrCnt;
            END
          END
        END
      END
    END
END

        static void DecodeLEA(Word Index)
BEGIN
   UNUSED(Index);

   if ((*AttrPart!='\0') AND (OpSize!=2)) WrError(1130);
   else if (ArgCnt!=2) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[2],Madr);
     if (AdrNum!=0) 
      BEGIN
       OpSize=0;
       WAsmCode[0]=0x41c0 | ((AdrMode & 7) << 9);
       DecodeAdr(ArgStr[1],Madri+Mdadri+Maix+Mpc+Mpcidx+Mabs);
       if (AdrNum!=0) 
        BEGIN
         WAsmCode[0]|=AdrMode; CodeLen=2+AdrCnt;
         CopyAdrVals(WAsmCode+1);
        END
      END
    END
END

/* 0=ASR 1=ASL 2=LSR 3=LSL 4=ROXR 5=ROXL 6=ROR 7=ROL */

        static void DecodeShift(Word Index)
BEGIN
   Boolean ValOK;
   Byte HVal8;
   Word LFlag=(Index>>2), Op=Index&3;

   if (ArgCnt==1) 
    BEGIN
     strcpy(ArgStr[2],ArgStr[1]); strcpy(ArgStr[1],"#1");
     ArgCnt=2;
    END
   if (ArgCnt!=2) WrError(1110);
   else if ((*OpPart=='R') AND (MomCPU==CPUCOLD)) WrError(1500);
   else
    BEGIN
     DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
     if (AdrNum==1) 
      BEGIN
       if (CheckColdSize())
        BEGIN
         WAsmCode[0]=0xe000 | AdrMode | (Op << 3) | (OpSize << 6) | (LFlag << 8);
         OpSize = 8;
         DecodeAdr(ArgStr[1],Mdata+Mimm);
         if ((AdrNum==1) OR ((AdrNum==11) AND (Lo(AdrVals[0])>=1) AND (Lo(AdrVals[0])<=8)))
          BEGIN
           CodeLen=2;
           WAsmCode[0] |= (AdrNum==1) ? 0x20|(AdrMode<<9) : ((AdrVals[0] & 7) << 9);
          END
         else WrError(1380);
        END
      END
     else if (AdrNum!=0) 
      BEGIN
       if (MomCPU==CPUCOLD) WrError(1350);
       else
        BEGIN
         if (OpSize!=1) WrError(1130);
         else
          BEGIN
           WAsmCode[0]=0xe0c0 | AdrMode | (Op << 9) | (LFlag << 8);
           CopyAdrVals(WAsmCode+1);
           if (*ArgStr[1]=='#') strcpy(ArgStr[1],ArgStr[1]+1);
           HVal8=EvalIntExpression(ArgStr[1],Int8,&ValOK);
           if ((ValOK) AND (HVal8==1)) CodeLen=2+AdrCnt;
           else WrError(1390);
          END
        END
      END
    END
END

/* ADDQ=0 SUBQ=1 */

        static void DecodeADDQSUBQ(Word Index)
BEGIN
   Byte HVal8;
   Boolean ValOK;

   if (CheckColdSize())
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[2],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
       if (AdrNum!=0) 
        BEGIN
         WAsmCode[0]=0x5000 | AdrMode | (OpSize << 6) | (Index << 8);
         CopyAdrVals(WAsmCode+1);
         if (*ArgStr[1]=='#') strcpy(ArgStr[1],ArgStr[1]+1);
         FirstPassUnknown=False;
         HVal8=EvalIntExpression(ArgStr[1],UInt4,&ValOK);
         if (FirstPassUnknown) HVal8=1;
         if ((ValOK) AND (HVal8>=1) AND (HVal8<=8)) 
          BEGIN
           CodeLen=2+AdrCnt;
           WAsmCode[0]|=(((Word) HVal8 & 7) << 9);
          END
         else WrError(1390);
        END
      END
    END
END

/* 0=SUBX 1=ADDX */

        static void DecodeADDXSUBX(Word Index)
BEGIN
   if (CheckColdSize())
    BEGIN
     if (ArgCnt != 2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1], Mdata + Mpre);
       if (AdrNum != 0)
        BEGIN
         WAsmCode[0] = 0x9100 | (OpSize << 6) | (AdrMode & 7) | (Index << 14);
         if (AdrNum == 5) WAsmCode[0] |= 8;
         DecodeAdr(ArgStr[2], Masks[AdrNum]);
         if (AdrNum != 0) 
          BEGIN
           CodeLen = 2;
           WAsmCode[0] |= (AdrMode & 7) << 9;
          END
        END
      END
    END
END

        static void DecodeCMPM(Word Index)
BEGIN
   UNUSED(Index);

   if (OpSize>2) WrError(1130);
   else if (ArgCnt!=2) WrError(1110);
   else if (MomCPU==CPUCOLD) WrError(1500);
   else
    BEGIN
     DecodeAdr(ArgStr[1],Mpost);
     if (AdrNum==4) 
      BEGIN
       WAsmCode[0]=0xb108 | (OpSize << 6) | (AdrMode & 7);
       DecodeAdr(ArgStr[2],Mpost);
       if (AdrNum==4) 
        BEGIN
         WAsmCode[0]|=(AdrMode & 7) << 9;
         CodeLen=2;
        END
      END
    END
END

/* 0=SUB 1=CMP 2=ADD +4=..I +8=..A */

        static void DecodeADDSUBCMP(Word Index)
BEGIN
   Word Op = Index & 3, Reg;
   char Variant = toupper(OpPart[strlen(OpPart) - 1]);
   Word PCMask;
   
   /* since CMP only reads operands, PC-relative addressing is also
      allowed for the second operand */

   PCMask = (toupper(*OpPart) == 'C') ? (Mpc+Mpcidx) : 0;
   if (CheckColdSize())
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[2],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mabs+PCMask);
       if (AdrNum==2)        /* ADDA ? */
        if (OpSize==0) WrError(1130);
       else
        BEGIN
         WAsmCode[0]=0x90c0 | ((AdrMode & 7) << 9) | (Op << 13);
         if (OpSize==2) WAsmCode[0]|=0x100;
         DecodeAdr(ArgStr[1],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
         if (AdrNum!=0) 
          BEGIN
           WAsmCode[0]|=AdrMode; CodeLen=2+AdrCnt;
           CopyAdrVals(WAsmCode+1);
          END
        END
       else if (AdrNum==1)      /* ADD <EA>,Dn ? */
        BEGIN
         WAsmCode[0]=0x9000 | (OpSize << 6) | ((Reg = AdrMode) << 9) | (Op << 13);
         if (Variant == 'I')
          DecodeAdr(ArgStr[1],Mimm);
         else
          DecodeAdr(ArgStr[1],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
         if (AdrNum!=0)
          BEGIN
           if ((AdrNum == 11) AND (Variant == 'I'))
            BEGIN
             if (Op == 1) Op = 8;
             WAsmCode[0]=0x400 | (OpSize << 6) | (Op << 8) | Reg;
            END
           else
            WAsmCode[0]|=AdrMode;
           CopyAdrVals(WAsmCode + 1);
           CodeLen = 2 + AdrCnt;
          END
        END
       else
        BEGIN
         DecodeAdr(ArgStr[1],Mdata+Mimm);
         if (AdrNum==11)        /* ADDI ? */
          BEGIN
           if (Op==1) Op=8;
           WAsmCode[0]=0x400 | (OpSize << 6) | (Op << 8);
           CodeLen=2+AdrCnt;
           CopyAdrVals(WAsmCode+1);
           if (MomCPU==CPUCOLD) DecodeAdr(ArgStr[2],Mdata);
           else DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+PCMask+Mabs);
           if (AdrNum!=0) 
            BEGIN
             WAsmCode[0]|=AdrMode;
             CopyAdrVals(WAsmCode+(CodeLen >> 1));
             CodeLen+=AdrCnt;
            END
           else CodeLen=0;
          END
         else if (AdrNum!=0)    /* ADD Dn,<EA> ? */
          BEGIN
           if (Op==1) WrError(1420);
           else
            BEGIN
             WAsmCode[0]=0x9100 | (OpSize << 6) | (AdrMode << 9) | (Op << 13);
             DecodeAdr(ArgStr[2],Madri+Mpost+Mpre+Mdadri+Maix+Mabs+PCMask);
             if (AdrNum!=0) 
              BEGIN
               CodeLen=2+AdrCnt; CopyAdrVals(WAsmCode+1);
               WAsmCode[0]|=AdrMode;
              END
            END
          END
        END
      END
    END
END

/* 0=OR 1=AND +4=..I */

        static void DecodeANDOR(Word Index)
BEGIN
   Word Op =Index & 3, Reg;
   char Variant = toupper(OpPart[strlen(OpPart) - 1]);

   if (ArgCnt!=2) WrError(1110);
   else if (CheckColdSize())
    BEGIN
     if ((strcasecmp(ArgStr[2],"CCR")!=0) AND (strcasecmp(ArgStr[2],"SR")!=0)) 
      DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
     if (strcasecmp(ArgStr[2],"CCR")==0)     /* AND #...,CCR */
      BEGIN
       if ((*AttrPart!='\0') AND (OpSize!=0)) WrError(1130);
       else if ((MomCPU==CPU68008) OR (MomCPU==CPUCOLD)) WrError(1500);
       else
        BEGIN
         WAsmCode[0] = 0x003c | (Op << 9);
         OpSize=0; DecodeAdr(ArgStr[1],Mimm);
         if (AdrNum!=0) 
          BEGIN
           CodeLen=4; WAsmCode[1]=AdrVals[0];
          END
        END
      END
     else if (strcasecmp(ArgStr[2],"SR")==0) /* AND #...,SR */
      BEGIN
       if ((*AttrPart!='\0') AND (OpSize!=1)) WrError(1130);
       else if ((MomCPU==CPU68008) OR (MomCPU==CPUCOLD)) WrError(1500);
       else
        BEGIN
         WAsmCode[0] = 0x007c | (Op << 9);
         OpSize=1; DecodeAdr(ArgStr[1],Mimm);
         if (AdrNum!=0) 
          BEGIN
           CodeLen=4; WAsmCode[1]=AdrVals[0]; CheckSup();
          END
        END
      END
     else if (AdrNum==1)                 /* AND <EA>,Dn */
      BEGIN
       WAsmCode[0]=0x8000 | (OpSize << 6) | ((Reg = AdrMode) << 9) | (Op << 14);
       if (Variant == 'I')
        DecodeAdr(ArgStr[1],Mimm);
       else
        DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
       if (AdrNum!=0) 
        BEGIN
         if ((AdrNum == 11) AND (Variant == 'I'))
           WAsmCode[0] = (OpSize << 6) | (Op << 9) | Reg;
         else
          WAsmCode[0] |= AdrMode;
         CodeLen = 2 + AdrCnt;
         CopyAdrVals(WAsmCode + 1);
        END
      END
     else if (AdrNum!=0)                 /* AND ...,<EA> */
      BEGIN
       DecodeAdr(ArgStr[1],Mdata+Mimm);
       if (AdrNum==11)                   /* AND #..,<EA> */
        BEGIN
         WAsmCode[0]=(OpSize << 6) | (Op << 9);
         CodeLen=2+AdrCnt;
         CopyAdrVals(WAsmCode+1);
         DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
         if (AdrNum!=0)
          BEGIN
           WAsmCode[0]|=AdrMode;
           CopyAdrVals(WAsmCode+(CodeLen >> 1));
           CodeLen+=AdrCnt;
          END
         else CodeLen=0;
        END
       else if (AdrNum!=0)               /* AND Dn,<EA> ? */
        BEGIN
         WAsmCode[0]=0x8100 | (OpSize << 6) | (AdrMode << 9) | (Op << 14);
         DecodeAdr(ArgStr[2],Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
         if (AdrNum!=0)
          BEGIN
           CodeLen=2+AdrCnt; CopyAdrVals(WAsmCode+1);
           WAsmCode[0]|=AdrMode;
          END
        END
      END
    END
END

/* 0=EOR 4=EORI */

        static void DecodeEOR(Word Index)
BEGIN
   UNUSED(Index);

   if (ArgCnt!=2) WrError(1110);
   else if (strcasecmp(ArgStr[2],"CCR")==0) 
    BEGIN
     if ((*AttrPart!='\0') AND (OpSize!=0)) WrError(1130);
     else if (MomCPU==CPUCOLD) WrError(1500);
     else
      BEGIN
       WAsmCode[0]=0xa3c; OpSize=0;
       DecodeAdr(ArgStr[1],Mimm);
       if (AdrNum!=0)
        BEGIN
         CodeLen=4; WAsmCode[1]=AdrVals[0];
        END
      END
    END
   else if (strcasecmp(ArgStr[2],"SR")==0) 
    BEGIN
     if (OpSize!=1) WrError(1130);
     else if (MomCPU==CPUCOLD) WrError(1500);
     else
      BEGIN
       WAsmCode[0]=0xa7c;
       DecodeAdr(ArgStr[1],Mimm);
       if (AdrNum!=0) 
        BEGIN
         CodeLen=4; WAsmCode[1]=AdrVals[0]; CheckSup();
         CheckCPU(CPU68000);
        END
      END
    END
   else if (CheckColdSize())
    BEGIN
     DecodeAdr(ArgStr[1],Mdata+Mimm);
     if (AdrNum==1) 
      BEGIN
       WAsmCode[0]=0xb100 | (AdrMode << 9) | (OpSize << 6);
       DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
       if (AdrNum!=0)
        BEGIN
         CodeLen=2+AdrCnt; CopyAdrVals(WAsmCode+1);
         WAsmCode[0]|=AdrMode;
        END
      END
     else if (AdrNum==11) 
      BEGIN
       WAsmCode[0]=0x0a00 | (OpSize << 6);
       CopyAdrVals(WAsmCode+1); CodeLen=2+AdrCnt;
       if (MomCPU==CPUCOLD) DecodeAdr(ArgStr[2],Mdata);
       else DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
       if (AdrNum!=0)
        BEGIN
         CopyAdrVals(WAsmCode+(CodeLen >> 1));
         CodeLen+=AdrCnt;
         WAsmCode[0]|=AdrMode;
        END
       else CodeLen=0;
      END
    END
END

        static void DecodePEA(Word Index)
BEGIN
   UNUSED(Index);

   if ((*AttrPart!='\0') AND (OpSize!=2)) WrError(1100);
   else if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     OpSize=0;
     DecodeAdr(ArgStr[1],Madri+Mdadri+Maix+Mpc+Mpcidx+Mabs);
     if (AdrNum!=0) 
      BEGIN
       CodeLen=2+AdrCnt;
       WAsmCode[0]=0x4840 | AdrMode;
       CopyAdrVals(WAsmCode+1);
      END
    END
END

/* 0=CLR 1=TST */

        static void DecodeCLRTST(Word Index)
BEGIN
   Word w1;

   if (OpSize>2) WrError(1130);
   else if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     w1=Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs;
     if ((Index==1) AND (OpSize>0) AND (MomCPU>=CPU68332))
      w1+=Madr+Mpc+Mpcidx+Mimm;
     DecodeAdr(ArgStr[1],w1);
     if (AdrNum!=0) 
      BEGIN
       CodeLen=2+AdrCnt;
       WAsmCode[0]=0x4200 | (Index << 11) | (OpSize << 6) | AdrMode;
       CopyAdrVals(WAsmCode+1);
      END
    END
END

/* 0=JSR 1=JMP */

        static void DecodeJSRJMP(Word Index)
BEGIN
   if (*AttrPart!='\0') WrError(1130);
   else if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1],Madri+Mdadri+Maix+Mpc+Mpcidx+Mabs);
     if (AdrNum!=0) 
      BEGIN
       CodeLen=2+AdrCnt;
       WAsmCode[0]=0x4e80 | (Index << 6) | AdrMode;
       CopyAdrVals(WAsmCode+1);
      END
    END
END

/* 0=TAS 1=NBCD */

        static void DecodeNBCDTAS(Word Index)
BEGIN
   if ((*AttrPart!='\0') AND (OpSize!=0)) WrError(1130);
   else if (ArgCnt!=1) WrError(1110);
   else if (MomCPU==CPUCOLD) WrError(1500);
   else
    BEGIN
     OpSize=0;
     DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
     if (AdrNum!=0) 
      BEGIN
       CodeLen=2+AdrCnt;
       WAsmCode[0]=(Index==1) ? 0x4800 : 0x4ac0;
       WAsmCode[0]|=AdrMode;
       CopyAdrVals(WAsmCode+1);
      END
    END
END

/* 0=NEGX 2=NEG 3=NOT */

        static void DecodeNEGNOT(Word Index)
BEGIN
   if (ArgCnt!=1) WrError(1110);
   else if (CheckColdSize())
    BEGIN
     if (MomCPU==CPUCOLD) DecodeAdr(ArgStr[1],Mdata);
     else DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
     if (AdrNum!=0)
      BEGIN
       CodeLen=2+AdrCnt;
       WAsmCode[0]=0x4000 | (Index << 9) | (OpSize << 6) | AdrMode;
       CopyAdrVals(WAsmCode+1);
      END
    END
END

        static void DecodeSWAP(Word Index)
BEGIN
   UNUSED(Index);

   if ((*AttrPart!='\0') AND (OpSize!=2)) WrError(1130);
   else if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1],Mdata);
     if (AdrNum!=0) 
      BEGIN
       CodeLen=2; WAsmCode[0]=0x4840 | AdrMode;
      END
    END
END

        static void DecodeUNLK(Word Index) 
BEGIN
   UNUSED(Index);

   if (*AttrPart!='\0') WrError(1130);
   else if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1],Madr);
     if (AdrNum!=0) 
      BEGIN
       CodeLen=2; WAsmCode[0]=0x4e58 | AdrMode;
      END
    END
END

        static void DecodeEXT(Word Index) 
BEGIN
   UNUSED(Index);

   if (ArgCnt!=1) WrError(1110);
   else if ((OpSize==0) OR (OpSize>2)) WrError(1130);
   else
    BEGIN
     DecodeAdr(ArgStr[1],Mdata);
     if (AdrNum==1) 
      BEGIN
       WAsmCode[0]=0x4880 | AdrMode | (((Word)OpSize-1) << 6);
       CodeLen=2;
      END
    END
END

        static void DecodeWDDATA(Word Index)
BEGIN
   UNUSED(Index);

   if (ArgCnt!=1) WrError(1110);
   else if (MomCPU!=CPUCOLD) WrError(1500);
   else if (OpSize>2) WrError(1130);
   else
    BEGIN
     DecodeAdr(ArgStr[1],Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
     if (AdrNum!=0)
      BEGIN
       WAsmCode[0]=0xf400+(OpSize << 6)+AdrMode;
       CopyAdrVals(WAsmCode+1); CodeLen=2+AdrCnt;
       CheckSup();
      END
    END
END

        static void DecodeWDEBUG(Word Index)
BEGIN
   UNUSED(Index);

   if (ArgCnt!=1) WrError(1110);
   else if (MomCPU!=CPUCOLD) WrError(1500);
   else if (CheckColdSize())
    BEGIN
     DecodeAdr(ArgStr[1],Madri+Mdadri);
     if (AdrNum!=0)
      BEGIN
       WAsmCode[0]=0xfbc0+AdrMode; WAsmCode[1]=0x0003;
       CopyAdrVals(WAsmCode+2); CodeLen=4+AdrCnt;
       CheckSup();
      END
    END
END

        static void DecodeFixed(Word Index)
BEGIN
   FixedOrder *FixedZ=FixedOrders+Index;

   if (*AttrPart!='\0') WrError(1100);
   else if (ArgCnt!=0) WrError(1110);
   else if ((FixedZ->CPUMask &(1 << (MomCPU-CPU68008)))==0) WrError(1500);
   else
    BEGIN
     CodeLen=2; WAsmCode[0]=FixedZ->Code;
     if (FixedZ->MustSup) CheckSup();
    END
END

        static void DecodeMOVEM(Word Index)
BEGIN
   int z;
   UNUSED(Index);

   if (ArgCnt != 2) WrError(1110);
   else if ((OpSize < 1) OR (OpSize > 2)) WrError(1130);
   else if ((MomCPU == CPUCOLD) AND (OpSize == 1)) WrError(1130);
   else
    BEGIN
     RelPos = 4;
     if (DecodeRegList(ArgStr[2], WAsmCode + 1))
      BEGIN
       if (MomCPU == CPUCOLD) DecodeAdr(ArgStr[1], Madri + Mdadri);
       else DecodeAdr(ArgStr[1], Madri + Mpost + Mdadri + Maix + Mpc + Mpcidx + Mabs);
       if (AdrNum != 0)
        BEGIN
         WAsmCode[0] = 0x4c80 | AdrMode | ((OpSize-1) << 6);
         CodeLen=4 + AdrCnt; CopyAdrVals(WAsmCode + 2);
        END
      END
     else if (DecodeRegList(ArgStr[1], WAsmCode + 1)) 
      BEGIN
       if (MomCPU == CPUCOLD) DecodeAdr(ArgStr[2], Madri + Mdadri);
       else DecodeAdr(ArgStr[2], Madri + Mpre + Mdadri + Maix + Mabs);
       if (AdrNum != 0) 
        BEGIN
         WAsmCode[0] = 0x4880 | AdrMode | ((OpSize-1) << 6);
         CodeLen = 4 + AdrCnt; CopyAdrVals(WAsmCode + 2);
         if (AdrNum == 5) 
          BEGIN
           WAsmCode[9] = WAsmCode[1]; WAsmCode[1] = 0;
           for (z = 0; z < 16; z++)
            BEGIN
             WAsmCode[1] = WAsmCode[1] << 1;
             if ((WAsmCode[9] & 1) == 1) WAsmCode[1]++;
             WAsmCode[9] = WAsmCode[9] >> 1;
            END
          END
        END
      END
     else WrError(1410);
    END
END

        static void DecodeMOVEQ(Word Index)
BEGIN
    UNUSED(Index);

    if (ArgCnt!=2) WrError(1110);
    else if ((*AttrPart!='\0') AND (OpSize!=2)) WrError(1130);
    else
     BEGIN
      DecodeAdr(ArgStr[2],Mdata);
      if (AdrNum!=0)
       BEGIN
        WAsmCode[0]=0x7000 | (AdrMode << 9);
        OpSize=0;
        DecodeAdr(ArgStr[1],Mimm);
        if (AdrNum!=0) 
         BEGIN
          CodeLen=2; WAsmCode[0]|=AdrVals[0];
         END
       END
     END
END

        static void DecodeSTOP(Word Index)
BEGIN
   Word HVal;
   Boolean ValOK;
   UNUSED(Index);

   if (*AttrPart!='\0') WrError(1100);
   else if (ArgCnt!=1) WrError(1110);
   else if (*ArgStr[1]!='#') WrError(1120);
   else
    BEGIN
     HVal=EvalIntExpression(ArgStr[1]+1,Int16,&ValOK);
     if (ValOK) 
      BEGIN
       CodeLen=4; WAsmCode[0]=0x4e72; WAsmCode[1]=HVal; CheckSup();
      END
    END
END

        static void DecodeLPSTOP(Word Index) 
BEGIN
   Word HVal;
   Boolean ValOK;
   UNUSED(Index);

   if (*AttrPart!='\0') WrError(1100);
   else if (ArgCnt!=1) WrError(1110);
   else if (*ArgStr[1]!='#') WrError(1120);
   else
    BEGIN
     HVal=EvalIntExpression(ArgStr[1]+1,Int16,&ValOK);
     if (ValOK) 
      BEGIN
       CodeLen=6;
       WAsmCode[0]=0xf800;
       WAsmCode[1]=0x01c0;
       WAsmCode[2]=HVal;
       CheckSup(); Check32();
      END
    END
END

        static void DecodeTRAP(Word Index) 
BEGIN
   Byte HVal8;
   Boolean ValOK;
   UNUSED(Index);

   if (*AttrPart!='\0') WrError(1100);
   else if (ArgCnt!=1) WrError(1110);
   else if (*ArgStr[1]!='#') WrError(1120);
   else
    BEGIN
     HVal8=EvalIntExpression(ArgStr[1]+1,Int4,&ValOK);
     if (ValOK) 
      BEGIN
       CodeLen=2; WAsmCode[0]=0x4e40+(HVal8 & 15);
      END
    END
END

        static void DecodeBKPT(Word Index) 
BEGIN
   Byte HVal8;
   Boolean ValOK;
   UNUSED(Index);

   if (*AttrPart!='\0') WrError(1100);
   else if (ArgCnt!=1) WrError(1110);
   else if (MomCPU==CPUCOLD) WrError(1500);
   else if (*ArgStr[1]!='#') WrError(1120);
   else
    BEGIN
     HVal8=EvalIntExpression(ArgStr[1]+1,UInt3,&ValOK);
     if (ValOK) 
      BEGIN
       CodeLen=2; WAsmCode[0]=0x4848+(HVal8 & 7);
       CheckCPU(CPU68010);
      END
    END
   UNUSED(Index);
END

        static void DecodeRTD(Word Index) 
BEGIN
   Word HVal;
   Boolean ValOK;
   UNUSED(Index);

   if (*AttrPart!='\0') WrError(1100);
   else if (ArgCnt!=1) WrError(1110);
   else if (MomCPU==CPUCOLD) WrError(1500);
   else if (*ArgStr[1]!='#') WrError(1120);
   else
    BEGIN
     HVal=EvalIntExpression(ArgStr[1]+1,Int16,&ValOK);
     if (ValOK) 
      BEGIN
       CodeLen=4; WAsmCode[0]=0x4e74; WAsmCode[1]=HVal;
       CheckCPU(CPU68010);
      END
    END
END

        static void DecodeEXG(Word Index) 
BEGIN
   Word HReg;
   UNUSED(Index);

   if ((*AttrPart!='\0') AND (OpSize!=2)) WrError(1130);
   else if (ArgCnt!=2) WrError(1110);
   else if (MomCPU==CPUCOLD) WrError(1500);
   else
    BEGIN
     DecodeAdr(ArgStr[1],Mdata+Madr);
     if (AdrNum==1)
      BEGIN
       WAsmCode[0]=0xc100 | (AdrMode << 9);
       DecodeAdr(ArgStr[2],Mdata+Madr);
       if (AdrNum==1)
        BEGIN
         WAsmCode[0]|=0x40 | AdrMode; CodeLen=2;
        END
       else if (AdrNum==2)
        BEGIN
         WAsmCode[0]|=0x88 | (AdrMode & 7); CodeLen=2;
        END
      END
     else if (AdrNum==2)
      BEGIN
       WAsmCode[0]=0xc100;
       HReg = AdrMode & 7;
       DecodeAdr(ArgStr[2],Mdata+Madr);
       if (AdrNum==1)
        BEGIN
         WAsmCode[0]|=0x88 | (AdrMode << 9) | HReg; CodeLen=2;
        END
       else
        BEGIN
         WAsmCode[0]|=0x48 | (HReg << 9) | (AdrMode & 7); CodeLen=2;
        END
      END
    END
END

        static void DecodeMOVE16(Word Index)
BEGIN
   Word z,z2,w1,w2;
   UNUSED(Index);
   
   if (*AttrPart!='\0') WrError(1100);
   else if (ArgCnt!=2) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1],Mpost+Madri+Mabs);
     if (AdrNum!=0)
      BEGIN
       w1=AdrNum; z=AdrMode & 7;
       if ((w1==10) AND (AdrCnt==2))
        BEGIN
         AdrVals[1]=AdrVals[0];
         AdrVals[0]=0-(AdrVals[1] >> 15);
        END
       DecodeAdr(ArgStr[2],Mpost+Madri+Mabs);
       if (AdrNum!=0)
        BEGIN
         w2=AdrNum; z2=AdrMode & 7;
         if ((w2==10) AND (AdrCnt==2))
          BEGIN
           AdrVals[1]=AdrVals[0];
           AdrVals[0]=0-(AdrVals[1] >> 15);
          END
         if ((w1==4) AND (w2==4))
          BEGIN
           WAsmCode[0]=0xf620+z;
           WAsmCode[1]=0x8000+(z2 << 12);
           CodeLen=4;
          END
         else
          BEGIN
           WAsmCode[1]=AdrVals[0]; WAsmCode[2]=AdrVals[1];
           CodeLen=6;
           if ((w1==4) AND (w2==10)) WAsmCode[0]=0xf600+z;
           else if ((w1==10) AND (w2==4)) WAsmCode[0]=0xf608+z2;
           else if ((w1==3) AND (w2==10)) WAsmCode[0]=0xf610+z;
           else if ((w1==10) AND (w2==3)) WAsmCode[0]=0xf618+z2;
           else
            BEGIN
             WrError(1350); CodeLen=0;
            END
          END
         if (CodeLen>0) CheckCPU(CPU68040);
        END
      END
    END
END     

        static void DecodeCacheAll(Word Index)
BEGIN
   Word w1;
   
   if (*AttrPart!='\0') WrError(1100);
   else if (ArgCnt!=1) WrError(1110);
   else if (NOT CodeCache(ArgStr[1],&w1)) WrXError(1440,ArgStr[1]);
   else
    BEGIN
     WAsmCode[0]=0xf418+(w1 << 6)+(Index << 5);
     CodeLen=2;
     CheckCPU(CPU68040); CheckSup();
    END   
END     

        static void DecodeCache(Word Index)
BEGIN
   Word w1;

   if (*AttrPart!='\0') WrError(1100);
   else if (ArgCnt!=2) WrError(1110);
   else if (NOT CodeCache(ArgStr[1],&w1)) WrXError(1440,ArgStr[1]);
   else
    BEGIN
     DecodeAdr(ArgStr[2],Madri);
     if (AdrNum!=0)
      BEGIN
       WAsmCode[0]=0xf400+(w1 << 6)+(Index << 3)+(AdrMode & 7);
       CodeLen=2;
       CheckCPU(CPU68040); CheckSup();
      END
    END   
END     

        static void DecodeDIVL(Word Index) 
BEGIN
   Word w1,w2;

   if (*AttrPart=='\0') OpSize=2;
   if (ArgCnt!=2) WrError(1110);
   else if (OpSize!=2) WrError(1130);
   else if (NOT CodeRegPair(ArgStr[2],&w1,&w2)) WrXError(1760, ArgStr[2]);
   else
    BEGIN
     RelPos=4;
     WAsmCode[1]=w1|(w2 << 12)|(Index << 11);
     DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
     if (AdrNum!=0)
      BEGIN
       WAsmCode[0]=0x4c40+AdrMode;
       CopyAdrVals(WAsmCode+2); CodeLen=4+AdrCnt;
       CheckCPU(CPU68332);
      END
    END
END

        static void DecodeASBCD(Word Index)
BEGIN
   if ((OpSize!=0) AND (*AttrPart!='\0')) WrError(1130);
   else if (ArgCnt!=2) WrError(1110);
   else if (MomCPU==CPUCOLD) WrError(1500);
   else
    BEGIN
     OpSize=0;
     DecodeAdr(ArgStr[1],Mdata+Mpre);
     if (AdrNum!=0)
      BEGIN
       WAsmCode[0]=0x8100 | (AdrMode & 7) | (Index << 14);
       if (AdrNum==5) WAsmCode[0]|=8;
       DecodeAdr(ArgStr[2],Masks[AdrNum]);
       if (AdrNum!=0)
        BEGIN
         CodeLen=2;
         WAsmCode[0]|=(AdrMode & 7) << 9;
        END
      END
    END
END

        static void DecodeCHK(Word Index) 
BEGIN
   UNUSED(Index);

   if ((OpSize != 1) AND (OpSize != 2)) WrError(1130);
   else if (ArgCnt != 2) WrError(1110);
   else if (MomCPU == CPUCOLD) WrError(1500);
   else
    BEGIN
     DecodeAdr(ArgStr[1], Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
     if (AdrNum != 0)
      BEGIN
       WAsmCode[0] = 0x4000 | AdrMode | ((4 - OpSize) << 7);
       CodeLen = 2 + AdrCnt;
       CopyAdrVals(WAsmCode + 1);
       DecodeAdr(ArgStr[2], Mdata);
       if (AdrNum == 1) WAsmCode[0] |= WAsmCode[0] | (AdrMode << 9);
       else CodeLen = 0;
      END
    END
END

        static void DecodeLINK(Word Index) 
BEGIN
   UNUSED(Index);

   if ((*AttrPart=='\0') AND (MomCPU==CPUCOLD)) OpSize=1;
   if ((OpSize<1) OR (OpSize>2)) WrError(1130);
   else if ((OpSize==2) AND (MomCPU<CPU68332)) WrError(1500);
   else if (ArgCnt!=2) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1],Madr);
     if (AdrNum!=0)
      BEGIN
       WAsmCode[0]=(OpSize==1) ? 0x4e50 : 0x4808;
       WAsmCode[0]+=AdrMode & 7;
       DecodeAdr(ArgStr[2],Mimm);
       if (AdrNum==11)
        BEGIN
         CodeLen=2+AdrCnt; memcpy(WAsmCode+1,AdrVals,AdrCnt);
        END
      END
    END
END

        static void DecodeMOVEP(Word Index) 
BEGIN
   UNUSED(Index);

   if ((OpSize==0) OR (OpSize>2)) WrError(1130);
   else if (ArgCnt!=2) WrError(1110);
   else if (MomCPU==CPUCOLD) WrError(1500);
   else
    BEGIN
     DecodeAdr(ArgStr[1],Mdata+Mdadri);
     if (AdrNum==1)
      BEGIN
       WAsmCode[0]=0x188 | ((OpSize-1) << 6) | (AdrMode << 9);
       DecodeAdr(ArgStr[2],Mdadri);
       if (AdrNum==6)
        BEGIN
         WAsmCode[0]|=AdrMode & 7;
         CodeLen=4; WAsmCode[1]=AdrVals[0];
        END
      END
     else if (AdrNum==6)
      BEGIN
       WAsmCode[0]=0x108 | ((OpSize-1) << 6) | (AdrMode & 7);
       WAsmCode[1]=AdrVals[0];
       DecodeAdr(ArgStr[2],Mdata);
       if (AdrNum==1)
        BEGIN
         WAsmCode[0]|=(AdrMode & 7) << 9;
         CodeLen=4;
        END
      END
    END
END

        static void DecodeMOVEC(Word Index) 
BEGIN
   UNUSED(Index);

   if ((*AttrPart!='\0') AND (OpSize!=2)) WrError(1130);
   else if (ArgCnt!=2) WrError(1110);
    BEGIN
     if (DecodeCtrlReg(ArgStr[1],WAsmCode+1)) 
      BEGIN
       DecodeAdr(ArgStr[2],Mdata+Madr);
       if (AdrNum!=0)
        BEGIN
         CodeLen=4; WAsmCode[0]=0x4e7a;
         WAsmCode[1]|=AdrMode << 12; CheckSup();
        END
      END
     else if (DecodeCtrlReg(ArgStr[2],WAsmCode+1)) 
      BEGIN
       DecodeAdr(ArgStr[1],Mdata+Madr);
       if (AdrNum!=0) 
        BEGIN
         CodeLen=4; WAsmCode[0]=0x4e7b;
         WAsmCode[1]|=AdrMode << 12; CheckSup();
        END
      END
     else WrError(1440);
    END
END

        static void DecodeMOVES(Word Index) 
BEGIN
   UNUSED(Index);

   if (ArgCnt!=2) WrError(1110);
   else if (OpSize>2) WrError(1130);
   else if (MomCPU==CPUCOLD) WrError(1500);
   else
    BEGIN
     DecodeAdr(ArgStr[1],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
     if ((AdrNum==1) OR (AdrNum==2)) 
      BEGIN
       WAsmCode[1]=0x800 | (AdrMode << 12);
       DecodeAdr(ArgStr[2],Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
       if (AdrNum!=0)
        BEGIN
         WAsmCode[0]=0xe00 | AdrMode | (OpSize << 6); CodeLen=4+AdrCnt;
         CopyAdrVals(WAsmCode+2); CheckSup();
         CheckCPU(CPU68010);
        END
      END
     else if (AdrNum!=0)
      BEGIN
       WAsmCode[0]=0xe00 | AdrMode | (OpSize << 6);
       CodeLen=4+AdrCnt; CopyAdrVals(WAsmCode+2);
       DecodeAdr(ArgStr[2],Mdata+Madr);
       if (AdrNum!=0)
        BEGIN
         WAsmCode[1]=AdrMode << 12;
         CheckSup();
         CheckCPU(CPU68010);
        END
       else CodeLen=0;
      END
    END
END

        static void DecodeCALLM(Word Index) 
BEGIN
   UNUSED(Index);

   if (*AttrPart!='\0') WrError(1130);
   else if (ArgCnt!=2) WrError(1110);
   else
    BEGIN
     OpSize=0;
     DecodeAdr(ArgStr[1],Mimm);
     if (AdrNum!=0)
      BEGIN
       WAsmCode[1]=AdrVals[0]; RelPos=4;
       DecodeAdr(ArgStr[2],Madri+Mdadri+Maix+Mpc+Mpcidx+Mabs);
       if (AdrNum!=0)
        BEGIN
         WAsmCode[0]=0x06c0+AdrMode;
         CopyAdrVals(WAsmCode+2); CodeLen=4+AdrCnt;
         CheckCPU(CPU68020); Check020();
        END
      END
    END
END

        static void DecodeCAS(Word Index) 
BEGIN
   UNUSED(Index);

   if (OpSize>2) WrError(1130);
   else if (ArgCnt!=3) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1],Mdata);
     if (AdrNum!=0)
      BEGIN
       WAsmCode[1]=AdrMode;
       DecodeAdr(ArgStr[2],Mdata);
       if (AdrNum!=0)
        BEGIN
         RelPos=4;
         WAsmCode[1]+=(((Word)AdrMode) << 6);
         DecodeAdr(ArgStr[3],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
         if (AdrNum!=0)
          BEGIN
           WAsmCode[0]=0x08c0+AdrMode+(((Word)OpSize+1) << 9);
           CopyAdrVals(WAsmCode+2); CodeLen=4+AdrCnt;
           CheckCPU(CPU68020);
          END
        END
      END
    END
END

        static void DecodeCAS2(Word Index) 
BEGIN
   Word w1,w2;
   UNUSED(Index);

   if ((OpSize!=1) AND (OpSize!=2)) WrError(1130);
   else if (ArgCnt!=3) WrError(1110);
   else if (NOT CodeRegPair(ArgStr[1],WAsmCode+1,WAsmCode+2)) WrXError(1760, ArgStr[1]);
   else if (NOT CodeRegPair(ArgStr[2],&w1,&w2)) WrXError(1760, ArgStr[2]);
   else
    BEGIN
     WAsmCode[1]+=(w1 << 6);
     WAsmCode[2]+=(w2 << 6);
     if (NOT CodeIndRegPair(ArgStr[3],&w1,&w2)) WrXError(1760, ArgStr[3]);
     else
      BEGIN
       WAsmCode[1]+=(w1 << 12);
       WAsmCode[2]+=(w2 << 12);
       WAsmCode[0]=0x0cfc+(((Word)OpSize-1) << 9);
       CodeLen=6;
       CheckCPU(CPU68020);
      END
    END
END

        static void DecodeCMPCHK2(Word Index)
BEGIN
   if (OpSize>2) WrError(1130);
   else if (ArgCnt!=2) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[2],Mdata+Madr);
     if (AdrNum!=0)
      BEGIN
       RelPos=4;
       WAsmCode[1]=(((Word)AdrMode) << 12) | (Index << 11);
       DecodeAdr(ArgStr[1],Madri+Mdadri+Maix+Mpc+Mpcidx+Mabs);
       if (AdrNum!=0)
        BEGIN
         WAsmCode[0]=0x00c0+(((Word)OpSize) << 9)+AdrMode;
         CopyAdrVals(WAsmCode+2); CodeLen=4+AdrCnt;
         CheckCPU(CPU68332);
        END
      END
    END
END

        static void DecodeEXTB(Word Index)
BEGIN
   UNUSED(Index);

   if ((OpSize!=2) AND (*AttrPart!='\0')) WrError(1130);
   else if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1],Mdata);
     if (AdrNum!=0)
      BEGIN
       WAsmCode[0]=0x49c0+AdrMode; CodeLen=2;
       CheckCPU(CPU68332);
      END
    END
END

        static void DecodePACK(Word Index) 
BEGIN
   if (ArgCnt!=3) WrError(1110);
   else if (*AttrPart!='\0') WrError(1130);
   else
    BEGIN
     DecodeAdr(ArgStr[1],Mdata+Mpre);
     if (AdrNum!=0)
      BEGIN
       WAsmCode[0]=(0x8140+(Index<<6)) | (AdrMode & 7);
       if (AdrNum==5) WAsmCode[0]+=8;
       DecodeAdr(ArgStr[2],Masks[AdrNum]);
       if (AdrNum!=0)
        BEGIN
         WAsmCode[0]|=((AdrMode & 7) << 9);
         DecodeAdr(ArgStr[3],Mimm);
         if (AdrNum!=0)
          BEGIN
           WAsmCode[1]=AdrVals[0]; CodeLen=4;
           CheckCPU(CPU68020);
          END
        END
      END
    END
END

        static void DecodeRTM(Word Index) 
BEGIN
   UNUSED(Index);

   if (*AttrPart!='\0') WrError(1130);
   else if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1],Mdata+Madr);
     if (AdrNum!=0)
      BEGIN
       WAsmCode[0]=0x06c0+AdrMode; CodeLen=2;
       CheckCPU(CPU68020); Check020();
      END
    END
END

        static void DecodeTBL(Word Index)
BEGIN
   char *p;
   Word w2,Mode;

   if (ArgCnt!=2) WrError(1110);
   else if (OpSize>2) WrError(1130);
   else if (MomCPU<CPU68332) WrError(1500);
   else
    BEGIN
     DecodeAdr(ArgStr[2],Mdata);
     if (AdrNum!=0)
      BEGIN
       Mode=AdrMode;
       p=strchr(ArgStr[1],':');
       if (p==0)
        BEGIN
         DecodeAdr(ArgStr[1],Madri+Mdadri+Maix+Mabs+Mpc+Mpcidx);
         if (AdrNum!=0) 
          BEGIN
           WAsmCode[0]=0xf800+AdrMode;
           WAsmCode[1]=0x0100+(OpSize << 6)+(Mode << 12)+(Index << 10);
           memcpy(WAsmCode+2,AdrVals,AdrCnt);
           CodeLen=4+AdrCnt; Check32();
          END
        END
       else
        BEGIN
         strcpy(ArgStr[3],p+1); *p='\0';
         DecodeAdr(ArgStr[1],Mdata);
         if (AdrNum!=0)
          BEGIN
           w2=AdrMode;
           DecodeAdr(ArgStr[3],Mdata);
           if (AdrNum!=0)
            BEGIN
             WAsmCode[0]=0xf800+w2;
             WAsmCode[1]=0x0000+(OpSize << 6)+(Mode << 12)+AdrMode;
             if (OpPart[3]=='S') WAsmCode[1]+=0x0800;
             if (OpPart[strlen(OpPart)-1]=='N') WAsmCode[1]+=0x0400;
             CodeLen=4; Check32();
            END
          END
        END
      END
    END
END

/* 0=BTST 1=BCHG 2=BCLR 3=BSET */

        static void DecodeBits(Word Index)
BEGIN
   if (ArgCnt!=2) WrError(1110);
   else
    BEGIN
     if (*AttrPart=='\0') OpSize=0;
     if (Index!=0) DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
     else DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs);
     if (*AttrPart=='\0') OpSize=(AdrNum==1) ? 2 : 0;
     if (AdrNum!=0)
      BEGIN
       if (((AdrNum==1) AND (OpSize!=2)) OR ((AdrNum!=1) AND (OpSize!=0))) WrError(1130);
       else
        BEGIN
         WAsmCode[0]=AdrMode+(Index << 6);
         CodeLen=2+AdrCnt; CopyAdrVals(WAsmCode+1);
         OpSize=0;
         DecodeAdr(ArgStr[1],Mdata+Mimm);
         if (AdrNum==1) 
          BEGIN
           WAsmCode[0]|=0x100 | (AdrMode << 9);
          END
         else if (AdrNum==11) 
          BEGIN
           memmove(WAsmCode+2,WAsmCode+1,CodeLen-2); WAsmCode[1]=AdrVals[0];
           WAsmCode[0]|=0x800;
           CodeLen+=2;
           if ((AdrVals[0]>31)
           OR  (((WAsmCode[0] & 0x38)!=0) AND (AdrVals[0]>7))) 
            BEGIN
             CodeLen=0; WrError(1510);
            END
          END
         else CodeLen=0;
        END
      END
    END
END

/* 0=BFTST 1=BFCHG 2=BFCLR 3=BFSET */

        static void DecodeFBits(Word Index)
BEGIN
    if (ArgCnt!=1) WrError(1110);
    else if (*AttrPart!='\0') WrError(1130);
    else if (NOT SplitBitField(ArgStr[1],WAsmCode+1)) WrError(1750);
    else
     BEGIN
      RelPos=4;
      OpSize=0;
      if (Memo("BFTST")) DecodeAdr(ArgStr[1],Mdata+Madri+Mdadri+Maix+Mpc+Mpcidx+Mabs);
      else DecodeAdr(ArgStr[1],Mdata+Madri+Mdadri+Maix+Mabs);
      if (AdrNum!=0) 
       BEGIN
        WAsmCode[0]=0xe8c0 | AdrMode | (Index << 10);
        CopyAdrVals(WAsmCode+2); CodeLen=4+AdrCnt;
        CheckCPU(CPU68020);
       END
     END
END

/* 0=BFEXTU 1=BFEXTS 2=BFFFO */

        static void DecodeEBits(Word Index)
BEGIN
   if (ArgCnt!=2) WrError(1110);
   else if (*AttrPart!='\0') WrError(1130);
   else if (NOT SplitBitField(ArgStr[1],WAsmCode+1)) WrError(1750);
   else
    BEGIN
     RelPos=4;
     OpSize=0;
     DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs);
     if (AdrNum!=0)
      BEGIN
       WAsmCode[0]=0xe9c0+AdrMode+(Index << 9); CopyAdrVals(WAsmCode+2);
       CodeLen=4+AdrCnt;
       DecodeAdr(ArgStr[2],Mdata);
       if (AdrNum!=0)
        BEGIN
         WAsmCode[1]|=AdrMode << 12;
         CheckCPU(CPU68020);
        END
       else CodeLen=0;
      END
    END
END

        static void DecodeBFINS(Word Index)
BEGIN
   UNUSED(Index);

   if (ArgCnt!=2) WrError(1110);
   else if (*AttrPart!='\0') WrError(1130);
   else if (NOT SplitBitField(ArgStr[2],WAsmCode+1)) WrError(1750);
   else
    BEGIN
     OpSize=0;
     DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
     if (AdrNum!=0)
      BEGIN
       WAsmCode[0]=0xefc0+AdrMode; CopyAdrVals(WAsmCode+2);
       CodeLen=4+AdrCnt;
       DecodeAdr(ArgStr[1],Mdata);
       if (AdrNum!=0)
        BEGIN
         WAsmCode[1]|=AdrMode << 12;
         CheckCPU(CPU68020);
        END
       else CodeLen=0;
      END
    END
END

static Word CondIndex;
        static void DecodeCondition(Word Index)
BEGIN
   CondIndex=Index;
END

/*-------------------------------------------------------------------------*/
/* Dekodierroutinen Gleitkommaeinheit */

        static void DecodeFPUOp(Word Index)
BEGIN
   FPUOp *Op=FPUOps+Index;

   if ((ArgCnt==1) AND (NOT Op->Dya))
    BEGIN
     strcpy(ArgStr[2],ArgStr[1]); ArgCnt=2;
    END
   if (*AttrPart=='\0') OpSize=6;
   if (OpSize==3) WrError(1130);
   else if (ArgCnt!=2) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[2],Mfpn);
     if (AdrNum==12)
      BEGIN
       WAsmCode[0]=0xf200;
       WAsmCode[1]=Op->Code | (AdrMode << 7);
       RelPos=4;
       DecodeAdr(ArgStr[1],((OpSize<=2) OR (OpSize==4))?
                           Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm+Mfpn:
                           Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm+Mfpn);
       if (AdrNum==12)
        BEGIN
         WAsmCode[1]|=AdrMode << 10;
         if (OpSize==6) CodeLen=4; else WrError(1130);
         CheckCPU(Op->MinCPU);
        END
       else if (AdrNum!=0)
        BEGIN
         CodeLen=4+AdrCnt; CopyAdrVals(WAsmCode+2);
         WAsmCode[0]|=AdrMode;
         WAsmCode[1]|=0x4000 | (((Word)FSizeCodes[OpSize]) << 10);
         CheckCPU(Op->MinCPU);
        END
      END
    END
END

/*-------------------------------------------------------------------------*/
/* Dekodierroutinen Pseudoinstruktionen: */

        static void PutByte(Byte b)
BEGIN
   if (((CodeLen&1)==1) AND (NOT BigEndian))
    BEGIN
     BAsmCode[CodeLen]=BAsmCode[CodeLen-1];
     BAsmCode[CodeLen-1]=b;
    END
   else
    BEGIN
     BAsmCode[CodeLen]=b;
    END
   CodeLen++;
END

        static void DecodeSTR(Word Index)
BEGIN
   int l,z;
   UNUSED(Index);

   if (ArgCnt!=1) WrError(1110);
   else if ((l=strlen(ArgStr[1]))<2) WrError(1135);
   else if (*ArgStr[1]!='\'') WrError(1135);
   else if (ArgStr[1][l-1]!='\'') WrError(1135);
   else
    BEGIN
     PutByte(l-2);
     for (z=1; z<l-1; z++)
      PutByte(CharTransTable[((usint) ArgStr[1][z])&0xff]);
     if ((Odd(CodeLen)) AND (DoPadding)) PutByte(0);
    END
END

/*-------------------------------------------------------------------------*/
/* Codetabellenverwaltung */

        static void AddFixed(char *NName, Word NCode, Boolean NSup, Word NMask)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Code=NCode;
   FixedOrders[InstrZ].MustSup=NSup;
   FixedOrders[InstrZ].CPUMask=NMask;
   AddInstTable(InstTable,NName,InstrZ++,DecodeFixed);
END

        static void AddCtReg(char *NName, Word NCode, CPUVar NFirst, CPUVar NLast)
BEGIN
   if (InstrZ>=CtRegCnt) exit(255);
   CtRegs[InstrZ].Name=NName;
   CtRegs[InstrZ].Code=NCode;
   CtRegs[InstrZ].FirstCPU=NFirst;
   CtRegs[InstrZ++].LastCPU=NLast;
END

        static void AddCond(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=CondCnt) exit(255);
   AddInstTable(CInstTable,NName,NCode,DecodeCondition);
   CondNams[InstrZ]=NName;
   CondVals[InstrZ++]=NCode;
END

        static void AddFPUOp(char *NName, Byte NCode, Boolean NDya, CPUVar NMin)
BEGIN
   if (InstrZ >= FPUOpCnt) exit(255);
   FPUOps[InstrZ].Name = NName;
   FPUOps[InstrZ].Code = NCode;
   FPUOps[InstrZ].Dya = NDya;
   FPUOps[InstrZ].MinCPU = NMin; 
   AddInstTable(FInstTable, NName, InstrZ++, DecodeFPUOp);
END

        static void AddFPUCond(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=FPUCondCnt) exit(255);
   FPUConds[InstrZ].Name=NName;
   FPUConds[InstrZ++].Code=NCode;
END

        static void AddPMMUCond(char *NName)
BEGIN
   if (InstrZ>=PMMUCondCnt) exit(255);
   PMMUConds[InstrZ++]=NName;
END

        static void AddPMMUReg(char *Name, Byte Size, Word Code)
BEGIN
   if (InstrZ>=PMMURegCnt) exit(255);
   PMMURegNames[InstrZ]=Name;
   PMMURegSizes[InstrZ]=Size;
   PMMURegCodes[InstrZ++]=Code;
END

        static void InitFields(void)
BEGIN
   InstTable=CreateInstTable(201);
   FInstTable=CreateInstTable(201);
   CInstTable=CreateInstTable(47);

   AddInstTable(InstTable,"MOVE"   ,0,DecodeMOVE);
   AddInstTable(InstTable,"MOVEA"  ,1,DecodeMOVE);
   AddInstTable(InstTable,"LEA"    ,0,DecodeLEA);
   AddInstTable(InstTable,"ASR"    ,0,DecodeShift);
   AddInstTable(InstTable,"ASL"    ,4,DecodeShift);
   AddInstTable(InstTable,"LSR"    ,1,DecodeShift);
   AddInstTable(InstTable,"LSL"    ,5,DecodeShift);
   AddInstTable(InstTable,"ROXR"   ,2,DecodeShift);
   AddInstTable(InstTable,"ROXL"   ,6,DecodeShift);
   AddInstTable(InstTable,"ROR"    ,3,DecodeShift);
   AddInstTable(InstTable,"ROL"    ,7,DecodeShift);
   AddInstTable(InstTable,"ADDQ"   ,0,DecodeADDQSUBQ);
   AddInstTable(InstTable,"SUBQ"   ,1,DecodeADDQSUBQ);
   AddInstTable(InstTable,"ADDX"   ,1,DecodeADDXSUBX);
   AddInstTable(InstTable,"SUBX"   ,0,DecodeADDXSUBX);
   AddInstTable(InstTable,"CMPM"   ,0,DecodeCMPM);
   AddInstTable(InstTable,"SUB"    ,0,DecodeADDSUBCMP);
   AddInstTable(InstTable,"CMP"    ,1,DecodeADDSUBCMP);
   AddInstTable(InstTable,"ADD"    ,2,DecodeADDSUBCMP);
   AddInstTable(InstTable,"SUBI"   ,4,DecodeADDSUBCMP);
   AddInstTable(InstTable,"CMPI"   ,5,DecodeADDSUBCMP);
   AddInstTable(InstTable,"ADDI"   ,6,DecodeADDSUBCMP);
   AddInstTable(InstTable,"SUBA"   ,8,DecodeADDSUBCMP);
   AddInstTable(InstTable,"CMPA"   ,9,DecodeADDSUBCMP);
   AddInstTable(InstTable,"ADDA"   ,10,DecodeADDSUBCMP);
   AddInstTable(InstTable,"AND"    ,1,DecodeANDOR);
   AddInstTable(InstTable,"OR"     ,0,DecodeANDOR);
   AddInstTable(InstTable,"ANDI"   ,5,DecodeANDOR);
   AddInstTable(InstTable,"ORI"    ,4,DecodeANDOR);
   AddInstTable(InstTable,"EOR"    ,0,DecodeEOR);
   AddInstTable(InstTable,"EORI"   ,4,DecodeEOR);
   AddInstTable(InstTable,"PEA"    ,0,DecodePEA);
   AddInstTable(InstTable,"CLR"    ,0,DecodeCLRTST);
   AddInstTable(InstTable,"TST"    ,1,DecodeCLRTST);
   AddInstTable(InstTable,"JSR"    ,0,DecodeJSRJMP);
   AddInstTable(InstTable,"JMP"    ,1,DecodeJSRJMP);
   AddInstTable(InstTable,"TAS"    ,0,DecodeNBCDTAS);
   AddInstTable(InstTable,"NBCD"   ,1,DecodeNBCDTAS);
   AddInstTable(InstTable,"NEGX"   ,0,DecodeNEGNOT);
   AddInstTable(InstTable,"NEG"    ,2,DecodeNEGNOT);
   AddInstTable(InstTable,"NOT"    ,3,DecodeNEGNOT);
   AddInstTable(InstTable,"SWAP"   ,0,DecodeSWAP);
   AddInstTable(InstTable,"UNLK"   ,0,DecodeUNLK);
   AddInstTable(InstTable,"EXT"    ,0,DecodeEXT);
   AddInstTable(InstTable,"WDDATA" ,0,DecodeWDDATA);
   AddInstTable(InstTable,"WDEBUG" ,0,DecodeWDEBUG);
   AddInstTable(InstTable,"MOVEM"  ,0,DecodeMOVEM); 
   AddInstTable(InstTable,"MOVEQ"  ,0,DecodeMOVEQ);
   AddInstTable(InstTable,"STOP"   ,0,DecodeSTOP);
   AddInstTable(InstTable,"LPSTOP" ,0,DecodeLPSTOP);
   AddInstTable(InstTable,"TRAP"   ,0,DecodeTRAP);
   AddInstTable(InstTable,"BKPT"   ,0,DecodeBKPT);
   AddInstTable(InstTable,"RTD"    ,0,DecodeRTD);
   AddInstTable(InstTable,"EXG"    ,0,DecodeEXG);
   AddInstTable(InstTable,"MOVE16" ,0,DecodeMOVE16);
   AddInstTable(InstTable,"DIVUL"  ,0,DecodeDIVL);
   AddInstTable(InstTable,"DIVSL"  ,1,DecodeDIVL);
   AddInstTable(InstTable,"ABCD"   ,1,DecodeASBCD);
   AddInstTable(InstTable,"SBCD"   ,0,DecodeASBCD);
   AddInstTable(InstTable,"CHK"    ,0,DecodeCHK);
   AddInstTable(InstTable,"LINK"   ,0,DecodeLINK);
   AddInstTable(InstTable,"MOVEP"  ,0,DecodeMOVEP);
   AddInstTable(InstTable,"MOVEC"  ,0,DecodeMOVEC);
   AddInstTable(InstTable,"MOVES"  ,0,DecodeMOVES);
   AddInstTable(InstTable,"CALLM"  ,0,DecodeCALLM);
   AddInstTable(InstTable,"CAS"    ,0,DecodeCAS);
   AddInstTable(InstTable,"CAS2"   ,0,DecodeCAS2);
   AddInstTable(InstTable,"CMP2"   ,0,DecodeCMPCHK2);
   AddInstTable(InstTable,"CHK2"   ,1,DecodeCMPCHK2);
   AddInstTable(InstTable,"EXTB"   ,0,DecodeEXTB);
   AddInstTable(InstTable,"PACK"   ,0,DecodePACK);
   AddInstTable(InstTable,"UNPK"   ,1,DecodePACK);
   AddInstTable(InstTable,"RTM"    ,0,DecodeRTM);
   AddInstTable(InstTable,"TBLU"   ,0,DecodeTBL);
   AddInstTable(InstTable,"TBLUN"  ,1,DecodeTBL);
   AddInstTable(InstTable,"TBLS"   ,2,DecodeTBL);
   AddInstTable(InstTable,"TBLSN"  ,3,DecodeTBL);
   AddInstTable(InstTable,"BTST"   ,0,DecodeBits);
   AddInstTable(InstTable,"BSET"   ,3,DecodeBits);
   AddInstTable(InstTable,"BCLR"   ,2,DecodeBits);
   AddInstTable(InstTable,"BCHG"   ,1,DecodeBits);
   AddInstTable(InstTable,"BFTST"  ,0,DecodeFBits);
   AddInstTable(InstTable,"BFSET"  ,3,DecodeFBits);
   AddInstTable(InstTable,"BFCLR"  ,2,DecodeFBits);
   AddInstTable(InstTable,"BFCHG"  ,1,DecodeFBits);
   AddInstTable(InstTable,"BFEXTU" ,0,DecodeEBits);
   AddInstTable(InstTable,"BFEXTS" ,1,DecodeEBits);
   AddInstTable(InstTable,"BFFFO"  ,2,DecodeEBits);
   AddInstTable(InstTable,"BFINS"  ,0,DecodeBFINS);
   AddInstTable(InstTable,"CINVA"  ,0,DecodeCacheAll);
   AddInstTable(InstTable,"CPUSHA" ,1,DecodeCacheAll);
   AddInstTable(InstTable,"CINVL"  ,1,DecodeCache);
   AddInstTable(InstTable,"CPUSHL" ,5,DecodeCache);
   AddInstTable(InstTable,"CINVP"  ,2,DecodeCache);
   AddInstTable(InstTable,"CPUSHP" ,6,DecodeCache);
   AddInstTable(InstTable,"STR"    ,0,DecodeSTR);

   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("NOP"    ,0x4e71,False,0x07ff);
   AddFixed("RESET"  ,0x4e70,False,0x07ef);
   AddFixed("ILLEGAL",0x4afc,False,0x07ff);
   AddFixed("TRAPV"  ,0x4e76,False,0x07ef);
   AddFixed("RTE"    ,0x4e73,True ,0x07ff);
   AddFixed("RTR"    ,0x4e77,False,0x07ef);
   AddFixed("RTS"    ,0x4e75,False,0x07ff);
   AddFixed("BGND"   ,0x4afa,False,0x00e0);
   AddFixed("HALT"   ,0x4ac8,True ,0x0010);
   AddFixed("PULSE"  ,0x4acc,True ,0x0010);

   CtRegs=(CtReg *) malloc(sizeof(CtReg)*CtRegCnt); InstrZ=0;
   AddCtReg("SFC"  ,0x000, CPU68010, CPU68040);
   AddCtReg("DFC"  ,0x001, CPU68010, CPU68040);
   AddCtReg("CACR" ,0x002, CPU68020, CPU68040);
   AddCtReg("TC"   ,0x003, CPU68040, CPU68040);
   AddCtReg("ITT0" ,0x004, CPU68040, CPU68040);
   AddCtReg("ITT1" ,0x005, CPU68040, CPU68040);
   AddCtReg("DTT0" ,0x006, CPU68040, CPU68040);
   AddCtReg("DTT1" ,0x007, CPU68040, CPU68040);
   AddCtReg("USP"  ,0x800, CPU68010, CPU68040);
   AddCtReg("VBR"  ,0x801, CPU68010, CPU68040);
   AddCtReg("CAAR" ,0x802, CPU68020, CPU68030);
   AddCtReg("MSP"  ,0x803, CPU68020, CPU68040);
   AddCtReg("ISP"  ,0x804, CPU68020, CPU68040);
   AddCtReg("MMUSR",0x805, CPU68040, CPU68040);
   AddCtReg("URP"  ,0x806, CPU68040, CPU68040);
   AddCtReg("SRP"  ,0x807, CPU68040, CPU68040);
   AddCtReg("IACR0",0x004, CPU68040, CPU68040);
   AddCtReg("IACR1",0x005, CPU68040, CPU68040);
   AddCtReg("DACR0",0x006, CPU68040, CPU68040);
   AddCtReg("DACR1",0x007, CPU68040, CPU68040);
   AddCtReg("TCR"  ,0x003, CPUCOLD , CPUCOLD );
   AddCtReg("ACR2" ,0x004, CPUCOLD , CPUCOLD );
   AddCtReg("ACR3" ,0x005, CPUCOLD , CPUCOLD );
   AddCtReg("ACR0" ,0x006, CPUCOLD , CPUCOLD );
   AddCtReg("ACR1" ,0x007, CPUCOLD , CPUCOLD );
   AddCtReg("ROMBAR",0xc00,CPUCOLD , CPUCOLD );
   AddCtReg("RAMBAR0",0xc04,CPUCOLD, CPUCOLD );
   AddCtReg("RAMBAR1",0xc05,CPUCOLD, CPUCOLD );
   AddCtReg("MBAR" ,0xc0f, CPUCOLD , CPUCOLD );

   CondNams=(char **) malloc(sizeof(char *)*CondCnt); 
   CondVals=(Byte *) malloc(sizeof(Byte)*CondCnt); InstrZ=0;
   AddCond("T" , 0);  AddCond("F" , 1);  AddCond("HI", 2);  AddCond("LS", 3);
   AddCond("CC", 4);  AddCond("CS", 5);  AddCond("NE", 6);  AddCond("EQ", 7);
   AddCond("VC", 8);  AddCond("VS", 9);  AddCond("PL",10);  AddCond("MI",11);
   AddCond("GE",12);  AddCond("LT",13);  AddCond("GT",14);  AddCond("LE",15);
   AddCond("HS", 4);  AddCond("LO", 5);  AddCond("RA", 0);  AddCond("SR", 1);

   FPUOps=(FPUOp *) malloc(sizeof(FPUOp)*FPUOpCnt); InstrZ=0;
   AddFPUOp("INT"   ,0x01, False, CPU68000);  AddFPUOp("SINH"  ,0x02, False, CPU68000);
   AddFPUOp("INTRZ" ,0x03, False, CPU68000);  AddFPUOp("SQRT"  ,0x04, False, CPU68000);
   AddFPUOp("LOGNP1",0x06, False, CPU68000);  AddFPUOp("ETOXM1",0x08, False, CPU68000);
   AddFPUOp("TANH"  ,0x09, False, CPU68000);  AddFPUOp("ATAN"  ,0x0a, False, CPU68000);
   AddFPUOp("ASIN"  ,0x0c, False, CPU68000);  AddFPUOp("ATANH" ,0x0d, False, CPU68000);
   AddFPUOp("SIN"   ,0x0e, False, CPU68000);  AddFPUOp("TAN"   ,0x0f, False, CPU68000);
   AddFPUOp("ETOX"  ,0x10, False, CPU68000);  AddFPUOp("TWOTOX",0x11, False, CPU68000);
   AddFPUOp("TENTOX",0x12, False, CPU68000);  AddFPUOp("LOGN"  ,0x14, False, CPU68000);
   AddFPUOp("LOG10" ,0x15, False, CPU68000);  AddFPUOp("LOG2"  ,0x16, False, CPU68000);
   AddFPUOp("ABS"   ,0x18, False, CPU68000);  AddFPUOp("COSH"  ,0x19, False, CPU68000);
   AddFPUOp("NEG"   ,0x1a, False, CPU68000);  AddFPUOp("ACOS"  ,0x1c, False, CPU68000);
   AddFPUOp("COS"   ,0x1d, False, CPU68000);  AddFPUOp("GETEXP",0x1e, False, CPU68000);
   AddFPUOp("GETMAN",0x1f, False, CPU68000);  AddFPUOp("DIV"   ,0x20, True , CPU68000);
   AddFPUOp("SDIV"  ,0x60, False, CPU68040);  AddFPUOp("DDIV"  ,0x64, True , CPU68040);
   AddFPUOp("MOD"   ,0x21, True , CPU68000);  AddFPUOp("ADD"   ,0x22, True , CPU68000);
   AddFPUOp("SADD"  ,0x62, True , CPU68040);  AddFPUOp("DADD"  ,0x66, True , CPU68040);
   AddFPUOp("MUL"   ,0x23, True , CPU68000);  AddFPUOp("SMUL"  ,0x63, True , CPU68040);
   AddFPUOp("DMUL"  ,0x67, True , CPU68040);  AddFPUOp("SGLDIV",0x24, True , CPU68000);
   AddFPUOp("REM"   ,0x25, True , CPU68000);  AddFPUOp("SCALE" ,0x26, True , CPU68000);
   AddFPUOp("SGLMUL",0x27, True , CPU68000);  AddFPUOp("SUB"   ,0x28, True , CPU68000);
   AddFPUOp("SSUB"  ,0x68, True , CPU68040);  AddFPUOp("DSUB"  ,0x6c, True , CPU68040);
   AddFPUOp("CMP"   ,0x38, True , CPU68000);

   FPUConds=(FPUCond *) malloc(sizeof(FPUCond)*FPUCondCnt); InstrZ=0;
   AddFPUCond("EQ"  , 0x01); AddFPUCond("NE"  , 0x0e);
   AddFPUCond("GT"  , 0x12); AddFPUCond("NGT" , 0x1d);
   AddFPUCond("GE"  , 0x13); AddFPUCond("NGE" , 0x1c);
   AddFPUCond("LT"  , 0x14); AddFPUCond("NLT" , 0x1b);
   AddFPUCond("LE"  , 0x15); AddFPUCond("NLE" , 0x1a);
   AddFPUCond("GL"  , 0x16); AddFPUCond("NGL" , 0x19);
   AddFPUCond("GLE" , 0x17); AddFPUCond("NGLE", 0x18);
   AddFPUCond("OGT" , 0x02); AddFPUCond("ULE" , 0x0d);
   AddFPUCond("OGE" , 0x03); AddFPUCond("ULT" , 0x0c);
   AddFPUCond("OLT" , 0x04); AddFPUCond("UGE" , 0x0b);
   AddFPUCond("OLE" , 0x05); AddFPUCond("UGT" , 0x0a);
   AddFPUCond("OGL" , 0x06); AddFPUCond("UEQ" , 0x09);
   AddFPUCond("OR"  , 0x07); AddFPUCond("UN"  , 0x08);

   PMMUConds=(char **) malloc(sizeof(char *)*PMMUCondCnt); InstrZ=0;
   AddPMMUCond("BS"); AddPMMUCond("BC"); AddPMMUCond("LS"); AddPMMUCond("LC"); 
   AddPMMUCond("SS"); AddPMMUCond("SC"); AddPMMUCond("AS"); AddPMMUCond("AC"); 
   AddPMMUCond("WS"); AddPMMUCond("WC"); AddPMMUCond("IS"); AddPMMUCond("IC"); 
   AddPMMUCond("GS"); AddPMMUCond("GC"); AddPMMUCond("CS"); AddPMMUCond("CC"); 

   PMMURegNames=(char **) malloc(sizeof(char *)*PMMURegCnt);
   PMMURegSizes=(Byte *) malloc(sizeof(Byte)*PMMURegCnt);
   PMMURegCodes=(Word *) malloc(sizeof(Word)*PMMURegCnt); InstrZ=0;
   AddPMMUReg("TC"   ,2,16); AddPMMUReg("DRP"  ,3,17);
   AddPMMUReg("SRP"  ,3,18); AddPMMUReg("CRP"  ,3,19);
   AddPMMUReg("CAL"  ,0,20); AddPMMUReg("VAL"  ,0,21);
   AddPMMUReg("SCC"  ,0,22); AddPMMUReg("AC"   ,1,23);
   AddPMMUReg("PSR"  ,1,24); AddPMMUReg("PCSR" ,1,25);
   AddPMMUReg("TT0"  ,2, 2); AddPMMUReg("TT1"  ,2, 3);
   AddPMMUReg("MMUSR",1,24); 
END

        static void DeinitFields(void)
BEGIN
   DestroyInstTable(InstTable); DestroyInstTable(FInstTable);
   DestroyInstTable(CInstTable);
   free(FixedOrders);
   free(CtRegs);
   free(CondNams); free(CondVals);
   free(FPUOps);
   free(FPUConds);
   free(PMMUConds);
   free(PMMURegNames); free(PMMURegSizes); free(PMMURegCodes);
END

/*---------------------------------------------------------------------------*/

        static Boolean DecodePseudo(void)
BEGIN
   return False;
END

/*-------------------------------------------------------------------------*/


        static Boolean DecodeOneFPReg(char *Asc, Byte * h)
BEGIN
   if ((strlen(Asc)==3) AND (strncasecmp(Asc,"FP",2)==0) AND ValReg(Asc[2]))
    BEGIN
     *h=Asc[2]-'0'; return True;
    END
   else return False;
END

        static void DecodeFRegList(char *Asc_o, Byte *Typ, Byte *Erg)
BEGIN
   String s,Asc;
   Word hw;
   Byte h2,h3,z;
   char *h1;

   strmaxcpy(Asc,Asc_o,255);
   *Typ=0; if (*Asc=='\0') return;

   if ((strlen(Asc)==2) AND (*Asc=='D') AND ValReg(Asc[1]))
    BEGIN
     *Typ = 1; *Erg = (Asc[1]-'0') << 4; return;
    END;

   hw=0;
   do
    BEGIN
     h1=strchr(Asc,'/');
     if (h1==Nil)
      BEGIN
       strcpy(s,Asc); *Asc='\0';
      END
     else
      BEGIN
       *h1='\0'; strcpy(s,Asc); strcpy(Asc,h1+1);
      END
     if (strcasecmp(s,"FPCR")==0) hw|=0x400;
     else if (strcasecmp(s,"FPSR")==0) hw|=0x200;
     else if (strcasecmp(s,"FPIAR")==0) hw|=0x100;
     else
      BEGIN
       h1=strchr(s,'-');
       if (h1==Nil)
        BEGIN
         if (NOT DecodeOneFPReg(s,&h2)) return;
         hw|=(1 << (7-h2));
        END
       else
        BEGIN
         *h1='\0';
         if (NOT DecodeOneFPReg(s,&h2)) return;
         if (NOT DecodeOneFPReg(h1+1,&h3)) return;
         for (z=h2; z<=h3; z++) hw|=(1 << (7-z));
        END
      END
    END
   while (*Asc!='\0');
   if (Hi(hw)==0)
    BEGIN
     *Typ=2; *Erg=Lo(hw);
    END
   else if (Lo(hw)==0)
    BEGIN
     *Typ=3; *Erg=Hi(hw);
    END
END

        static void GenerateMovem(Byte z1, Byte z2)
BEGIN
   Byte hz2,z;

   if (AdrNum==0) return;
   CodeLen=4+AdrCnt; CopyAdrVals(WAsmCode+2);
   WAsmCode[0]=0xf200 | AdrMode;
   switch (z1)
    BEGIN
     case 1:
     case 2:
      WAsmCode[1]|=0xc000;
      if (z1==1) WAsmCode[1]|=0x800;
      if (AdrNum!=5) WAsmCode[1]|=0x1000;
      if ((AdrNum==5) AND (z1==2))
       BEGIN
        hz2=z2; z2=0;
        for (z=0; z<8; z++)
         BEGIN
          z2=z2 << 1; if ((hz2&1)==1) z2|=1;
          hz2=hz2 >> 1;
         END
       END
      WAsmCode[1]|=z2;
      break;
     case 3:
      WAsmCode[1]|=0x8000 | (((Word)z2) << 10);
    END
END

        static void DecodeFPUOrders(void)
BEGIN
   Byte z,z1,z2;
   char *p;
   String sk;
   LongInt HVal;
   Integer HVal16;
   Boolean ValOK;
   Word Mask;

   if (LookupInstTable(FInstTable,OpPart)) return;

   if (Memo("SAVE"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1130);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Madri+Mpre+Mdadri+Maix+Mabs);
       if (AdrNum!=0)
        BEGIN
         CodeLen=2+AdrCnt; WAsmCode[0]=0xf300 | AdrMode;
         CopyAdrVals(WAsmCode+1); CheckSup();
        END
      END
     return;
    END

   if (Memo("RESTORE"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1130);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Madri+Mpost+Mdadri+Maix+Mabs);
       if (AdrNum!=0)
        BEGIN
         CodeLen=2+AdrCnt; WAsmCode[0]=0xf340 | AdrMode;
         CopyAdrVals(WAsmCode+1); CheckSup();
        END
      END
     return;
    END

   if (Memo("NOP"))
    BEGIN
     if (ArgCnt!=0) WrError(1110);
     else if (*AttrPart!='\0') WrError(1130);
     else
      BEGIN
       CodeLen=4; WAsmCode[0]=0xf280; WAsmCode[1]=0;
      END
     return;
    END

   if (Memo("MOVE"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (OpSize==3) WrError(1130);
     else
      BEGIN
       p=strchr(AttrPart,'{');
       if (p!=0)                               /* k-Faktor abspalten */
        BEGIN
         strcpy(sk,p); *p='\0';
        END
       else *sk='\0';
       DecodeAdr(ArgStr[2],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mabs+Mfpn+Mfpcr);
       if (AdrNum==12)                         /* FMOVE.x <ea>/FPm,FPn ? */
        BEGIN
         WAsmCode[0]=0xf200; WAsmCode[1]=AdrMode << 7;
         RelPos=4;
         if (*AttrPart=='\0') OpSize=6;
         DecodeAdr(ArgStr[1],((OpSize<=2) OR (OpSize==4))?
                             Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm+Mfpn:
                             Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm+Mfpn);
         if (AdrNum==12)                       /* FMOVE.X FPm,FPn ? */
          BEGIN
           WAsmCode[1]|=AdrMode << 10;
           if (OpSize==6) CodeLen=4; else WrError(1130);
          END
         else if (AdrNum!=0)                   /* FMOVE.x <ea>,FPn ? */
          BEGIN
           CodeLen=4+AdrCnt; CopyAdrVals(WAsmCode+2);
           WAsmCode[0]|=AdrMode;
           WAsmCode[1]|=0x4000 | (((Word)FSizeCodes[OpSize]) << 10);
          END
        END
       else if (AdrNum==13)                    /* FMOVE.L <ea>,FPcr ? */
        BEGIN
         if ((OpSize!=2) AND (*AttrPart!='\0')) WrError(1130);
         else
          BEGIN
           RelPos=4;
           WAsmCode[0]=0xf200; WAsmCode[1]=0x8000 | (AdrMode << 10);
           DecodeAdr(ArgStr[1],(AdrMode==1)?
                     Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Mpc+Mpcidx+Mabs+Mimm:
                     Mdata+Madri+Mpost+Mpre+Mdadri+Mpc+Mpcidx+Mabs+Mimm);
           if (AdrNum!=0)
            BEGIN
             WAsmCode[0]|=AdrMode; CodeLen=4+AdrCnt;
             CopyAdrVals(WAsmCode+2);
            END
          END
        END
       else if (AdrNum!=0)                     /* FMOVE.x ????,<ea> ? */
        BEGIN
         WAsmCode[0]=0xf200 | AdrMode;
         CodeLen=4+AdrCnt; CopyAdrVals(WAsmCode+2);
         DecodeAdr(ArgStr[1], (AdrNum == 2) ? Mfpcr : Mfpn + Mfpcr);
         if (AdrNum==12)                       /* FMOVE.x FPn,<ea> ? */
          BEGIN
           if (*AttrPart=='\0') OpSize=6;
           WAsmCode[1]=0x6000 | (((Word)FSizeCodes[OpSize]) << 10) | (AdrMode << 7);
           if (OpSize==7)
            BEGIN
             if (strlen(sk)>2)
              BEGIN
               OpSize=0; strcpy(sk,sk+1); sk[strlen(sk)-1]='\0';
               DecodeAdr(sk,Mdata+Mimm);
               if (AdrNum==1) WAsmCode[1]|=(AdrMode << 4) | 0x1000;
               else if (AdrNum==11) WAsmCode[1]|=(AdrVals[0] & 127);
               else CodeLen=0;
              END
             else WAsmCode[1]|=17;
            END
          END
         else if (AdrNum==13)                  /* FMOVE.L FPcr,<ea> ? */
          BEGIN
           if ((*AttrPart!='\0') AND (OpSize!=2))
            BEGIN
             WrError(1130); CodeLen=0;
            END
           else
            BEGIN
             WAsmCode[1]=0xa000 | (AdrMode << 10);
             if ((AdrMode!=1) AND ((WAsmCode[0] & 0x38)==8))
              BEGIN
               WrError(1350); CodeLen=0;
              END
            END
          END
         else CodeLen=0;
        END
      END
     return;
    END

   if (Memo("MOVECR"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if ((*AttrPart!='\0') AND (OpSize!=6)) WrError(1130);
     else
      BEGIN
       DecodeAdr(ArgStr[2],Mfpn);
       if (AdrNum==12)
        BEGIN
         WAsmCode[0]=0xf200; WAsmCode[1]=0x5c00 | (AdrMode << 7);
         OpSize=0;
         DecodeAdr(ArgStr[1],Mimm);
         if (AdrNum==11)
          BEGIN
           if (AdrVals[0]>63) WrError(1700);
           else
            BEGIN
             CodeLen=4;
             WAsmCode[1]|=AdrVals[0];
            END
          END
        END
      END
     return;
    END

   if (Memo("TST"))
    BEGIN
     if (*AttrPart=='\0') OpSize=6;
     else if (OpSize==3) WrError(1130);
     else if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       RelPos=4;
       DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm+Mfpn);
       if (AdrNum==12)
        BEGIN
         WAsmCode[0]=0xf200; WAsmCode[1]=0x3a | (AdrMode << 10);
         CodeLen=4;
        END
       else if (AdrNum!=0)
        BEGIN
         WAsmCode[0]=0xf200 | AdrMode;
         WAsmCode[1]=0x403a | (((Word)FSizeCodes[OpSize]) << 10);
         CodeLen=4+AdrCnt; CopyAdrVals(WAsmCode+2);
        END
      END
     return;
    END

   if (Memo("SINCOS"))
    BEGIN
     if (*AttrPart=='\0') OpSize=6;
     if (OpSize==3) WrError(1130);
     else if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       p=strrchr(ArgStr[2],':');
       if (p!=Nil)
        BEGIN
         *p='\0'; strcpy(sk,ArgStr[2]); strcpy(ArgStr[2],p+1);
        END
       else *sk='\0';
       DecodeAdr(sk,Mfpn);
       if (AdrNum==12)
        BEGIN
         WAsmCode[1]=AdrMode | 0x30;
         DecodeAdr(ArgStr[2],Mfpn);
         if (AdrNum==12)
          BEGIN
           WAsmCode[1]|=(AdrMode << 7);
           RelPos=4;
           DecodeAdr(ArgStr[1],((OpSize<=2) OR (OpSize==4))?
                               Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm+Mfpn:
                               Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm+Mfpn);
           if (AdrNum==12)
            BEGIN
             WAsmCode[0]=0xf200; WAsmCode[1]|=(AdrMode << 10);
             CodeLen=4;
            END
           else if (AdrNum!=0)
            BEGIN
             WAsmCode[0]=0xf200 | AdrMode;
             WAsmCode[1]|=0x4000 | (((Word)FSizeCodes[OpSize]) << 10);
             CodeLen=4+AdrCnt; CopyAdrVals(WAsmCode+2);
            END
          END
        END
      END
     return;
    END

   if (*OpPart=='B')
    BEGIN
     for (z=0; z<FPUCondCnt; z++)
      if (strcmp(OpPart+1,FPUConds[z].Name)==0) break;
     if (z>=FPUCondCnt) WrError(1360);
     else
      BEGIN
       if ((OpSize!=1) AND (OpSize!=2) AND (OpSize!=6)) WrError(1130);
       else if (ArgCnt!=1) WrError(1110);
       else
        BEGIN
         HVal=EvalIntExpression(ArgStr[1],Int32,&ValOK)-(EProgCounter()+2);
         HVal16=HVal;

         if (OpSize==1)
          BEGIN
           OpSize=(IsDisp16(HVal))?2:6;
          END

         if (OpSize==2)
          BEGIN
           if ((NOT IsDisp16(HVal)) AND (NOT SymbolQuestionable)) WrError(1370);
           else
            BEGIN
             CodeLen=4; WAsmCode[0]=0xf280 | FPUConds[z].Code;
             WAsmCode[1]=HVal16;
            END
          END
         else
          BEGIN
           CodeLen=6; WAsmCode[0]=0xf2c0 | FPUConds[z].Code;
           WAsmCode[2]=HVal & 0xffff; WAsmCode[1]=HVal >> 16;
           if ((IsDisp16(HVal)) AND (PassNo>1) AND (*AttrPart=='\0'))
            BEGIN
             WrError(20); WAsmCode[0]^=0x40;
             CodeLen-=2; WAsmCode[1]=WAsmCode[2]; StopfZahl++;
            END
          END
        END
      END
     return;
    END

   if (strncmp(OpPart,"DB",2)==0)
    BEGIN
     for (z=0; z<FPUCondCnt; z++)
      if (strcmp(OpPart+2,FPUConds[z].Name)==0) break;
     if (z>=FPUCondCnt) WrError(1360);
     else
      BEGIN
       if ((OpSize!=1) AND (*AttrPart!='\0')) WrError(1130);
       else if (ArgCnt!=2) WrError(1110);
       else
        BEGIN
         DecodeAdr(ArgStr[1],Mdata);
         if (AdrNum!=0)
          BEGIN
           WAsmCode[0]=0xf248 | AdrMode; WAsmCode[1]=FPUConds[z].Code;
           HVal=EvalIntExpression(ArgStr[2],Int32,&ValOK)-(EProgCounter()+4);
           if (ValOK)
            BEGIN
             HVal16=HVal; WAsmCode[2]=HVal16;
             if ((NOT IsDisp16(HVal)) AND (NOT SymbolQuestionable)) WrError(1370); else CodeLen=6;
            END
          END
        END
      END
     return;
    END

   if ((Memo("DMOVE")) OR (Memo("SMOVE")))
    BEGIN
     if (ArgCnt != 2) WrError(1110);
     else if (MomCPU < CPU68040) WrError(1500);
     else
      BEGIN
       DecodeAdr(ArgStr[2], Mfpn);
       if (AdrNum == 12)
        BEGIN
         WAsmCode[0] = 0xf200;
         WAsmCode[1] = 0x0040 | AdrMode << 7;
         if (*OpPart == 'D') WAsmCode[1] |= 4;
         RelPos = 4;
         if (*AttrPart == '\0') OpSize = 6;
         Mask = Mfpn+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm;
         if ((OpSize <= 2) OR (OpSize == 4))
          Mask |= Mdata;
         DecodeAdr(ArgStr[1], Mask);
         if (AdrNum == 12)
          BEGIN
           CodeLen = 4;
           WAsmCode[1] |= (AdrMode << 10);
          END
         else if (AdrNum != 0)
          BEGIN
           CodeLen = 4 + AdrCnt; CopyAdrVals(WAsmCode + 2);
           WAsmCode[0] |= AdrMode;
           WAsmCode[1] |= 0x4000 | (((Word)FSizeCodes[OpSize]) << 10);
          END
        END
      END
     return;
    END

   if (*OpPart=='S')
    BEGIN
     for (z=0; z<FPUCondCnt; z++)
      if (strcmp(OpPart+1,FPUConds[z].Name)==0) break;
     if (z>=FPUCondCnt) WrError(1360);
     else
      BEGIN
       if ((OpSize!=0) AND (*AttrPart!='\0')) WrError(1130);
       else if (ArgCnt!=1) WrError(1110);
       else
        BEGIN
         DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
         if (AdrNum!=0)
          BEGIN
           CodeLen=4+AdrCnt; WAsmCode[0]=0xf240 | AdrMode;
           WAsmCode[1]=FPUConds[z].Code; CopyAdrVals(WAsmCode+2);
          END
        END
      END
     return;
    END

   if (strncmp(OpPart,"TRAP",4)==0)
    BEGIN
     for (z=0; z<FPUCondCnt; z++)
      if (strcmp(OpPart+4,FPUConds[z].Name)==0) break;
     if (z>=FPUCondCnt) WrError(1360);
     else
      BEGIN
       if (*AttrPart=='\0') OpSize=0;
       if (OpSize>2) WrError(1130);
       else if (((OpSize==0) AND (ArgCnt!=0)) OR ((OpSize!=0) AND (ArgCnt!=1))) WrError(1110);
       else
        BEGIN
         WAsmCode[0]=0xf278; WAsmCode[1]=FPUConds[z].Code;
         if (OpSize==0)
          BEGIN
           WAsmCode[0]|=4; CodeLen=4;
          END
         else
          BEGIN
           DecodeAdr(ArgStr[1],Mimm);
           if (AdrNum!=0)
            BEGIN
             WAsmCode[0]|=(OpSize+1);
             CopyAdrVals(WAsmCode+2); CodeLen=4+AdrCnt;
            END
          END
        END
      END
     return;
    END

   if (Memo("MOVEM"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeFRegList(ArgStr[2], &z1, &z2);
       if (z1 != 0)
        BEGIN
         if ((*AttrPart != '\0')
         AND (((z1 < 3) AND (OpSize != 6))
           OR ((z1 == 3) AND (OpSize != 2))))
          WrError(1130);
         else
          BEGIN
           RelPos = 4;
           Mask = Madri + Mpost + Mdadri + Maix + Mpc + Mpcidx + Mabs + Mimm;
           if (z1 == 3)   /* Steuerregister auch Predekrement */
            BEGIN
             Mask |= Mpre;
             if ((z2 == 4) | (z2 == 2) | (z2 == 1)) /* nur ein Register */
              Mask |= Mdata;
             if (z2 == 1) /* nur FPIAR */
              Mask |= Madr;
            END
           DecodeAdr(ArgStr[1], Mask);
           WAsmCode[1] = 0; GenerateMovem(z1, z2);
          END
        END
       else
        BEGIN
         DecodeFRegList(ArgStr[1],&z1,&z2);
         if (z1!=0)
          BEGIN
           if ((*AttrPart!='\0') AND (((z1<3) AND (OpSize!=6)) OR ((z1==3) AND (OpSize!=2))))
            WrError(1130);
           else
            BEGIN
             Mask = Madri + Mpost + Mdadri + Maix + Mabs;
             if (z1 == 3)   /* Steuerregister auch Postinkrement */
              BEGIN
               Mask |= Mpre;
               if ((z2 == 4) | (z2 == 2) | (z2 == 1)) /* nur ein Register */
                Mask |= Mdata;
               if (z2 == 1) /* nur FPIAR */
                Mask |= Madr;
              END
             DecodeAdr(ArgStr[2], Mask);
             WAsmCode[1] = 0x2000; GenerateMovem(z1, z2);
            END
          END
         else WrError(1410);
        END
      END
     return;
    END

   WrXError(1200,OpPart);
END

/*-------------------------------------------------------------------------*/


        static Boolean DecodeFC(char *Asc, Word *erg)
BEGIN
   Boolean OK;
   Word Val;
   String Asc_N;

   strmaxcpy(Asc_N,Asc,255); NLS_UpString(Asc_N); Asc=Asc_N;

   if (strcmp(Asc,"SFC")==0)
    BEGIN
     *erg=0; return True;
    END

   if (strcmp(Asc,"DFC")==0)
    BEGIN
     *erg=1; return True;
    END

   if ((strlen(Asc)==2) AND (*Asc=='D') AND ValReg(Asc[1]))
    BEGIN
     *erg=Asc[2]-'0'+8; return True;
    END

   if (*Asc=='#')
    BEGIN
     Val=EvalIntExpression(Asc+1,Int4,&OK);
     if (OK) *erg=Val+16; return OK;
    END

   return False;
END

        static Boolean DecodePMMUReg(char *Asc, Word *erg, Byte *Size)
BEGIN
   Byte z;

   if ((strlen(Asc)==4) AND (strncasecmp(Asc,"BAD",3)==0) AND ValReg(Asc[3]))
    BEGIN
     *Size=1;
     *erg=0x7000+((Asc[3]-'0') << 2); return True;
    END
   if ((strlen(Asc)==4) AND (strncasecmp(Asc,"BAC",3)==0) AND ValReg(Asc[3]))
    BEGIN
     *Size=1;
     *erg=0x7400+((Asc[3]-'0') << 2); return True;
    END

   for (z=0; z<PMMURegCnt; z++)
    if (strcasecmp(Asc,PMMURegNames[z])==0) break;
   if (z<PMMURegCnt)
    BEGIN
     *Size=PMMURegSizes[z];
     *erg=PMMURegCodes[z] << 10;
    END
   return (z<PMMURegCnt);
END

        static void DecodePMMUOrders(void)
BEGIN
   Byte z;
   Word Mask;
   LongInt HVal;
   Integer HVal16;
   Boolean ValOK;

   if (Memo("SAVE"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1130);
     else if (NOT FullPMMU) WrError(1500);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Madri+Mpre+Mdadri+Maix+Mabs);
       if (AdrNum!=0)
        BEGIN
         CodeLen=2+AdrCnt; WAsmCode[0]=0xf100 | AdrMode;
         CopyAdrVals(WAsmCode+1); CheckSup();
        END
      END
     return;
    END

   if (Memo("RESTORE"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1130);
     else if (NOT FullPMMU) WrError(1500);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Madri+Mpre+Mdadri+Maix+Mabs);
       if (AdrNum!=0)
        BEGIN
         CodeLen=2+AdrCnt; WAsmCode[0]=0xf140 | AdrMode;
         CopyAdrVals(WAsmCode+1); CheckSup();
        END
      END
     return;
    END

   if (Memo("FLUSHA"))
    BEGIN
     if (*AttrPart!='\0') WrError(1130);
     else if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       if (MomCPU>=CPU68040)
        BEGIN
         CodeLen=2; WAsmCode[0]=0xf518;
        END
       else
        BEGIN
         CodeLen=4; WAsmCode[0]=0xf000; WAsmCode[1]=0x2400;
        END
       CheckSup();
      END
     return;
    END

   if (Memo("FLUSHAN"))
    BEGIN
     if (*AttrPart!='\0') WrError(1130);
     else if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       CodeLen=2; WAsmCode[0]=0xf510;
       CheckCPU(CPU68040); CheckSup();
      END
     return;
    END

   if ((Memo("FLUSH")) OR (Memo("FLUSHS")))
    BEGIN
     if (*AttrPart!='\0') WrError(1130);
     else if (MomCPU>=CPU68040)
      BEGIN
       if (Memo("FLUSHS")) WrError(1500);
       else if (ArgCnt!=1) WrError(1110);
       else
        BEGIN
         DecodeAdr(ArgStr[1],Madri);
         if (AdrNum!=0)
          BEGIN
           WAsmCode[0]=0xf508+(AdrMode & 7);
           CodeLen=2;
           CheckSup();
          END
        END
      END
     else if ((ArgCnt!=2) AND (ArgCnt!=3)) WrError(1110);
     else if ((Memo("FLUSHS")) AND (NOT FullPMMU)) WrError(1500);
     else if (NOT DecodeFC(ArgStr[1],WAsmCode+1)) WrError(1710);
     else
      BEGIN
       OpSize=0;
       DecodeAdr(ArgStr[2],Mimm);
       if (AdrNum!=0)
        BEGIN
         if (AdrVals[0]>15) WrError(1720);
         else
          BEGIN
           WAsmCode[1]|=(AdrVals[0] << 5) | 0x3000;
           if (Memo("FLUSHS")) WAsmCode[1]|=0x400;
           WAsmCode[0]=0xf000; CodeLen=4; CheckSup();
           if (ArgCnt==3)
            BEGIN
             WAsmCode[1]|=0x800;
             DecodeAdr(ArgStr[3],Madri+Mdadri+Maix+Mabs);
             if (AdrNum==0) CodeLen=0;
             else
              BEGIN
               WAsmCode[0]|=AdrMode; CodeLen+=AdrCnt;
               CopyAdrVals(WAsmCode+2);
              END
            END
          END
        END
      END
     return;
    END

   if (Memo("FLUSHN"))
    BEGIN
     if (*AttrPart!='\0') WrError(1100);
     else if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Madri);
       if (AdrNum!=0)
        BEGIN
         WAsmCode[0]=0xf500+(AdrMode & 7);
         CodeLen=2;
         CheckCPU(CPU68040); CheckSup();
        END
      END
     return;
    END

   if (Memo("FLUSHR"))
    BEGIN
     if (*AttrPart=='\0') OpSize=3;
     if (OpSize!=3) WrError(1130);
     else if (ArgCnt!=1) WrError(1110);
     else if (NOT FullPMMU) WrError(1500);
     else
      BEGIN
       RelPos=4;
       DecodeAdr(ArgStr[1],Madri+Mpre+Mpost+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
       if (AdrNum!=0)
        BEGIN
         WAsmCode[0]=0xf000 | AdrMode; WAsmCode[1]=0xa000;
         CopyAdrVals(WAsmCode+2); CodeLen=4+AdrCnt; CheckSup();
        END
      END
     return;
    END

   if ((Memo("LOADR")) OR (Memo("LOADW")))
    BEGIN
     if (*AttrPart!='\0') WrError(1130);
     else if (ArgCnt!=2) WrError(1110);
     else if (NOT DecodeFC(ArgStr[1],WAsmCode+1)) WrError(1710);
     else
      BEGIN
       DecodeAdr(ArgStr[2],Madri+Mdadri+Maix+Mabs);
       if (AdrNum!=0)
        BEGIN
         WAsmCode[0]=0xf000 | AdrMode; WAsmCode[1]|=0x2000;
         if (Memo("LOADR")) WAsmCode[1]|=0x200;
         CodeLen=4+AdrCnt; CopyAdrVals(WAsmCode+2); CheckSup();
        END
      END
     return;
    END

   if ((Memo("MOVE")) OR (Memo("MOVEFD")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       if (DecodePMMUReg(ArgStr[1],WAsmCode+1,&z))
        BEGIN
         WAsmCode[1]|=0x200;
         if (*AttrPart=='\0') OpSize=z;
         if (OpSize!=z) WrError(1130);
         else
          BEGIN
           Mask=Madri+Mdadri+Maix+Mabs;
           if (FullPMMU)
            BEGIN
             Mask*=Mpost+Mpre;
             if (z!=3) Mask+=Mdata+Madr;
            END
           DecodeAdr(ArgStr[2],Mask);
           if (AdrNum!=0)
            BEGIN
             WAsmCode[0]=0xf000 | AdrMode;
             CodeLen=4+AdrCnt; CopyAdrVals(WAsmCode+2);
             CheckSup();
            END
          END
        END
       else if (DecodePMMUReg(ArgStr[2],WAsmCode+1,&z))
        BEGIN
         if (*AttrPart=='\0') OpSize=z;
         if (OpSize!=z) WrError(1130);
         else
          BEGIN
           RelPos=4;
           Mask=Madri+Mdadri+Maix+Mabs;
           if (FullPMMU)
            BEGIN
             Mask+=Mpost+Mpre+Mpc+Mpcidx+Mimm;
             if (z!=3) Mask+=Mdata+Madr;
            END
           DecodeAdr(ArgStr[1],Mask);
           if (AdrNum!=0)
            BEGIN
             WAsmCode[0]=0xf000 | AdrMode;
             CodeLen=4+AdrCnt; CopyAdrVals(WAsmCode+2);
             if (Memo("MOVEFD")) WAsmCode[1]+=0x100;
             CheckSup();
            END
          END
        END
       else WrError(1730);
      END
     return;
    END

   if ((Memo("TESTR")) OR (Memo("TESTW")))
    BEGIN
     if (*AttrPart!='\0') WrError(1130);
     else if (MomCPU>=CPU68040)
      BEGIN
       if (ArgCnt!=1) WrError(1110);
       else
        BEGIN
         DecodeAdr(ArgStr[1],Madri);
         if (AdrNum!=0)
          BEGIN
           WAsmCode[0]=0xf548+(AdrMode & 7)+(Ord(Memo("TESTR")) << 5);
           CodeLen=2;
           CheckSup();
          END
        END
      END
     else if ((ArgCnt>4) OR (ArgCnt<3)) WrError(1110);
     else
      BEGIN
       if (NOT DecodeFC(ArgStr[1],WAsmCode+1)) WrError(1710);
       else
        BEGIN
         DecodeAdr(ArgStr[2],Madri+Mdadri+Maix+Mabs);
         if (AdrNum!=0)
          BEGIN
           WAsmCode[0]=0xf000 | AdrMode; CodeLen=4+AdrCnt;
           WAsmCode[1]|=0x8000;
           CopyAdrVals(WAsmCode+2);
           if (Memo("TESTR")) WAsmCode[1]|=0x200;
           DecodeAdr(ArgStr[3],Mimm);
           if (AdrNum!=0)
            if (AdrVals[0]>7)
             BEGIN
              WrError(1740); CodeLen=0;
             END
            else
             BEGIN
              WAsmCode[1]|=AdrVals[0] << 10;
              if (ArgCnt==4)
               BEGIN
                DecodeAdr(ArgStr[4],Madr);
                if (AdrNum==0) CodeLen=0; else WAsmCode[1]|=AdrMode << 5; 
                CheckSup();
               END
             END
           else CodeLen=0;
          END
        END
      END
     return;
    END

   if (Memo("VALID"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (NOT FullPMMU) WrError(1500);
     else if ((*AttrPart!='\0') AND (OpSize!=2)) WrError(1130);
     else
      BEGIN
       DecodeAdr(ArgStr[2],Madri+Mdadri+Maix+Mabs);
       if (AdrNum!=0)
        BEGIN
         WAsmCode[0]=0xf000 | AdrMode;
         WAsmCode[1]=0x2800; CodeLen=4+AdrCnt;
         CopyAdrVals(WAsmCode+1);
         if (strcasecmp(ArgStr[1],"VAL")==0);
         else
          BEGIN
           DecodeAdr(ArgStr[1],Madr);
           if (AdrNum!=0)
            BEGIN
             WAsmCode[1]|=0x400 | (AdrMode & 7);
            END
           else CodeLen=0;
          END
        END
      END
     return;
    END

   if (*OpPart=='B')
    BEGIN
     for (z=0; z<PMMUCondCnt; z++)
      if (strcmp(OpPart+1,PMMUConds[z])==0) break;
     if (z==PMMUCondCnt) WrError(1360);
     else
      BEGIN
       if ((OpSize!=1) AND (OpSize!=2) AND (OpSize!=6)) WrError(1130);
       else if (ArgCnt!=1) WrError(1110);
       else if (NOT FullPMMU) WrError(1500);
       else
        BEGIN

         HVal=EvalIntExpression(ArgStr[1],Int32,&ValOK)-(EProgCounter()+2);
         HVal16=HVal;

         if (OpSize==1) OpSize=(IsDisp16(HVal))?2:6;

         if (OpSize==2)
          BEGIN
           if ((NOT IsDisp16(HVal)) AND (NOT SymbolQuestionable)) WrError(1370);
           else
            BEGIN
             CodeLen=4; WAsmCode[0]=0xf080 | z;
             WAsmCode[1]=HVal16; CheckSup();
            END
          END
         else
          BEGIN
           CodeLen=6; WAsmCode[0]=0xf0c0 | z;
           WAsmCode[2]=HVal & 0xffff; WAsmCode[1]=HVal >> 16;
           CheckSup();
          END
        END
      END
     return;
    END;

   if (strncmp(OpPart,"DB",2)==0)
    BEGIN
     for (z=0; z<PMMUCondCnt; z++)
      if (strcmp(OpPart+2,PMMUConds[z])==0) break;
     if (z==PMMUCondCnt) WrError(1360);  
     else
      BEGIN
       if ((OpSize!=1) AND (*AttrPart!='\0')) WrError(1130);
       else if (ArgCnt!=2) WrError(1110);
       else if (NOT FullPMMU) WrError(1500);
       else
        BEGIN
         DecodeAdr(ArgStr[1],Mdata);
         if (AdrNum!=0)
          BEGIN
           WAsmCode[0]=0xf048 | AdrMode; WAsmCode[1]=z;
           HVal=EvalIntExpression(ArgStr[2],Int32,&ValOK)-(EProgCounter()+4);
           if (ValOK)
            BEGIN
             HVal16=HVal; WAsmCode[2]=HVal16;
             if ((NOT IsDisp16(HVal)) AND (NOT SymbolQuestionable)) WrError(1370); 
             else CodeLen=6; 
             CheckSup();
            END
          END
        END
      END
     return;
    END

   if (*OpPart=='S')
    BEGIN
     for (z=0; z<PMMUCondCnt; z++)
      if (strcmp(OpPart+1,PMMUConds[z])==0) break;
     if (z==PMMUCondCnt) WrError(1360);  
     else
      BEGIN
       if ((OpSize!=0) AND (*AttrPart!='\0')) WrError(1130);
       else if (ArgCnt!=1) WrError(1110);
       else if (NOT FullPMMU) WrError(1500);
       else
        BEGIN
         DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
         if (AdrNum!=0)
          BEGIN
           CodeLen=4+AdrCnt; WAsmCode[0]=0xf040 | AdrMode;
           WAsmCode[1]=z; CopyAdrVals(WAsmCode+2); CheckSup();
          END
        END
      END
     return;
    END

   if (strncmp(OpPart,"TRAP",4)==0)
    BEGIN
     for (z=0; z<PMMUCondCnt; z++)
      if (strcmp(OpPart+4,PMMUConds[z])==0) break;
     if (z==PMMUCondCnt) WrError(1360);  
     else
      BEGIN
       if (*AttrPart=='\0') OpSize=0;
       if (OpSize>2) WrError(1130);
       else if (((OpSize==0) AND (ArgCnt!=0)) OR ((OpSize!=0) AND (ArgCnt!=1))) WrError(1110);
       else if (NOT FullPMMU) WrError(1500);
       else
        BEGIN
         WAsmCode[0]=0xf078; WAsmCode[1]=z;
         if (OpSize==0)
          BEGIN
           WAsmCode[0]|=4; CodeLen=4; CheckSup();
          END
         else
          BEGIN
           DecodeAdr(ArgStr[1],Mimm);
           if (AdrNum!=0)
            BEGIN
             WAsmCode[0]|=(OpSize+1);
             CopyAdrVals(WAsmCode+2); CodeLen=4+AdrCnt; CheckSup();
            END
          END
        END
      END
     return;
    END

   WrError(1200);
END

/*-------------------------------------------------------------------------*/

        static void MakeCode_68K(void)
BEGIN
   Boolean ValOK;
   LongInt HVal;
   Integer i,HVal16;
   ShortInt HVal8;
   Word w1,w2;

   CodeLen=0;
   OpSize=(MomCPU==CPUCOLD) ? 2 : 1;
   DontPrint=False; RelPos=2;

   if (*AttrPart!='\0')
    switch (toupper(*AttrPart))
     BEGIN
      case 'B': OpSize=0; break;
      case 'W': OpSize=1; break;
      case 'L': OpSize=2; break;
      case 'Q': OpSize=3; break;
      case 'S': OpSize=4; break;
      case 'D': OpSize=5; break;
      case 'X': OpSize=6; break;
      case 'P': OpSize=7; break;
      default: WrError(1107); return;
     END

   /* Nullanweisung */

   if ((*OpPart=='\0') AND (*AttrPart=='\0') AND (ArgCnt==0)) return;

   /* Pseudoanweisungen */

   if (DecodeMoto16Pseudo(OpSize,True)) return;

   if (DecodePseudo()) return;

   /* Befehlszaehler ungerade ? */

   if (Odd(EProgCounter())) WrError(180);

   /* Befehlserweiterungen */

   if ((*OpPart=='F') AND (FPUAvail))
    BEGIN
     strcpy(OpPart,OpPart+1);
     DecodeFPUOrders();
     return;
    END

   if ((*OpPart=='P') AND (NOT Memo("PEA")) AND (PMMUAvail))
    BEGIN
     strcpy(OpPart,OpPart+1);
     DecodePMMUOrders();
     return;
    END

   /* vermischtes */

   if (LookupInstTable(InstTable,OpPart)) return;

   /* Anweisungen mit konstantem Argument */

   /* zwei Operanden */

   if ((strncmp(OpPart,"MUL",3)==0) OR (strncmp(OpPart,"DIV",3)==0)) 
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if ((MomCPU==CPUCOLD) AND (*OpPart=='D')) WrError(1500);
     else if (OpSize==1) 
      BEGIN
       DecodeAdr(ArgStr[2],Mdata);
       if (AdrNum!=0)
        BEGIN
         WAsmCode[0]=0x80c0 | (AdrMode << 9);
         if (strncmp(OpPart,"MUL",3)==0) WAsmCode[0]|=0x4000;
         if (OpPart[3]=='S') WAsmCode[0]|=0x100;
         DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
         if (AdrNum!=0)
          BEGIN
           WAsmCode[0]|=AdrMode;
           CodeLen=2+AdrCnt; CopyAdrVals(WAsmCode+1);
          END
        END
      END
     else if (OpSize==2)
      BEGIN
       if (strchr(ArgStr[2],':')==Nil)
        BEGIN
         strcpy(ArgStr[3], ArgStr[2]);
         strcat(ArgStr[2], ":");
         strcat(ArgStr[2], ArgStr[3]);
        END
       if (NOT CodeRegPair(ArgStr[2],&w1,&w2)) WrXError(1760, ArgStr[2]);
       else
        BEGIN
         WAsmCode[1]=w1+(w2 << 12); RelPos=4;
         if (w1!=w2) WAsmCode[1]|=0x400;
         if (OpPart[3]=='S') WAsmCode[1]|=0x800;
         DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
         if (AdrNum!=0)
          BEGIN
           WAsmCode[0]=0x4c00+AdrMode;
           if (strncmp(OpPart,"DIV",3)==0) WAsmCode[0]|=0x40;
           CopyAdrVals(WAsmCode+2); CodeLen=4+AdrCnt;
           CheckCPU((w1!=w2) ? CPU68332 : CPUCOLD);
          END
        END
      END
     else WrError(1130);
     return;
    END

   /* bedingte Befehle */

   if ((strlen(OpPart)<=3) AND (*OpPart=='B')) 
    BEGIN
     /* .W, .S, .L, .X erlaubt */

     if ((OpSize!=1) AND (OpSize!=2) AND (OpSize!=4) AND (OpSize!=6)) 
      WrError(1130);

     /* nur ein Operand erlaubt */

     else if (ArgCnt!=1) WrError(1110);
     else
      BEGIN

       /* Bedingung finden, evtl. meckern */

       if (NOT LookupInstTable(CInstTable,OpPart+1)) WrError(1360);
       else
        BEGIN

         /* Zieladresse ermitteln, zum Programmzaehler relativieren */

         HVal=EvalIntExpression(ArgStr[1],Int32,&ValOK);
         HVal=HVal-(EProgCounter()+2);

         /* Bei Automatik Groesse festlegen */

         if (OpSize==1) 
          BEGIN
           if (IsDisp8(HVal)) OpSize=4;
           else if (IsDisp16(HVal)) OpSize=2;
           else OpSize=6;
          END

         if (ValOK)
          BEGIN

           /* 16 Bit ? */

           if (OpSize==2)
            BEGIN

             /* zu weit ? */

             HVal16=HVal;
             if ((NOT IsDisp16(HVal)) AND (NOT SymbolQuestionable)) WrError(1370);
             else
              BEGIN

               /* Code erzeugen */

               CodeLen=4; WAsmCode[0]=0x6000 | (CondIndex << 8);
               WAsmCode[1]=HVal16;
              END
            END

           /* 8 Bit ? */

           else if (OpSize==4)
            BEGIN

             /* zu weit ? */

             HVal8=HVal;
             if ((NOT IsDisp8(HVal)) AND (NOT SymbolQuestionable)) WrError(1370);

             /* Code erzeugen */
             else
              BEGIN
               CodeLen=2;
               if (HVal8!=0)
                BEGIN
                 WAsmCode[0]=0x6000 | (CondIndex << 8) | ((Byte)HVal8);
                END
               else
                BEGIN
                 WAsmCode[0]=NOPCode;
                 if ((NOT Repass) AND (*AttrPart!='\0')) WrError(60);
                END
              END
            END

           /* 32 Bit ? */

           else
            BEGIN
             CodeLen=6; WAsmCode[0]=0x60ff | (CondIndex << 8);
             WAsmCode[1]=HVal >> 16; WAsmCode[2]=HVal & 0xffff;
             CheckCPU(CPU68332);
            END
          END
        END
       return;
      END
    END

   if ((strlen(OpPart)<=3) AND (*OpPart=='S')) 
    BEGIN
     if ((*AttrPart!='\0') AND (OpSize!=0)) WrError(1130);
     else if (ArgCnt!=1) WrError(1130);
     else
      BEGIN
       i=FindICondition(OpPart+1);
       if (i>=CondCnt-2) WrError(1360);
       else
        BEGIN
         OpSize=0;
         if (MomCPU==CPUCOLD) DecodeAdr(ArgStr[1],Mdata);
         else DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
         if (AdrNum!=0)
          BEGIN
           WAsmCode[0]=0x50c0 | (CondVals[i] << 8) | AdrMode;
           CodeLen=2+AdrCnt; CopyAdrVals(WAsmCode+1);
          END
        END
      END
     return;
    END

   if ((strlen(OpPart)<=4) AND (strncmp(OpPart,"DB",2)==0)) 
    BEGIN
     if (OpSize!=1) WrError(1130);
     else if (ArgCnt!=2) WrError(1110);
     else if (MomCPU==CPUCOLD) WrError(1500);
     else
      BEGIN
       i=FindICondition(OpPart+2);
       if (i==18) i=1;
       if (i>=CondCnt-1) WrError(1360);
       else
        BEGIN
         HVal=EvalIntExpression(ArgStr[2],Int32,&ValOK);
         if (ValOK)
          BEGIN
           HVal-=(EProgCounter()+2); HVal16=HVal;
           if ((NOT IsDisp16(HVal)) AND (NOT SymbolQuestionable)) WrError(1370);
           else
            BEGIN
             CodeLen=4; WAsmCode[0]=0x50c8 | (CondVals[i] << 8);
             WAsmCode[1]=HVal16;
             DecodeAdr(ArgStr[1],Mdata);
             if (AdrNum==1) WAsmCode[0]|=AdrMode;
             else CodeLen=0;
            END
          END
        END
       return;
      END
    END

   if ((strlen(OpPart)<=6) AND (strncmp(OpPart,"TRAP",4)==0)) 
    BEGIN
     if (*AttrPart=='\0') OpSize=0;
     i=(OpSize==0) ? 0 : 1;
     if (OpSize>2) WrError(1130);
     else if (ArgCnt!=i) WrError(1110);
     else
      BEGIN
       i=FindICondition(OpPart+4);
       if ((i==18) OR (i>=CondCnt-2)) WrError(1360);
       else if ((MomCPU==CPUCOLD) AND (i!=1)) WrError(1500);
       else
        BEGIN
         WAsmCode[0]=0x50f8+(i << 8);
         if (OpSize==0)
          BEGIN
           WAsmCode[0]+=4; CodeLen=2;
          END
         else
          BEGIN
           DecodeAdr(ArgStr[1],Mimm);
           if (AdrNum!=0)
            BEGIN
             WAsmCode[0]+=OpSize+1;
             CopyAdrVals(WAsmCode+1); CodeLen=2+AdrCnt;
            END
          END
         CheckCPU(CPUCOLD);
        END
      END
     return;
    END

   /* unbekannter Befehl */

   WrXError(1200,OpPart);
END

        static void InitCode_68K(void)
BEGIN
   SaveInitProc();
   SetFlag(&PMMUAvail,PMMUAvailName,False);
   SetFlag(&FullPMMU,FullPMMUName,True);
END

        static Boolean IsDef_68K(void)
BEGIN
   return False;
END

        static void SwitchFrom_68K(void)
BEGIN
   DeinitFields(); ClearONOFF();
END

        static void SwitchTo_68K(void)
BEGIN
   TurnWords=True; ConstMode=ConstModeMoto; SetIsOccupied=False;

   PCSymbol="*"; HeaderID=0x01; NOPCode=0x4e71;
   DivideChars=","; HasAttrs=True; AttrChars=".";

   ValidSegs=(1<<SegCode);
   Grans[SegCode]=1; ListGrans[SegCode]=2; SegInits[SegCode]=0;
#ifdef __STDC__
   SegLimits[SegCode] = 0xfffffffful;
#else
   SegLimits[SegCode] = 0xffffffffl;
#endif

   MakeCode=MakeCode_68K; IsDef=IsDef_68K;

   SwitchFrom=SwitchFrom_68K; InitFields();
   AddONOFF("PMMU"    , &PMMUAvail , PMMUAvailName, False);
   AddONOFF("FULLPMMU", &FullPMMU  , FullPMMUName , False);
   AddONOFF("FPU"     , &FPUAvail  , FPUAvailName , False);
   AddONOFF("SUPMODE" , &SupAllowed, SupAllowedName,False);
   AddMoto16PseudoONOFF();

   SetFlag(&FullPMMU,FullPMMUName,MomCPU<=CPU68020);
   SetFlag(&DoPadding,DoPaddingName,True);
END

#ifdef DEBSTRCNT
        static void wrstrcnt(void)
BEGIN
   printf("\n%d comps\n",strcnt);
END
#endif

        void code68k_init(void)
BEGIN
   CPU68008=AddCPU("68008",SwitchTo_68K);
   CPU68000=AddCPU("68000",SwitchTo_68K);
   CPU68010=AddCPU("68010",SwitchTo_68K);
   CPU68012=AddCPU("68012",SwitchTo_68K);
   CPUCOLD =AddCPU("MCF5200",SwitchTo_68K);
   CPU68332=AddCPU("68332",SwitchTo_68K);
   CPU68340=AddCPU("68340",SwitchTo_68K);
   CPU68360=AddCPU("68360",SwitchTo_68K);
   CPU68020=AddCPU("68020",SwitchTo_68K);
   CPU68030=AddCPU("68030",SwitchTo_68K);
   CPU68040=AddCPU("68040",SwitchTo_68K);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_68K;
#ifdef DEBSTRCNT
   atexit(wrstrcnt);
#endif
END

