/* codem16.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator Mitsubishi M16                                              */
/*                                                                           */
/* Historie: 27.12.1996 Grundsteinlegung                                     */
/*            3. 1.1999 ChkPC-Anpassung                                      */
/*            9. 3.2000 'ambigious else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: codem16.c,v 1.2 2004/05/29 11:33:03 alfred Exp $                     */
/*****************************************************************************
 * $Log: codem16.c,v $
 * Revision 1.2  2004/05/29 11:33:03  alfred
 * - relocated DecodeIntelPseudo() into own module
 *
 *****************************************************************************/

#include "stdinc.h"

#include <string.h>
#include <ctype.h>

#include "bpemu.h"
#include "nls.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "intpseudo.h"
#include "codevars.h"

#define ModNone      (-1)
#define ModReg       0
#define MModReg      (1 << ModReg)
#define ModIReg      1
#define MModIReg     (1 << ModIReg)
#define ModDisp16    2
#define MModDisp16   (1 << ModDisp16)
#define ModDisp32    3
#define MModDisp32   (1 << ModDisp32)
#define ModImm       4
#define MModImm      (1 << ModImm)
#define ModAbs16     5
#define MModAbs16    (1 << ModAbs16)
#define ModAbs32     6
#define MModAbs32    (1 << ModAbs32)
#define ModPCRel16   7
#define MModPCRel16  (1 << ModPCRel16)
#define ModPCRel32   8
#define MModPCRel32  (1 << ModPCRel32)
#define ModPop       9
#define MModPop      (1 << ModPop)
#define ModPush      10
#define MModPush     (1 << ModPush)
#define ModRegChain  11
#define MModRegChain (1 << ModRegChain)
#define ModPCChain   12
#define MModPCChain  (1 << ModPCChain)
#define ModAbsChain  13
#define MModAbsChain (1 << ModAbsChain)

#define Mask_RegOnly    (MModReg)
#define Mask_AllShort   (MModReg+MModIReg+MModDisp16+MModImm+MModAbs16+MModAbs32+MModPCRel16+MModPCRel32+MModPop+MModPush+MModPCChain+MModAbsChain)
#define Mask_AllGen     (Mask_AllShort+MModDisp32+MModRegChain)
#define Mask_NoImmShort (Mask_AllShort-MModImm)
#define Mask_NoImmGen   (Mask_AllGen-MModImm)
#define Mask_MemShort   (Mask_NoImmShort-MModReg)
#define Mask_MemGen     (Mask_NoImmGen-MModReg)

#define Mask_Source     (Mask_AllGen-MModPush)
#define Mask_Dest       (Mask_NoImmGen-MModPop)
#define Mask_PureDest   (Mask_NoImmGen-MModPush-MModPop)
#define Mask_PureMem    (Mask_MemGen-MModPush-MModPop)

#define FixedOrderCount 7
#define OneOrderCount 13
#define GE2OrderCount 11
#define BitOrderCount 6
#define GetPutOrderCount 8
#define BFieldOrderCount 4
#define MulOrderCount 4
#define ConditionCount 14
#define LogOrderCount 3

typedef struct
         {
          char *Name;
          Word Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          Word Mask;
          Byte OpMask;
          Word Code;
         } OneOrder;

typedef struct
         {
          char *Name;
          Word Mask1,Mask2;
          Word SMask1,SMask2;
          Word Code;
          Boolean Signed;
         } GE2Order;

typedef struct
         {
          char *Name;
          Boolean MustByte;
          Word Code1,Code2;
         } BitOrder;

typedef struct
         {
          char *Name;
          ShortInt Size;
          Word Code;
          Boolean Turn;
         } GetPutOrder;


static CPUVar CPUM16;

static String Format;
static Byte FormatCode;
static ShortInt DOpSize,OpSize[5];
static Word AdrMode[5];
static ShortInt AdrType[5];
static Byte AdrCnt1[5],AdrCnt2[5];
static Word AdrVals[5][8];

static Byte OptionCnt;
static char Options[2][5];

static FixedOrder *FixedOrders;
static OneOrder *OneOrders;
static GE2Order *GE2Orders;
static BitOrder *BitOrders;
static GetPutOrder *GetPutOrders;
static char **BFieldOrders;
static char **MulOrders;
static char **Conditions;
static char **LogOrders;

/*------------------------------------------------------------------------*/

        static void AddFixed(char *NName, Word NCode)
BEGIN
   if (InstrZ>=FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
END

        static void AddOne(char *NName, Byte NOpMask, Word NMask, Word NCode)
BEGIN
   if (InstrZ>=OneOrderCount) exit(255);
   OneOrders[InstrZ].Name=NName;
   OneOrders[InstrZ].Code=NCode;
   OneOrders[InstrZ].Mask=NMask;
   OneOrders[InstrZ++].OpMask=NOpMask;
END

        static void AddGE2(char *NName, Word NMask1, Word NMask2,
                           Byte NSMask1, Byte NSMask2, Word NCode,
                           Boolean NSigned)
BEGIN
   if (InstrZ>=GE2OrderCount) exit(255);
   GE2Orders[InstrZ].Name=NName;
   GE2Orders[InstrZ].Mask1=NMask1;
   GE2Orders[InstrZ].Mask2=NMask2;
   GE2Orders[InstrZ].SMask1=NSMask1;
   GE2Orders[InstrZ].SMask2=NSMask2;
   GE2Orders[InstrZ].Code=NCode;
   GE2Orders[InstrZ++].Signed=NSigned;
END

        static void AddBit(char *NName, Boolean NMust, Word NCode1, Word NCode2)
BEGIN
   if (InstrZ>=BitOrderCount) exit(255);
   BitOrders[InstrZ].Name=NName;
   BitOrders[InstrZ].Code1=NCode1;
   BitOrders[InstrZ].Code2=NCode2;
   BitOrders[InstrZ++].MustByte=NMust;
END

        static void AddGetPut(char *NName, Byte NSize, Word NCode, Boolean NTurn)
BEGIN
   if (InstrZ>=GetPutOrderCount) exit(255);
   GetPutOrders[InstrZ].Name=NName;
   GetPutOrders[InstrZ].Code=NCode;
   GetPutOrders[InstrZ].Turn=NTurn;
   GetPutOrders[InstrZ++].Size=NSize;
END

        static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCount); InstrZ=0;
   AddFixed("NOP"  ,0x1bd6); AddFixed("PIB"  ,0x0bd6);
   AddFixed("RIE"  ,0x08f7); AddFixed("RRNG" ,0x3bd6);
   AddFixed("RTS"  ,0x2bd6); AddFixed("STCTX",0x07d6);
   AddFixed("REIT" ,0x2fd6);

   OneOrders=(OneOrder *) malloc(sizeof(OneOrder)*OneOrderCount); InstrZ=0;
   AddOne("ACS"   ,0x00,Mask_PureMem,                    0x8300);
   AddOne("NEG"   ,0x07,Mask_PureDest,                   0xc800);
   AddOne("NOT"   ,0x07,Mask_PureDest,                   0xcc00);
   AddOne("JMP"   ,0x00,Mask_PureMem,                    0x8200);
   AddOne("JSR"   ,0x00,Mask_PureMem,                    0xaa00);
   AddOne("LDCTX" ,0x00,MModIReg+MModDisp16+MModDisp32+
          MModAbs16+MModAbs32+MModPCRel16+MModPCRel32,  0x8600);
   AddOne("LDPSB" ,0x02,Mask_Source,                     0xdb00);
   AddOne("LDPSM" ,0x02,Mask_Source,                     0xdc00);
   AddOne("POP"   ,0x04,Mask_PureDest,                   0x9000);
   AddOne("PUSH"  ,0x04,Mask_Source-MModPop,             0xb000);
   AddOne("PUSHA" ,0x00,Mask_PureMem,                    0xa200);
   AddOne("STPSB" ,0x02,Mask_Dest,                       0xdd00);
   AddOne("STPSM" ,0x02,Mask_Dest,                       0xde00);

   GE2Orders=(GE2Order *) malloc(sizeof(GE2Order)*GE2OrderCount); InstrZ=0;
   AddGE2("ADDU" ,Mask_Source,Mask_PureDest,7,7,0x0400,False);
   AddGE2("ADDX" ,Mask_Source,Mask_PureDest,7,7,0x1000,True );
   AddGE2("SUBU" ,Mask_Source,Mask_PureDest,7,7,0x0c00,False);
   AddGE2("SUBX" ,Mask_Source,Mask_PureDest,7,7,0x1800,True );
   AddGE2("CMPU" ,Mask_Source,Mask_PureDest,7,7,0x8400,False);
   AddGE2("LDC"  ,Mask_Source,Mask_PureDest,7,4,0x9800,True );
   AddGE2("LDP"  ,Mask_Source,Mask_PureMem ,7,7,0x9c00,True );
   AddGE2("MOVU" ,Mask_Source,Mask_Dest    ,7,7,0x8c00,True );
   AddGE2("REM"  ,Mask_Source,Mask_PureDest,7,7,0x5800,True );
   AddGE2("REMU" ,Mask_Source,Mask_PureDest,7,7,0x5c00,True );
   AddGE2("ROT"  ,Mask_Source,Mask_PureDest,1,7,0x3800,True );

   BitOrders=(BitOrder *) malloc(sizeof(BitOrder)*BitOrderCount); InstrZ=0;
   AddBit("BCLR" ,False,0xb400,0xa180);
   AddBit("BCLRI",True ,0xa400,0x0000);
   AddBit("BNOT" ,False,0xb800,0x0000);
   AddBit("BSET" ,False,0xb000,0x8180);
   AddBit("BSETI",True ,0xa000,0x81c0);
   AddBit("BTST" ,False,0xbc00,0xa1c0);

   GetPutOrders=(GetPutOrder *) malloc(sizeof(GetPutOrder)*GetPutOrderCount); InstrZ=0;
   AddGetPut("GETB0",0,0xc000,False);
   AddGetPut("GETB1",0,0xc400,False);
   AddGetPut("GETB2",0,0xc800,False);
   AddGetPut("GETH0",1,0xcc00,False);
   AddGetPut("PUTB0",0,0xd000,True );
   AddGetPut("PUTB1",0,0xd400,True );
   AddGetPut("PUTB2",0,0xd800,True );
   AddGetPut("PUTH0",1,0xdc00,True );

   BFieldOrders=(char **) malloc(sizeof(char *)*BFieldOrderCount); InstrZ=0;
   BFieldOrders[InstrZ++]="BFCMP"; BFieldOrders[InstrZ++]="BFCMPU";
   BFieldOrders[InstrZ++]="BFINS"; BFieldOrders[InstrZ++]="BFINSU";

   MulOrders=(char **) malloc(sizeof(char *)*MulOrderCount); InstrZ=0;
   MulOrders[InstrZ++]="MUL"; MulOrders[InstrZ++]="MULU";
   MulOrders[InstrZ++]="DIV"; MulOrders[InstrZ++]="DIVU";

   Conditions=(char **) malloc(sizeof(char *)*ConditionCount); InstrZ=0;
   Conditions[InstrZ++]="XS"; Conditions[InstrZ++]="XC";
   Conditions[InstrZ++]="EQ"; Conditions[InstrZ++]="NE";
   Conditions[InstrZ++]="LT"; Conditions[InstrZ++]="GE";
   Conditions[InstrZ++]="LE"; Conditions[InstrZ++]="GT";
   Conditions[InstrZ++]="VS"; Conditions[InstrZ++]="VC";
   Conditions[InstrZ++]="MS"; Conditions[InstrZ++]="MC";
   Conditions[InstrZ++]="FS"; Conditions[InstrZ++]="FC";

   LogOrders=(char **) malloc(sizeof(char *)*LogOrderCount); InstrZ=0;
   LogOrders[InstrZ++]="AND"; LogOrders[InstrZ]="OR"; LogOrders[InstrZ++]="XOR";
END

        static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(OneOrders);
   free(GE2Orders);
   free(BitOrders);
   free(GetPutOrders);
   free(BFieldOrders);
   free(MulOrders);
   free(Conditions);
   free(LogOrders);
END

/*------------------------------------------------------------------------*/

typedef enum {DispSizeNone,DispSize4,DispSize4Eps,DispSize16,DispSize32} DispSize;
typedef struct _TChainRec
                {
                 struct _TChainRec *Next;
                 Byte RegCnt;
                 Word Regs[5],Scales[5];
                 LongInt DispAcc;
                 Boolean HasDisp;
                 DispSize DSize;
                } *PChainRec,TChainRec;
static Boolean ErrFlag;


        static Boolean IsD4(LongInt inp)
BEGIN
   return ((inp>=-32) AND (inp<=28));
END

        static Boolean IsD16(LongInt inp)
BEGIN
   return ((inp>=-0x8000) AND (inp<=0x7fff));
END

        static Boolean DecodeReg(char *Asc, Word *Erg)
BEGIN
   Boolean IO;

   if (strcasecmp(Asc,"SP")==0) *Erg=15;
   else if (strcasecmp(Asc,"FP")==0) *Erg=14;
   else if ((strlen(Asc)>1) AND (toupper(*Asc)=='R'))
    BEGIN
     *Erg=ConstLongInt(Asc+1,&IO);
     return ((IO) AND (*Erg<=15));
    END
   else return False;
   return True;
END

        static void SplitSize(char *s, DispSize *Erg)
BEGIN
   int l=strlen(s);

   if ((l>2) AND (s[l-1]=='4') AND (s[l-2]==':'))
    BEGIN
     *Erg=DispSize4;
     s[l-2]='\0';
    END
   else if ((l>3) AND (s[l-1]=='6') AND (s[l-2]=='1') AND (s[l-3]==':'))
    BEGIN
     *Erg=DispSize16;
     s[l-3]='\0';
    END
   else if ((l>3) AND (s[l-1]=='2') AND (s[l-2]=='3') AND (s[l-3]==':'))
    BEGIN
     *Erg=DispSize32;
     s[l-3]='\0';
    END
END

        static void DecideAbs(LongInt Disp, DispSize Size, Word Mask, int Index)
BEGIN
   switch (Size)
    BEGIN
     case DispSize4:
      Size=DispSize16; break;
     case DispSizeNone:
      if ((IsD16(Disp)) AND ((Mask & MModAbs16)!=0)) Size=DispSize16;
      else Size=DispSize32;
      break;
     default:
      break;
    END

   switch (Size)
    BEGIN
     case DispSize16:
      if (ChkRange(Disp,-0x8000,0x7fff))
       BEGIN
        AdrType[Index]=ModAbs16; AdrMode[Index]=0x09;
        AdrVals[Index][0]=Disp & 0xffff; AdrCnt1[Index]=2;
       END
      break;
     case DispSize32:
      AdrType[Index]=ModAbs32; AdrMode[Index]=0x0a;
      AdrVals[Index][0]=Disp >> 16;
      AdrVals[Index][1]=Disp & 0xffff; AdrCnt1[Index]=4;
      break;
     default:
      WrError(10000);
    END
END

        static void SetError(Word Code)
BEGIN
   WrError(Code); ErrFlag=True;
END

        static PChainRec DecodeChain(char *Asc)
BEGIN
   PChainRec Rec;
   String Part,SReg;
   int z;
   char *p;
   Boolean OK;
   Byte Scale;

   ChkStack();
   Rec=(PChainRec) malloc(sizeof(TChainRec));
   Rec->Next=Nil; Rec->RegCnt=0; Rec->DispAcc=0; Rec->HasDisp=False;
   Rec->DSize=DispSizeNone;

   while ((*Asc!='\0') AND (NOT ErrFlag))
    BEGIN

     /* eine Komponente abspalten */

     p=QuotPos(Asc,',');
     if (p==Nil)
      BEGIN
       strmaxcpy(Part,Asc,255); *Asc='\0'; 
      END
     else
      BEGIN
       *p='\0'; strmaxcpy(Part,Asc,255); strcpy(Asc,p+1);
      END

     strcpy(SReg,Part); p=QuotPos(SReg,'*'); if (p!=Nil) *p='\0';

     /* weitere Indirektion ? */

     if (*Part=='@')
      if (Rec->Next!=Nil) SetError(1350);
      else
       BEGIN
        strcpy(Part,Part+1);
        if (IsIndirect(Part))
         BEGIN
          strcpy(Part,Part+1); Part[strlen(Part)-1]='\0';
         END
        Rec->Next=DecodeChain(Part);
       END

     /* Register, mit Skalierungsfaktor ? */

     else if (DecodeReg(SReg,Rec->Regs+Rec->RegCnt))
      BEGIN
       if (Rec->RegCnt>=5) SetError(1350);
       else
        BEGIN
         FirstPassUnknown=False;
         if (p==Nil)
          BEGIN
           OK=True; Scale=1;
          END
         else Scale=EvalIntExpression(p+1,UInt4,&OK);
         if (FirstPassUnknown) Scale=1;
         if (NOT OK) ErrFlag=True;
         else if ((Scale!=1) AND (Scale!=2) AND (Scale!=4) AND (Scale!=8)) SetError(1350);
         else
          BEGIN
           Rec->Scales[Rec->RegCnt]=0;
           while (Scale>1)
            BEGIN
             Rec->Scales[Rec->RegCnt]++; Scale=Scale >> 1;
            END
           Rec->RegCnt++;
          END
        END
      END

     /* PC, mit Skalierungsfaktor ? */

     else if (strcasecmp(SReg,"PC")==0)
      BEGIN
       if (Rec->RegCnt>=5) SetError(1350);
       else
        BEGIN
         FirstPassUnknown=False;
         if (p==Nil)
          BEGIN
           OK=True; Scale=1;
          END
         else Scale=EvalIntExpression(p+1,UInt4,&OK);
         if (FirstPassUnknown) Scale=1;
         if (NOT OK) ErrFlag=True;
         else if ((Scale!=1) AND (Scale!=2) AND (Scale!=4) AND (Scale!=8)) SetError(1350);
         else
          BEGIN
           for (z=Rec->RegCnt-1; z>=0; z--)
            BEGIN
             Rec->Regs[z+1]=Rec->Regs[z];
             Rec->Scales[z+1]=Rec->Scales[z];
            END
           Rec->Scales[0]=0;
           while (Scale>1)
            BEGIN
             Rec->Scales[0]++; Scale=Scale >> 1;
            END
           Rec->Regs[0]=16;
           Rec->RegCnt++;
          END
        END
      END

     /* ansonsten Displacement */

     else
      BEGIN
       SplitSize(Part,&(Rec->DSize));
       Rec->DispAcc+=EvalIntExpression(Part,Int32,&OK);
       if (NOT OK) ErrFlag=True;
       Rec->HasDisp=True;
      END
    END

   if (ErrFlag)
    BEGIN
     free(Rec); return Nil;
    END
   else return Rec;
END

        static Boolean ChkAdr(Word Mask, int Index)
BEGIN
   AdrCnt2[Index]=AdrCnt1[Index] >> 1;
   if ((AdrType[Index]!=-1) AND ((Mask & (1 << AdrType[Index]))==0))
    BEGIN
     AdrCnt1[Index]=AdrCnt2[Index]=0;
     AdrType[Index]=ModNone;
     WrError(1350);
     return False;
    END
   else return (AdrType[Index]!=ModNone);
END

        static Boolean DecodeAdr(char *Asc, int Index, Word Mask)
BEGIN
   LongInt AdrLong,MinReserve,MaxReserve;
   int z,z2,LastChain;
   Boolean OK;
   PChainRec RootChain,RunChain,PrevChain;
   DispSize DSize;

   AdrCnt1[Index]=0; AdrType[Index]=ModNone;

   /* Register ? */

   if (DecodeReg(Asc,AdrMode+Index))
    BEGIN
     AdrType[Index]=ModReg; AdrMode[Index]+=0x10; return ChkAdr(Mask,Index);
    END

   /* immediate ? */

   if (*Asc=='#')
    BEGIN
     switch (OpSize[Index])
      BEGIN
       case -1:
        WrError(1132); OK=False;
        break;
       case 0:
        AdrVals[Index][0]=EvalIntExpression(Asc+1,Int8,&OK) & 0xff;
        if (OK) AdrCnt1[Index]=2;
        break;
       case 1:
        AdrVals[Index][0]=EvalIntExpression(Asc+1,Int16,&OK);
        if (OK) AdrCnt1[Index]=2;
        break;
       case 2:
        AdrLong=EvalIntExpression(Asc+1,Int32,&OK);
        if (OK)
         BEGIN
          AdrVals[Index][0]=AdrLong >> 16;
          AdrVals[Index][1]=AdrLong & 0xffff;
          AdrCnt1[Index]=4;
         END
        break;
      END
     if (OK)
      BEGIN
       AdrType[Index]=ModImm; AdrMode[Index]=0x0c;
      END
     return ChkAdr(Mask,Index);
    END

   /* indirekt ? */

   if (*Asc=='@')
    BEGIN
     strcpy(Asc,Asc+1);
     if (IsIndirect(Asc))
      BEGIN
       strcpy(Asc,Asc+1); Asc[strlen(Asc)-1]='\0';
      END

     /* Stack Push ? */

     if ((strcasecmp(Asc,"-R15")==0) OR (strcasecmp(Asc,"-SP")==0))
      BEGIN
       AdrType[Index]=ModPush; AdrMode[Index]=0x05;
       return ChkAdr(Mask,Index);
      END

     /* Stack Pop ? */

     if ((strcasecmp(Asc,"R15+")==0) OR (strcasecmp(Asc,"SP+")==0))
      BEGIN
       AdrType[Index]=ModPop; AdrMode[Index]=0x04;
       return ChkAdr(Mask,Index);
      END

     /* Register einfach indirekt ? */

     if (DecodeReg(Asc,AdrMode+Index))
      BEGIN
       AdrType[Index]=ModIReg; AdrMode[Index]+=0x30;
       return ChkAdr(Mask,Index);
      END

     /* zusammengesetzt indirekt ? */

     ErrFlag=False;
     RootChain=DecodeChain(Asc);

     if (ErrFlag);

     else if (RootChain==Nil);

     /* absolut ? */

     else if ((RootChain->Next==Nil) AND (RootChain->RegCnt==0))
      BEGIN
       if (NOT RootChain->HasDisp) RootChain->DispAcc=0;
       DecideAbs(RootChain->DispAcc,RootChain->DSize,Mask,Index);
       free(RootChain);
      END

     /* einfaches Register/PC mit Displacement ? */

     else if ((RootChain->Next==Nil) AND (RootChain->RegCnt==1) AND (RootChain->Scales[0]==0))
      BEGIN
       if (RootChain->Regs[0]==16) RootChain->DispAcc-=EProgCounter();

       /* Displacement-Groesse entscheiden */

       if (RootChain->DSize==DispSizeNone)
        BEGIN
         if ((RootChain->DispAcc==0) AND (RootChain->Regs[0]<16));
         else if (IsD16(RootChain->DispAcc))
          RootChain->DSize=DispSize16;
         else RootChain->DSize=DispSize32;
        END

       switch (RootChain->DSize)
        BEGIN

         /* kein Displacement mit Register */

         case DispSizeNone:
          if (ChkRange(RootChain->DispAcc,0,0))
           BEGIN
            if (RootChain->Regs[0]>=16) WrError(1350);
            else
             BEGIN
              AdrType[Index]=ModIReg;
              AdrMode[Index]=0x30+RootChain->Regs[0];
             END
           END
          break;

         /* 16-Bit-Displacement */

         case DispSize4:
         case DispSize16:
          if (ChkRange(RootChain->DispAcc,-0x8000,0x7fff))
           BEGIN
            AdrVals[Index][0]=RootChain->DispAcc & 0xffff; AdrCnt1[Index]=2;
            if (RootChain->Regs[0]==16)
             BEGIN
              AdrType[Index]=ModPCRel16; AdrMode[Index]=0x0d;
             END
            else
             BEGIN
              AdrType[Index]=ModDisp16; AdrMode[Index]=0x20+RootChain->Regs[0];
             END
           END
          break;

         /* 32-Bit-Displacement */

         default:
          AdrVals[Index][1]=RootChain->DispAcc & 0xffff;
          AdrVals[Index][0]=RootChain->DispAcc >> 16; AdrCnt1[Index]=4;
          if (RootChain->Regs[0]==16)
           BEGIN
            AdrType[Index]=ModPCRel32; AdrMode[Index]=0x0e;
           END
          else
           BEGIN
            AdrType[Index]=ModDisp32; AdrMode[Index]=0x40+RootChain->Regs[0];
           END
        END

       free(RootChain);
      END

     /* komplex: dann chained iterieren */

     else
      BEGIN
       /* bis zum innersten Element der Indirektion als Basis laufen */

       RunChain=RootChain;
       while (RunChain->Next!=Nil) RunChain=RunChain->Next;

       /* Entscheidung des Basismodus: die Basis darf nicht skaliert
          sein, und wenn ein Modus nicht erlaubt ist, muessen wir mit
          Base-none anfangen... */

       z=0; while ((z<RunChain->RegCnt) AND (RunChain->Scales[z]!=0)) z++;
       if (z>=RunChain->RegCnt)
        BEGIN
         AdrType[Index]=ModAbsChain; AdrMode[Index]=0x0b;
        END
       else
        BEGIN
         if (RunChain->Regs[z]==16)
          BEGIN
           AdrType[Index]=ModPCChain; AdrMode[Index]=0x0f;
           RunChain->DispAcc-=EProgCounter();
          END
         else
          BEGIN
           AdrType[Index]=ModRegChain;
           AdrMode[Index]=0x60+RunChain->Regs[z];
          END
         for (z2=z; z2<=RunChain->RegCnt-2; z2++)
          BEGIN
           RunChain->Regs[z2]=RunChain->Regs[z2+1];
           RunChain->Scales[z2]=RunChain->Scales[z2+1];
          END
         RunChain->RegCnt--;
        END;

       /* Jetzt ueber die einzelnen Komponenten iterieren */

       LastChain=0;
       while (RootChain!=Nil)
        BEGIN
         RunChain=RootChain; PrevChain=Nil;
         while (RunChain->Next!=Nil)
          BEGIN
           PrevChain=RunChain;
           RunChain=RunChain->Next;
          END;

         /* noch etwas abzulegen ? */

         if ((RunChain->RegCnt!=0) OR (RunChain->HasDisp))
          BEGIN
           LastChain=AdrCnt1[Index] >> 1;

           /* Register ablegen */

           if (RunChain->RegCnt!=0)
            BEGIN
             if (RunChain->Regs[0]==16) AdrVals[Index][LastChain]=0x0600;
             else AdrVals[Index][LastChain]=RunChain->Regs[0] << 10;
             AdrVals[Index][LastChain]+=RunChain->Scales[0] << 5;
             for (z2=0; z2<=RunChain->RegCnt-2; z2++)
              BEGIN
               RunChain->Regs[z2]=RunChain->Regs[z2+1];
               RunChain->Scales[z2]=RunChain->Scales[z2+1];
              END
             RunChain->RegCnt--;
            END
           else AdrVals[Index][LastChain]=0x0200;
           AdrCnt1[Index]+=2;

           /* Displacement ablegen */

           if (RunChain->HasDisp)
            BEGIN
             if ((AdrVals[Index][LastChain] & 0x3e00)==0x0600)
              RunChain->DispAcc-=EProgCounter();

             if (RunChain->DSize==DispSizeNone)
              BEGIN
               MinReserve=32*RunChain->RegCnt; MaxReserve=28*RunChain->RegCnt;
               if (IsD4(RunChain->DispAcc))
                if ((RunChain->DispAcc & 3)==0) DSize=DispSize4;
                else DSize=DispSize16;
               else if ((RunChain->DispAcc>=-32-MinReserve) AND
                        (RunChain->DispAcc<=28+MaxReserve)) DSize=DispSize4Eps;
               else if (IsD16(RunChain->DispAcc)) DSize=DispSize16;
               else if ((RunChain->DispAcc>=-0x8000-MinReserve) AND
                        (RunChain->DispAcc<=0x7fff+MaxReserve)) DSize=DispSize4Eps;
               else DSize=DispSize32;
              END
             else DSize=RunChain->DSize;
             RunChain->DSize=DispSizeNone;

             switch (DSize)
              BEGIN

               /* Fall 1: passt komplett in 4er-Displacement */

               case DispSize4:
                if (ChkRange(RunChain->DispAcc,-32,28))
                 BEGIN
                  if ((RunChain->DispAcc & 3)!=0) WrError(1325);
                  else
                   BEGIN
                    AdrVals[Index][LastChain]+=(RunChain->DispAcc >> 2) & 0x000f;
                    RunChain->HasDisp=False;
                   END
                 END
                break;

               /* Fall 2: passt nicht mehr in naechstkleineres Displacement, aber wir
                  koennen hier schon einen Teil ablegen, um im naechsten Iterations-
                  schritt ein kuerzeres Displacement zu bekommen */

               case DispSize4Eps:
                if (RunChain->DispAcc>0)
                 BEGIN
                  AdrVals[Index][LastChain]+=0x0007;
                  RunChain->DispAcc-=28;
                 END
                else
                 BEGIN
                  AdrVals[Index][LastChain]+=0x0008;
                  RunChain->DispAcc+=32;
                 END
                break;

               /* Fall 3: 16 Bit */

               case DispSize16:
                if (ChkRange(RunChain->DispAcc,-0x8000,0x7fff))
                 BEGIN
                  AdrVals[Index][LastChain]+=0x0011;
                  AdrVals[Index][LastChain+1]=RunChain->DispAcc & 0xffff;
                  AdrCnt1[Index]+=2;
                  RunChain->HasDisp=False;
                 END
                break;

               /* Fall 4: 32 Bit */

               case DispSize32:
                AdrVals[Index][LastChain]+=0x0012;
                AdrVals[Index][LastChain+1]=RunChain->DispAcc >> 16;
                AdrVals[Index][LastChain+2]=RunChain->DispAcc & 0xffff;
                AdrCnt1[Index]+=4;
                RunChain->HasDisp=False;
                break;

               default:
                WrError(10000);
              END
            END
          END

         /* nichts mehr drin: dann ein leeres Steuerwort erzeugen.  Tritt
            auf, falls alles schon im Basisadressierungsbyte verschwunden */

         else if (RunChain!=RootChain)
          BEGIN
           LastChain=AdrCnt1[Index] >> 1;
           AdrVals[Index][LastChain]=0x0200; AdrCnt1[Index]+=2;
          END

         /* nichts mehr drin: wegwerfen
            wenn wir noch nicht ganz vorne angekommen sind, dann ein
            Indirektionsflag setzen */

         if ((RunChain->RegCnt==0) AND (NOT RunChain->HasDisp))
          BEGIN
           if (RunChain!=RootChain) AdrVals[Index][LastChain]+=0x4000;
           if (PrevChain==Nil) RootChain=Nil; else PrevChain->Next=Nil;
           free(RunChain);
          END
        END

       /* Ende-Kennung fuer letztes Glied */

       AdrVals[Index][LastChain]+=0x8000;
      END

     return ChkAdr(Mask,Index);
    END

   /* ansonsten absolut */

   DSize=DispSizeNone;
   SplitSize(Asc,&DSize);
   AdrLong=EvalIntExpression(Asc,Int32,&OK);
   if (OK) DecideAbs(AdrLong,DSize,Mask,Index);

   return ChkAdr(Mask,Index);
END

        static LongInt ImmVal(int Index)
BEGIN
   switch (OpSize[Index])
    BEGIN
     case 0: return (ShortInt) (AdrVals[Index][0] & 0xff);
     case 1: return (Integer) (AdrVals[Index][0]);
     case 2: return (((LongInt)AdrVals[Index][0]) << 16)+((Integer)AdrVals[Index][1]);
     default: WrError(10000); return 0;
    END
END

        static Boolean IsShort(int Index)
BEGIN
   return ((AdrMode[Index] & 0xc0)==0);
END

        static void AdaptImm(int Index, Byte NSize, Boolean Signed)
BEGIN
   switch (OpSize[Index])
    BEGIN
     case 0:
      if (NSize!=0)
       BEGIN
        if (((AdrVals[Index][0] & 0x80)==0x80) AND (Signed))
        AdrVals[Index][0]|=0xff00;
        else AdrVals[Index][0]&=0xff;
        if (NSize==2)
         BEGIN
          if (((AdrVals[Index][0] & 0x8000)==0x8000) AND (Signed))
          AdrVals[Index][1]=0xffff;
          else AdrVals[Index][1]=0;
          AdrCnt1[Index]+=2; AdrCnt2[Index]++;
         END
       END
      break;
     case 1:
      if (NSize==0) AdrVals[Index][0]&=0xff;
      else if (NSize==2)
       BEGIN
        if (((AdrVals[Index][0] & 0x8000)==0x8000) AND (Signed))
         AdrVals[Index][1]=0xffff;
        else AdrVals[Index][1]=0;
        AdrCnt1[Index]+=2; AdrCnt2[Index]++;
       END
      break;
     case 2:
      if (NSize!=2)
       BEGIN
        AdrCnt1[Index]-=2; AdrCnt2[Index]--;
        if (NSize==0) AdrVals[Index][0]&=0xff;
       END
      break;
    END
   OpSize[Index]=NSize;
END

        static ShortInt DefSize(Byte Mask)
BEGIN
   ShortInt z;

   z=2;
   while ((z>=0) AND ((Mask & 4)==0))
    BEGIN
     Mask=(Mask << 1) & 7; z--;
    END
   return z;
END

        static Word RMask(Word No, Boolean Turn)
BEGIN
   return (Turn) ? (0x8000 >> No) : (1 << No);
END

        static Boolean DecodeRegList(char *Asc, Word *Erg, Boolean Turn)
BEGIN
   char Part[11];
   char *p,*p1,*p2;
   Word r1,r2,z;

   if (IsIndirect(Asc))
    BEGIN
     strcpy(Asc,Asc+1); Asc[strlen(Asc)-1]='\0';
    END
   *Erg=0;
   while (*Asc!='\0')
    BEGIN
     p1=strchr(Asc,','); p2=strchr(Asc,'/');
     if ((p1!=Nil) AND (p1<p2)) p=p1; else p=p2;
     if (p==Nil)
      BEGIN
       strmaxcpy(Part,Asc,11); *Asc='\0';
      END
     else
      BEGIN
       *p='\0'; strmaxcpy(Part,Asc,11); strcpy(Asc,p+1);
      END
     p=strchr(Part,'-');
     if (p==Nil)
      BEGIN
       if (NOT DecodeReg(Part,&r1))
        BEGIN
         WrXError(1410,Part); return False;
        END
       *Erg|=RMask(r1,Turn);
      END
     else
      BEGIN
       *p='\0';
       if (NOT DecodeReg(Part,&r1))
        BEGIN
         WrXError(1410,Part); return False;
        END;
       if (NOT DecodeReg(p+1,&r2))
        BEGIN
         WrXError(1410,p+1); return False;
        END
       if (r1<=r2)
        for (z=r1; z<=r2; z++) *Erg|=RMask(z,Turn);
       else
        BEGIN
         for (z=r2; z<=15; z++) *Erg|=RMask(z,Turn);
         for (z=0; z<=r1; z++) *Erg|=RMask(z,Turn);
        END
      END
    END
   return True;
END

        static Boolean DecodeCondition(char *Asc, Word *Erg)
BEGIN
   int z;
   String Asc_N;

   strmaxcpy(Asc_N,Asc,255); NLS_UpString(Asc_N); Asc=Asc_N;

   for (z=0; z<ConditionCount; z++)
    if (strcmp(Asc,Conditions[z])==0) break;
   *Erg=z; return (z<ConditionCount);
END

/*------------------------------------------------------------------------*/

        static Boolean CheckFormat(char *FSet)
BEGIN
   char *p;

   if (strcmp(Format," ")==0) FormatCode=0;
   else
    BEGIN
     p=strchr(FSet,*Format);
     if (p!=Nil) FormatCode=p-FSet+1;
     else WrError(1090);
     return (p!=Nil);
    END;
   return True;
END

        static Boolean CheckBFieldFormat(void)
BEGIN
   if ((strcmp(Format,"G:R")==0) OR (strcmp(Format,"R:G")==0)) FormatCode=1;
   else if ((strcmp(Format,"G:I")==0) OR (strcmp(Format,"I:G")==0)) FormatCode=2;
   else if ((strcmp(Format,"E:R")==0) OR (strcmp(Format,"R:E")==0)) FormatCode=3;
   else if ((strcmp(Format,"E:I")==0) OR (strcmp(Format,"I:E")==0)) FormatCode=4;
   else
    BEGIN
     WrError(1090); return False;
    END
   return True;
END

        static Boolean GetOpSize(char *Asc, Byte Index)
BEGIN
   char *p;
   int l=strlen(Asc);

   p=RQuotPos(Asc,'.');
   if (p==Nil)
    BEGIN
     OpSize[Index]=DOpSize; return True;
    END
   else if (p==Asc+l-2)
    BEGIN
     switch (p[1])
      BEGIN
       case 'B': OpSize[Index]=0; break;
       case 'H': OpSize[Index]=1; break;
       case 'W': OpSize[Index]=2; break;
       default:
        WrError(1107); return False;
      END
     *p='\0';
     return True;
    END
   else
    BEGIN
     WrError(1107); return False;
    END
END

        static void SplitOptions(void)
BEGIN
   char *p;
   int z;

   OptionCnt=0; *Options[0]='\0'; *Options[1]='\0';
   do
    BEGIN
     p=RQuotPos(OpPart,'/');
     if (p!=Nil)
      BEGIN
       if (OptionCnt<2)
        BEGIN
         for (z=OptionCnt-1; z>=0; z--) strcpy(Options[z+1],Options[z]);
         OptionCnt++; strmaxcpy(Options[0],p+1,255);
        END
       *p='\0';
      END
    END
   while (p!=Nil);
END

/*------------------------------------------------------------------------*/

        static Boolean DecodePseudo(void)
BEGIN
   return False;
END

        static void DecideBranch(LongInt Adr, Byte Index)
BEGIN
   LongInt Dist=Adr-EProgCounter();

   if (FormatCode==0)
    BEGIN
     /* Groessenangabe erzwingt G-Format */
     if (OpSize[Index]!=-1) FormatCode=1;
     /* gerade 9-Bit-Zahl kurz darstellbar */
     else if (((Dist & 1)==0) AND (Dist<=254) AND (Dist>=-256)) FormatCode=2;
     /* ansonsten allgemein */
     else FormatCode=1;
    END
   if ((FormatCode==1) AND (OpSize[Index]==-1))
    BEGIN
     if ((Dist<=127) AND (Dist>=-128)) OpSize[Index]=0;
     else if ((Dist<=32767) AND (Dist>=-32768)) OpSize[Index]=1;
     else OpSize[Index]=2;
    END
END

        static Boolean DecideBranchLength(LongInt *Addr, int Index)
BEGIN
   *Addr-=EProgCounter();
   if (OpSize[Index]==-1)
    BEGIN
     if ((*Addr>=-128) AND (*Addr<=127)) OpSize[Index]=0;
     else if ((*Addr>=-32768) AND (*Addr<=32767)) OpSize[Index]=1;
     else OpSize[Index]=2;
    END

   if ((NOT SymbolQuestionable) AND
       (((OpSize[Index]==0) AND ((*Addr<-128) OR (*Addr>127)))
     OR ((OpSize[Index]==1) AND ((*Addr<-32768) OR (*Addr>32767)))))
    BEGIN
     WrError(1370); return False;
    END
   else return True;
END

        static void Make_G(Word Code)
BEGIN
   WAsmCode[0]=0xd000+(OpSize[1] << 8)+AdrMode[1];
   memcpy(WAsmCode+1,AdrVals[1],AdrCnt1[1]);
   WAsmCode[1+AdrCnt2[1]]=Code+(OpSize[2] << 8)+AdrMode[2];
   memcpy(WAsmCode+2+AdrCnt2[1],AdrVals[2],AdrCnt1[2]);
   CodeLen=4+AdrCnt1[1]+AdrCnt1[2];
END

        static void Make_E(Word Code, Boolean Signed)
BEGIN
   LongInt HVal,Min,Max;

   Min=128*(-Ord(Signed)); Max=Min+255;
   if (AdrType[1]!=ModImm) WrError(1350);
   else
    BEGIN
     HVal=ImmVal(1);
     if (ChkRange(HVal,Min,Max))
      BEGIN
       WAsmCode[0]=0xbf00+(HVal & 0xff);
       WAsmCode[1]=Code+(OpSize[2] << 8)+AdrMode[2];
       memcpy(WAsmCode+2,AdrVals[2],AdrCnt1[2]);
       CodeLen=4+AdrCnt1[2];
      END
    END
END

        static void Make_I(Word Code, Boolean Signed)
BEGIN
   if ((AdrType[1]!=ModImm) OR (NOT IsShort(2))) WrError(1350);
   else
    BEGIN
     AdaptImm(1,OpSize[2],Signed);
     WAsmCode[0]=Code+(OpSize[2] << 8)+AdrMode[2];
     memcpy(WAsmCode+1,AdrVals[2],AdrCnt1[2]);
     memcpy(WAsmCode+1+AdrCnt2[2],AdrVals[1],AdrCnt1[1]);
     CodeLen=2+AdrCnt1[1]+AdrCnt1[2];
    END
END

        static Boolean CodeAri(void)
BEGIN
    int z;
    Word AdrWord,Mask,Mask2;
    char Form[6];
    LongInt HVal;

   if ((Memo("ADD")) OR (Memo("SUB")))
    BEGIN
     z=Ord(Memo("SUB"));
     if (ArgCnt!=2) WrError(1110);
     else if (CheckFormat("GELQI"))
      if (GetOpSize(ArgStr[2],2))
       if (GetOpSize(ArgStr[1],1))
        BEGIN
         if (OpSize[2]==-1) OpSize[2]=2;
         if (OpSize[1]==-1) OpSize[1]=OpSize[2];
         if (DecodeAdr(ArgStr[1],1,Mask_Source))
          if (DecodeAdr(ArgStr[2],2,Mask_PureDest))
           BEGIN
            if (FormatCode==0)
             BEGIN
              if (AdrType[1]==ModImm)
               BEGIN
                HVal=ImmVal(1);
                if (IsShort(2))
                 if ((HVal>=1) AND (HVal<=8)) FormatCode=4;
                 else FormatCode=5;
                else if ((HVal>=-128) AND (HVal<127)) FormatCode=2;
                else FormatCode=1;
               END
              else if (IsShort(1) AND (AdrType[2]==ModReg) AND (OpSize[1]==2) AND (OpSize[2]==2)) FormatCode=3;
              else FormatCode=1;
             END
            switch (FormatCode)
             BEGIN
              case 1:
               Make_G(z << 11);
               break;
              case 2:
               Make_E(z << 11,True);
               break;
              case 3:
               if ((NOT IsShort(1)) OR (AdrType[2]!=ModReg)) WrError(1350);
               else if ((OpSize[1]!=2) OR (OpSize[2]!=2)) WrError(1130);
               else
                BEGIN
                 WAsmCode[0]=0x8100+(z << 6)+((AdrMode[2] & 15) << 10)+AdrMode[1];
                 memcpy(WAsmCode+1,AdrVals[1],AdrCnt1[1]);
                 CodeLen=2+AdrCnt1[1];
                 if ((AdrMode[1]==0x04) & (AdrMode[2]==15)) WrError(140);
                END
               break;
              case 4:
               if ((AdrType[1]!=ModImm) OR (NOT IsShort(2))) WrError(1350);
               else
                BEGIN
                 HVal=ImmVal(1);
                 if (ChkRange(HVal,1,8))
                  BEGIN
                   WAsmCode[0]=0x4040+(z << 13)+((HVal & 7) << 10)+(OpSize[2] << 8)+AdrMode[2];
                   memcpy(WAsmCode+1,AdrVals[2],AdrCnt1[2]);
                   CodeLen=2+AdrCnt1[2];
                  END
                END
               break;
              case 5:
               Make_I(0x44c0+(z << 11),True);
               break;
             END
           END
        END
     return True;
    END

   if (Memo("CMP"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (CheckFormat("GELZQI"))
      if (GetOpSize(ArgStr[1],1))
       if (GetOpSize(ArgStr[2],2))
        BEGIN
         if (OpSize[2]==-1) OpSize[2]=2;
         if (OpSize[1]==-1) OpSize[1]=OpSize[2];
         if (DecodeAdr(ArgStr[1],1,Mask_Source))
          if (DecodeAdr(ArgStr[2],2,Mask_NoImmGen-MModPush))
           BEGIN
            if (FormatCode==0)
             BEGIN
              if (AdrType[1]==ModImm)
               BEGIN
                HVal=ImmVal(1);
                if (HVal==0) FormatCode=4;
                else if ((HVal>=1) AND (HVal<=8) AND (IsShort(2))) FormatCode=5;
                else if ((HVal>=-128) AND (HVal<=127)) FormatCode=2;
                else if (AdrType[2]==ModReg) FormatCode=3;
                else if (IsShort(2)) FormatCode=5;
                else FormatCode=1;
               END
              else if ((IsShort(1)) AND (AdrType[2]==ModReg)) FormatCode=3;
              else FormatCode=1;
             END
            switch (FormatCode)
             BEGIN
              case 1:
               Make_G(0x8000);
               break;
              case 2:
               Make_E(0x8000,True);
               break;
              case 3:
               if ((NOT IsShort(1)) OR (AdrType[2]!=ModReg)) WrError(1350);
               else if (OpSize[1]!=2) WrError(1130);
               else
                BEGIN
                 WAsmCode[0]=((AdrMode[2] & 15) << 10)+(OpSize[2] << 8)+AdrMode[1];
                 memcpy(WAsmCode+1,AdrVals[1],AdrCnt1[1]);
                 CodeLen=2+AdrCnt1[1];
                END
               break;
              case 4:
               if (AdrType[1]!=ModImm) WrError(1350);
               else
                BEGIN
                 HVal=ImmVal(1);
                 if (ChkRange(HVal,0,0))
                  BEGIN
                   WAsmCode[0]=0xc000+(OpSize[2] << 8)+AdrMode[2];
                   memcpy(WAsmCode+1,AdrVals[2],AdrCnt1[2]);
                   CodeLen=2+AdrCnt1[2];
                  END
                END
               break;
              case 5:
               if ((AdrType[1]!=ModImm) OR (NOT IsShort(2))) WrError(1350);
               else
                BEGIN
                 HVal=ImmVal(1);
                 if (ChkRange(HVal,1,8))
                  BEGIN
                   WAsmCode[0]=0x4000+(OpSize[2] << 8)+AdrMode[2]+((HVal & 7) << 10);
                   memcpy(WAsmCode+1,AdrVals[2],AdrCnt1[2]);
                   CodeLen=2+AdrCnt1[2];
                  END
                END
               break;
              case 6:
               Make_I(0x40c0,True);
               break;
             END
           END
        END
     return True;
    END

   for (z=0; z<GE2OrderCount; z++)
    if (Memo(GE2Orders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (CheckFormat("GE"))
       if (GetOpSize(ArgStr[2],2))
        if (GetOpSize(ArgStr[1],1))
         BEGIN
          if (OpSize[2]==-1) OpSize[2]=DefSize(GE2Orders[z].SMask2);
          if (OpSize[1]==-1) OpSize[1]=DefSize(GE2Orders[z].SMask1);
          if (((GE2Orders[z].SMask1 & (1 << OpSize[1]))==0) OR ((GE2Orders[z].SMask2 & (1 << OpSize[2]))==0)) WrError(1130);
          else if (DecodeAdr(ArgStr[1],1,GE2Orders[z].Mask1))
           if (DecodeAdr(ArgStr[2],2,GE2Orders[z].Mask2))
            BEGIN
             if (FormatCode==0)
              BEGIN
               if (AdrType[1]==ModImm)
                BEGIN
                 HVal=ImmVal(1);
                 if ((GE2Orders[z].Signed) AND (HVal>=-128) AND (HVal<=127)) FormatCode=2;
                 else if ((NOT GE2Orders[z].Signed) AND (HVal>=0) AND (HVal<=255)) FormatCode=2;
                 else FormatCode=1;
                END
               else FormatCode=1;
              END
             switch (FormatCode)
              BEGIN
               case 1:
                Make_G(GE2Orders[z].Code); break;
               case 2:
                Make_E(GE2Orders[z].Code,GE2Orders[z].Signed); break;
              END
            END
         END
      return True;
     END

   for (z=0; z<LogOrderCount; z++)
    if (Memo(LogOrders[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (CheckFormat("GERI"))
       if (GetOpSize(ArgStr[1],1))
        if (GetOpSize(ArgStr[2],2))
         BEGIN
          if (OpSize[2]==-1) OpSize[2]=2;
          if (OpSize[1]==-1) OpSize[1]=OpSize[2];
          if (DecodeAdr(ArgStr[1],1,Mask_Source))
           if (DecodeAdr(ArgStr[2],2,Mask_Dest-MModPush))
            BEGIN
             if (FormatCode==0)
              BEGIN
               if (AdrType[1]==ModImm)
                BEGIN
                 HVal=ImmVal(1);
                 if ((HVal>=0) AND (HVal<=255)) FormatCode=2;
                 else if (IsShort(2)) FormatCode=4;
                 else FormatCode=1;
                END
               else if ((AdrType[1]==ModReg) AND (AdrType[2]==ModReg) AND (OpSize[1]==2) AND (OpSize[2]==2))
                FormatCode=3;
               else FormatCode=1;
              END
             switch (FormatCode)
              BEGIN
               case 1:
                Make_G(0x2000+(z << 10));
                break;
               case 2:
                Make_E(0x2000+(z << 10),False);
                break;
               case 3:
                if ((AdrType[1]!=ModReg) OR (AdrType[2]!=ModReg)) WrError(1350);
                else if ((OpSize[1]!=2) OR (OpSize[2]!=2)) WrError(1130);
                else
                 BEGIN
                  WAsmCode[0]=0x00c0+(z << 8)+(AdrMode[1] & 15)+((AdrMode[2] & 15) << 10);
                  CodeLen=2;
                 END
                break;
               case 4:
                if ((AdrType[1]!=ModImm) OR (NOT IsShort(2))) WrError(1350);
                else
                 BEGIN
                  WAsmCode[0]=0x50c0+(OpSize[2] << 8)+(z << 10)+AdrMode[2];
                  memcpy(WAsmCode+1,AdrVals[2],AdrCnt1[2]);
                  AdaptImm(1,OpSize[2],False);
                  memcpy(WAsmCode+1+AdrCnt2[2],AdrVals[1],AdrCnt1[1]);
                  CodeLen=2+AdrCnt1[1]+AdrCnt1[2];
                 END
                break;
              END
             if (OpSize[1]>OpSize[2]) WrError(140);
            END
         END
      return True;
     END

   for (z=0; z<MulOrderCount; z++)
    if (Memo(MulOrders[z]))
     BEGIN
      strcpy(Form,(Odd(z)) ? "GE" : "GER");
      if (ArgCnt!=2) WrError(1110);
      else if (CheckFormat(Form))
       if (GetOpSize(ArgStr[1],1))
        if (GetOpSize(ArgStr[2],2))
         BEGIN
          if (OpSize[2]==-1) OpSize[2]=2;
          if (OpSize[1]==-1) OpSize[1]=OpSize[2];
          if (DecodeAdr(ArgStr[1],1,Mask_Source))
           if (DecodeAdr(ArgStr[2],2,Mask_PureDest))
            BEGIN
             if (FormatCode==0)
              BEGIN
               if (AdrType[1]==ModImm)
                BEGIN
                 HVal=ImmVal(1);
                 if ((HVal>=-128+(Ord(Odd(z)) << 7)) AND
                     (HVal<=127+(Ord(Odd(z)) << 7))) FormatCode=2;
                 else FormatCode=1;
                END
               else if ((NOT Odd(z)) AND (AdrType[1]==ModReg) AND (OpSize[1]==2)
                    AND (AdrType[2]==ModReg) AND (OpSize[2]==2)) FormatCode=3;
               else FormatCode=1;
              END
             switch (FormatCode)
              BEGIN
               case 1:
                Make_G(0x4000+(z << 10)); break;
               case 2:
                Make_E(0x4000+(z << 10),NOT Odd(z)); break;
               case 3:
                if ((AdrType[1]!=ModReg) OR (AdrType[2]!=ModReg)) WrError(1350);
                else if ((OpSize[1]!=2) OR (OpSize[2]!=2)) WrError(1130);
                else
                 BEGIN
                  WAsmCode[0]=0x00d0+((AdrMode[2] & 15) << 10)+(z << 7)+
                                      (AdrMode[1] & 15);
                  CodeLen=2;
                 END
              END
            END
         END
      return True;
     END

   for (z=0; z<GetPutOrderCount; z++)
    if (Memo(GetPutOrders[z].Name))
     BEGIN
      if (GetPutOrders[z].Turn)
       BEGIN
        Mask=Mask_Source; Mask2=MModReg; AdrWord=1;
       END
      else
       BEGIN
        Mask=MModReg; Mask2=Mask_Dest; AdrWord=2;
       END;
      if (ArgCnt!=2) WrError(1110);
      else if (CheckFormat("G"))
       if (GetOpSize(ArgStr[1],1))
        if (GetOpSize(ArgStr[2],2))
         BEGIN
          if (OpSize[AdrWord]==-1) OpSize[AdrWord]=GetPutOrders[z].Size;
          if (OpSize[3-AdrWord]==-1) OpSize[3-AdrWord]=2;
          if ((OpSize[AdrWord]!=GetPutOrders[z].Size) OR (OpSize[3-AdrWord]!=2)) WrError(1130);
          else if (DecodeAdr(ArgStr[1],1,Mask))
           if (DecodeAdr(ArgStr[2],2,Mask2))
            BEGIN
             Make_G(GetPutOrders[z].Code); WAsmCode[0]+=0x0400;
            END
         END
      return True;
     END

   if (Memo("MOVA"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (CheckFormat("GR"))
      if (GetOpSize(ArgStr[2],2))
       BEGIN
        if (OpSize[2]==-1) OpSize[2]=2;
        OpSize[1]=0;
        if (OpSize[2]!=2) WrError(1110);
        else if (DecodeAdr(ArgStr[1],1,Mask_PureMem))
         if (DecodeAdr(ArgStr[2],2,Mask_Dest))
          BEGIN
           if (FormatCode==0)
            BEGIN
             if ((AdrType[1]==ModDisp16) AND (AdrType[2]==ModReg)) FormatCode=2;
             else FormatCode=1;
            END
           switch (FormatCode)
            BEGIN
             case 1:
              Make_G(0xb400); WAsmCode[0]+=0x800;
              break;
             case 2:
              if ((AdrType[1]!=ModDisp16) OR (AdrType[2]!=ModReg)) WrError(1350);
              else
               BEGIN
                WAsmCode[0]=0x03c0+((AdrMode[2] & 15) << 10)+(AdrMode[1] & 15);
                WAsmCode[1]=AdrVals[1][0];
                CodeLen=4;
               END
              break;
            END
          END
       END
     return True;
    END

   if ((Memo("QINS")) OR (Memo("QDEL")))
    BEGIN
     z=Ord(Memo("QINS")) << 11;
     Mask=Mask_PureMem;
     if (ArgCnt!=2) WrError(1110);
     else if (CheckFormat("G"))
      if ((Memo("QINS")) OR (GetOpSize(ArgStr[2],2)))
       BEGIN
        if (OpSize[2]==-1) OpSize[2]=2;
        OpSize[1]=0;
        if (OpSize[2]!=2) WrError(1130);
        else if (DecodeAdr(ArgStr[1],1,Mask))
         if (DecodeAdr(ArgStr[2],2,Mask+(Ord(Memo("QDEL"))*MModReg)))
          BEGIN
           Make_G(0xb000+z); WAsmCode[0]+=0x800;
          END
       END
     return True;
    END

   if (Memo("RVBY"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (CheckFormat("G"))
      if (GetOpSize(ArgStr[1],1))
       if (GetOpSize(ArgStr[2],2))
        BEGIN
         if (OpSize[2]==-1) OpSize[2]=2;
         if (OpSize[1]==-1) OpSize[1]=OpSize[2];
         if (DecodeAdr(ArgStr[1],1,Mask_Source))
          if (DecodeAdr(ArgStr[2],2,Mask_Dest))
           BEGIN
            Make_G(0x4000); WAsmCode[0]+=0x400;
           END
        END
     return True;
    END

   if ((Memo("SHL")) OR (Memo("SHA")))
    BEGIN
     z=Ord(Memo("SHA"));
     if (ArgCnt!=2) WrError(1110);
     else if (CheckFormat("GEQ"))
      if (GetOpSize(ArgStr[1],1))
       if (GetOpSize(ArgStr[2],2))
        BEGIN
         if (OpSize[1]==-1) OpSize[1]=0;
         if (OpSize[2]==-1) OpSize[2]=2;
         if (OpSize[1]!=0) WrError(1130);
         else if (DecodeAdr(ArgStr[1],1,Mask_Source))
          if (DecodeAdr(ArgStr[2],2,Mask_PureDest))
           BEGIN
            if (FormatCode==0)
             BEGIN
              if (AdrType[1]==ModImm)
               BEGIN
                HVal=ImmVal(1);
                if ((IsShort(2)) AND (abs(HVal)>=1) AND (abs(HVal)<=8) AND ((z==0) OR (HVal<0))) FormatCode=3;
                else if ((HVal>=-128) AND (HVal<=127)) FormatCode=2;
                else FormatCode=1;
               END
              else FormatCode=1;
             END
            switch (FormatCode)
             BEGIN
              case 1:
               Make_G(0x3000+(z << 10)); break;
              case 2:
               Make_E(0x3000+(z << 10),True); break;
              case 3:
               if ((AdrType[1]!=ModImm) OR (NOT IsShort(2))) WrError(1350);
               else
                BEGIN
                 HVal=ImmVal(1);
                 if (ChkRange(HVal,-8,(1-z) << 3))
                  BEGIN
                   if (HVal==0) WrError(1135);
                   else
                    BEGIN
                     if (HVal<0) HVal+=16;
                     else HVal&=7;
                     WAsmCode[0]=0x4080+(HVal << 10)+(z << 6)+(OpSize[2] << 8)+AdrMode[2];
                     memcpy(WAsmCode+1,AdrVals[2],AdrCnt1[2]);
                     CodeLen=2+AdrCnt1[2];
                    END
                  END
                END
               break;
             END
           END
        END
     return True;
    END

   if ((Memo("SHXL")) OR (Memo("SHXR")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (CheckFormat("G"))
      if (GetOpSize(ArgStr[1],1))
       BEGIN
        if (OpSize[1]==-1) OpSize[1]=2;
        if (OpSize[1]!=2) WrError(1130);
        else if (DecodeAdr(ArgStr[1],1,Mask_PureDest))
         BEGIN
          WAsmCode[0]=0x02f7;
          WAsmCode[1]=0x8a00+(Ord(Memo("SHXR")) << 12)+AdrMode[1];
          memcpy(WAsmCode+1,AdrVals,AdrCnt1[1]);
          CodeLen=4+AdrCnt1[1];
         END
       END
     return True;
    END

   return False;
END

        static Boolean CodeBits(void)
BEGIN
   int z;
   char Form[6];
   LongInt HVal,AdrLong;
   Word Mask;

   for (z=0; z<BitOrderCount; z++)
    if (Memo(BitOrders[z].Name))
     BEGIN
      strcpy(Form,(BitOrders[z].Code2!=0) ? "GER" : "GE");
      if (ArgCnt!=2) WrError(1110);
      else if (CheckFormat(Form))
       if (GetOpSize(ArgStr[1],1))
        if (GetOpSize(ArgStr[2],2))
         BEGIN
          if (OpSize[1]==-1) OpSize[1]=2;
          if (DecodeAdr(ArgStr[1],1,Mask_Source))
           if (DecodeAdr(ArgStr[2],2,Mask_PureDest))
            BEGIN
             if (OpSize[2]==-1)
              BEGIN
               if ((AdrType[2]==ModReg) AND (NOT BitOrders[z].MustByte)) OpSize[2]=2;
               else OpSize[2]=0;
              END
             if (((AdrType[2]!=ModReg) OR (BitOrders[z].MustByte)) AND (OpSize[2]!=0)) WrError(1130);
             else
              BEGIN
               if (FormatCode==0)
                BEGIN
                 if (AdrType[1]==ModImm)
                  BEGIN
                   HVal=ImmVal(1);
                   if ((HVal>=0) AND (HVal<=7) AND (IsShort(2)) AND (BitOrders[z].Code2!=0) AND (OpSize[2]==0)) FormatCode=3;
                   else if ((HVal>=-128) AND (HVal<127)) FormatCode=2;
                   else FormatCode=1;
                  END
                 else FormatCode=1;
                END;
               switch (FormatCode)
                BEGIN
                 case 1:
                  Make_G(BitOrders[z].Code1); break;
                 case 2:
                  Make_E(BitOrders[z].Code1,True); break;
                 case 3:
                  if ((AdrType[1]!=ModImm) OR (NOT IsShort(2))) WrError(1350);
                  else if (OpSize[2]!=0) WrError(1130);
                  else
                   BEGIN
                    HVal=ImmVal(1);
                    if (ChkRange(HVal,0,7))
                     BEGIN
                      WAsmCode[0]=BitOrders[z].Code2+((HVal & 7) << 10)+AdrMode[2];
                      memcpy(WAsmCode+1,AdrVals[2],AdrCnt1[2]);
                      CodeLen=2+AdrCnt1[2];
                     END
                   END
                  break; 
                END
              END
            END
         END
      return True;
     END

   for (z=0; z<BFieldOrderCount; z++)
    if (Memo(BFieldOrders[z]))
     BEGIN
      if (ArgCnt!=4) WrError(1110);
      else if (CheckBFieldFormat())
       if (GetOpSize(ArgStr[1],1))
        if (GetOpSize(ArgStr[2],2))
         if (GetOpSize(ArgStr[3],3))
          if (GetOpSize(ArgStr[4],4))
           BEGIN
            if (OpSize[1]==-1) OpSize[1]=2;
            if (OpSize[2]==-1) OpSize[2]=2;
            if (OpSize[3]==-1) OpSize[3]=2;
            if (OpSize[4]==-1) OpSize[4]=2;
            if (DecodeAdr(ArgStr[1],1,MModReg+MModImm))
             if (DecodeAdr(ArgStr[3],3,MModReg+MModImm))
              BEGIN
               Mask=(AdrType[3]==ModReg) ? Mask_Source : MModImm;
               if (DecodeAdr(ArgStr[2],2,Mask))
                if (DecodeAdr(ArgStr[4],4,Mask_PureMem))
                 BEGIN
                  if (FormatCode==0)
                   BEGIN
                    if (AdrType[3]==ModReg)
                     if (AdrType[1]==ModReg) FormatCode=1; else FormatCode=2;
                    else
                     if (AdrType[1]==ModReg) FormatCode=3; else FormatCode=4;
                   END
                  switch (FormatCode)
                   BEGIN
                    case 1:
                     if ((AdrType[1]!=ModReg) OR (AdrType[3]!=ModReg)) WrError(1350);
                     else if ((OpSize[1]!=2) OR (OpSize[3]!=2) OR (OpSize[4]!=2)) WrError(1130);
                     else
                      BEGIN
                       WAsmCode[0]=0xd000+(OpSize[2] << 8)+AdrMode[2];
                       memcpy(WAsmCode+1,AdrVals[2],AdrCnt1[2]);
                       WAsmCode[1+AdrCnt2[2]]=0xc200+(z << 10)+AdrMode[4];
                       memcpy(WAsmCode+2+AdrCnt2[2],AdrVals[4],AdrCnt1[4]);
                       WAsmCode[2+AdrCnt2[2]+AdrCnt2[4]]=((AdrMode[3] & 15) << 10)+(AdrMode[1] & 15);
                       CodeLen=6+AdrCnt1[2]+AdrCnt1[4];
                      END
                     break;
                    case 2:
                     if ((AdrType[1]!=ModImm) OR (AdrType[3]!=ModReg)) WrError(1350);
                     else if ((OpSize[3]!=2) OR (OpSize[4]!=2)) WrError(1130);
                     else
                      BEGIN
                       WAsmCode[0]=0xd000+(OpSize[2] << 8)+AdrMode[2];
                       memcpy(WAsmCode+1,AdrVals[2],AdrCnt1[2]);
                       WAsmCode[1+AdrCnt2[2]]=0xd200+(z << 10)+AdrMode[4];
                       memcpy(WAsmCode+2+AdrCnt2[2],AdrVals[4],AdrCnt1[4]);
                       WAsmCode[2+AdrCnt2[2]+AdrCnt2[4]]=((AdrMode[3] & 15) << 10)+(OpSize[1] << 8);
                       CodeLen=6+AdrCnt1[2]+AdrCnt1[4];
                       if (OpSize[1]==0) WAsmCode[(CodeLen-2) >> 1]+=AdrVals[1][0] & 0xff;
                       else
                        BEGIN
                         memcpy(WAsmCode+(CodeLen >> 1),AdrVals[1],AdrCnt1[1]);
                         CodeLen+=AdrCnt1[1];
                        END
                      END
                     break;
                    case 3:
                     if ((AdrType[1]!=ModReg) OR (AdrType[2]!=ModImm) OR (AdrType[3]!=ModImm)) WrError(1350);
                     else if ((OpSize[1]!=2) OR (OpSize[4]!=2)) WrError(1130);
                     else
                      BEGIN
                       HVal=ImmVal(2);
                       if (ChkRange(HVal,-128,-127))
                        BEGIN
                         AdrLong=ImmVal(3);
                         if (ChkRange(AdrLong,1,32))
                          BEGIN
                           WAsmCode[0]=0xbf00+(HVal & 0xff);
                           WAsmCode[1]=0xc200+(z << 10)+AdrMode[4];
                           memcpy(WAsmCode+2,AdrVals[4],AdrCnt1[4]);
                           WAsmCode[2+AdrCnt2[4]]=((AdrLong & 31) << 10)+(AdrMode[1] & 15);
                           CodeLen=6+AdrCnt1[4];
                          END
                        END
                      END
                     break;
                    case 4:
                     if ((AdrType[1]!=ModImm) OR (AdrType[2]!=ModImm) OR (AdrType[3]!=ModImm)) WrError(1350);
                     else if (OpSize[4]!=2) WrError(1130);
                     else
                      BEGIN
                       HVal=ImmVal(2);
                       if (ChkRange(HVal,-128,-127))
                        BEGIN
                         AdrLong=ImmVal(3);
                         if (ChkRange(AdrLong,1,32))
                          BEGIN
                           WAsmCode[0]=0xbf00+(HVal & 0xff);
                           WAsmCode[1]=0xd200+(z << 10)+AdrMode[4];
                           memcpy(WAsmCode+2,AdrVals[4],AdrCnt1[4]);
                           WAsmCode[2+AdrCnt2[4]]=((AdrLong & 31) << 10)+(OpSize[1] << 8);
                           CodeLen=6+AdrCnt1[4];
                           if (OpSize[1]==0) WAsmCode[(CodeLen-1) >> 1]+=AdrVals[1][0] & 0xff;
                           else
                            BEGIN
                             memcpy(WAsmCode+(CodeLen >> 1),AdrVals[1],AdrCnt1[1]);
                             CodeLen+=AdrCnt1[1];
                            END
                          END
                        END
                      END
                     break;
                   END
                 END
              END
           END
      return True;
     END

   if ((Memo("BFEXT")) OR (Memo("BFEXTU")))
    BEGIN
     z=Ord(Memo("BFEXTU"));
     if (ArgCnt!=4) WrError(1110);
     else if (CheckFormat("GE"))
      if (GetOpSize(ArgStr[1],1))
       if (GetOpSize(ArgStr[2],2))
        if (GetOpSize(ArgStr[3],3))
         if (GetOpSize(ArgStr[4],4))
          BEGIN
           if (OpSize[1]==-1) OpSize[1]=2;
           if (OpSize[2]==-1) OpSize[2]=2;
           if (OpSize[3]==-1) OpSize[3]=2;
           if (OpSize[4]==-1) OpSize[4]=2;
           if (DecodeAdr(ArgStr[4],4,MModReg))
            if (DecodeAdr(ArgStr[3],3,Mask_MemGen-MModPop-MModPush))
             if (DecodeAdr(ArgStr[2],2,MModReg+MModImm))
              BEGIN
               if (AdrType[2]==ModReg) Mask=Mask_Source; else Mask=MModImm;
               if (DecodeAdr(ArgStr[1],1,Mask))
                BEGIN
                 if (FormatCode==0)
                  BEGIN
                   if (AdrType[2]==ModReg) FormatCode=1; else FormatCode=2;
                  END
                 switch (FormatCode)
                  BEGIN
                   case 1:
                    if ((OpSize[2]!=2) OR (OpSize[3]!=2) OR (OpSize[4]!=2)) WrError(1130);
                    else
                     BEGIN
                      WAsmCode[0]=0xd000+(OpSize[1] << 8)+AdrMode[1];
                      memcpy(WAsmCode+1,AdrVals[1],AdrCnt1[1]);
                      WAsmCode[1+AdrCnt2[1]]=0xea00+(z << 10)+AdrMode[3];
                      memcpy(WAsmCode+2+AdrCnt2[1],AdrVals[3],AdrCnt1[3]);
                      WAsmCode[2+AdrCnt2[1]+AdrCnt2[3]]=((AdrMode[2] & 15) << 10)+(AdrMode[4] & 15);
                      CodeLen=6+AdrCnt1[1]+AdrCnt1[3];
                     END
                    break;
                   case 2:
                    if ((AdrType[1]!=ModImm) OR (AdrType[2]!=ModImm)) WrError(1350);
                    else if ((OpSize[3]!=2) OR (OpSize[4]!=2)) WrError(1350);
                    else
                     BEGIN
                      HVal=ImmVal(1);
                      if (ChkRange(HVal,-128,127))
                       BEGIN
                        AdrLong=ImmVal(2);
                        if (ChkRange(AdrLong,1,32))
                         BEGIN
                          WAsmCode[0]=0xbf00+(HVal & 0xff);
                          WAsmCode[1]=0xea00+(z << 10)+AdrMode[3];
                          memcpy(WAsmCode+2,AdrVals[3],AdrCnt1[3]);
                          WAsmCode[2+AdrCnt2[3]]=((AdrLong & 31) << 10)+(AdrMode[4] & 15);
                          CodeLen=6+AdrCnt1[3];
                         END
                       END
                     END
                    break;
                  END
                END
              END
          END
     return True;
    END

   if ((Memo("BSCH/0")) OR (Memo("BSCH/1")))
    BEGIN
     z=OpPart[5]-'0';
     if (ArgCnt!=2) WrError(1110);
     else if (CheckFormat("G"))
      if (GetOpSize(ArgStr[1],1))
       if (GetOpSize(ArgStr[2],2))
        BEGIN
         if (OpSize[1]==-1) OpSize[1]=2;
         if (OpSize[2]==-1) OpSize[2]=2;
         if (OpSize[1]!=2) WrError(1130);
         else
          if (DecodeAdr(ArgStr[1],1,Mask_Source))
           if (DecodeAdr(ArgStr[2],2,Mask_PureDest))
            BEGIN
             /* immer G-Format */
             WAsmCode[0]=0xd600+AdrMode[1];
             memcpy(WAsmCode+1,AdrVals[1],AdrCnt1[1]);
             WAsmCode[1+AdrCnt2[1]]=0x5000+(z << 10)+(OpSize[2] << 8)+AdrMode[2];
             memcpy(WAsmCode+2+AdrCnt2[1],AdrVals[2],AdrCnt1[2]);
             CodeLen=4+AdrCnt1[1]+AdrCnt1[2];
            END
        END
     return True;
    END

   return False;
END

        static void MakeCode_M16(void)
BEGIN
   int z;
   char *p;
   Word AdrWord,HReg,Mask;
   LongInt AdrLong,HVal;
   Boolean OK;

   DOpSize=(-1); for (z=1; z<=ArgCnt; OpSize[z++]=(-1));

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Formatangabe abspalten */

   switch (AttrSplit)
    BEGIN
     case '.':
      p=strchr(AttrPart,':');
      if (p!=Nil)
       BEGIN
        if (p<AttrPart+strlen(AttrPart)-1) strmaxcpy(Format,p+1,255);
        else strcpy(Format," ");
        *p='\0';
       END
      else strcpy(Format," ");
      break;
     case ':':
      p=strchr(AttrPart,'.');
      if (p==Nil)
       BEGIN
        strmaxcpy(Format,AttrPart,255); *AttrPart='\0';
       END
      else
       BEGIN
        *p='\0';
        if (p==AttrPart) strcpy(Format," "); else strmaxcpy(Format,AttrPart,255);
       END
      break;
     default:
      strcpy(Format," ");
    END
   NLS_UpString(Format);

   /* Attribut abarbeiten */

   if (*AttrPart=='\0') DOpSize=(-1);
   else
    switch (toupper(*AttrPart))
     BEGIN
      case 'B': DOpSize=0; break;
      case 'H': DOpSize=1; break;
      case 'W': DOpSize=2; break;
      default:
       WrError(1107); return;
     END

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(False)) return;

   /* ohne Argument */

   for (z=0; z<FixedOrderCount; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else if (strcmp(Format," ")!=0) WrError(1090);
      else
       BEGIN
        CodeLen=2; WAsmCode[0]=FixedOrders[z].Code;
       END
      return;
     END

   if ((Memo("STOP")) OR (Memo("SLEEP")))
    BEGIN
     if (ArgCnt!=0) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else if (strcmp(Format," ")!=0) WrError(1090);
     else
      BEGIN
       CodeLen=10;
       WAsmCode[0]=0xd20c;
       if (Memo("STOP"))
        BEGIN
         WAsmCode[1]=0x5374;
         WAsmCode[2]=0x6f70;
        END
       else
        BEGIN
         WAsmCode[1]=0x5761;
         WAsmCode[2]=0x6974;
        END;
       WAsmCode[3]=0x9e09;
       WAsmCode[4]=0x0700;
      END
     return;
    END

   /* Datentransfer */

   if (Memo("MOV"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (CheckFormat("GELSZQI"))
      if (GetOpSize(ArgStr[1],1))
       if (GetOpSize(ArgStr[2],2))
        BEGIN
         if (OpSize[2]==-1) OpSize[2]=2;
         if (OpSize[1]==-1) OpSize[1]=OpSize[2];
         if (DecodeAdr(ArgStr[1],1,Mask_Source))
          if (DecodeAdr(ArgStr[2],2,Mask_AllGen-MModPop))
           BEGIN
            if (FormatCode==0)
             BEGIN
              if (AdrType[1]==ModImm)
               BEGIN
                HVal=ImmVal(1);
                if (HVal==0) FormatCode=5;
                else if ((HVal>=1) AND (HVal<=8) AND (IsShort(2))) FormatCode=6;
                else if ((HVal>=-128) AND (HVal<=127)) FormatCode=2;
                else if (IsShort(2)) FormatCode=7;
                else FormatCode=1;
               END
              else if ((AdrType[1]==ModReg) AND (OpSize[1]==2) AND (IsShort(2))) FormatCode=4;
              else if ((AdrType[2]==ModReg) AND (OpSize[2]==2) AND (IsShort(1))) FormatCode=3;
              else FormatCode=1;
             END
            switch (FormatCode)
             BEGIN
              case 1:
               Make_G(0x8800);
               break;
              case 2:
               Make_E(0x8800,True);
               break;
              case 3:
               if ((NOT IsShort(1)) OR (AdrType[2]!=ModReg)) WrError(1350);
               else if (OpSize[2]!=2) WrError(1130);
               else
                BEGIN
                 WAsmCode[0]=0x0040+((AdrMode[2] & 15) << 10)+(OpSize[1] << 8)+AdrMode[1];
                 memcpy(WAsmCode+1,AdrVals[1],AdrCnt1[1]);
                 CodeLen=2+AdrCnt1[1];
                END
               break;
              case 4:
               if ((NOT IsShort(2)) OR (AdrType[1]!=ModReg)) WrError(1350);
               else if (OpSize[1]!=2) WrError(1130);
               else
                BEGIN
                 WAsmCode[0]=0x0080+((AdrMode[1] & 15) << 10)+(OpSize[2] << 8)+AdrMode[2];
                 memcpy(WAsmCode+1,AdrVals[2],AdrCnt1[2]);
                 CodeLen=2+AdrCnt1[2];
                END
               break;
              case 5:
               if (AdrType[1]!=ModImm) WrError(1350);
               else
                BEGIN
                 HVal=ImmVal(1);
                 if (ChkRange(HVal,0,0))
                  BEGIN
                   WAsmCode[0]=0xc400+(OpSize[2] << 8)+AdrMode[2];
                   memcpy(WAsmCode+1,AdrVals[2],AdrCnt1[2]);
                   CodeLen=2+AdrCnt1[2];
                  END
                END
               break;
              case 6:
               if ((AdrType[1]!=ModImm) OR (NOT IsShort(2))) WrError(1350);
               else
                BEGIN
                 HVal=ImmVal(1);
                 if (ChkRange(HVal,1,8))
                  BEGIN
                   WAsmCode[0]=0x6000+((HVal & 7) << 10)+(OpSize[2] << 8)+AdrMode[2];
                   memcpy(WAsmCode+1,AdrVals[2],AdrCnt1[2]);
                   CodeLen=2+AdrCnt1[2];
                  END
                END
               break;
              case 7:
               Make_I(0x48c0,True);
               break;
             END
           END
        END
     return;
    END

   /* ein Operand */

   for (z=0; z<OneOrderCount; z++)
    if (Memo(OneOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (CheckFormat("G"))
       if (GetOpSize(ArgStr[1],1))
        BEGIN
         if ((OpSize[1]==-1) AND (OneOrders[z].OpMask!=0)) OpSize[1]=DefSize(OneOrders[z].OpMask);
         if ((OpSize[1]!=-1) AND (((1 << OpSize[1]) & OneOrders[z].OpMask)==0)) WrError(1130);
         else
          BEGIN
           if (DecodeAdr(ArgStr[1],1,OneOrders[z].Mask))
            BEGIN
             /* da nur G, Format ignorieren */
             WAsmCode[0]=OneOrders[z].Code+AdrMode[1];
             if (OneOrders[z].OpMask!=0) WAsmCode[0]+=OpSize[1] << 8;
             memcpy(WAsmCode+1,AdrVals[1],AdrCnt1[1]);
             CodeLen=2+AdrCnt1[1];
            END
          END
        END
    return;
   END

   /* zwei Operanden */

   if (CodeAri()) return;
   
   /* drei Operanden */

   if ((Memo("CHK/N")) OR (Memo("CHK/S")) OR (Memo("CHK")))
    BEGIN
     z=Ord(OpPart[strlen(OpPart)-1]=='S');
     if (ArgCnt!=3) WrError(1110);
     else if (CheckFormat("G"))
      if (GetOpSize(ArgStr[1],2))
       if (GetOpSize(ArgStr[2],1))
        if (GetOpSize(ArgStr[3],3))
         BEGIN
          if (OpSize[3]==-1) OpSize[3]=2;
          if (OpSize[2]==-1) OpSize[2]=OpSize[3];
          if (OpSize[1]==-1) OpSize[1]=OpSize[3];
          if ((OpSize[1]!=OpSize[2]) OR (OpSize[2]!=OpSize[3])) WrError(1131);
          else
           if (DecodeAdr(ArgStr[1],2,Mask_MemGen-MModPop-MModPush))
            if (DecodeAdr(ArgStr[2],1,Mask_Source))
             if (DecodeAdr(ArgStr[3],3,MModReg))
              BEGIN
               OpSize[2]=2+z;
               Make_G((AdrMode[3] & 15) << 10);
               WAsmCode[0]+=0x400;
              END
         END
     return;
    END

   if (Memo("CSI"))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else if (CheckFormat("G"))
      if (GetOpSize(ArgStr[1],3))
       if (GetOpSize(ArgStr[2],1))
        if (GetOpSize(ArgStr[3],2))
         BEGIN
          if (OpSize[3]==-1) OpSize[3]=2;
          if (OpSize[2]==-1) OpSize[2]=OpSize[3];
          if (OpSize[1]==-1) OpSize[1]=OpSize[2];
          if ((OpSize[1]!=OpSize[2]) OR (OpSize[2]!=OpSize[3])) WrError(1131);
          else if (DecodeAdr(ArgStr[1],3,MModReg))
           if (DecodeAdr(ArgStr[2],1,Mask_Source))
            if (DecodeAdr(ArgStr[3],2,Mask_PureMem))
             BEGIN
              OpSize[2]=0;
              Make_G((AdrMode[3] & 15) << 10);
              WAsmCode[0]+=0x400;
             END
         END
     return;
    END

   if ((Memo("DIVX")) OR (Memo("MULX")))
    BEGIN
     z=Ord(Memo("DIVX"));
     if (ArgCnt!=3) WrError(1110);
     else if (CheckFormat("G"))
      if (GetOpSize(ArgStr[1],1))
       if (GetOpSize(ArgStr[2],2))
        if (GetOpSize(ArgStr[3],3))
         BEGIN
          if (OpSize[3]==-1) OpSize[3]=2;
          if (OpSize[2]==-1) OpSize[2]=OpSize[3];
          if (OpSize[1]==-1) OpSize[1]=OpSize[2];
          if ((OpSize[1]!=2) OR (OpSize[2]!=2) OR (OpSize[3]!=2)) WrError(1130);
          else if (DecodeAdr(ArgStr[1],1,Mask_Source))
           if (DecodeAdr(ArgStr[2],2,Mask_PureDest))
            if (DecodeAdr(ArgStr[3],3,MModReg))
             BEGIN
              OpSize[2]=0;
              Make_G(0x8200+((AdrMode[3] & 15) << 10)+(z << 8));
              WAsmCode[0]+=0x400;
             END
         END
     return;
    END

   /* Bitoperationen */

   if (CodeBits()) return;

   /* Spruenge */

   if ((Memo("BSR")) OR (Memo("BRA")))
    BEGIN
     z=Ord(Memo("BSR"));
     if (ArgCnt!=1) WrError(1110);
     else
      if (CheckFormat("GD"))
       if (GetOpSize(ArgStr[1],1))
        BEGIN
         AdrLong=EvalIntExpression(ArgStr[1],Int32,&OK);
         if (OK)
          BEGIN
           DecideBranch(AdrLong,1);
           switch (FormatCode)
            BEGIN
             case 2:
              if (OpSize[1]!=-1) WrError(1100);
              else
               BEGIN
                AdrLong-=EProgCounter();
                if ((NOT SymbolQuestionable) AND ((AdrLong<-256) OR (AdrLong>254))) WrError(1370);
                else if (Odd(AdrLong)) WrError(1375);
                else
                 BEGIN
                  CodeLen=2;
                  WAsmCode[0]=0xae00+(z << 8)+Lo(AdrLong >> 1);
                 END
               END
              break;
             case 1:
              WAsmCode[0]=0x20f7+(z << 11)+(((Word)OpSize[1]) << 8);
              AdrLong-=EProgCounter();
              switch (OpSize[1])
               BEGIN
                case 0:
                 if ((NOT SymbolQuestionable) AND ((AdrLong<-128) OR (AdrLong>127))) WrError(1370);
                 else
                  BEGIN
                   CodeLen=4; WAsmCode[1]=Lo(AdrLong);
                  END
                 break;
                case 1:
                 if ((NOT SymbolQuestionable) AND ((AdrLong<-32768) OR (AdrLong>32767))) WrError(1370);
                 else
                  BEGIN
                   CodeLen=4; WAsmCode[1]=AdrLong & 0xffff;
                  END
                 break;
                case 2:
                 CodeLen=6; WAsmCode[1]=AdrLong >> 16;
                 WAsmCode[2]=AdrLong & 0xffff;
                 break;
               END
              break;
            END
          END
        END
     return;
    END

   if (*OpPart=='B')
    for (z=0; z<ConditionCount; z++)
     if (strcmp(OpPart+1,Conditions[z])==0)
      BEGIN
       if (ArgCnt!=1) WrError(1110);
       else
        if (CheckFormat("GD"))
         if (GetOpSize(ArgStr[1],1))
          BEGIN
           AdrLong=EvalIntExpression(ArgStr[1],Int32,&OK);
           if (OK)
            BEGIN
             DecideBranch(AdrLong,1);
             switch (FormatCode)
              BEGIN
               case 2:
                if (OpSize[1]!=-1) WrError(1100);
                else
                 BEGIN
                  AdrLong-=EProgCounter();
                  if ((NOT SymbolQuestionable) AND ((AdrLong<-256) OR (AdrLong>254))) WrError(1370);
                  else if (Odd(AdrLong)) WrError(1375);
                  else
                   BEGIN
                    CodeLen=2;
                    WAsmCode[0]=0x8000+(z << 10)+Lo(AdrLong >> 1);
                   END
                 END
                break;
               case 1:
                WAsmCode[0]=0x00f6+(z << 10)+(((Word)OpSize[1]) << 8);
                AdrLong-=EProgCounter();
                switch (OpSize[1])
                 BEGIN
                  case 0:
                   if ((AdrLong<-128) OR (AdrLong>127)) WrError(1370);
                   else
                    BEGIN
                     CodeLen=4; WAsmCode[1]=Lo(AdrLong);
                    END
                   break;
                  case 1:
                   if ((AdrLong<-32768) OR (AdrLong>32767)) WrError(1370);
                   else
                    BEGIN
                     CodeLen=4; WAsmCode[1]=AdrLong & 0xffff;
                    END
                   break;
                  case 2:
                   CodeLen=6; WAsmCode[1]=AdrLong >> 16;
                   WAsmCode[2]=AdrLong & 0xffff;
                   break;
                 END
                break;
              END
            END
          END
       return;
      END

   if ((Memo("ACB")) OR (Memo("SCB")))
    BEGIN
     AdrWord=Ord(Memo("SCB"));
     if (ArgCnt!=4) WrError(1110);
     else if (CheckFormat("GEQR"))
      if (GetOpSize(ArgStr[2],3))
      if (GetOpSize(ArgStr[4],4))
      if (GetOpSize(ArgStr[1],1))
      if (GetOpSize(ArgStr[3],2))
       BEGIN
        if ((OpSize[3]==-1) AND (OpSize[2]==-1)) OpSize[3]=2;
        if ((OpSize[3]==-1) AND (OpSize[2]!=-1)) OpSize[3]=OpSize[2];
        else if ((OpSize[3]!=-1) AND (OpSize[2]==-1)) OpSize[2]=OpSize[3];
        if (OpSize[1]==-1) OpSize[1]=OpSize[2];
        if (OpSize[3]!=OpSize[2]) WrError(1131);
        else if (NOT DecodeReg(ArgStr[2],&HReg)) WrError(1350);
        else
         BEGIN
          AdrLong=EvalIntExpression(ArgStr[4],Int32,&OK);
          if (OK)
           BEGIN
            if (DecodeAdr(ArgStr[1],1,Mask_Source))
             if (DecodeAdr(ArgStr[3],2,Mask_Source))
              BEGIN
               if (FormatCode==0)
                BEGIN
                 if (AdrType[1]!=ModImm) FormatCode=1;
                 else
                  BEGIN
                   HVal=ImmVal(1);
                   if ((HVal==1) AND (AdrType[2]==ModReg)) FormatCode=4;
                   else if ((HVal==1) AND (AdrType[2]==ModImm))
                    BEGIN
                     HVal=ImmVal(2);
                     if ((HVal>=1-AdrWord) AND (HVal<=64-AdrWord)) FormatCode=3;
                     else FormatCode=2;
                    END
                   else if ((HVal>=-128) AND (HVal<=127)) FormatCode=2;
                   else FormatCode=1;
                  END
                END
               switch (FormatCode)
                BEGIN
                 case 1:
                  if (DecideBranchLength(&AdrLong,4))  /* ??? */
                   BEGIN
                    WAsmCode[0]=0xd000+(OpSize[1] << 8)+AdrMode[1];
                    memcpy(WAsmCode+1,AdrVals[1],AdrCnt1[1]);
                    WAsmCode[1+AdrCnt2[1]]=0xf000+(AdrWord << 11)+(OpSize[2] << 8)+AdrMode[2];
                    memcpy(WAsmCode+2+AdrCnt2[1],AdrVals[2],AdrCnt1[2]);
                    WAsmCode[2+AdrCnt2[1]+AdrCnt2[2]]=(HReg << 10)+(OpSize[4] << 8);
                    CodeLen=6+AdrCnt1[1]+AdrCnt1[2];
                   END
                  break;
                 case 2:
                  if (DecideBranchLength(&AdrLong,4))  /* ??? */
                   BEGIN
                    if (AdrType[1]!=ModImm) WrError(1350);
                    else
                     BEGIN
                      HVal=ImmVal(1);
                      if (ChkRange(HVal,-128,127))
                       BEGIN
                        WAsmCode[0]=0xbf00+(HVal & 0xff);
                        WAsmCode[1]=0xf000+(AdrWord << 11)+(OpSize[2] << 8)+AdrMode[2];
                        memcpy(WAsmCode+2,AdrVals[2],AdrCnt1[2]);
                        WAsmCode[2+AdrCnt2[2]]=(HReg << 10)+(OpSize[4] << 8);
                        CodeLen=6+AdrCnt1[2];
                       END
                     END
                   END
                  break;
                 case 3:
                  if (DecideBranchLength(&AdrLong,4))  /* ??? */
                   BEGIN
                    if (AdrType[1]!=ModImm) WrError(1350);
                    else if (ImmVal(1)!=1) WrError(1135);
                    else if (AdrType[2]!=ModImm) WrError(1350);
                    else
                     BEGIN
                      HVal=ImmVal(2);
                      if (ChkRange(HVal,1-AdrWord,64-AdrWord))
                       BEGIN
                        WAsmCode[0]=0x03d1+(HReg << 10)+(AdrWord << 1);
                        WAsmCode[1]=((HVal & 0x3f) << 10)+(OpSize[4] << 8);
                        CodeLen=4;
                       END
                     END
                   END
                  break;
                 case 4:
                  if (DecideBranchLength(&AdrLong,4))  /* ??? */
                   BEGIN
                    if (AdrType[1]!=ModImm) WrError(1350);
                    else if (ImmVal(1)!=1) WrError(1135);
                    else if (OpSize[2]!=2) WrError(1130);
                    else if (AdrType[2]!=ModReg) WrError(1350);
                    else
                     BEGIN
                      WAsmCode[0]=0x03d0+(HReg << 10)+(AdrWord << 1);
                      WAsmCode[1]=((AdrMode[2] & 15) << 10)+(OpSize[4] << 8);
                      CodeLen=4;
                     END
                   END
                  break;
                END
               if (CodeLen>0)
                switch (OpSize[4])
                 BEGIN
                  case 0:
                   WAsmCode[(CodeLen >> 1)-1]+=AdrLong & 0xff;
                   break;
                  case 1:
                   WAsmCode[CodeLen >> 1]=AdrLong & 0xffff;
                   CodeLen+=2;
                   break;
                  case 2:
                   WAsmCode[ CodeLen >> 1   ]=AdrLong >> 16;
                   WAsmCode[(CodeLen >> 1)+1]=AdrLong & 0xffff;
                   CodeLen+=4;
                   break;
                 END
              END
           END
         END
       END
     return;
    END

   if (Memo("TRAPA"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else if (strcmp(Format," ")!=0) WrError(1090);
     else if (*ArgStr[1]!='#') WrError(1350);
     else
      BEGIN
       AdrWord=EvalIntExpression(ArgStr[1]+1,UInt4,&OK);
       if (OK)
        BEGIN
         CodeLen=2; WAsmCode[0]=0x03d5+(AdrWord << 10);
        END
      END
     return;
    END

   if (strncmp(OpPart,"TRAP",4)==0)
    BEGIN
     if (ArgCnt!=0) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else if (strcmp(Format," ")!=0) WrError(1090);
     else
      BEGIN
       SplitOptions();
       if (OptionCnt!=1) WrError(1115);
       else if (NOT DecodeCondition(Options[0],&AdrWord)) WrError(1360);
       else
        BEGIN
         CodeLen=2; WAsmCode[0]=0x03d4+(AdrWord << 10);
        END
      END
     return;
    END

   /* Specials */

   if ((Memo("ENTER")) OR (Memo("EXITD")))
    BEGIN
     if (Memo("EXITD"))
      BEGIN
       z=1; strcpy(ArgStr[3],ArgStr[1]);
       strcpy(ArgStr[1],ArgStr[2]); strcpy(ArgStr[2],ArgStr[3]);
      END
     else z=0;
     if (ArgCnt!=2) WrError(1110);
     else if (CheckFormat("GE"))
      if (GetOpSize(ArgStr[1],1))
       if (GetOpSize(ArgStr[2],2))
        BEGIN
         if (OpSize[1]==-1) OpSize[1]=2;
         if (OpSize[2]==-1) OpSize[2]=2;
         if (OpSize[2]!=2) WrError(1130);
         else if (DecodeAdr(ArgStr[1],1,MModReg+MModImm))
          if (DecodeRegList(ArgStr[2],&AdrWord,z==1))
           BEGIN
            if ((z & 0xc000)!=0) WrXError(1410,"SP/FP");
            else
             BEGIN
              if (FormatCode==0)
               BEGIN
                if (AdrType[1]==ModImm)
                 BEGIN
                  HVal=ImmVal(1);
                  if ((HVal>=-128) AND (HVal<=127)) FormatCode=2;
                  else FormatCode=1;
                 END
                else FormatCode=1;
               END
              switch (FormatCode)
               BEGIN
                case 1:
                 WAsmCode[0]=0x02f7;
                 WAsmCode[1]=0x8c00+(z << 12)+(OpSize[1] << 8)+AdrMode[1];
                 memcpy(WAsmCode+2,AdrVals[1],AdrCnt1[1]);
                 WAsmCode[2+AdrCnt2[1]]=AdrWord;
                 CodeLen=6+AdrCnt1[1];
                 break;
                case 2:
                 if (AdrType[1]!=ModImm) WrError(1350);
                 else
                  BEGIN
                   HVal=ImmVal(1);
                   if (ChkRange(HVal,-128,127))
                    BEGIN
                     WAsmCode[0]=0x8e00+(z << 12)+(HVal & 0xff);
                     WAsmCode[1]=AdrWord;
                     CodeLen=4;
                    END
                  END
                 break;
               END
             END
           END
        END
     return;
    END

   if (strncmp(OpPart,"SCMP",4)==0)
    BEGIN
     if (DOpSize==-1) DOpSize=2;
     if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       SplitOptions();
       if (OptionCnt>1) WrError(1115);
       else
        BEGIN
         OK=True;
         if (OptionCnt==0) AdrWord=6;
         else if (strcasecmp(Options[0],"LTU")==0) AdrWord=0;
         else if (strcasecmp(Options[0],"GEU")==0) AdrWord=1;
         else OK=(DecodeCondition(Options[0],&AdrWord) AND (AdrWord>1) AND (AdrWord<6));
         if (NOT OK) WrXError(1360,Options[0]);
         else
          BEGIN
           WAsmCode[0]=0x00e0+(DOpSize << 8)+(AdrWord << 10);
           CodeLen=2;
          END
        END
      END
     return;
    END

   if ((strncmp(OpPart,"SMOV",4)==0) OR (strncmp(OpPart,"SSCH",4)==0))
    BEGIN
     if (DOpSize==-1) DOpSize=2;
     z=Ord(OpPart[1]=='S') << 4;
     if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       SplitOptions();
       if (strcasecmp(Options[0],"F")==0)
        BEGIN
         Mask=0; strcpy(Options[0],Options[1]); OptionCnt--;
        END
       else if (strcasecmp(Options[0],"B")==0)
        BEGIN
         Mask=1; strcpy(Options[0],Options[1]); OptionCnt--;
        END
       else if (strcasecmp(Options[1],"F")==0)
        BEGIN
         Mask=0; OptionCnt--;
        END
       else if (strcasecmp(Options[1],"B")==0)
        BEGIN
         Mask=1; OptionCnt--;
        END
       else Mask=0;
       if (OptionCnt>1) WrError(1115);
       else
        BEGIN
         OK=True;
         if (OptionCnt==0) AdrWord=6;
         else if (strcasecmp(Options[0],"LTU")==0) AdrWord=0;
         else if (strcasecmp(Options[0],"GEU")==0) AdrWord=1;
         else OK=(DecodeCondition(Options[0],&AdrWord)) AND (AdrWord>1) AND (AdrWord<6);
         if (NOT OK) WrXError(1360,Options[0]);
         else
          BEGIN
           WAsmCode[0]=0x00e4+(DOpSize << 8)+(AdrWord << 10)+Mask+z;
           CodeLen=2;
          END
        END
      END
     return;
    END

   if (Memo("SSTR"))
    BEGIN
     if (DOpSize==-1) DOpSize=2;
     if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       WAsmCode[0]=0x24f7+(DOpSize << 8); CodeLen=2;
      END
     return;
    END

   if ((Memo("LDM")) OR (Memo("STM")))
    BEGIN
     Mask=MModIReg+MModDisp16+MModDisp32+MModAbs16+MModAbs32+MModPCRel16+MModPCRel32;
     if (Memo("LDM"))
      BEGIN
       z=0x1000; Mask+=MModPop;
       strcpy(ArgStr[3],ArgStr[1]); strcpy(ArgStr[1],ArgStr[2]); strcpy(ArgStr[2],ArgStr[3]);
      END
     else
      BEGIN
       z=0; Mask+=MModPush;
      END
     if (ArgCnt!=2) WrError(1110);
     else if (CheckFormat("G"))
      if (GetOpSize(ArgStr[1],1))
       if (GetOpSize(ArgStr[2],2))
        BEGIN
         if (OpSize[1]==-1) OpSize[1]=2;
         if (OpSize[2]==-1) OpSize[2]=2;
         if ((OpSize[1]!=2) OR (OpSize[2]!=2)) WrError(1130);
         else if (DecodeAdr(ArgStr[2],2,Mask))
          if (DecodeRegList(ArgStr[1],&AdrWord,AdrType[2]!=ModPush))
           BEGIN
            WAsmCode[0]=0x8a00+z+AdrMode[2];
            memcpy(WAsmCode+1,AdrVals[2],AdrCnt1[2]);
            WAsmCode[1+AdrCnt2[2]]=AdrWord;
            CodeLen=4+AdrCnt1[2];
           END
        END
     return;
    END

   if ((Memo("STC")) OR (Memo("STP")))
    BEGIN
     z=Ord(Memo("STP")) << 10;
     if (ArgCnt!=2) WrError(1110);
     else if (CheckFormat("G"))
      if (GetOpSize(ArgStr[1],1))
       if (GetOpSize(ArgStr[2],2))
        BEGIN
         if (OpSize[2]==-1) OpSize[2]=2;
         if (OpSize[1]==-1) OpSize[1]=OpSize[1];
         if (OpSize[1]!=OpSize[2]) WrError(1132);
         else if ((z==0) AND (OpSize[2]!=2)) WrError(1130);
         else if (DecodeAdr(ArgStr[1],1,Mask_PureMem))
          if (DecodeAdr(ArgStr[2],2,Mask_Dest))
           BEGIN
            OpSize[1]=0;
            Make_G(0xa800+z);
            WAsmCode[0]+=0x800;
           END
        END
     return;
    END

   if (Memo("WAIT"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else if (strcmp(Format," ")!=0) WrError(1090);
     else if (*ArgStr[1]!='#') WrError(1350);
     else
      BEGIN
       WAsmCode[1]=EvalIntExpression(ArgStr[1]+1,UInt3,&OK);
       if (OK)
        BEGIN
         WAsmCode[0]=0x0fd6; CodeLen=4;
        END
      END
     return;
    END

   if (Memo("JRNG"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (CheckFormat("GE"))
      if (GetOpSize(ArgStr[1],1))
       BEGIN
        if (OpSize[1]==-1) OpSize[1]=1;
        if (OpSize[1]!=1) WrError(1130);
        else if (DecodeAdr(ArgStr[1],1,MModReg+MModImm))
         BEGIN
          if (FormatCode==0)
           BEGIN
            if (AdrType[1]==ModImm)
             BEGIN
              HVal=ImmVal(1);
              if ((HVal>=0) AND (HVal<=255)) FormatCode=2;
              else FormatCode=1;
             END
            else FormatCode=1;
           END
          switch (FormatCode)
           BEGIN
            case 1:
             WAsmCode[0]=0xba00+AdrMode[1];
             memcpy(WAsmCode+1,AdrVals[1],AdrCnt1[1]);
             CodeLen=2+AdrCnt1[1];
             break;
            case 2:
             if (AdrType[1]!=ModImm) WrError(1350);
             else
              BEGIN
               HVal=ImmVal(1);
               if (ChkRange(HVal,0,255))
                BEGIN
                 WAsmCode[0]=0xbe00+(HVal & 0xff); CodeLen=2;
                END
              END
             break;
           END
         END
       END
     return;
    END

   WrXError(1200,OpPart);
END

        static Boolean IsDef_M16(void)
BEGIN
   return False;
END

        static void SwitchFrom_M16(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_M16(void)
BEGIN
   TurnWords=True; ConstMode=ConstModeIntel; SetIsOccupied=False;

   PCSymbol="$"; HeaderID=0x13; NOPCode=0x1bd6;
   DivideChars=","; HasAttrs=True; AttrChars=".:";

   ValidSegs=1<<SegCode;
   Grans[SegCode]=1; ListGrans[SegCode]=2; SegInits[SegCode]=0;
#ifdef __STDC__
   SegLimits[SegCode] = 0xfffffffful;
#else
   SegLimits[SegCode] = 0xffffffffl;
#endif

   MakeCode=MakeCode_M16; IsDef=IsDef_M16;
   SwitchFrom=SwitchFrom_M16; InitFields();
END

        void codem16_init(void)
BEGIN
   CPUM16=AddCPU("M16",SwitchTo_M16);
END

