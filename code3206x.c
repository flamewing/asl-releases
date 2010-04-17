/* code3206x.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator TMS320C6x                                                   */
/*                                                                           */
/* Historie: 24. 2.1997 Grundsteinlegung                                     */
/*           22. 5.1998 Schoenheitsoperatioenen fuer K&R-Compiler            */
/*            3. 1.1999 ChkPC-Anpassung                                      */
/*           23. 1.1999 DecodeCtrlReg jetzt mit unsigned-Ergebnis            */
/*           30. 1.1999 Formate maschinenunabhaengig gemacht                 */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*           14. 1.2001 silenced warnings about unused parameters            */
/*           2001-11-19 B fix (input from Johannes)                          */
/*           2001-11-26 scaling fix (input from Johannes)                    */
/*                                                                           */
/*****************************************************************************/
/* $Id: code3206x.c,v 1.7 2010/04/17 13:14:19 alfred Exp $                   */
/***************************************************************************** 
 * $Log: code3206x.c,v $
 * Revision 1.7  2010/04/17 13:14:19  alfred
 * - address overlapping strcpy()
 *
 * Revision 1.6  2008/11/23 10:39:16  alfred
 * - allow strings with NUL characters
 *
 * Revision 1.5  2007/11/24 22:48:03  alfred
 * - some NetBSD changes
 *
 * Revision 1.4  2005/09/08 16:53:40  alfred
 * - use common PInstTable
 *
 * Revision 1.3  2005/05/21 16:35:04  alfred
 * - removed variables available globally
 *
 * Revision 1.2  2003/12/07 14:01:16  alfred
 * - added missing static defs
 *
 * Revision 1.1  2003/11/06 02:49:19  alfred
 * - recreated
 *
 * Revision 1.8  2003/05/02 21:23:10  alfred
 * - strlen() updates
 *
 * Revision 1.7  2002/08/14 18:43:48  alfred
 * - warn null allocation, remove some warnings
 *
 * Revision 1.6  2002/07/14 18:39:58  alfred
 * - fixed TempAll-related warnings
 *
 * Revision 1.5  2002/05/12 20:57:58  alfred
 * - error msg 20000 -> 2009
 *
 * Revision 1.3  2002/05/12 13:59:47  alfred
 * - added pseudo instructions
 *
 * Revision 1.2  2002/05/11 16:47:22  alfred
 * - added pseudo instructions
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>
#include "endian.h"

#include "strutil.h"
#include "bpemu.h"
#include "nls.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmcode.h"
#include "codepseudo.h"
#include "asmitree.h"
#include "codevars.h"
#include "nlmessages.h"
#include "as.rsc"

/*---------------------------------------------------------------------------*/

typedef enum {NoUnit,L1,L2,S1,S2,M1,M2,D1,D2,LastUnit,UnitCnt} TUnit;

typedef struct
         {
          LongInt OpCode;
          LongInt SrcMask,SrcMask2,DestMask;
          Byte CrossUsed; /* Bit 0 -->X1 benutzt, Bit 1 -->X2 benutzt */
          Byte AddrUsed;  /* Bit 0 -->Addr1 benutzt, Bit 1 -->Addr2 benutzt
                             Bit 2 -->LdSt1 benutzt, Bit 3 -->LdSt2 benutzt */
          Byte LongUsed;  /* Bit 0 -->lange Quelle, Bit 1-->langes Ziel */
          Boolean AbsBranch; 
          Boolean StoreUsed,LongSrc,LongDest;
          TUnit U;
         } InstrRec;

typedef struct
         {
          char *Name;
          LongInt Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          LongInt Code;
          LongInt Scale;
         } MemOrder;

typedef struct
         {
          char *Name;
          LongInt Code;
          Boolean DSign,SSign1,SSign2;
          Boolean MayImm;
         } MulOrder;

typedef struct
         {
          char *Name;
          LongInt Code;
          Boolean Rd,Wr;
         } CtrlReg;

static char *UnitNames[UnitCnt]={"  ","L1","L2","S1","S2","M1","M2","D1","D2","  "};
#define MaxParCnt 8
#define FirstUnit L1

#define LinAddCnt 6
#define CmpCnt 5
#define MemCnt 8
#define MulCnt 20
#define CtrlCnt 13

#define ModNone (-1)
#define ModReg 0
#define MModReg (1 << ModReg)
#define ModLReg 1
#define MModLReg (1 << ModLReg)
#define ModImm 2
#define MModImm (1 << ModImm)

static ShortInt AdrMode;

static CPUVar CPU32060;

static Boolean ThisPar, ThisCross, ThisStore, ThisAbsBranch;
static Byte ThisAddr,ThisLong;
static LongInt ThisSrc,ThisSrc2,ThisDest;
static LongInt Condition;
static TUnit ThisUnit;
static LongWord UnitFlag,ThisInst;
static Integer ParCnt;
static LongWord PacketAddr;

static InstrRec ParRecs[MaxParCnt];

static FixedOrder *LinAddOrders;
static FixedOrder *CmpOrders;
static MemOrder *MemOrders;
static MulOrder *MulOrders;
static CtrlReg *CtrlRegs;

/*-------------------------------------------------------------------------*/

        static Boolean CheckOpt(char *Asc)
BEGIN
   Boolean Flag,erg=True;
   int l=strlen(Asc);

   if (strcmp(Asc,"||")==0) ThisPar=True;
   else if ((*Asc=='[') AND (Asc[l-1]==']'))
    BEGIN
     Asc++; Asc[l-2]='\0'; l-=2;
     if (*Asc=='!')
      BEGIN
       Asc++; l--; Condition=1;
      END
     else Condition=0;
     Flag=True;
     if (l!=2) Flag=False;
     else if (mytoupper(*Asc)=='A')
      if ((Asc[1]>='1') AND (Asc[1]<='2')) Condition+=(Asc[1]-'0'+3) << 1;
      else Flag=False;
     else if (mytoupper(*Asc)=='B')
      BEGIN
       if ((Asc[1]>='0') AND (Asc[1]<='2')) Condition+=(Asc[1]-'0'+1) << 1;
       else Flag=False;
      END
     if (NOT Flag) WrXError(1445,Asc); erg=Flag;
    END
   else erg=False;

   return erg;
END

        static Boolean ReiterateOpPart(void)
BEGIN
   char *p;
   int z;

   if (NOT CheckOpt(OpPart)) return False;

   if (ArgCnt<1)
    BEGIN
     WrError(1210); return False;
    END
   p=FirstBlank(ArgStr[1]);
   if (p==Nil)
    BEGIN
     strcpy(OpPart,ArgStr[1]);
     for (z=2; z<=ArgCnt; z++) strcpy(ArgStr[z-1],ArgStr[z]);
     ArgCnt--;
    END
   else
    BEGIN
     *p='\0'; strcpy(OpPart,ArgStr[1]); strcpy(ArgStr[1],p+1);
     KillPrefBlanks(ArgStr[1]);
    END
   NLS_UpString(OpPart);
   p=strchr(OpPart,'.');
   if (p==Nil) *AttrPart='\0';
   else
    BEGIN
     strcpy(AttrPart,p+1);
     *p='\0';
    END;
   return True;
END

/*-------------------------------------------------------------------------*/

        static void AddSrc(LongWord Reg)
BEGIN
   LongWord Mask=1 << Reg;

   if ((ThisSrc & Mask)==0) ThisSrc|=Mask;
   else ThisSrc2|=Mask;
END

        static void AddLSrc(LongWord Reg)
BEGIN
   AddSrc(Reg); AddSrc(Reg+1);
   ThisLong|=1;
END

        static void AddDest(LongWord Reg)
BEGIN
   ThisDest|=(1 << Reg);
END

        static void AddLDest(LongWord Reg)
BEGIN
   ThisDest|=(3 << Reg);
   ThisLong|=2;
END

        static LongInt FindReg(LongInt Mask)
BEGIN
   int z;

   for (z=0; z<32; z++)
    BEGIN
     if ((Mask&1)!=0) break;
     Mask=Mask >> 1;
    END
   return z;
END

        static char *RegName(LongInt Num)
BEGIN
   static char s[5];

   Num&=31;
   sprintf(s, "%c%ld", 'A' + (Num >> 4), (long) (Num & 15));
   return s;
END

        static Boolean DecodeSReg(char *Asc, LongWord *Reg, Boolean Quarrel)
BEGIN
   char *end;
   Byte RVal;
   Boolean TFlag;

   TFlag=True;
   if (mytoupper(*Asc)=='A') *Reg=0;
   else if (mytoupper(*Asc)=='B') *Reg=16;
   else TFlag=False;
   if (TFlag)
    BEGIN
     RVal=strtol(Asc+1,&end,10);
     if (*end!='\0') TFlag=False;
     else if (RVal>15) TFlag=False;
     else *Reg+=RVal;
    END
   if ((NOT TFlag) AND (Quarrel)) WrXError(1445,Asc);
   return TFlag;
END

        static Boolean DecodeReg(char *Asc, LongWord *Reg, Boolean *PFlag, Boolean Quarrel)
BEGIN
   char *p;
   LongWord NextReg;

   p=strchr(Asc,':');
   if (p==0)
    BEGIN
     *PFlag=False; return DecodeSReg(Asc,Reg,Quarrel);
    END
   else
    BEGIN
     *PFlag=True; *p='\0';
     if (NOT DecodeSReg(Asc,&NextReg,Quarrel)) return False;
     else if (NOT DecodeSReg(p+1,Reg,Quarrel)) return False;
     else if ((Odd(*Reg)) OR (NextReg!=(*Reg)+1) OR ((((*Reg) ^ NextReg) & 0x10)!=0))
      BEGIN
       if (Quarrel) WrXError(1760,Asc); return False;
      END
     else return True;
    END
END

        static Boolean DecodeCtrlReg(char *Asc, LongWord *Erg, Boolean Write)
BEGIN
   int z;

   for (z=0; z<CtrlCnt; z++)
    if (strcasecmp(Asc,CtrlRegs[z].Name)==0)
     BEGIN
      *Erg=CtrlRegs[z].Code;
      return (Write AND CtrlRegs[z].Wr) OR ((NOT Write) AND CtrlRegs[z].Rd);
     END
   return False;
END

/* Was bedeutet das r-Feld im Adressoperanden mit kurzem Offset ???
   und wie ist das genau mit der Skalierung gemeint ??? */

        static Boolean DecodeMem(char *Asc, LongWord *Erg, LongWord Scale)
BEGIN
   String RegPart,DispPart;
   LongInt DispAcc,Mode;
   LongWord BaseReg,IndReg;
   int l;
   char Counter;
   char *p;
   Boolean OK;

   /* das muss da sein */

   if (*Asc!='*')
    BEGIN
     WrError(1350); return False;
    END;
   Asc++;

   /* teilen */

   p=strchr(Asc,'['); Counter=']';
   if (p==Nil)
    BEGIN
     p=strchr(Asc,'('); Counter=')';
    END
   if (p!=Nil)
    BEGIN
     if (Asc[strlen(Asc)-1]!=Counter)
      BEGIN
       WrError(1350); return False;
      END
     Asc[strlen(Asc)-1]='\0'; *p='\0';
     strmaxcpy(RegPart,Asc,255); strmaxcpy(DispPart,p+1,255);
    END
   else
    BEGIN
     strcpy(RegPart,Asc); *DispPart='\0';
    END

   /* Registerfeld entschluesseln */

   l=strlen(RegPart);
   Mode=1; /* Default ist *+R */
   if (*RegPart=='+')
    BEGIN
     strmov(RegPart,RegPart+1); Mode=1;
     if (*RegPart=='+')
      BEGIN
       strmov(RegPart,RegPart+1); Mode=9;
      END
    END
   else if (*RegPart=='-')
    BEGIN
     strmov(RegPart,RegPart+1); Mode=0;
     if (*RegPart=='-')
      BEGIN
       strmov(RegPart,RegPart+1); Mode=8;
      END
    END
   else if (RegPart[l-1]=='+')
    BEGIN
     if (RegPart[l-2]!='+')
      BEGIN
       WrError(1350); return False;
      END
     RegPart[l-2]='\0'; Mode=11;
    END
   else if (RegPart[l-1]=='-')
    BEGIN
     if (RegPart[l-2]!='-')
      BEGIN
       WrError(1350); return False;
      END
     RegPart[l-2]='\0'; Mode=10;
    END
   if (NOT DecodeSReg(RegPart,&BaseReg,False))
    BEGIN
     WrXError(1445,RegPart); return False;
    END
   AddSrc(BaseReg);

   /* kein Offsetfeld ? --> Skalierungsgroesse bei Autoinkrement/De-
      krement, sonst 0 */

   if (*DispPart=='\0')
    DispAcc=(Mode<2) ? 0 : Scale;

   /* Register als Offsetfeld? Dann Bit 2 in Modus setzen */

   else if (DecodeSReg(DispPart,&IndReg,False))
    BEGIN
     if ((IndReg ^ BaseReg)>15)
      BEGIN
       WrError(1350); return False;
      END
     Mode+=4; AddSrc(DispAcc=IndReg);
    END

   /* ansonsten normaler Offset */

   else
    BEGIN
     FirstPassUnknown=False;
     DispAcc=EvalIntExpression(DispPart,UInt15,&OK);
     if (NOT OK) return False;
     if (FirstPassUnknown) DispAcc &= 7;
     if (Counter == ')')
      BEGIN
       if (DispAcc % Scale != 0)
        BEGIN
         WrError(1325); return False;
        END
       else
        DispAcc /= Scale;
      END
    END

   /* Benutzung des Adressierers markieren */

   ThisAddr|=(BaseReg>15) ? 2 : 1;

   /* Wenn Offset>31, muessen wir Variante 2 benutzen */

   if (((Mode & 4)==0) AND (DispAcc>31))
    if ((BaseReg<0x1e) OR (Mode!=1)) WrError(1350);
    else
     BEGIN
      *Erg=((DispAcc & 0x7fff) << 8)+((BaseReg & 1) << 7)+12;
      return True;
     END

   else
    BEGIN
     *Erg=(BaseReg << 18)+((DispAcc & 0x1f) << 13)+(Mode << 9)
         +((BaseReg & 0x10) << 3)+4;
     return True;
    END

   return False;
END

        static Boolean DecodeAdr(char *Asc, Byte Mask, Boolean Signed, LongWord *AdrVal)
BEGIN
   Boolean OK;

   AdrMode=ModNone;

   if (DecodeReg(Asc,AdrVal,&OK,False))
    BEGIN
     AdrMode=(OK) ? ModLReg : ModReg;
    END
   else
    BEGIN
     if (Signed) *AdrVal=EvalIntExpression(Asc,SInt5,&OK) & 0x1f;
     else *AdrVal=EvalIntExpression(Asc,UInt5,&OK);
     if (OK) AdrMode=ModImm;
    END

   if ((AdrMode!=ModNone) AND (((1 << AdrMode) AND Mask)==0))
    BEGIN
     WrError(1350); AdrMode=ModNone; return False;
    END
   else return True;
END

        static Boolean ChkUnit(LongWord Reg, TUnit U1, TUnit U2)
BEGIN
   UnitFlag=Ord(Reg>15);
   if (ThisUnit==NoUnit)
    BEGIN
     ThisUnit=(Reg>15) ? U2 : U1;
     return True;
    END
   else if (((ThisUnit==U1) AND (Reg<16)) OR ((ThisUnit==U2) AND (Reg>15))) return True;
   else
    BEGIN
     WrError(1107); return False;
    END
END

        static TUnit UnitCode(char c)
BEGIN
   switch (c)
    BEGIN
     case 'L': return L1;
     case 'S': return S1;
     case 'D': return D1;
     case 'M': return M1;
     default: return NoUnit;
   END
END

        static Boolean UnitUsed(TUnit TestUnit)
BEGIN
   Integer z;

   for (z=0; z<ParCnt; z++)
    if (ParRecs[z].U==TestUnit) return True;

   return False;
END

        static Boolean IsCross(LongWord Reg)
BEGIN
   return (Reg >> 4)!=UnitFlag;
END

        static void SetCross(LongWord Reg)
BEGIN
   ThisCross=((Reg >> 4)!=UnitFlag);
END

        static Boolean DecideUnit(LongWord Reg, char *Units)
BEGIN
   Integer z;
   TUnit TestUnit;

   if (ThisUnit==NoUnit)
    BEGIN
     z=0;
     while ((Units[z]!='\0') AND (ThisUnit==NoUnit))
      BEGIN
       TestUnit=UnitCode(Units[z]);
       if (Reg>=16) TestUnit++;
       if (NOT UnitUsed(TestUnit)) ThisUnit=TestUnit;
       z++;
      END
     if (ThisUnit==NoUnit)
      BEGIN
       ThisUnit=UnitCode(*Units);
       if (Reg>16) TestUnit++;
      END
    END
   UnitFlag=(ThisUnit-FirstUnit) & 1;
   if (IsCross(Reg))
    BEGIN
     WrError(1107); return False;
    END
   else return True;
END

        static void SwapReg(LongWord *r1, LongWord *r2)
BEGIN
   LongWord tmp;

   tmp=(*r1); *r1=(*r2); *r2=tmp;
END

        static Boolean DecodePseudo(void)
BEGIN
   Boolean OK;
   int z, cnt;
   TempResult t;
   LongInt Size;

   if (Memo("SINGLE"))
   {
     if (ArgCnt == 0) WrError(1110);
     else
     {
       OK = True;
       for (z = 0; z < ArgCnt; z++)
       {
         t.Contents.Float = EvalFloatExpression(ArgStr[z + 1], Float32, &OK);
         if (!OK)
           break;
         Double_2_ieee4(t.Contents.Float, (Byte *) (DAsmCode + z), BigEndian);
       }
       if (OK) CodeLen = ArgCnt << 2;
      END
     return True;
   }

   if (Memo("DOUBLE"))
   {
     if (ArgCnt == 0) WrError(1110);
     else
     {
       int z2;

       OK = True;
       for (z = 0; z < ArgCnt; z++)
       {
         z2 = z << 1;
         t.Contents.Float = EvalFloatExpression(ArgStr[z + 1], Float64, &OK);
         if (!OK)
           break;
         Double_2_ieee8(t.Contents.Float, (Byte *) (DAsmCode + z2), BigEndian);
         if (!BigEndian)
         {
           DAsmCode[z2 + 2] = DAsmCode[z2 + 0];
           DAsmCode[z2 + 0] = DAsmCode[z2 + 1];
           DAsmCode[z2 + 1] = DAsmCode[z2 + 2];
         }
       }
       if (OK) CodeLen = ArgCnt << 3;
      END
     return True;
   }

   if (Memo("DATA"))
   {
     if (ArgCnt == 0) WrError(1110);
     else
     {
       OK = True; cnt = 0;
       for (z = 1; z <= ArgCnt; z++)
        if (OK)
        {
          EvalExpression(ArgStr[z], &t);
          switch (t.Typ)
           BEGIN
            case TempInt:
#ifdef HAS64
             if (NOT RangeCheck(t.Contents.Int, Int32))
              BEGIN
               OK = False; WrError(1320);
              END
             else
#endif
              DAsmCode[cnt++] = t.Contents.Int;
             break;
            case TempFloat:
             if (NOT FloatRangeCheck(t.Contents.Float, Float32))
              BEGIN
               OK=False; WrError(1320);
              END
             else
              BEGIN
               Double_2_ieee4(t.Contents.Float, (Byte *) (DAsmCode + cnt), BigEndian);
               cnt++;
              END
             break;
            case TempString:
            {
              unsigned z2;

              for (z2 = 0; z2 < t.Contents.Ascii.Length; z2++)
              {
                if ((z2 & 3) == 0) DAsmCode[cnt++] = 0;
                DAsmCode[cnt - 1] +=
                   (((LongWord)CharTransTable[((usint)t.Contents.Ascii.Contents[z2]) & 0xff])) << (8 * (3 - (z2 & 3)));
              }
              break;
            }
            default:
             OK = False;
           END
        }
       if (OK) CodeLen = cnt << 2;
     }
     return True;
   }

   if (Memo("BSS"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       FirstPassUnknown=False;
       Size=EvalIntExpression(ArgStr[1],UInt24,&OK);
       if (FirstPassUnknown) WrError(1820);
       if ((OK) AND (NOT FirstPassUnknown))
        BEGIN
         DontPrint=True;
         if (!Size) WrError(290);
         CodeLen=Size;
         BookKeeping();
        END
      END
     return True;
    END

   return False;
END


        static Boolean CodeL(LongWord OpCode,LongWord Dest,LongWord Src1,LongWord Src2)
BEGIN
   ThisInst=0x18+(OpCode << 5)+(UnitFlag << 1)+(Ord(ThisCross) << 12)
                +(Dest << 23)+(Src2 << 18)+(Src1 << 13);
   return True;
END

        static Boolean CodeM(LongWord OpCode, LongWord Dest, LongWord Src1, LongWord Src2)
BEGIN
   ThisInst=0x00+(OpCode << 7)+(UnitFlag << 1)+(Ord(ThisCross) << 12)
                +(Dest << 23)+(Src2 << 18)+(Src1 << 13);
   return True;
END

        static Boolean CodeS(LongWord OpCode, LongWord Dest, LongWord Src1, LongWord Src2)
BEGIN
   ThisInst=0x20+(OpCode << 6)+(UnitFlag << 1)+(Ord(ThisCross) << 12)
                +(Dest << 23)+(Src2 << 18)+(Src1 << 13);
   return True;
END

        static Boolean CodeD(LongWord OpCode, LongWord Dest, LongWord Src1, LongWord Src2)
BEGIN
   ThisInst=0x40+(OpCode << 7)+(UnitFlag << 1)
                +(Dest << 23)+(Src2 << 18)+(Src1 << 13);
   return True;
END

/*-------------------------------------------------------------------------*/

static Boolean __erg;

        static void DecodeIDLE(Word Index)
BEGIN
   UNUSED(Index);

   if (ArgCnt!=0) WrError(1110);
   else if ((ThisCross) OR (ThisUnit!=NoUnit)) WrError(1107);
   else
    BEGIN
     ThisInst=0x0001e000; __erg=True;
    END
END

        static void DecodeNOP(Word Index)
BEGIN
   LongInt Count;
   Boolean OK;
   UNUSED(Index);

   if ((ArgCnt!=0) AND (ArgCnt!=1)) WrError(1110);
   else if ((ThisCross) OR (ThisUnit!=NoUnit)) WrError(1107);
   else
    BEGIN
     if (ArgCnt==0)
      BEGIN
       OK=True; Count=0;
      END
     else
      BEGIN
       FirstPassUnknown=False;
       Count=EvalIntExpression(ArgStr[1],UInt4,&OK);
       if (FirstPassUnknown) Count=0; else Count--;
       OK=ChkRange(Count,0,8);
      END
     if (OK)
      BEGIN
       ThisInst=Count << 13; __erg=True;
      END
    END
END

        static void DecodeMul(Word Index)
BEGIN
   LongWord DReg,S1Reg,S2Reg;
   MulOrder *POrder=MulOrders+Index;

   if (ArgCnt!=3) WrError(1110);
   else
    BEGIN
     if (DecodeAdr(ArgStr[3],MModReg,POrder->DSign,&DReg))
      if (ChkUnit(DReg,M1,M2))
       BEGIN
        if (DecodeAdr(ArgStr[2],MModReg,POrder->SSign2,&S2Reg))
         BEGIN
          AddSrc(S2Reg);
          DecodeAdr(ArgStr[1],(POrder->MayImm?MModImm:0)+MModReg,
                    POrder->SSign1,&S1Reg);
          switch (AdrMode)
           BEGIN
            case ModReg:
             if ((ThisCross) AND (NOT IsCross(S2Reg)) AND (NOT IsCross(S1Reg))) WrError(1350);
             else if ((IsCross(S2Reg)) AND (IsCross(S1Reg))) WrError(1350);
             else
              BEGIN
               if (IsCross(S1Reg)) SwapReg(&S1Reg,&S2Reg);
               SetCross(S2Reg);
               AddSrc(S1Reg);
               __erg=CodeM(POrder->Code,DReg,S1Reg,S2Reg);
              END
             break;
            case ModImm:
             if (Memo("MPY")) __erg=CodeM(POrder->Code-1,DReg,S1Reg,S2Reg);
             else __erg=CodeM(POrder->Code+3,DReg,S1Reg,S2Reg);
             break;
           END
         END
       END
    END
END

        static void DecodeMemO(Word Index)
BEGIN
   LongWord DReg,S1Reg;
   MemOrder *POrder=MemOrders+Index;
   Boolean OK,IsStore;

   if (ArgCnt!=2) WrError(1110);
   else
    BEGIN
     IsStore=(*OpPart)=='S';
     if (IsStore)
      BEGIN
       strcpy(ArgStr[3],ArgStr[1]); strcpy(ArgStr[1],ArgStr[2]);
       strcpy(ArgStr[2],ArgStr[3]);
       ThisStore=True;
      END
     if (DecodeAdr(ArgStr[2],MModReg,False,&DReg))
      BEGIN
       if (IsStore) AddSrc(DReg);
       ThisAddr|=(DReg>15) ? 8 : 4;
       /* Zielregister 4 Takte verzoegert, nicht als Dest eintragen */
       OK=DecodeMem(ArgStr[1],&S1Reg,POrder->Scale);
       if (OK)
        BEGIN
         if ((S1Reg & 8)==0) OK=ChkUnit((S1Reg >> 18) & 31,D1,D2);
         else OK=ChkUnit(0x1e,D1,D2);
        END
       if (OK)
        BEGIN
         ThisInst=S1Reg+(DReg << 23)+(POrder->Code << 4)
                 +((DReg & 16) >> 3);
         __erg=True;
        END
      END;
    END
END

        static void DecodeSTP(Word Index)
BEGIN
   LongWord S2Reg;
   UNUSED(Index);

   if (ArgCnt!=1) WrError(1110);
   else if (ChkUnit(0x10,S1,S2))
    BEGIN
     if (DecodeAdr(ArgStr[1],MModReg,False,&S2Reg))
      BEGIN
       if ((ThisCross) OR (S2Reg<16)) WrError(1110);
       else
        BEGIN
         AddSrc(S2Reg);
         __erg=CodeS(0x0c,0,0,S2Reg);
        END
      END
    END
END

        static void DecodeABS(Word Index)
BEGIN
   Boolean DPFlag,S1Flag;
   LongWord DReg,S1Reg;
   UNUSED(Index);

   if (ArgCnt!=2) WrError(1110);
   else if (DecodeReg(ArgStr[2],&DReg,&DPFlag,True))
    if (ChkUnit(DReg,L1,L2))
     if (DecodeReg(ArgStr[1],&S1Reg,&S1Flag,True))
      BEGIN
       if (DPFlag!=S1Flag) WrError(1350);
       else if ((ThisCross) AND ((S1Reg >> 4)==UnitFlag)) WrError(1350);
       else
        BEGIN
         SetCross(S1Reg);
         if (DPFlag) __erg=CodeL(0x38,DReg,0,S1Reg);
         else __erg=CodeL(0x1a,DReg,0,S1Reg);
         if (DPFlag) AddLSrc(S1Reg); else AddSrc(S1Reg);
         if (DPFlag) AddLDest(DReg); else AddDest(DReg);
        END
      END
END

        static void DecodeADD(Word Index)
BEGIN
   LongWord S1Reg,S2Reg,DReg;
   Boolean OK;
   UNUSED(Index);

   if (ArgCnt!=3) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[3],MModReg+MModLReg,True,&DReg);
     UnitFlag=DReg >> 4;
     switch (AdrMode)
      BEGIN
       case ModLReg:      /* ADD ?,?,long */
        AddLDest(DReg);
        DecodeAdr(ArgStr[1],MModReg+MModLReg+MModImm,True,&S1Reg);
        switch (AdrMode)
         BEGIN
          case ModReg:    /* ADD int,?,long */
           AddSrc(S1Reg);
           DecodeAdr(ArgStr[2],MModReg+MModLReg,True,&S2Reg);
           switch (AdrMode)
            BEGIN
             case ModReg: /* ADD int,int,long */
              if (ChkUnit(DReg,L1,L2))
               BEGIN
                if ((ThisCross) AND (NOT IsCross(S1Reg)) AND (NOT IsCross(S2Reg))) WrError(1350);
                else if ((IsCross(S1Reg)) AND (IsCross(S2Reg))) WrError(1350);
                else
                 BEGIN
                  AddSrc(S2Reg);
                  if (IsCross(S1Reg)) SwapReg(&S1Reg,&S2Reg);
                  SetCross(S2Reg);
                  __erg=CodeL(0x23,DReg,S1Reg,S2Reg);
                 END
               END
              break;
             case ModLReg:/* ADD int,long,long */
              if (ChkUnit(DReg,L1,L2))
               BEGIN
                if (IsCross(S2Reg)) WrError(1350);
                else if ((ThisCross) AND (NOT IsCross(S1Reg))) WrError(1350);
                else
                 BEGIN
                  AddLSrc(S2Reg);
                  SetCross(S1Reg);
                  __erg=CodeL(0x21,DReg,S1Reg,S2Reg);
                 END
               END
              break;
            END
           break;
          case ModLReg:   /* ADD long,?,long */
           AddLSrc(S1Reg);
           DecodeAdr(ArgStr[2],MModReg+MModImm,True,&S2Reg);
           switch (AdrMode)
            BEGIN
             case ModReg: /* ADD long,int,long */
              if (ChkUnit(DReg,L1,L2))
               BEGIN
                if (IsCross(S1Reg)) WrError(1350);
                else if ((ThisCross) AND (NOT IsCross(S2Reg))) WrError(1350);
                else
                 BEGIN
                  AddSrc(S2Reg);
                  SetCross(S2Reg);
                  __erg=CodeL(0x21,DReg,S2Reg,S1Reg);
                 END
               END
              break;
             case ModImm: /* ADD long,imm,long */
              if (ChkUnit(DReg,L1,L2))
               BEGIN
                if (IsCross(S1Reg)) WrError(1350);
                else if (ThisCross) WrError(1350);
                else __erg=CodeL(0x20,DReg,S2Reg,S1Reg);
               END
              break;
            END
           break;
          case ModImm:    /* ADD imm,?,long */
           if (DecodeAdr(ArgStr[2],MModLReg,True,&S2Reg))
            BEGIN         /* ADD imm,long,long */
             if (ChkUnit(DReg,L1,L2))
              BEGIN
               if (IsCross(S2Reg)) WrError(1350);
               else if (ThisCross) WrError(1350);
               else
                BEGIN
                 AddLSrc(S2Reg);
                 __erg=CodeL(0x20,DReg,S1Reg,S2Reg);
                END
              END
            END
           break;
         END
        break;
       case ModReg:       /* ADD ?,?,int */
        AddDest(DReg);
        DecodeAdr(ArgStr[1],MModReg+MModImm,True,&S1Reg);
        switch (AdrMode)
         BEGIN
          case ModReg:    /* ADD int,?,int */
           AddSrc(S1Reg);
           DecodeAdr(ArgStr[2],MModReg+MModImm,True,&S2Reg);
           switch (AdrMode)
            BEGIN
             case ModReg: /* ADD int,int,int */
              AddSrc(S2Reg);
              if (((DReg^S1Reg)>15) AND ((DReg^S2Reg)>15)) WrError(1350);
              else if ((ThisCross) AND ((DReg^S1Reg)<16) AND ((DReg^S2Reg)<15)) WrError(1350);
              else
               BEGIN
                if ((S1Reg^DReg)>15) SwapReg(&S1Reg,&S2Reg);
                OK=DecideUnit(DReg,((S2Reg^DReg)>15) ? "LS" : "LSD");
                if (OK)
                 BEGIN
                  switch (ThisUnit)
                   BEGIN
                    case L1: case L2: __erg=CodeL(0x03,DReg,S1Reg,S2Reg); break; /* ADD.Lx int,int,int */
                    case S1: case S2: __erg=CodeS(0x07,DReg,S1Reg,S2Reg); break; /* ADD.Sx int,int,int */
                    case D1: case D2: __erg=CodeD(0x10,DReg,S1Reg,S2Reg); break; /* ADD.Dx int,int,int */
                    default: WrError(2009);
                   END
                 END
               END
              break;
             case ModImm: /* ADD int,imm,int */
              if ((ThisCross) AND ((S1Reg^DReg)<16)) WrError(1350);
              else
               BEGIN
                SetCross(S1Reg);
                if (DecideUnit(DReg,"LS"))
                 switch (ThisUnit)
                  BEGIN
                   case L1: case L2: __erg=CodeL(0x02,DReg,S2Reg,S1Reg); break;
                   case S1: case S2: __erg=CodeS(0x06,DReg,S2Reg,S1Reg); break;
                   default: WrError(2009);
                  END
               END
              break;
            END
           break;
          case ModImm:   /* ADD imm,?,int */
           if (DecodeAdr(ArgStr[2],MModReg,True,&S2Reg))
            BEGIN
             AddSrc(S2Reg);
             if ((ThisCross) AND ((S2Reg^DReg)<16)) WrError(1350);
             else
              BEGIN
               SetCross(S2Reg);
               if (DecideUnit(DReg,"LS"))
                switch (ThisUnit)
                 BEGIN
                  case L1: case L2: __erg=CodeL(0x02,DReg,S1Reg,S2Reg); break;
                  case S1: case S2: __erg=CodeS(0x06,DReg,S1Reg,S2Reg); break;
                  default: WrError(2009);
                 END
              END
            END
           break;
         END
        break;
      END
    END
END

        static void DecodeADDU(Word Index)
BEGIN
   LongWord DReg,S1Reg,S2Reg;
   UNUSED(Index);

   if (ArgCnt!=3) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[3],MModReg+MModLReg,False,&DReg);
     switch (AdrMode)
      BEGIN
       case ModReg:      /* ADDU ?,?,int */
        if (ChkUnit(DReg,D1,D2))
         BEGIN
          AddDest(DReg);
          DecodeAdr(ArgStr[1],MModReg+MModImm,False,&S1Reg);
          switch (AdrMode)
           BEGIN
            case ModReg: /* ADDU int,?,int */
             if (IsCross(S1Reg)) WrError(1350);
             else
              BEGIN
               AddSrc(S1Reg);
               if (DecodeAdr(ArgStr[2],MModImm,False,&S2Reg))
                __erg=CodeD(0x12,DReg,S2Reg,S1Reg);
              END
             break;
            case ModImm: /* ADDU imm,?,int */
             if (DecodeAdr(ArgStr[2],MModReg,False,&S2Reg))
              BEGIN
               if (IsCross(S2Reg)) WrError(1350);
               else
                BEGIN
                 AddSrc(S2Reg);
                 __erg=CodeD(0x12,DReg,S1Reg,S2Reg);
                END
              END
             break;
           END
         END
        break;
       case ModLReg:     /* ADDU ?,?,long */
        if (ChkUnit(DReg,L1,L2))
         BEGIN
          AddLDest(DReg);
          DecodeAdr(ArgStr[1],MModReg+MModLReg,False,&S1Reg);
          switch (AdrMode)
           BEGIN
            case ModReg: /* ADDU int,?,long */
             AddSrc(S1Reg);
             DecodeAdr(ArgStr[2],MModReg+MModLReg,False,&S2Reg);
             switch (AdrMode)
              BEGIN
               case ModReg: /* ADDU int,int,long */
                if ((IsCross(S1Reg)) AND (IsCross(S2Reg))) WrError(1350);
                else if ((ThisCross) AND (((S1Reg^DReg)<16) AND ((S2Reg^DReg)<16))) WrError(1350);
                else
                 BEGIN
                  if ((S1Reg^DReg)>15) SwapReg(&S1Reg,&S2Reg);
                  SetCross(S2Reg);
                  __erg=CodeL(0x2b,DReg,S1Reg,S2Reg);
                 END
                break;
               case ModLReg: /* ADDU int,long,long */
                if (IsCross(S2Reg)) WrError(1350);
                else if ((ThisCross) AND ((S1Reg^DReg)<16)) WrError(1350);
                else
                 BEGIN
                  AddLSrc(S2Reg);
                  SetCross(S1Reg);
                  __erg=CodeL(0x29,DReg,S1Reg,S2Reg);
                 END
                break;
              END
             break;
            case ModLReg:
             if (IsCross(S1Reg)) WrError(1350);
             else
              BEGIN
               AddLSrc(S1Reg);
               if (DecodeAdr(ArgStr[2],MModReg,False,&S2Reg))
                BEGIN
                 if ((ThisCross) AND ((S2Reg^DReg)<16)) WrError(1350);
                 else
                  BEGIN
                   AddSrc(S2Reg); SetCross(S2Reg);
                   __erg=CodeL(0x29,DReg,S2Reg,S1Reg);
                  END
                END
              END
             break;
           END
         END
        break;
      END
    END
END

        static void DecodeSUB(Word Index)
BEGIN
   LongWord DReg,S1Reg,S2Reg;
   Boolean OK;
   UNUSED(Index);

   if (ArgCnt!=3) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[3],MModReg+MModLReg,True,&DReg);
     switch (AdrMode)
      BEGIN
       case ModReg:
        AddDest(DReg);
        DecodeAdr(ArgStr[1],MModReg+MModImm,True,&S1Reg);
        switch (AdrMode)
         BEGIN
          case ModReg:
           AddSrc(S1Reg);
           DecodeAdr(ArgStr[2],MModReg+MModImm,True,&S2Reg);
           switch (AdrMode)
            BEGIN
             case ModReg:
              if ((ThisCross) AND ((S1Reg^DReg)<16) AND ((S2Reg^DReg)<16)) WrError(1350);
              else if (((S1Reg^DReg)>15) AND ((S2Reg^DReg)>15)) WrError(1350);
              else
               BEGIN
                AddSrc(S2Reg);
                ThisCross=((S1Reg^DReg)>15) OR ((S2Reg^DReg)>15);
                if ((S1Reg^DReg)>15) OK=DecideUnit(DReg,"L");
                else if ((S2Reg^DReg)>15) OK=DecideUnit(DReg,"LS");
                else OK=DecideUnit(DReg,"LSD");
                if (OK)
                 switch (ThisUnit)
                  BEGIN
                   case L1: case L2:
                    if ((S1Reg^DReg)>15) __erg=CodeL(0x17,DReg,S1Reg,S2Reg);
                    else __erg=CodeL(0x07,DReg,S1Reg,S2Reg);
                    break;
                   case S1: case S2:
                    __erg=CodeS(0x17,DReg,S1Reg,S2Reg);
                    break;
                   case D1: case D2:
                    __erg=CodeD(0x11,DReg,S2Reg,S1Reg);
                    break;
                   default:
                    WrError(2009);
                  END
               END
              break;
             case ModImm:
              if (ChkUnit(DReg,D1,D2))
               BEGIN
                if ((ThisCross) OR ((S1Reg^DReg)>15)) WrError(1350);
                else __erg=CodeD(0x13,DReg,S2Reg,S1Reg);
               END
              break;
            END
           break;
          case ModImm:
           if (DecodeAdr(ArgStr[2],MModReg,True,&S2Reg))
            BEGIN
             if ((ThisCross) AND ((S2Reg^DReg)<16)) WrError(1350);
             else
              BEGIN
               AddSrc(S2Reg);
               if (DecideUnit(DReg,"LS"))
                switch (ThisUnit)
                 BEGIN
                  case L1: case L2: __erg=CodeL(0x06,DReg,S1Reg,S2Reg); break;
                  case S1: case S2: __erg=CodeS(0x16,DReg,S1Reg,S2Reg); break;
                  default: WrError(2009);
                 END
              END
            END
           break;
         END
        break;
       case ModLReg:
        AddLDest(DReg);
        if (ChkUnit(DReg,L1,L2))
         BEGIN
          DecodeAdr(ArgStr[1],MModImm+MModReg,True,&S1Reg);
          switch (AdrMode)
           BEGIN
            case ModImm:
             if (DecodeAdr(ArgStr[2],MModLReg,True,&S2Reg))
              BEGIN
               if ((ThisCross) OR (/*NOT*/ IsCross(S2Reg))) WrError(1350);
               else
                BEGIN
                 AddLSrc(S2Reg);
                 __erg=CodeL(0x24,DReg,S1Reg,S2Reg);
                END
              END
             break;
            case ModReg:
             AddSrc(S1Reg);
             if (DecodeAdr(ArgStr[2],MModReg,True,&S2Reg))
              BEGIN
               if ((ThisCross) AND (NOT IsCross(S1Reg)) AND (NOT IsCross(S2Reg))) WrError(1350);
               else if ((IsCross(S1Reg)) AND (IsCross(S2Reg))) WrError(1350);
               else
                BEGIN
                 AddSrc(S2Reg);
                 ThisCross=(IsCross(S1Reg)) OR (IsCross(S2Reg));
                 /* what did I do here? */
                 if (IsCross(S1Reg)) __erg=CodeL(0x37,DReg,S1Reg,S2Reg);
                 else __erg=CodeL(0x47,DReg,S1Reg,S2Reg);
                END
              END
             break;
           END
         END
        break;
      END
    END
END

        static void DecodeSUBU(Word Index)
BEGIN
   LongWord S1Reg,S2Reg,DReg;
   UNUSED(Index);

   if (ArgCnt!=3) WrError(1110);
   else
    BEGIN
     if ((DecodeAdr(ArgStr[3],MModLReg,False,&DReg)) AND (ChkUnit(DReg,L1,L2)))
      BEGIN
       AddLDest(DReg);
       if (DecodeAdr(ArgStr[1],MModReg,False,&S1Reg))
        BEGIN
         AddSrc(S1Reg);
         if (DecodeAdr(ArgStr[2],MModReg,False,&S2Reg))
          BEGIN
           if ((ThisCross) AND (NOT IsCross(S1Reg)) AND (NOT IsCross(S2Reg))) WrError(1350);
           else if ((IsCross(S1Reg)) AND (IsCross(S2Reg))) WrError(1350);
           else
            BEGIN
             AddSrc(S2Reg);
             ThisCross=IsCross(S1Reg) OR IsCross(S2Reg);
             if (IsCross(S1Reg)) __erg=CodeL(0x3f,DReg,S1Reg,S2Reg);
             else __erg=CodeL(0x2f,DReg,S1Reg,S2Reg);
            END
          END
        END
      END
    END
END

        static void DecodeSUBC(Word Index)
BEGIN
   LongWord DReg,S1Reg,S2Reg;
   UNUSED(Index);

   if (ArgCnt!=3) WrError(1110);
   else
    BEGIN
     if ((DecodeAdr(ArgStr[3],MModReg,False,&DReg)) AND (ChkUnit(DReg,L1,L2)))
      BEGIN
       AddLDest(DReg);
       if (DecodeAdr(ArgStr[1],MModReg,False,&S1Reg))
        BEGIN
         if (DecodeAdr(ArgStr[2],MModReg,False,&S2Reg))
          BEGIN
           if ((ThisCross) AND (NOT IsCross(S2Reg))) WrError(1350);
           else if (IsCross(S1Reg)) WrError(1350);
           else
            BEGIN
             AddSrc(S2Reg); SetCross(S2Reg);
             __erg=CodeL(0x4b,DReg,S1Reg,S2Reg);
            END
          END
        END
      END
    END
END

        static void DecodeLinAdd(Word Index)
BEGIN
   LongWord DReg,S1Reg,S2Reg;
   FixedOrder *POrder=LinAddOrders+Index;

   if (ArgCnt!=3) WrError(1110);
   else if (ThisCross) WrError(1350);
   else
    BEGIN
     if (DecodeAdr(ArgStr[3],MModReg,True,&DReg))
      if (ChkUnit(DReg,D1,D2))
       BEGIN
        AddDest(DReg);
        if (DecodeAdr(ArgStr[1],MModReg,True,&S2Reg))
         BEGIN
          if (IsCross(S2Reg)) WrError(1350);
          else
           BEGIN
            AddSrc(S2Reg);
            DecodeAdr(ArgStr[2],MModReg+MModImm,False,&S1Reg);
            switch (AdrMode)
             BEGIN
              case ModReg:
               if (IsCross(S1Reg)) WrError(1350);
               else
                BEGIN
                 AddSrc(S1Reg);
                 __erg=CodeD(POrder->Code,DReg,S1Reg,S2Reg);
                END
               break;
              case ModImm:
               __erg=CodeD(POrder->Code+2,DReg,S1Reg,S2Reg);
               break;
             END
           END
         END
       END
    END
END

        static void DecodeADDK(Word Index)
BEGIN
   LongInt Value;
   LongWord DReg;
   Boolean OK;
   UNUSED(Index);

   if (ArgCnt!=2) WrError(1110);
   else
    BEGIN
     if (DecodeAdr(ArgStr[2],MModReg,False,&DReg))
      if (ChkUnit(DReg,S1,S2))
       BEGIN
        AddDest(DReg);
        Value=EvalIntExpression(ArgStr[1],SInt16,&OK);
        if (OK)
         BEGIN
          ThisInst=0x50+(UnitFlag << 1)+((Value & 0xffff) << 7)+(DReg << 23);
          __erg=True;
         END
       END
    END
END

        static void DecodeADD2_SUB2(Word Index)
BEGIN
   LongWord DReg,S1Reg,S2Reg;
   Boolean OK;

   Index=(Index<<5)+1;
   if (ArgCnt!=3) WrError(1110);
   else
    BEGIN
     if (DecodeAdr(ArgStr[3],MModReg,True,&DReg))
      if (ChkUnit(DReg,S1,S2))
       BEGIN
        AddDest(DReg);
        if (DecodeAdr(ArgStr[1],MModReg,True,&S1Reg))
         BEGIN
          AddSrc(S1Reg);
          if (DecodeAdr(ArgStr[2],MModReg,True,&S2Reg))
           BEGIN
            if ((ThisCross) AND (NOT IsCross(S1Reg)) AND (NOT IsCross(S2Reg))) WrError(1350);
            else if ((IsCross(S1Reg)) AND (IsCross(S2Reg))) WrError(1350);
            else
             BEGIN
              OK=True; AddSrc(S2Reg);
              if (IsCross(S1Reg))
               BEGIN
                if (Index>1)
                 BEGIN
                  WrError(1350); OK=False;
                 END
                else SwapReg(&S1Reg,&S2Reg);
               END
              if (OK)
               BEGIN
                SetCross(S2Reg);
                __erg=CodeS(Index,DReg,S1Reg,S2Reg);
               END
             END
           END
         END
       END
    END
END

        static void DecodeMV(Word Index)
{
   LongWord SReg,DReg;
   UNUSED(Index);

   /* MV src,dst == ADD 0,src,dst */

   if (ArgCnt != 2) WrError(1110);
   else
   {
     DecodeAdr(ArgStr[2], MModReg + MModLReg, True, &DReg);
     UnitFlag = DReg >> 4;
     switch (AdrMode)
     {
       case ModLReg:      /* MV ?,long */
        AddLDest(DReg);
        if (DecodeAdr(ArgStr[1], MModLReg, True, &SReg))
        {                 /* MV long,long */
          if (ChkUnit(DReg, L1, L2))
          {
            if (IsCross(SReg)) WrError(1350);
            else if (ThisCross) WrError(1350);
            else
            {
              AddLSrc(SReg);
              __erg = CodeL(0x20, DReg, 0, SReg);
            }
          }
        }
        break;

       case ModReg:       /* MV ?,int */
        AddDest(DReg);
        if (DecodeAdr(ArgStr[1], MModReg, True, &SReg))
        {
          AddSrc(SReg);
          if ((ThisCross) AND ((SReg ^ DReg) < 16)) WrError(1350);
          else
          {
            SetCross(SReg);
            if (DecideUnit(DReg,"LSD"))
            switch (ThisUnit)
            {
              case L1: case L2: __erg = CodeL(0x02, DReg, 0, SReg); break;
              case S1: case S2: __erg = CodeS(0x06, DReg, 0, SReg); break;
              case D1: case D2: __erg = CodeD(0x12, DReg, 0, SReg); break;
              default: WrError(2009);
            }
          }
        }
        break;
     }
   }
}

        static void DecodeNEG(Word Index)
{
   LongWord DReg, SReg;
   UNUSED(Index);

   /* NEG src,dst == SUB 0,src,dst */

   if (ArgCnt != 2) WrError(1110);
   else
   {
     DecodeAdr(ArgStr[2], MModReg + MModLReg, True, &DReg);
     switch (AdrMode)
     {
       case ModReg:
         AddDest(DReg);
         if (DecodeAdr(ArgStr[1], MModReg, True, &SReg))
         {
           if ((ThisCross) AND ((SReg ^ DReg) < 16)) WrError(1350);
           else
           {
             AddSrc(SReg);
             if (DecideUnit(DReg, "LS"))
               switch (ThisUnit)
               {
                 case L1: case L2: __erg = CodeL(0x06, DReg, 0, SReg); break;
                 case S1: case S2: __erg = CodeS(0x16, DReg, 0, SReg); break;
                 default: WrError(2009);
               }
           }
         }
         break;
       case ModLReg:
         AddLDest(DReg);
         if (ChkUnit(DReg, L1, L2))
         {
           if (DecodeAdr(ArgStr[1], MModLReg, True, &SReg))
           {
             if ((ThisCross) OR (IsCross(SReg))) WrError(1350);
             else
             {
               AddLSrc(SReg);
               __erg = CodeL(0x24, DReg, 0, SReg);
             }
           }
         }
         break;
     }
   }
}

        static void DecodeLogic(Word Index)
BEGIN
   LongWord S1Reg,S2Reg,DReg;
   LongWord Code1,Code2;
   Boolean OK,WithImm;

   Code1=Lo(Index); Code2=Hi(Index);

   if (ArgCnt!=3) WrError(1110);
   else
    BEGIN
     if (DecodeAdr(ArgStr[3],MModReg,True,&DReg))
      BEGIN
       AddDest(DReg);
       DecodeAdr(ArgStr[1],MModImm+MModReg,True,&S1Reg); WithImm=False;
       switch (AdrMode)
        BEGIN
         case ModImm:
          OK=DecodeAdr(ArgStr[2],MModReg,True,&S2Reg);
          if (OK) AddSrc(S2Reg);
          WithImm=True;
          break;
         case ModReg:
          AddSrc(S1Reg);
          OK=DecodeAdr(ArgStr[2],MModImm+MModReg,True,&S2Reg);
          switch (AdrMode)
           BEGIN
            case ModImm:
             SwapReg(&S1Reg,&S2Reg); WithImm=True;
             break;
            case ModReg:
             AddSrc(S2Reg); WithImm=False;
             break;
            default:
             OK=False;
           END
          break;
         default:
          OK=False;
        END
       if (OK)
        if (DecideUnit(DReg,"LS"))
         BEGIN
          if ((NOT WithImm) AND (IsCross(S1Reg)) AND (IsCross(S2Reg))) WrError(1350);
          else if ((ThisCross) AND (NOT IsCross(S2Reg)) AND ((WithImm) OR (NOT IsCross(S1Reg)))) WrError(1350);
          else
           BEGIN
            if ((NOT WithImm) AND (IsCross(S1Reg))) SwapReg(&S1Reg,&S2Reg);
            SetCross(S2Reg);
            switch (ThisUnit)
             BEGIN
              case L1: case L2:
               __erg = CodeL(Code1 - Ord(WithImm), DReg, S1Reg, S2Reg); break;
              case S1: case S2:
               __erg = CodeS(Code2 - Ord(WithImm), DReg, S1Reg, S2Reg); break;
              default:
               WrError(2009);
             END
           END
         END
      END
    END
END

        static void DecodeNOT(Word Index)
BEGIN
   LongWord SReg, DReg;

   UNUSED(Index);

   /* NOT src,dst == XOR -1,src,dst */

   if (ArgCnt != 2) WrError(1110);
   else
   {
     if (DecodeAdr(ArgStr[2], MModReg, True, &DReg))
     {
       AddDest(DReg);
       if (DecodeAdr(ArgStr[1], MModReg, True, &SReg))
       {
         AddSrc(SReg);
         if (DecideUnit(DReg,"LS"))
         {
           if ((ThisCross) AND (NOT IsCross(SReg))) WrError(1350);
           else
           {
             SetCross(SReg);
             switch (ThisUnit)
              BEGIN
               case L1: case L2:
                __erg = CodeL(0x6e, DReg, 0x1f, SReg); break;
               case S1: case S2:
                __erg = CodeS(0x0a, DReg, 0x1f, SReg); break;
               default:
                WrError(2009);
              END
           }
         }
       }
     }
   }
END

        static void DecodeZERO(Word Index)
{
   LongWord DReg;
   UNUSED(Index);

   /* ZERO dst == SUB dst,dst,dst */

   if (ArgCnt != 1) WrError(1110);
   else
   {
     DecodeAdr(ArgStr[1], MModReg + MModLReg, True, &DReg);
     if ((ThisCross) OR (IsCross(DReg))) WrError(1350);
     else switch (AdrMode)
     {
       case ModReg:
         AddDest(DReg);
         AddSrc(DReg);
         if (DecideUnit(DReg, "LSD"))
           switch (ThisUnit)
           {
             case L1: case L2: __erg = CodeL(0x17, DReg, DReg, DReg); break;
             case S1: case S2: __erg = CodeS(0x17, DReg, DReg, DReg); break;
             case D1: case D2: __erg = CodeD(0x11, DReg, DReg, DReg); break;
             default: WrError(2009);
           }
         break;
       case ModLReg:
         AddLDest(DReg);
         AddLSrc(DReg);
         if (ChkUnit(DReg, L1, L2))
           __erg = CodeL(0x37, DReg, DReg, DReg);
         break;
     }
   }
}

        static Boolean DecodeInst(void)
BEGIN
   Boolean OK,erg;
   LongInt Dist;
   int z;
   LongWord DReg,S1Reg,S2Reg,HReg;
   LongWord Code1;
   Boolean WithImm,HasSign;

   erg=__erg=False;

   /* ueber Tabelle: */

   if (LookupInstTable(InstTable,OpPart)) return __erg;

   /* jetzt geht's los... */

   if ((Memo("CLR") OR (Memo("EXT")) OR (Memo("EXTU")) OR (Memo("SET"))))
    BEGIN
     if ((ArgCnt!=3) AND (ArgCnt!=4)) WrError(1110);
     else
      BEGIN
       if (DecodeAdr(ArgStr[ArgCnt],MModReg,Memo("EXT"),&DReg))
        if (ChkUnit(DReg,S1,S2))
         BEGIN
          AddDest(DReg);
          if (DecodeAdr(ArgStr[1],MModReg,Memo("EXT"),&S2Reg))
           BEGIN
            AddSrc(S2Reg);
            if (ArgCnt==3)
             BEGIN
              if (DecodeAdr(ArgStr[2],MModReg,False,&S1Reg))
               BEGIN
                if (IsCross(S1Reg)) WrError(1350);
                else if ((ThisCross) AND (NOT IsCross(S2Reg))) WrError(1350);
                else
                 BEGIN
                  SetCross(S2Reg);
                  if (Memo("CLR")) erg=CodeS(0x3f,DReg,S1Reg,S2Reg);
                  else if (Memo("EXTU")) erg=CodeS(0x2b,DReg,S1Reg,S2Reg);
                  else if (Memo("SET")) erg=CodeS(0x3b,DReg,S1Reg,S2Reg);
                  else erg=CodeS(0x2f,DReg,S1Reg,S2Reg);
                 END
               END
             END
            else if ((ThisCross) OR (IsCross(S2Reg))) WrError(1350);
            else
             BEGIN
              S1Reg=EvalIntExpression(ArgStr[2],UInt5,&OK);
              if (OK)
               BEGIN
                HReg=EvalIntExpression(ArgStr[3],UInt5,&OK);
                if (OK)
                 BEGIN
                  ThisInst=(DReg << 23)+(S2Reg << 18)+(S1Reg << 13)+
                           (HReg << 8)+(UnitFlag << 1);
                  if (Memo("CLR")) ThisInst+=0xc8;
                  else if (Memo("SET")) ThisInst+=0x88;
                  else if (Memo("EXT")) ThisInst+=0x48;
                  else ThisInst+=0x08;
                  erg=True;
                 END
               END
             END
           END
         END
      END
     return erg;
    END

   for (z=0; z<CmpCnt; z++)
    if (Memo(CmpOrders[z].Name))
     BEGIN
      WithImm=OpPart[strlen(OpPart)-1]!='U';
      if (ArgCnt!=3) WrError(1110);
      else
       BEGIN
        if (DecodeAdr(ArgStr[3],MModReg,False,&DReg))
         if (ChkUnit(DReg,L1,L2))
          BEGIN
           AddDest(DReg);
           DecodeAdr(ArgStr[1],MModReg+MModImm,WithImm,&S1Reg);
           switch (AdrMode)
            BEGIN
             case ModReg:
              AddSrc(S1Reg);
              DecodeAdr(ArgStr[2],MModReg+MModLReg,WithImm,&S2Reg);
              switch (AdrMode)
               BEGIN
                case ModReg:
                 if ((IsCross(S1Reg)) AND (IsCross(S2Reg))) WrError(1350);
                 else if ((ThisCross) AND (NOT IsCross(S1Reg)) AND (NOT IsCross(S2Reg))) WrError(1350);
                 else
                  BEGIN
                   AddSrc(S2Reg);
                   if (IsCross(S1Reg)) SwapReg(&S1Reg,&S2Reg);
                   SetCross(S2Reg);
                   erg=CodeL(CmpOrders[z].Code+3,DReg,S1Reg,S2Reg);
                  END
                 break;
                case ModLReg:
                 if (IsCross(S2Reg)) WrError(1350);
                 else if ((ThisCross) AND (NOT IsCross(S1Reg))) WrError(1350);
                 else
                  BEGIN
                   AddLSrc(S2Reg); SetCross(S1Reg);
                   erg=CodeL(CmpOrders[z].Code+1,DReg,S1Reg,S2Reg);
                  END
                 break;
               END
              break;
             case ModImm:
              DecodeAdr(ArgStr[2],MModReg+MModLReg,WithImm,&S2Reg);
              switch (AdrMode)
               BEGIN
                case ModReg:
                 if ((ThisCross) AND (NOT IsCross(S2Reg))) WrError(1350);
                 else
                  BEGIN
                   AddSrc(S2Reg); SetCross(S2Reg);
                   erg=CodeL(CmpOrders[z].Code+2,DReg,S1Reg,S2Reg);
                  END
                 break;
                case ModLReg:
                 if ((ThisCross) OR (IsCross(S2Reg))) WrError(1350);
                 else
                  BEGIN
                   AddLSrc(S2Reg);
                   erg=CodeL(CmpOrders[z].Code,DReg,S1Reg,S2Reg);
                  END
                 break;
               END
              break;
            END
          END
       END
      return erg;
     END

   if (Memo("LMBD"))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else
      BEGIN
       if (DecodeAdr(ArgStr[3],MModReg,False,&DReg))
        if (ChkUnit(DReg,L1,L2))
         BEGIN
          AddDest(DReg);
          if (DecodeAdr(ArgStr[2],MModReg,False,&S2Reg))
           BEGIN
            if ((ThisCross) AND (NOT IsCross(S2Reg))) WrError(1350);
            else
             BEGIN
              SetCross(S2Reg);
              if (DecodeAdr(ArgStr[1],MModImm+MModReg,False,&S1Reg))
               BEGIN
                if (AdrMode==ModReg) AddSrc(S1Reg);
                erg=CodeL(0x6a+Ord(AdrMode==ModImm),DReg,S1Reg,S2Reg);
               END
             END
           END
         END
      END
     return erg;
    END

   if (Memo("NORM"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       if (DecodeAdr(ArgStr[2],MModReg,False,&DReg))
        if (ChkUnit(DReg,L1,L2))
         BEGIN
          AddDest(DReg);
          DecodeAdr(ArgStr[1],MModReg+MModLReg,True,&S2Reg);
          switch (AdrMode)
           BEGIN
            case ModReg:
             if ((ThisCross) AND (NOT IsCross(S2Reg))) WrError(1350);
             else
              BEGIN
               SetCross(S2Reg); AddSrc(S2Reg);
               erg=CodeL(0x63,DReg,0,S2Reg);
              END
             break;
            case ModLReg:
             if ((ThisCross) OR (IsCross(S2Reg))) WrError(1350);
             else
              BEGIN
               AddLSrc(S2Reg);
               erg=CodeL(0x60,DReg,0,S2Reg);
              END
             break;
           END
         END
      END
     return erg;
    END

   if (Memo("SADD"))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else
      BEGIN
       if ((DecodeAdr(ArgStr[3],MModReg+MModLReg,True,&DReg)) AND (ChkUnit(DReg,L1,L2)))
        switch (AdrMode)
         BEGIN
          case ModReg:
           AddDest(DReg);
           DecodeAdr(ArgStr[1],MModReg+MModImm,True,&S1Reg);
           switch (AdrMode)
            BEGIN
             case ModReg:
              AddSrc(S1Reg);
              DecodeAdr(ArgStr[2],MModReg+MModImm,True,&S2Reg);
              switch (AdrMode)
               BEGIN
                case ModReg:
                 if ((ThisCross) AND (NOT IsCross(S1Reg)) AND (NOT IsCross(S2Reg))) WrError(1350);
                 else if ((IsCross(S1Reg)) AND (IsCross(S2Reg))) WrError(1350);
                 else
                  BEGIN
                   AddSrc(S2Reg);
                   if (IsCross(S1Reg)) SwapReg(&S1Reg,&S2Reg);
                   SetCross(S2Reg);
                   erg=CodeL(0x13,DReg,S1Reg,S2Reg);
                  END
                 break;
                case ModImm:
                 if ((ThisCross) AND (NOT IsCross(S1Reg))) WrError(1350);
                 else
                  BEGIN
                   SetCross(S1Reg);
                   erg=CodeL(0x12,DReg,S2Reg,S1Reg);
                  END
                 break;
               END
              break;
             case ModImm:
              if (DecodeAdr(ArgStr[2],MModReg,True,&S2Reg))
               BEGIN
                if ((ThisCross) AND (NOT IsCross(S2Reg))) WrError(1350);
                else
                 BEGIN
                  SetCross(S2Reg);
                  erg=CodeL(0x12,DReg,S1Reg,S2Reg);
                 END
               END
              break;
            END
           break;
          case ModLReg:
           AddLDest(DReg);
           DecodeAdr(ArgStr[1],MModReg+MModLReg+MModImm,True,&S1Reg);
           switch (AdrMode)
            BEGIN
             case ModReg:
              AddSrc(S1Reg);
              if (DecodeAdr(ArgStr[2],MModLReg,True,&S2Reg))
               BEGIN
                if ((ThisCross) AND (NOT IsCross(S1Reg))) WrError(1350);
                else
                 BEGIN
                  AddLSrc(S2Reg); SetCross(S1Reg);
                  erg=CodeL(0x31,DReg,S1Reg,S2Reg);
                 END
               END
              break;
             case ModLReg:
              if (IsCross(S1Reg)) WrError(1350);
              else
               BEGIN
                AddLSrc(S1Reg);
                DecodeAdr(ArgStr[2],MModReg+MModImm,True,&S2Reg);
                switch (AdrMode)
                 BEGIN
                  case ModReg:
                   if ((ThisCross) AND (NOT IsCross(S2Reg))) WrError(1350);
                   else
                    BEGIN
                     AddSrc(S2Reg); SetCross(S2Reg);
                     erg=CodeL(0x31,DReg,S2Reg,S1Reg);
                    END
                   break;
                  case ModImm:
                   erg=CodeL(0x30,DReg,S2Reg,S1Reg);
                   break;
                 END
               END
              break;
             case ModImm:
              if (DecodeAdr(ArgStr[2],MModLReg,True,&S2Reg))
               BEGIN
                if (IsCross(S2Reg)) WrError(1350);
                else
                 BEGIN
                  AddLSrc(S2Reg);
                  erg=CodeL(0x30,DReg,S1Reg,S2Reg);
                 END
               END
              break;
            END
           break;
         END
      END
     return erg;
    END

   if (Memo("SAT"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       if (DecodeAdr(ArgStr[2],MModReg,True,&DReg))
        if (ChkUnit(DReg,L1,L2))
         BEGIN
          AddDest(DReg);
          if (DecodeAdr(ArgStr[1],MModLReg,True,&S2Reg))
           BEGIN
            if ((ThisCross) OR (IsCross(S2Reg))) WrError(1350);
            else
             BEGIN
              AddLSrc(S2Reg); erg=CodeL(0x40,DReg,0,S2Reg);
             END
           END
         END
      END
     return erg;
    END

   if (Memo("MVC"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if ((ThisUnit!=NoUnit) AND (ThisUnit!=S2)) WrError(1350);
     else
      BEGIN
       z=0; ThisUnit=S2; UnitFlag=1;
       if (DecodeCtrlReg(ArgStr[1],&S2Reg,False)) z=2;
       else if (DecodeCtrlReg(ArgStr[2],&DReg,True)) z=1;
       else WrXError(1440,ArgStr[1]);
       if (z>0)
        BEGIN
         if (DecodeAdr(ArgStr[z],MModReg,False,&S1Reg))
          BEGIN
           if ((ThisCross) AND ((z==2) OR (IsCross(S1Reg)))) WrError(1350);
           else
            BEGIN
             if (z==1)
              BEGIN
               S2Reg=S1Reg;
               AddSrc(S2Reg); SetCross(S2Reg);
              END
             else
              BEGIN
               DReg=S1Reg; AddDest(DReg);
              END
             erg=CodeS(0x0d+z,DReg,0,S2Reg);
            END
          END
        END
      END
     return erg;
    END

   if ((Memo("MVKL")) OR (Memo("MVK")) OR (Memo("MVKH")) OR (Memo("MVKLH")))
    BEGIN
     if (ArgCnt != 2) WrError(1110);
     else
      BEGIN
       if (DecodeAdr(ArgStr[2], MModReg, True, &DReg))
        if (ChkUnit(DReg, S1, S2))
         BEGIN
          if (Memo("MVKLH"))
            S1Reg = EvalIntExpression(ArgStr[1], Int16, &OK);
          else if (Memo("MVKL"))
            S1Reg = EvalIntExpression(ArgStr[1], SInt16, &OK);
          else
            S1Reg = EvalIntExpression(ArgStr[1], Int32, &OK);
          if (OK)
           BEGIN
            AddDest(DReg);
            if (Memo("MVKH")) S1Reg=S1Reg >> 16;
            ThisInst=(DReg << 23)+((S1Reg & 0xffff) << 7)+(UnitFlag << 1);
            ThisInst += ((Memo("MVK")) || (Memo("MVKL"))) ? 0x28 : 0x68;
            erg=True;
           END
         END
      END
     return True;
    END

   if (Memo("SHL"))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[3],MModReg+MModLReg,True,&DReg);
       if ((AdrMode!=ModNone) AND (ChkUnit(DReg,S1,S2)))
        switch (AdrMode)
         BEGIN
          case ModReg:
           AddDest(DReg);
           if (DecodeAdr(ArgStr[1],MModReg,True,&S2Reg))
            BEGIN
             if ((ThisCross) AND (NOT IsCross(S2Reg))) WrError(1350);
             else
              BEGIN
               AddSrc(S2Reg);
               SetCross(S2Reg);
               DecodeAdr(ArgStr[2],MModReg+MModImm,False,&S1Reg);
               switch (AdrMode)
                BEGIN
                 case ModReg:
                  if (IsCross(S1Reg)) WrError(1350);
                  else
                   BEGIN
                    AddSrc(S1Reg);
                    erg=CodeS(0x33,DReg,S1Reg,S2Reg);
                   END
                  break;
                 case ModImm:
                  erg=CodeS(0x32,DReg,S1Reg,S2Reg);
                  break;
                END
              END
            END
           break;
          case ModLReg:
           AddLDest(DReg);
           DecodeAdr(ArgStr[1],MModReg+MModLReg,True,&S2Reg);
           switch (AdrMode)
            BEGIN
             case ModReg:
              if ((ThisCross) AND (NOT IsCross(S2Reg))) WrError(1350);
              else
               BEGIN
                AddSrc(S2Reg); SetCross(S2Reg);
                DecodeAdr(ArgStr[2],MModImm+MModReg,False,&S1Reg);
                switch (AdrMode)
                 BEGIN
                  case ModReg:
                   if (IsCross(S1Reg)) WrError(1350);
                   else
                    BEGIN
                     AddSrc(S1Reg);
                     erg=CodeS(0x13,DReg,S1Reg,S2Reg);
                    END
                   break;
                  case ModImm:
                   erg=CodeS(0x12,DReg,S1Reg,S2Reg);
                   break;
                 END
               END
              break;
             case ModLReg:
              if ((ThisCross) OR (IsCross(S2Reg))) WrError(1350);
              else
               BEGIN
                AddLSrc(S2Reg);
                DecodeAdr(ArgStr[2],MModImm+MModReg,False,&S1Reg);
                switch (AdrMode)
                 BEGIN
                  case ModReg:
                   if (IsCross(S1Reg)) WrError(1350);
                   else
                    BEGIN
                     AddSrc(S1Reg);
                     erg=CodeS(0x31,DReg,S1Reg,S2Reg);
                    END
                   break;
                  case ModImm:
                   erg=CodeS(0x30,DReg,S1Reg,S2Reg);
                   break;
                 END
               END
              break;
            END
           break;
         END
      END
     return erg;
    END

   if ((Memo("SHR")) OR (Memo("SHRU")))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else
      BEGIN
       HasSign=Memo("SHR"); z=Ord(HasSign) << 4;
       DecodeAdr(ArgStr[3],MModReg+MModLReg,HasSign,&DReg);
       if ((AdrMode!=ModNone) AND (ChkUnit(DReg,S1,S2)))
        switch (AdrMode)
         BEGIN
          case ModReg:
           AddDest(DReg);
           if (DecodeAdr(ArgStr[1],MModReg,HasSign,&S2Reg))
            BEGIN
             if ((ThisCross) AND (NOT IsCross(S2Reg))) WrError(1350);
             else
              BEGIN
               AddSrc(S2Reg); SetCross(S2Reg);
               DecodeAdr(ArgStr[2],MModReg+MModImm,False,&S1Reg);
               switch (AdrMode)
                BEGIN
                 case ModReg:
                  if (IsCross(S1Reg)) WrError(1350);
                  else
                   BEGIN
                    AddSrc(S1Reg);
                    erg=CodeS(0x27+z,DReg,S1Reg,S2Reg);
                   END
                  break;
                 case ModImm:
                  erg=CodeS(0x26+z,DReg,S1Reg,S2Reg);
                  break;
                END
              END
            END
           break;
          case ModLReg:
           AddLDest(DReg);
           if (DecodeAdr(ArgStr[1],MModLReg,HasSign,&S2Reg))
            BEGIN
             if ((ThisCross) OR (IsCross(S2Reg))) WrError(1350);
             else
              BEGIN
               AddLSrc(S2Reg);
               DecodeAdr(ArgStr[2],MModReg+MModImm,False,&S1Reg);
               switch (AdrMode)
                BEGIN
                 case ModReg:
                  if (IsCross(S1Reg)) WrError(1350);
                  else
                   BEGIN
                    AddSrc(S1Reg);
                    erg=CodeS(0x25+z,DReg,S1Reg,S2Reg);
                   END
                  break;
                 case ModImm:
                  erg=CodeS(0x24+z,DReg,S1Reg,S2Reg);
                  break;
                END
              END
            END
           break;
         END
      END
     return erg;
    END

   if (Memo("SSHL"))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else
      BEGIN
       if (DecodeAdr(ArgStr[3],MModReg,True,&DReg))
        if (ChkUnit(DReg,S1,S2))
         BEGIN
          AddDest(DReg);
          if (DecodeAdr(ArgStr[1],MModReg,True,&S2Reg))
           BEGIN
            if ((ThisCross) AND (NOT IsCross(S2Reg))) WrError(1350);
            else
             BEGIN
              AddSrc(S2Reg); SetCross(S2Reg);
              DecodeAdr(ArgStr[2],MModReg+MModImm,False,&S1Reg);
              switch (AdrMode)
               BEGIN
                case ModReg:
                 if (IsCross(S1Reg)) WrError(1350);
                 else
                  BEGIN
                   AddSrc(S1Reg);
                   erg=CodeS(0x23,DReg,S1Reg,S2Reg);
                  END
                 break;
                case ModImm:
                 erg=CodeS(0x22,DReg,S1Reg,S2Reg);
                 break;
               END
             END
           END
         END
      END
     return erg;
    END

   if (Memo("SSUB"))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[3],MModReg+MModLReg,True,&DReg);
       if ((AdrMode!=ModNone) AND (ChkUnit(DReg,L1,L2)))
        switch (AdrMode)
         BEGIN
          case ModReg:
           AddDest(DReg);
           DecodeAdr(ArgStr[1],MModReg+MModImm,True,&S1Reg);
           switch (AdrMode)
            BEGIN
             case ModReg:
              AddSrc(S1Reg);
              if (DecodeAdr(ArgStr[2],MModReg,True,&S2Reg))
               BEGIN
                if ((ThisCross) AND (NOT IsCross(S1Reg)) AND (NOT IsCross(S2Reg))) WrError(1350);
                else if ((IsCross(S1Reg)) AND (IsCross(S2Reg))) WrError(1350);
                else if (IsCross(S1Reg))
                 BEGIN
                  ThisCross=True;
                  erg=CodeL(0x1f,DReg,S1Reg,S2Reg);
                 END
                else
                 BEGIN
                  SetCross(S2Reg);
                  erg=CodeL(0x0f,DReg,S1Reg,S2Reg);
                 END
               END
              break;
             case ModImm:
              if (DecodeAdr(ArgStr[2],MModReg,True,&S2Reg))
               BEGIN
                if ((ThisCross) AND (NOT IsCross(S2Reg)<16)) WrError(1350);
                else
                 BEGIN
                  AddSrc(S2Reg); SetCross(S2Reg);
                  erg=CodeL(0x0e,DReg,S1Reg,S2Reg);
                 END
               END
              break;
            END
           break;
          case ModLReg:
           AddLDest(DReg);
           if (DecodeAdr(ArgStr[1],MModImm,True,&S1Reg))
            BEGIN
             if (DecodeAdr(ArgStr[2],MModLReg,True,&S2Reg))
              BEGIN
               if ((ThisCross) OR (IsCross(S2Reg))) WrError(1350);
               else
                BEGIN
                 AddLSrc(S2Reg);
                 erg=CodeL(0x2c,DReg,S1Reg,S2Reg);
                END
              END
            END
           break;
         END
      END;
     return erg;
    END

   /* Spruenge */

   if (Memo("B"))  
    BEGIN
     if (ArgCnt != 1) WrError(1350);
     else if (ThisCross) WrError(1350);
     else if ((ThisUnit != NoUnit) AND (ThisUnit != S1) AND (ThisUnit != S2)) WrError(1350);
     else
      BEGIN
       OK = True; S2Reg = 0; WithImm=False; Code1 = 0;
       if (strcasecmp(ArgStr[1],"IRP") == 0)
        BEGIN
         Code1 = 0x03; S2Reg = 0x06;
        END
       else if (strcasecmp(ArgStr[1],"NRP")==0)
        BEGIN
         Code1 = 0x03; S2Reg = 0x07;
        END
       else if (DecodeReg(ArgStr[1], &S2Reg, &OK, False))
        BEGIN
         if (OK) WrError(1350); OK = NOT OK;
         Code1 = 0x0d;
        END
       else WithImm = OK = True;
       if (OK)
        BEGIN
         if (WithImm)
          BEGIN
           if (ThisUnit == NoUnit)
            ThisUnit = (UnitUsed(S1)) ? S2 : S1;
           UnitFlag = Ord(ThisUnit == S2);
           /* branches relative to fetch packet */
           Dist = EvalIntExpression(ArgStr[1] , Int32, &OK) - (PacketAddr & (~31));
           if (OK)
            BEGIN
             if ((Dist & 3) != 0) WrError(1325);
             else if ((NOT SymbolQuestionable) AND ((Dist > 0x3fffff) OR (Dist < -0x400000))) WrError(1370);
             else
              BEGIN
               ThisInst = 0x10 + ((Dist & 0x007ffffc) << 5) + (UnitFlag << 1);
               ThisAbsBranch = True;
               erg = True;
              END
            END
          END
         else
          BEGIN
           if (ChkUnit(0x10, S1, S2))
            BEGIN
             SetCross(S2Reg);
             erg = CodeS(Code1, 0, 0, S2Reg);
            END
          END
        END
      END
     return erg;
    END

   WrXError(1200,OpPart);

   return erg;
END

        static void ChkPacket(void)
BEGIN
   LongWord EndAddr,Mask;
   LongInt z,z1,z2;
   Integer RegReads[32];
   char TestUnit[4];
   int BranchCnt;

   /* nicht ueber 8er-Grenze */

   EndAddr=PacketAddr+((ParCnt << 2)-1);
   if ((PacketAddr >> 5)!=(EndAddr >> 5)) WrError(2000);

   /* doppelte Units,Crosspaths,Adressierer,Zielregister */

   for (z1=0; z1<ParCnt; z1++)
    for (z2=z1+1; z2<ParCnt; z2++)
     if ((ParRecs[z1].OpCode >> 28)==(ParRecs[z2].OpCode >> 28))
      BEGIN
       /* doppelte Units */
       if ((ParRecs[z1].U!=NoUnit) AND (ParRecs[z1].U==ParRecs[z2].U))
        WrXError(2001,UnitNames[ParRecs[z1].U]);

       /* Crosspaths */
       z=ParRecs[z1].CrossUsed & ParRecs[z2].CrossUsed;
       if (z!=0)
        BEGIN
         *TestUnit=z+'0'; TestUnit[1]='X'; TestUnit[2]='\0';
         WrXError(2001,TestUnit);
        END

       z=ParRecs[z1].AddrUsed & ParRecs[z2].AddrUsed;
       /* Adressgeneratoren */
       if ((z & 1)==1) WrXError(2001,"Addr. A");
       if ((z & 2)==2) WrXError(2001,"Addr. B");
       /* Hauptspeicherpfade */
       if ((z & 4)==4) WrXError(2001,"LdSt. A");
       if ((z & 8)==8) WrXError(2001,"LdSt. B");

       /* ueberlappende Zielregister */
       z=ParRecs[z1].DestMask & ParRecs[z2].DestMask;
       if (z!=0) WrXError(2006,RegName(FindReg(z)));

       if ((ParRecs[z1].U & 1)==(ParRecs[z2].U & 1))
        BEGIN
         TestUnit[0]=ParRecs[z1].U-NoUnit-1+'A';
         TestUnit[1]='\0';

         /* mehrere Long-Reads */
         if ((ParRecs[z1].LongSrc) AND (ParRecs[z2].LongSrc))
          WrXError(2002,TestUnit);

         /* mehrere Long-Writes */
         if ((ParRecs[z1].LongDest) AND (ParRecs[z2].LongDest))
          WrXError(2003,TestUnit);

         /* Long-Read mit Store */
         if ((ParRecs[z1].StoreUsed) AND (ParRecs[z2].LongSrc))
          WrXError(2004,TestUnit);
         if ((ParRecs[z2].StoreUsed) AND (ParRecs[z1].LongSrc))
          WrXError(2004,TestUnit);
        END
      END

   for (z2=0; z2<32; RegReads[z2++]=0);
   for (z1=0; z1<ParCnt; z1++)
    BEGIN
     Mask=1;
     for (z2=0; z2<32; z2++)
      BEGIN
       if ((ParRecs[z1].SrcMask & Mask)!=0) RegReads[z2]++;
       if ((ParRecs[z1].SrcMask2 & Mask)!=0) RegReads[z2]++;
       Mask=Mask << 1;
      END
    END

   /* Register mehr als 4mal gelesen */

   for (z1=0; z1<32; z1++)
    if (RegReads[z1]>4) WrXError(2005,RegName(z1));

   /* more than one branch to an absolute address */

   BranchCnt = 0;
   for (z1 = 0; z1 < ParCnt; z1++)
     if (ParRecs[z1].AbsBranch)
       BranchCnt++;
   if (BranchCnt > 1)
     WrError(2008);
END

        static void MakeCode_3206X(void)
BEGIN
   CodeLen=0; DontPrint=False;

   /* zu ignorierendes */

   if ((*OpPart=='\0') AND (*LabPart=='\0')) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   /* Flags zuruecksetzen */

   ThisPar=False; Condition=0;

   /* Optionen aus Label holen */

   if (*LabPart!='\0')
    if ((strcmp(LabPart,"||")==0) OR (*LabPart=='['))
     if (NOT CheckOpt(LabPart)) return;

   /* eventuell falsche Mnemonics verwerten */

   if (strcmp(OpPart,"||")==0)
    if (NOT ReiterateOpPart()) return;
   if (*OpPart=='[')
    if (NOT ReiterateOpPart()) return;

   if (Memo("")) return;

   /* Attribut auswerten */

   ThisUnit=NoUnit; ThisCross=False;
   if (*AttrPart!='\0')
    BEGIN
     if (mytoupper(AttrPart[strlen(AttrPart)-1])=='X')
      BEGIN
       ThisCross=True;
       AttrPart[strlen(AttrPart)-1]='\0';
      END
     if (*AttrPart=='\0') ThisUnit=NoUnit;
     else 
      for (; ThisUnit!=LastUnit; ThisUnit++)
       if (strcasecmp(AttrPart,UnitNames[ThisUnit])==0) break;
     if (ThisUnit==LastUnit)
      BEGIN
       WrError(1107); return;
      END
     if (((ThisUnit==D1) OR (ThisUnit==D2)) AND (ThisCross))
      BEGIN
       WrError(1350); return;
      END
    END

   /* falls nicht parallel, vorherigen Stack durchpruefen und verwerfen */

   if ((NOT ThisPar) AND (ParCnt > 0))
    BEGIN
     ChkPacket();
     ParCnt=0; PacketAddr=EProgCounter();
    END

   /* dekodieren */

   ThisSrc = 0; ThisSrc2 = 0; ThisDest = 0;
   ThisAddr = 0;
   ThisStore = ThisAbsBranch = False;
   ThisLong = 0;
   if (NOT DecodeInst()) return;

   /* einsortieren */

   ParRecs[ParCnt].OpCode = (Condition << 28) + ThisInst;
   ParRecs[ParCnt].U = ThisUnit;
   if (ThisCross)
    switch (ThisUnit)
     BEGIN
      case L1: case S1: case M1: case D1: ParRecs[ParCnt].CrossUsed=1; break;
      default: ParRecs[ParCnt].CrossUsed=2;
     END
   else ParRecs[ParCnt].CrossUsed = 0;
   ParRecs[ParCnt].AddrUsed = ThisAddr;
   ParRecs[ParCnt].SrcMask = ThisSrc;
   ParRecs[ParCnt].SrcMask2 = ThisSrc2;
   ParRecs[ParCnt].DestMask = ThisDest;
   ParRecs[ParCnt].LongSrc = ((ThisLong & 1) == 1);
   ParRecs[ParCnt].LongDest = ((ThisLong & 2) == 2);
   ParRecs[ParCnt].StoreUsed = ThisStore;
   ParRecs[ParCnt].AbsBranch = ThisAbsBranch;
   ParCnt++;

   /* wenn mehr als eine Instruktion, Ressourcenkonflikte abklopfen und
     vorherige Instruktion zuruecknehmen */

   if (ParCnt>1)
    BEGIN
     RetractWords(4);
     DAsmCode[CodeLen >> 2]=ParRecs[ParCnt-2].OpCode | 1;
     CodeLen+=4;
    END

   /* aktuelle Instruktion auswerfen: fuer letzte kein Parallelflag setzen */

   DAsmCode[CodeLen >> 2]=ParRecs[ParCnt-1].OpCode;
   CodeLen+=4;
END

/*-------------------------------------------------------------------------*/

        static void AddLinAdd(char *NName, LongInt NCode)
BEGIN
   if (InstrZ>=LinAddCnt) exit(255);
   LinAddOrders[InstrZ].Name=NName;
   LinAddOrders[InstrZ].Code=NCode;
   AddInstTable(InstTable,NName,InstrZ++,DecodeLinAdd);
END

        static void AddCmp(char *NName, LongInt NCode)
BEGIN
   if (InstrZ>=CmpCnt) exit(255);
   CmpOrders[InstrZ].Name=NName;
   CmpOrders[InstrZ++].Code=NCode;
END

        static void AddMem(char *NName, LongInt NCode, LongInt NScale)
BEGIN
   if (InstrZ>=MemCnt) exit(255);
   MemOrders[InstrZ].Name=NName;
   MemOrders[InstrZ].Code=NCode;
   MemOrders[InstrZ].Scale=NScale;
   AddInstTable(InstTable,NName,InstrZ++,DecodeMemO);
END

        static void AddMul(char *NName, LongInt NCode,
                           Boolean NDSign,Boolean NSSign1,Boolean NSSign2, Boolean NMay)
BEGIN
   if (InstrZ>=MulCnt) exit(255);
   MulOrders[InstrZ].Name=NName;
   MulOrders[InstrZ].Code=NCode;
   MulOrders[InstrZ].DSign=NDSign;
   MulOrders[InstrZ].SSign1=NSSign1;
   MulOrders[InstrZ].SSign2=NSSign2;
   MulOrders[InstrZ].MayImm=NMay;
   AddInstTable(InstTable,NName,InstrZ++,DecodeMul);
END

        static void AddCtrl(char *NName, LongInt NCode,
                            Boolean NWr, Boolean NRd)
BEGIN
   if (InstrZ>=CtrlCnt) exit(255);
   CtrlRegs[InstrZ].Name=NName;
   CtrlRegs[InstrZ].Code=NCode;
   CtrlRegs[InstrZ].Wr=NWr;
   CtrlRegs[InstrZ++].Rd=NRd;
END

        static void InitFields(void)
BEGIN
   InstTable=CreateInstTable(203);

   AddInstTable(InstTable,"IDLE",0,DecodeIDLE);
   AddInstTable(InstTable,"NOP",0,DecodeNOP);
   AddInstTable(InstTable,"STP",0,DecodeSTP);
   AddInstTable(InstTable,"ABS",0,DecodeABS);
   AddInstTable(InstTable,"ADD",0,DecodeADD);
   AddInstTable(InstTable,"ADDU",0,DecodeADDU);
   AddInstTable(InstTable,"SUB",0,DecodeSUB);
   AddInstTable(InstTable,"SUBU",0,DecodeSUBU);
   AddInstTable(InstTable,"SUBC",0,DecodeSUBC);
   AddInstTable(InstTable,"ADDK",0,DecodeADDK);
   AddInstTable(InstTable,"ADD2",0,DecodeADD2_SUB2);
   AddInstTable(InstTable,"SUB2",1,DecodeADD2_SUB2);
   AddInstTable(InstTable,"AND",0x1f7b,DecodeLogic);
   AddInstTable(InstTable,"OR",0x1b7f,DecodeLogic);
   AddInstTable(InstTable,"XOR",0x0b6f,DecodeLogic);
   AddInstTable(InstTable,"MV",0,DecodeMV);
   AddInstTable(InstTable,"NEG",0,DecodeNEG);
   AddInstTable(InstTable,"NOT",0,DecodeNOT);
   AddInstTable(InstTable,"ZERO",0,DecodeZERO);

   LinAddOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*LinAddCnt); InstrZ=0;
   AddLinAdd("ADDAB",0x30); AddLinAdd("ADDAH",0x34); AddLinAdd("ADDAW",0x38);
   AddLinAdd("SUBAB",0x31); AddLinAdd("SUBAH",0x35); AddLinAdd("SUBAW",0x39);

   CmpOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*CmpCnt); InstrZ=0;
   AddCmp("CMPEQ",0x50); AddCmp("CMPGT",0x44); AddCmp("CMPGTU",0x4c);
   AddCmp("CMPLT",0x54); AddCmp("CMPLTU",0x5c);

   MemOrders=(MemOrder *) malloc(sizeof(MemOrder)*MemCnt); InstrZ=0;
   AddMem("LDB",2,1);  AddMem("LDH",4,2);  AddMem("LDW",6,4);
   AddMem("LDBU",1,1); AddMem("LDHU",0,2); AddMem("STB",3,1);
   AddMem("STH",5,2);  AddMem("STW",7,4);

   MulOrders=(MulOrder *) malloc(sizeof(MulOrder)*MulCnt); InstrZ=0;
   AddMul("MPY"    ,0x19,True ,True ,True ,True );
   AddMul("MPYU"   ,0x1f,False,False,False,False);
   AddMul("MPYUS"  ,0x1d,True ,False,True ,False);
   AddMul("MPYSU"  ,0x1b,True ,True ,False,True );
   AddMul("MPYH"   ,0x01,True ,True ,True ,False);
   AddMul("MPYHU"  ,0x07,False,False,False,False);
   AddMul("MPYHUS" ,0x05,True ,False,True ,False);
   AddMul("MPYHSU" ,0x03,True ,True ,False,False);
   AddMul("MPYHL"  ,0x09,True ,True ,True ,False);
   AddMul("MPYHLU" ,0x0f,False,False,False,False);
   AddMul("MPYHULS",0x0d,True ,False,True ,False);
   AddMul("MPYHSLU",0x0b,True ,True ,False,False);
   AddMul("MPYLH"  ,0x11,True ,True ,True ,False);
   AddMul("MPYLHU" ,0x17,False,False,False,False);
   AddMul("MPYLUHS",0x15,True ,False,True ,False);
   AddMul("MPYLSHU",0x13,True ,True ,False,False);
   AddMul("SMPY"   ,0x1a,True ,True ,True ,False);
   AddMul("SMPYHL" ,0x0a,True ,True ,True ,False);
   AddMul("SMPYLH" ,0x12,True ,True ,True ,False);
   AddMul("SMPYH"  ,0x02,True ,True ,True ,False);

   CtrlRegs=(CtrlReg *) malloc(sizeof(CtrlReg)*CtrlCnt); InstrZ=0;
   AddCtrl("AMR"    , 0,True ,True );
   AddCtrl("CSR"    , 1,True ,True );
   AddCtrl("IFR"    , 2,False,True );
   AddCtrl("ISR"    , 2,True ,False);
   AddCtrl("ICR"    , 3,True ,False);
   AddCtrl("IER"    , 4,True ,True );
   AddCtrl("ISTP"   , 5,True ,True );
   AddCtrl("IRP"    , 6,True ,True );
   AddCtrl("NRP"    , 7,True ,True );
   AddCtrl("IN"     , 8,False,True );
   AddCtrl("OUT"    , 9,True ,True );
   AddCtrl("PCE1"   ,16,False,True );
   AddCtrl("PDATA_O",15,True ,True );
END

        static void DeinitFields(void)
BEGIN
   DestroyInstTable(InstTable);
   free(LinAddOrders);
   free(CmpOrders);
   free(MemOrders);
   free(MulOrders);
   free(CtrlRegs);
END

/*------------------------------------------------------------------------*/

        static Boolean IsDef_3206X(void)
BEGIN
   return (strcmp(LabPart,"||")==0) OR (*LabPart=='[');
END

        static void SwitchFrom_3206X(void)
BEGIN
   if (ParCnt>1) ChkPacket();
   DeinitFields();
END

        static void SwitchTo_3206X(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=False;

   PCSymbol="$"; HeaderID=0x47; NOPCode=0x00000000;
   DivideChars=","; HasAttrs=True; AttrChars=".";
   SetIsOccupied=True;

   ValidSegs=1<<SegCode;
   Grans[SegCode]=1; ListGrans[SegCode]=4; SegInits[SegCode]=0;
#ifdef __STDC__
   SegLimits[SegCode] = 0xfffffffful;
#else
   SegLimits[SegCode] = 0xffffffffl;
#endif

   MakeCode=MakeCode_3206X; IsDef=IsDef_3206X;
   SwitchFrom=SwitchFrom_3206X; InitFields();

   ParCnt=0; PacketAddr=0;
END

        void code3206x_init(void)
BEGIN
   CPU32060=AddCPU("32060",SwitchTo_3206X);
END
