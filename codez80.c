/* codez80.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator Zilog Z80/180/380                                           */
/*                                                                           */
/* Historie: 26. 8.1996 Grundsteinlegung                                     */
/*            1. 2.1998 ChkPC ersetzt                                        */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "nls.h"
#include "strutil.h"
#include "bpemu.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmcode.h"
#include "asmallg.h"
#include "codepseudo.h"
#include "codevars.h"

/*-------------------------------------------------------------------------*/
/* Instruktionsgruppendefinitionen */

typedef struct
         {
          char *Name;
          CPUVar MinCPU;
          Byte Len;
          Word Code;
         } BaseOrder;

typedef struct
         {
          char *Name;
          Byte Code;
         } Condition; 

typedef struct
         {
          char *Name;
          Byte Code;
         } ALUOrder; 

/*-------------------------------------------------------------------------*/
/* Praefixtyp */

typedef enum {Pref_IN_N,Pref_IN_W ,Pref_IB_W ,Pref_IW_W ,Pref_IB_N ,
                        Pref_IN_LW,Pref_IB_LW,Pref_IW_LW,Pref_IW_N} PrefType;


#define ExtFlagName    "INEXTMODE"       /* Flag-Symbolnamen */
#define LWordFlagName  "INLWORDMODE"

#define ModNone (-1)
#define ModReg8 1
#define ModReg16 2
#define ModIndReg16 3
#define ModImm 4
#define ModAbs 5
#define ModRef 6
#define ModInt 7
#define ModSPRel 8

#define FixedOrderCnt 53
#define AccOrderCnt 3
#define HLOrderCnt 3
#define ALUOrderCnt 5
#define ShiftOrderCnt 8
#define BitOrderCnt 3
#define ConditionCnt 12

#define IXPrefix 0xdd
#define IYPrefix 0xfd

/*-------------------------------------------------------------------------*/

static Byte PrefixCnt;
static Byte AdrPart,OpSize;
static Byte AdrVals[4];
static ShortInt AdrMode;

static BaseOrder *FixedOrders;
static BaseOrder *AccOrders;
static BaseOrder *HLOrders;
static ALUOrder *ALUOrders;
static char **ShiftOrders;
static char **BitOrders;
static Condition *Conditions;

static SimpProc SaveInitProc;

static CPUVar CPUZ80,CPUZ80U,CPUZ180,CPUZ380;

static Boolean MayLW,             /* Instruktion erlaubt 32 Bit */
               ExtFlag,           /* Prozessor im 4GByte-Modus ? */
               LWordFlag;         /* 32-Bit-Verarbeitung ? */

static PrefType CurrPrefix,       /* mom. explizit erzeugter Praefix */
                LastPrefix;       /* von der letzten Anweisung generierter Praefix */

/*==========================================================================*/
/* Codetabellenerzeugung */

        static void AddFixed(char *NewName, CPUVar NewMin, Byte NewLen, Word NewCode)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NewName;
   FixedOrders[InstrZ].MinCPU=NewMin;
   FixedOrders[InstrZ].Len=NewLen;
   FixedOrders[InstrZ++].Code=NewCode;
END

        static void AddAcc(char *NewName, CPUVar NewMin, Byte NewLen, Word NewCode)
BEGIN
   if (InstrZ>=AccOrderCnt) exit(255);
   AccOrders[InstrZ].Name=NewName;
   AccOrders[InstrZ].MinCPU=NewMin;
   AccOrders[InstrZ].Len=NewLen;
   AccOrders[InstrZ++].Code=NewCode;
END

        static void AddHL(char *NewName, CPUVar NewMin, Byte NewLen, Word NewCode)
BEGIN
   if (InstrZ>=HLOrderCnt) exit(255);
   HLOrders[InstrZ].Name=NewName;
   HLOrders[InstrZ].MinCPU=NewMin;
   HLOrders[InstrZ].Len=NewLen;
   HLOrders[InstrZ++].Code=NewCode;
END

        static void AddALU(char *NewName, Byte NCode)
BEGIN
   if (InstrZ>=ALUOrderCnt) exit(255);
   ALUOrders[InstrZ].Name=NewName;
   ALUOrders[InstrZ++].Code=NCode;
END

        static void AddShift(char *NName)
BEGIN
   if (InstrZ>=ShiftOrderCnt) exit(255);
   ShiftOrders[InstrZ++]=NName;
END

        static void AddBit(char *NName)
BEGIN
   if (InstrZ>=BitOrderCnt) exit(255);
   BitOrders[InstrZ++]=NName;
END

        static void AddCondition(char *NewName, Byte NewCode)
BEGIN
   if (InstrZ>=ConditionCnt) exit(255);
   Conditions[InstrZ].Name=NewName;
   Conditions[InstrZ++].Code=NewCode;
END

        static void InitFields(void)
BEGIN
   InstrZ=0; Conditions=(Condition *) malloc(sizeof(Condition)*ConditionCnt);
   AddCondition("NZ",0); AddCondition("Z" ,1);
   AddCondition("NC",2); AddCondition("C" ,3);
   AddCondition("PO",4); AddCondition("NV",4);
   AddCondition("PE",5); AddCondition("V" ,5);
   AddCondition("P" ,6); AddCondition("NS",6);
   AddCondition("M" ,7); AddCondition("S" ,7);

   InstrZ=0; FixedOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*FixedOrderCnt);
   AddFixed("EXX"  ,CPUZ80 ,1,0x00d9); AddFixed("LDI"  ,CPUZ80 ,2,0xeda0);
   AddFixed("LDIR" ,CPUZ80 ,2,0xedb0); AddFixed("LDD"  ,CPUZ80 ,2,0xeda8);
   AddFixed("LDDR" ,CPUZ80 ,2,0xedb8); AddFixed("CPI"  ,CPUZ80 ,2,0xeda1);
   AddFixed("CPIR" ,CPUZ80 ,2,0xedb1); AddFixed("CPD"  ,CPUZ80 ,2,0xeda9);
   AddFixed("CPDR" ,CPUZ80 ,2,0xedb9); AddFixed("RLCA" ,CPUZ80 ,1,0x0007);
   AddFixed("RRCA" ,CPUZ80 ,1,0x000f); AddFixed("RLA"  ,CPUZ80 ,1,0x0017);
   AddFixed("RRA"  ,CPUZ80 ,1,0x001f); AddFixed("RLD"  ,CPUZ80 ,2,0xed6f);
   AddFixed("RRD"  ,CPUZ80 ,2,0xed67); AddFixed("DAA"  ,CPUZ80 ,1,0x0027);
   AddFixed("CCF"  ,CPUZ80 ,1,0x003f); AddFixed("SCF"  ,CPUZ80 ,1,0x0037);
   AddFixed("NOP"  ,CPUZ80 ,1,0x0000); AddFixed("HALT" ,CPUZ80 ,1,0x0076);
   AddFixed("RETI" ,CPUZ80 ,2,0xed4d); AddFixed("RETN" ,CPUZ80 ,2,0xed45);
   AddFixed("INI"  ,CPUZ80 ,2,0xeda2); AddFixed("INIR" ,CPUZ80 ,2,0xedb2);
   AddFixed("IND"  ,CPUZ80 ,2,0xedaa); AddFixed("INDR" ,CPUZ80 ,2,0xedba);
   AddFixed("OUTI" ,CPUZ80 ,2,0xeda3); AddFixed("OTIR" ,CPUZ80 ,2,0xedb3);
   AddFixed("OUTD" ,CPUZ80 ,2,0xedab); AddFixed("OTDR" ,CPUZ80 ,2,0xedbb);
   AddFixed("SLP"  ,CPUZ180,2,0xed76); AddFixed("OTIM" ,CPUZ180,2,0xed83);
   AddFixed("OTIMR",CPUZ180,2,0xed93); AddFixed("OTDM" ,CPUZ180,2,0xed8b);
   AddFixed("OTDMR",CPUZ180,2,0xed9b); AddFixed("BTEST",CPUZ380,2,0xedcf);
   AddFixed("EXALL",CPUZ380,2,0xedd9); AddFixed("EXXX" ,CPUZ380,2,0xddd9);
   AddFixed("EXXY" ,CPUZ380,2,0xfdd9); AddFixed("INDW" ,CPUZ380,2,0xedea);
   AddFixed("INDRW",CPUZ380,2,0xedfa); AddFixed("INIW" ,CPUZ380,2,0xede2);
   AddFixed("INIRW",CPUZ380,2,0xedf2); AddFixed("LDDW" ,CPUZ380,2,0xede8);
   AddFixed("LDDRW",CPUZ380,2,0xedf8); AddFixed("LDIW" ,CPUZ380,2,0xede0);
   AddFixed("LDIRW",CPUZ380,2,0xedf0); AddFixed("MTEST",CPUZ380,2,0xddcf);
   AddFixed("OTDRW",CPUZ380,2,0xedfb); AddFixed("OTIRW",CPUZ380,2,0xedf3);
   AddFixed("OUTDW",CPUZ380,2,0xedeb); AddFixed("OUTIW",CPUZ380,2,0xede3);
   AddFixed("RETB" ,CPUZ380,2,0xed55);

   InstrZ=0; AccOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*AccOrderCnt);
   AddAcc("CPL"  ,CPUZ80 ,1,0x002f); AddAcc("NEG"  ,CPUZ80 ,2,0xed44);
   AddAcc("EXTS" ,CPUZ380,2,0xed65);

   InstrZ=0; HLOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*HLOrderCnt);
   AddHL("CPLW" ,CPUZ380,2,0xdd2f);  AddHL("NEGW" ,CPUZ380,2,0xed54);
   AddHL("EXTSW",CPUZ380,2,0xed75);

   InstrZ=0; ALUOrders=(ALUOrder *) malloc(sizeof(ALUOrder)*ALUOrderCnt);
   AddALU("SUB", 2); AddALU("AND", 4);
   AddALU("OR" , 6); AddALU("XOR", 5);
   AddALU("CP" , 7);

   InstrZ=0; ShiftOrders=(char **) malloc(sizeof(char *)*ShiftOrderCnt);
   AddShift("RLC"); AddShift("RRC"); AddShift("RL"); AddShift("RR"); 
   AddShift("SLA"); AddShift("SRA"); AddShift("SLIA"); AddShift("SRL");

   InstrZ=0; BitOrders=(char **) malloc(sizeof(char *)*BitOrderCnt);
   AddBit("BIT"); AddBit("RES"); AddBit("SET");
END

        static void DeinitFields(void)
BEGIN
   free(Conditions);
   free(FixedOrders);
   free(AccOrders);
   free(HLOrders);
   free(ALUOrders);
   free(ShiftOrders);
   free(BitOrders);
END

/*==========================================================================*/
/* Adressbereiche */

        static LargeWord CodeEnd(void)
BEGIN
#ifdef __STDC__
   if (ExtFlag) return 0xfffffffflu;
#else
   if (ExtFlag) return 0xffffffffl;
#endif
   else if (MomCPU==CPUZ180) return 0x7ffffl;
   else return 0xffff;
END

        static LargeWord PortEnd(void)
BEGIN
#ifdef __STDC__
   if (ExtFlag) return 0xfffffffflu;
#else
   if (ExtFlag) return 0xffffffffl;
#endif
   else return 0xff;
END

/*==========================================================================*/
/* Praefix dazuaddieren */

        static Boolean ExtendPrefix(PrefType *Dest, char *AddArg)
BEGIN
   Byte SPart,IPart;

   switch (*Dest)
    BEGIN
     case Pref_IB_N:
     case Pref_IB_W:
     case Pref_IB_LW: IPart=1; break;
     case Pref_IW_N:
     case Pref_IW_W:
     case Pref_IW_LW: IPart=2; break;
     default: IPart=0;
    END

   switch (*Dest)
    BEGIN
     case Pref_IN_W:
     case Pref_IB_W:
     case Pref_IW_W:  SPart=1; break;
     case Pref_IN_LW:
     case Pref_IB_LW: 
     case Pref_IW_LW: SPart=2; break;
     default: SPart=0;
    END

   if (strcmp(AddArg,"W")==0)         /* Wortverarbeitung */
    SPart=1;
   else if (strcmp(AddArg,"LW")==0)   /* Langwortverarbeitung */
    SPart=2;
   else if (strcmp(AddArg,"IB")==0)   /* ein Byte im Argument mehr */
    IPart=1;
   else if (strcmp(AddArg,"IW")==0)   /* ein Wort im Argument mehr */
    IPart=2;
   else return False;

   switch ((IPart << 4)+SPart)
    BEGIN
     case 0x00:*Dest=Pref_IN_N; break;
     case 0x01:*Dest=Pref_IN_W; break;
     case 0x02:*Dest=Pref_IN_LW; break;
     case 0x10:*Dest=Pref_IB_N; break;
     case 0x11:*Dest=Pref_IB_W; break;
     case 0x12:*Dest=Pref_IB_LW; break;
     case 0x20:*Dest=Pref_IW_N; break;
     case 0x21:*Dest=Pref_IW_W; break;
     case 0x22:*Dest=Pref_IW_LW; break;
    END

   return True;
END

/*--------------------------------------------------------------------------*/
/* Code fuer Praefix bilden */

        static void GetPrefixCode(PrefType inp, Byte *b1 ,Byte *b2)
BEGIN
   int z;

   z=((int)inp)-1;
   *b1=0xdd+((z & 4) << 3);
   *b2=0xc0+(z & 3);
END

/*--------------------------------------------------------------------------*/
/* DD-Praefix addieren, nur EINMAL pro Instruktion benutzen! */

        static void ChangeDDPrefix(char *Add)
BEGIN
   PrefType ActPrefix;
   int z;

   ActPrefix=LastPrefix;
   if (ExtendPrefix(&ActPrefix,Add))
    if (LastPrefix!=ActPrefix)
     BEGIN
      if (LastPrefix!=Pref_IN_N) RetractWords(2);
      for (z=PrefixCnt-1; z>=0; z--) BAsmCode[2+z]=BAsmCode[z];
      PrefixCnt+=2;
      GetPrefixCode(ActPrefix,BAsmCode+0,BAsmCode+1);
     END
END

/*--------------------------------------------------------------------------*/
/* Wortgroesse ? */

        static Boolean InLongMode(void)
BEGIN
   switch (LastPrefix)
    BEGIN
     case Pref_IN_W:
     case Pref_IB_W:
     case Pref_IW_W: return False;
     case Pref_IN_LW:
     case Pref_IB_LW:
     case Pref_IW_LW: return MayLW;
     default: return LWordFlag AND MayLW;
    END
END

/*--------------------------------------------------------------------------*/
/* absolute Adresse */

        static LongWord EvalAbsAdrExpression(char *inp, Boolean *OK)
BEGIN
   return EvalIntExpression(inp,ExtFlag?Int32:UInt16,OK);
END

/*==========================================================================*/
/* Adressparser */

        static Boolean DecodeReg8(char *Asc, Byte *Erg)
BEGIN
#define Reg8Cnt 7
   static char *Reg8Names[Reg8Cnt]={"B","C","D","E","H","L","A"};
   int z;

   for (z=0; z<Reg8Cnt; z++)
    if (strcasecmp(Asc,Reg8Names[z])==0)
     BEGIN
      *Erg=z; if (z==6) (*Erg)++;
      return True;
     END

   return False;
END

        static Boolean IsSym(char ch)
BEGIN
   return ((ch=='_') OR ((ch>='0') AND (ch<='9')) OR ((ch>='A') AND (ch<='Z')) OR ((ch>='a') AND (ch<='z')));
END

        static void DecodeAdr(char *Asc_O)
BEGIN
#define Reg8XCnt 4
   static char *Reg8XNames[Reg8XCnt]={"IXU","IXL","IYU","IYL"};
#define Reg16Cnt 6
   static char *Reg16Names[Reg16Cnt]={"BC","DE","HL","SP","IX","IY"};

   int z;
   Integer AdrInt;
   LongInt AdrLong;
   Boolean OK;
   String Asc;

   AdrMode=ModNone; AdrCnt=0; AdrPart=0;

   /* 0. Sonderregister */

   if (strcasecmp(Asc_O,"R")==0)
    BEGIN
     AdrMode=ModRef; return;
    END

   if (strcasecmp(Asc_O,"I")==0)
    BEGIN
     AdrMode=ModInt; return;
    END

   /* 1. 8-Bit-Register ? */

   if (DecodeReg8(Asc_O,&AdrPart))
    BEGIN
     AdrMode=ModReg8;
     return;
    END

   /* 1a. 8-Bit-Haelften von IX/IY ? (nur Z380, sonst als Symbole zulassen) */

   if ((MomCPU>=CPUZ380) OR (MomCPU==CPUZ80U))
    for (z=0; z<Reg8XCnt; z++)
     if (strcasecmp(Asc_O,Reg8XNames[z])==0)
      BEGIN
       AdrMode=ModReg8;
       BAsmCode[PrefixCnt++]=(z<=1)?IXPrefix:IYPrefix;
       AdrPart=4+(z & 1); /* = H /L */
       return;
      END

   /* 2. 16-Bit-Register ? */

   for (z=0; z<Reg16Cnt; z++)
    if (strcasecmp(Asc_O,Reg16Names[z])==0)
     BEGIN
      AdrMode=ModReg16;
      if (z<=3) AdrPart=z;
      else
       BEGIN
        BAsmCode[PrefixCnt++]=(z==4)?IXPrefix:IYPrefix;
        AdrPart=2; /* = HL */
       END
      return;
     END

   /* 3. 16-Bit-Register indirekt ? */

   if ((strlen(Asc_O)>=4) AND (*Asc_O=='(') AND (Asc_O[strlen(Asc_O)-1]==')')) 
    for (z=0; z<Reg16Cnt; z++)
     if ((strncasecmp(Asc_O+1,Reg16Names[z],2)==0)
     AND (NOT IsSym(Asc_O[3])))
      BEGIN
       if (z<3)
        BEGIN
         if (strlen(Asc_O)!=4)
          BEGIN
           WrError(1350); return;
          END
         switch (z)
          BEGIN
           case 0:
           case 1:   /* BC,DE */
            AdrMode=ModIndReg16; AdrPart=z;
            break;
           case 2:   /* HL=M-Register */
            AdrMode=ModReg8; AdrPart=6;
            break;
          END
        END
       else
        BEGIN        /* SP,IX,IY */
         strmaxcpy(Asc,Asc_O+3,255); Asc[strlen(Asc)-1]='\0';
         if (*Asc=='+') strcpy(Asc,Asc+1);
         AdrLong=EvalIntExpression(Asc,(MomCPU>=CPUZ380)?SInt24:SInt8,&OK);
         if (OK)
          BEGIN
           if (z==3) AdrMode=ModSPRel;
           else
            BEGIN
             AdrMode=ModReg8; AdrPart=6;
             BAsmCode[PrefixCnt++]=(z==4)?IXPrefix:IYPrefix;
            END
           AdrVals[AdrCnt++]=AdrLong & 0xff;
           if ((AdrLong>=-0x80l) AND (AdrLong<=0x7fl));
           else
            BEGIN
             AdrVals[AdrCnt++]=(AdrLong >> 8) & 0xff;
             if ((AdrLong>=-0x8000l) AND (AdrLong<=0x7fffl)) ChangeDDPrefix("IB");
             else
              BEGIN
               AdrVals[AdrCnt++]=(AdrLong >> 16) & 0xff;
               ChangeDDPrefix("IW");
              END
            END
          END
        END
       return;
      END

   /* absolut ? */

   if (IsIndirect(Asc_O))
    BEGIN
     AdrLong=EvalAbsAdrExpression(Asc_O,&OK);
     if (OK)
      BEGIN
       ChkSpace(SegCode);
       AdrMode=ModAbs;
       AdrVals[0]=AdrLong & 0xff;
       AdrVals[1]=(AdrLong >> 8) & 0xff;
       AdrCnt=2;
#ifdef __STDC__
       if ((AdrLong & 0xffff0000u)==0);
#else
       if ((AdrLong & 0xffff0000)==0);
#endif
       else
        BEGIN
         AdrVals[AdrCnt++]=((AdrLong >> 16) & 0xff);
#ifdef __STDC__
         if ((AdrLong & 0xff000000u)==0) ChangeDDPrefix("IB");
#else
         if ((AdrLong & 0xff000000)==0) ChangeDDPrefix("IB");
#endif
         else
          BEGIN
           AdrVals[AdrCnt++]=((AdrLong >> 24) & 0xff);
           ChangeDDPrefix("IW");
          END
        END
      END
     return;
    END

   /* ...immediate */

   switch (OpSize)
    BEGIN
     case 0xff:
      WrError(1132);
      break;
     case 0:
      AdrVals[0]=EvalIntExpression(Asc_O,Int8,&OK);
      if (OK)
       BEGIN
        AdrMode=ModImm; AdrCnt=1;
       END;
      break;
     case 1:
      if (InLongMode())
       BEGIN
        AdrLong=EvalIntExpression(Asc_O,Int32,&OK);
        if (OK)
         BEGIN
          AdrVals[0]=Lo(AdrLong); AdrVals[1]=Hi(AdrLong);
          AdrMode=ModImm; AdrCnt=2;
#ifdef __STDC__
          if ((AdrLong & 0xffff0000u)==0);
#else
          if ((AdrLong & 0xffff0000)==0);
#endif
          else
           BEGIN
            AdrVals[AdrCnt++]=(AdrLong >> 16) & 0xff;
#ifdef __STDC__
            if ((AdrLong & 0xff000000u)==0) ChangeDDPrefix("IB");
#else
            if ((AdrLong & 0xff000000)==0) ChangeDDPrefix("IB");
#endif
            else
             BEGIN
              AdrVals[AdrCnt++]=(AdrLong >> 24) & 0xff;
              ChangeDDPrefix("IW");
             END
           END
         END
       END
      else
       BEGIN
        AdrInt=EvalIntExpression(Asc_O,Int16,&OK);
        if (OK)
         BEGIN
          AdrVals[0]=Lo(AdrInt); AdrVals[1]=Hi(AdrInt);
          AdrMode=ModImm; AdrCnt=2;
         END
       END
      break;
    END
END

/*-------------------------------------------------------------------------*/
/* Bedingung entschluesseln */

        static Boolean DecodeCondition(char *Name, int *Erg)
BEGIN
   int z;
   String Name_N;

   strmaxcpy(Name_N,Name,255); NLS_UpString(Name_N);

   z=0;
   while ((z<ConditionCnt) AND (strcmp(Conditions[z].Name,Name_N)!=0)) z++;
   if (z>ConditionCnt) return False;
   else
    BEGIN
     *Erg=Conditions[z].Code; return True;
    END
END

/*-------------------------------------------------------------------------*/
/* Sonderregister dekodieren */

        static Boolean DecodeSFR(char *Inp, Byte *Erg)
BEGIN
   if (strcasecmp(Inp,"SR")==0) *Erg=1;
   else if (strcasecmp(Inp,"XSR")==0) *Erg=5;
   else if (strcasecmp(Inp,"DSR")==0) *Erg=6;
   else if (strcasecmp(Inp,"YSR")==0) *Erg=7;
   else return False;
   return True;
END

/*=========================================================================*/

        static Boolean DecodePseudo(void)
BEGIN
   if (Memo("PORT"))
    BEGIN
     CodeEquate(SegIO,0,PortEnd());
     return True;
    END

   /* Kompatibilitaet zum M80 */

   if (Memo("DEFB")) strmaxcpy(OpPart,"DB",255);
   if (Memo("DEFW")) strmaxcpy(OpPart,"DW",255);

   return False;
END

        static void DecodeLD(void)
BEGIN
   Byte AdrByte,HLen;
   int z;
   Byte HVals[5];

   if (ArgCnt!=2) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1]);
     switch (AdrMode)
      BEGIN
       case ModReg8:
        if (AdrPart==7) /* LD A,... */
         BEGIN
          OpSize=0; DecodeAdr(ArgStr[2]);
          switch (AdrMode)
           BEGIN
            case ModReg8: /* LD A,R8/RX8/(HL)/(XY+D) */
             BAsmCode[PrefixCnt]=0x78+AdrPart;
             memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
             CodeLen=PrefixCnt+1+AdrCnt;
             break;
            case ModIndReg16: /* LD A,(BC)/(DE) */
             BAsmCode[0]=0x0a+(AdrPart << 4); CodeLen=1;
             break;
            case ModImm: /* LD A,imm8 */
             BAsmCode[0]=0x3e; BAsmCode[1]=AdrVals[0]; CodeLen=2;
             break;
            case ModAbs: /* LD a,(adr) */
             BAsmCode[PrefixCnt]=0x3a;
             memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
             CodeLen=PrefixCnt+1+AdrCnt;
             break;
            case ModRef: /* LD A,R */
             BAsmCode[0]=0xed; BAsmCode[1]=0x5f;
             CodeLen=2;
             break;
            case ModInt: /* LD A,I */
             BAsmCode[0]=0xed; BAsmCode[1]=0x57;
             CodeLen=2;
             break;
            default: if (AdrMode!=ModNone) WrError(1350);
           END
         END
        else if ((AdrPart!=6) AND (PrefixCnt==0)) /* LD R8,... */
         BEGIN
          AdrByte=AdrPart; OpSize=0; DecodeAdr(ArgStr[2]);
          switch (AdrMode)
           BEGIN
            case ModReg8: /* LD R8,R8/RX8/(HL)/(XY+D) */
             if (((AdrByte==4) OR (AdrByte==5)) AND (PrefixCnt==1) AND (AdrCnt==0)) WrError(1350);
             else
              BEGIN
               BAsmCode[PrefixCnt]=0x40+(AdrByte << 3)+AdrPart;
               memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
               CodeLen=PrefixCnt+1+AdrCnt;
              END
             break;
            case ModImm: /* LD R8,imm8 */
             BAsmCode[0]=0x06+(AdrByte << 3); BAsmCode[1]=AdrVals[0];
             CodeLen=2;
             break;
            default: if (AdrMode!=ModNone) WrError(1350);
           END
         END
        else if ((AdrPart==4) OR (AdrPart==5)) /* LD RX8,... */
         BEGIN
          AdrByte=AdrPart; OpSize=0; DecodeAdr(ArgStr[2]);
          switch (AdrMode)
           BEGIN
            case ModReg8: /* LD RX8,R8/RX8 */
             if (AdrPart==6) WrError(1350);
             else if ((AdrPart>=4) AND (AdrPart<=5) AND (PrefixCnt!=2)) WrError(1350);
             else if ((AdrPart>=4) AND (AdrPart<=5) AND (BAsmCode[0]!=BAsmCode[1])) WrError(1350);
             else
              BEGIN
               if (PrefixCnt==2) PrefixCnt--;
               BAsmCode[PrefixCnt]=0x40+(AdrByte << 3)+AdrPart;
               CodeLen=PrefixCnt+1;
              END
             break;
            case ModImm: /* LD RX8,imm8 */
             BAsmCode[PrefixCnt]=0x06+(AdrByte << 3);
             BAsmCode[PrefixCnt+1]=AdrVals[0];
             CodeLen=PrefixCnt+2;
             break;
            default: if (AdrMode!=ModNone) WrError(1350);
           END
         END
        else /* LD (HL)/(XY+d),... */
         BEGIN
          HLen=AdrCnt; memcpy(HVals,AdrVals,AdrCnt); z=PrefixCnt;
          if ((z==0) AND (Memo("LDW")))
           BEGIN
            OpSize=1; MayLW=True;
           END
          else OpSize=0;
          DecodeAdr(ArgStr[2]);
          switch (AdrMode)
           BEGIN
            case ModReg8: /* LD (HL)/(XY+D),R8 */
             if ((PrefixCnt!=z) OR (AdrPart==6)) WrError(1350);
             else
              BEGIN
               BAsmCode[PrefixCnt]=0x70+AdrPart;
               memcpy(BAsmCode+PrefixCnt+1,HVals,HLen);
               CodeLen=PrefixCnt+1+HLen;
              END
             break;
            case ModImm: /* LD (HL)/(XY+D),imm8:16:32 */
             if ((z==0) AND (Memo("LDW")))
              if (MomCPU<CPUZ380) WrError(1500);
              else
               BEGIN
                BAsmCode[PrefixCnt]=0xed; BAsmCode[PrefixCnt+1]=0x36;
                memcpy(BAsmCode+PrefixCnt+2,AdrVals,AdrCnt);
                CodeLen=PrefixCnt+2+AdrCnt;
               END
             else
              BEGIN
               BAsmCode[PrefixCnt]=0x36;
               memcpy(BAsmCode+1+PrefixCnt,HVals,HLen);
               BAsmCode[PrefixCnt+1+HLen]=AdrVals[0];
               CodeLen=PrefixCnt+1+HLen+AdrCnt;
              END
             break;
            case ModReg16: /* LD (HL)/(XY+D),R16/XY */
             if (MomCPU<CPUZ380) WrError(1500);
             else if (AdrPart==3) WrError(1350);
             else if (HLen==0)
              if (PrefixCnt==z) /* LD (HL),R16 */
               BEGIN
                if (AdrPart==2) AdrPart=3;
                BAsmCode[0]=0xfd; BAsmCode[1]=0x0f+(AdrPart << 4);
                CodeLen=2;
               END
              else /* LD (HL),XY */
               BEGIN
                CodeLen=PrefixCnt+1; BAsmCode[PrefixCnt]=0x31;
                CodeLen=1+PrefixCnt;
               END
             else
              if (PrefixCnt==z) /* LD (XY+D),R16 */
               BEGIN
                if (AdrPart==2) AdrPart=3;
                BAsmCode[PrefixCnt]=0xcb;
                memcpy(BAsmCode+PrefixCnt+1,HVals,HLen);
                BAsmCode[PrefixCnt+1+HLen]=0x0b+(AdrPart << 4);
                CodeLen=PrefixCnt+1+HLen+1;
               END
              else if (BAsmCode[0]==BAsmCode[1]) WrError(1350);
              else
               BEGIN
                PrefixCnt--;
                BAsmCode[PrefixCnt]=0xcb;
                memcpy(BAsmCode+PrefixCnt+1,HVals,HLen);
                BAsmCode[PrefixCnt+1+HLen]=0x2b;
                CodeLen=PrefixCnt+1+HLen+1;
               END
             break;
            default: if (AdrMode!=ModNone) WrError(1350);
           END
         END
        break;
       case ModReg16:
        if (AdrPart==3) /* LD SP,... */
         BEGIN
          OpSize=1; MayLW=True; DecodeAdr(ArgStr[2]);
          switch (AdrMode)
           BEGIN
            case ModReg16: /* LD SP,HL/XY */
             if (AdrPart!=2) WrError(1350);
             else
              BEGIN
               BAsmCode[PrefixCnt]=0xf9; CodeLen=PrefixCnt+1;
              END
             break;
            case ModImm: /* LD SP,imm16:32 */
             BAsmCode[PrefixCnt]=0x31;
             memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
             CodeLen=PrefixCnt+1+AdrCnt;
             break;
            case ModAbs: /* LD SP,(adr) */
             BAsmCode[PrefixCnt]=0xed; BAsmCode[PrefixCnt+1]=0x7b;
             memcpy(BAsmCode+PrefixCnt+2,AdrVals,AdrCnt);
             CodeLen=PrefixCnt+2+AdrCnt;
             break;
            default: if (AdrMode!=ModNone) WrError(1350);
           END
         END
        else if (PrefixCnt==0) /* LD R16,... */
         BEGIN
          AdrByte=(AdrPart==2) ? 3 : AdrPart;
          OpSize=1; MayLW=True; DecodeAdr(ArgStr[2]);
          switch (AdrMode)
           BEGIN
            case ModInt: /* LD HL,I */
             if (MomCPU<CPUZ380) WrError(1500);
             else if (AdrByte!=3) WrError(1350);
             else
              BEGIN
               BAsmCode[0]=0xdd; BAsmCode[1]=0x57; CodeLen=2;
              END
             break;
            case ModReg8:
             if (AdrPart!=6) WrError(1350);
             else if (MomCPU<CPUZ380) WrError(1500);
             else if (PrefixCnt==0) /* LD R16,(HL) */
              BEGIN
               BAsmCode[0]=0xdd; BAsmCode[1]=0x0f+(AdrByte << 4);
               CodeLen=2;
              END
             else /* LD R16,(XY+d) */
              BEGIN
               BAsmCode[PrefixCnt]=0xcb;
               memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
               BAsmCode[PrefixCnt+1+AdrCnt]=0x03+(AdrByte << 4);
               CodeLen=PrefixCnt+1+AdrCnt+1;
              END
             break;
            case ModReg16:
             if (AdrPart==3) WrError(1350);
             else if (MomCPU<CPUZ380) WrError(1500);
             else if (PrefixCnt==0) /* LD R16,R16 */
              BEGIN
               if (AdrPart==2) AdrPart=3;
               else if (AdrPart==0) AdrPart=2;
               BAsmCode[0]=0xcd+(AdrPart << 4);
               BAsmCode[1]=0x02+(AdrByte << 4);
               CodeLen=2;
              END
             else /* LD R16,XY */
              BEGIN
               BAsmCode[PrefixCnt]=0x0b+(AdrByte << 4);
               CodeLen=PrefixCnt+1;
              END
             break;
            case ModIndReg16: /* LD R16,(R16) */
             if (MomCPU<CPUZ380) WrError(1500);
             else
              BEGIN
               CodeLen=2; BAsmCode[0]=0xdd;
               BAsmCode[1]=0x0c+(AdrByte << 4)+AdrPart;
              END
             break;
            case ModImm: /* LD R16,imm */
             if (AdrByte==3) AdrByte=2;
             CodeLen=PrefixCnt+1+AdrCnt;
             BAsmCode[PrefixCnt]=0x01+(AdrByte << 4);
             memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
             break;
            case ModAbs: /* LD R16,(adr) */
             if (AdrByte==3)
              BEGIN
               BAsmCode[PrefixCnt]=0x2a;
               memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
               CodeLen=1+PrefixCnt+AdrCnt;
              END
             else
              BEGIN
               BAsmCode[PrefixCnt]=0xed;
               BAsmCode[PrefixCnt+1]=0x4b+(AdrByte << 4);
               memcpy(BAsmCode+PrefixCnt+2,AdrVals,AdrCnt);
               CodeLen=PrefixCnt+2+AdrCnt;
              END
             break;
            case ModSPRel: /* LD R16,(SP+D) */
             if (MomCPU<CPUZ380) WrError(1500);
             else
              BEGIN
               BAsmCode[PrefixCnt]=0xdd;
               BAsmCode[PrefixCnt+1]=0xcb;
               memcpy(BAsmCode+PrefixCnt+2,AdrVals,AdrCnt);
               BAsmCode[PrefixCnt+2+AdrCnt]=0x01+(AdrByte << 4);
               CodeLen=PrefixCnt+3+AdrCnt;
              END
             break;
            default: if (AdrMode!=ModNone) WrError(1350);
           END
         END
        else /* LD XY,... */
         BEGIN
          OpSize=1; MayLW=True; DecodeAdr(ArgStr[2]);
          switch (AdrMode)
           BEGIN
            case ModReg8:
             if (AdrPart!=6) WrError(1350);
             else if (MomCPU<CPUZ380) WrError(1500);
             else if (AdrCnt==0) /* LD XY,(HL) */
              BEGIN
               BAsmCode[PrefixCnt]=0x33; CodeLen=PrefixCnt+1;
              END
             else if (BAsmCode[0]==BAsmCode[1]) WrError(1350);
             else /* LD XY,(XY+D) */
              BEGIN
               BAsmCode[0]=BAsmCode[1]; PrefixCnt--;
               BAsmCode[PrefixCnt]=0xcb;
               memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
               BAsmCode[PrefixCnt+1+AdrCnt]=0x23;
               CodeLen=PrefixCnt+1+AdrCnt+1;
              END
             break;
            case ModReg16:
             if (MomCPU<CPUZ380) WrError(1500);
             else if (AdrPart==3) WrError(1350);
             else if (PrefixCnt==1) /* LD XY,R16 */
              BEGIN
               if (AdrPart==2) AdrPart=3;
               CodeLen=1+PrefixCnt;
               BAsmCode[PrefixCnt]=0x07+(AdrPart << 4);
              END
             else if (BAsmCode[0]==BAsmCode[1]) WrError(1350);
             else /* LD XY,XY */
              BEGIN
               BAsmCode[--PrefixCnt]=0x27;
               CodeLen=1+PrefixCnt;
              END
             break;
            case ModIndReg16:
             if (MomCPU<CPUZ380) WrError(1500);
             else /* LD XY,(R16) */
              BEGIN
               BAsmCode[PrefixCnt]=0x03+(AdrPart << 4);
               CodeLen=PrefixCnt+1;
              END
             break;
            case ModImm: /* LD XY,imm16:32 */
             BAsmCode[PrefixCnt]=0x21;
             memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
             CodeLen=PrefixCnt+1+AdrCnt;
             break;
            case ModAbs: /* LD XY,(adr) */
             BAsmCode[PrefixCnt]=0x2a;
             memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
             CodeLen=PrefixCnt+1+AdrCnt;
             break;
            case ModSPRel: /* LD XY,(SP+D) */
             if (MomCPU<CPUZ380) WrError(1500);
             else
              BEGIN
               BAsmCode[PrefixCnt]=0xcb;
               memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
               BAsmCode[PrefixCnt+1+AdrCnt]=0x21;
               CodeLen=PrefixCnt+1+AdrCnt+1;
              END
             break;
            default: if (AdrMode!=ModNone) WrError(1350);
           END
         END
        break;
       case ModIndReg16:
        AdrByte=AdrPart;
        if (Memo("LDW"))
         BEGIN
          OpSize=1; MayLW=True;
         END
        else OpSize=0;
        DecodeAdr(ArgStr[2]);
        switch (AdrMode)
         BEGIN
          case ModReg8: /* LD (R16),A */
           if (AdrPart!=7) WrError(1350);
           else
            BEGIN
             CodeLen=1; BAsmCode[0]=0x02+(AdrByte << 4);
            END
           break;
          case ModReg16:
           if (AdrPart==3) WrError(1350);
           else if (MomCPU<CPUZ380) WrError(1500);
           else if (PrefixCnt==0) /* LD (R16),R16 */
            BEGIN
             if (AdrPart==2) AdrPart=3;
             BAsmCode[0]=0xfd; BAsmCode[1]=0x0c+AdrByte+(AdrPart << 4);
             CodeLen=2;
            END
           else /* LD (R16),XY */
            BEGIN
             BAsmCode[PrefixCnt]=0x01+(AdrByte << 4);
             CodeLen=PrefixCnt+1;
            END
           break;
          case ModImm:
           if (NOT Memo("LDW")) WrError(1350);
           else if (MomCPU<CPUZ380) WrError(1500);
           else
            BEGIN
             BAsmCode[PrefixCnt]=0xed;
             BAsmCode[PrefixCnt+1]=0x06+(AdrByte << 4);
             memcpy(BAsmCode+PrefixCnt+2,AdrVals,AdrCnt);
             CodeLen=PrefixCnt+2+AdrCnt;
            END
           break;
          default: if (AdrMode!=ModNone) WrError(1350);
         END
        break;
       case ModAbs:
        HLen=AdrCnt; memcpy(HVals,AdrVals,AdrCnt);
        OpSize=0; DecodeAdr(ArgStr[2]);
        switch (AdrMode)
         BEGIN
          case ModReg8: /* LD (adr),A */
           if (AdrPart!=7) WrError(1350);
           else
            BEGIN
             BAsmCode[PrefixCnt]=0x32;
             memcpy(BAsmCode+PrefixCnt+1,HVals,HLen);
             CodeLen=PrefixCnt+1+HLen;
            END
           break;
          case ModReg16:
           if (AdrPart==2) /* LD (adr),HL/XY */
            BEGIN
             BAsmCode[PrefixCnt]=0x22;
             memcpy(BAsmCode+PrefixCnt+1,HVals,HLen);
             CodeLen=PrefixCnt+1+HLen;
            END
           else /* LD (adr),R16 */
            BEGIN
             BAsmCode[PrefixCnt]=0xed;
             BAsmCode[PrefixCnt+1]=0x43+(AdrPart << 4);
             memcpy(BAsmCode+PrefixCnt+2,HVals,HLen);
             CodeLen=PrefixCnt+2+HLen;
            END
           break;
          default: if (AdrMode!=ModNone) WrError(1350);
         END
        break;
       case ModInt:
        if (strcasecmp(ArgStr[2],"A")==0) /* LD I,A */
         BEGIN
          CodeLen=2; BAsmCode[0]=0xed; BAsmCode[1]=0x47;
         END
        else if (strcasecmp(ArgStr[2],"HL")==0) /* LD I,HL */
         if (MomCPU<CPUZ380) WrError(1500);
         else
          BEGIN
           CodeLen=2; BAsmCode[0]=0xdd; BAsmCode[1]=0x47;
          END
         else WrError(1350);
        break;
       case ModRef:
        if (strcasecmp(ArgStr[2],"A")==0) /* LD R,A */
         BEGIN
          CodeLen=2; BAsmCode[0]=0xed; BAsmCode[1]=0x4f;
         END
        else WrError(1350);
        break;
       case ModSPRel:
        if (MomCPU<CPUZ380) WrError(1500);
        else
         BEGIN
          HLen=AdrCnt; memcpy(HVals,AdrVals,AdrCnt);
          OpSize=0; DecodeAdr(ArgStr[2]);
          switch (AdrMode)
           BEGIN
            case ModReg16:
             if (AdrPart==3) WrError(1350);
             else if (PrefixCnt==0) /* LD (SP+D),R16 */
              BEGIN
               if (AdrPart==2) AdrPart=3;
               BAsmCode[PrefixCnt]=0xdd;
               BAsmCode[PrefixCnt+1]=0xcb;
               memcpy(BAsmCode+PrefixCnt+2,HVals,HLen);
               BAsmCode[PrefixCnt+2+HLen]=0x09+(AdrPart << 4);
               CodeLen=PrefixCnt+2+HLen+1;
              END
             else /* LD (SP+D),XY */
              BEGIN
               BAsmCode[PrefixCnt]=0xcb;
               memcpy(BAsmCode+PrefixCnt+1,HVals,HLen);
               BAsmCode[PrefixCnt+1+HLen]=0x29;
               CodeLen=PrefixCnt+1+HLen+1;
              END
             break;
            default: if (AdrMode!=ModNone) WrError(1350);
           END
         END
        break;
       default: if (AdrMode!=ModNone) WrError(1350);
      END /* outer switch */
    END
END

        static Boolean ParPair(char *Name1, char *Name2)
BEGIN
   return (((strcasecmp(ArgStr[1],Name1)==0) AND (strcasecmp(ArgStr[2],Name2)==0)) OR
           ((strcasecmp(ArgStr[1],Name2)==0) AND (strcasecmp(ArgStr[2],Name1)==0)));
END

        static Boolean ImmIs8(void)
BEGIN
   Word tmp;

   if (AdrCnt<2) return True;

   tmp=(Word) AdrVals[AdrCnt-2];

   return ((tmp<=255) OR (tmp>=0xff80));
END

        static Boolean CodeAri(void)
BEGIN
   int z;
   Byte AdrByte;
   Boolean OK;

   for (z=0; z<ALUOrderCnt; z++)
    if (Memo(ALUOrders[z].Name))
     BEGIN
      if (ArgCnt==1)
       BEGIN
        strcpy(ArgStr[2],ArgStr[1]); strmaxcpy(ArgStr[1],"A",255); ArgCnt=2;
       END
      if (ArgCnt!=2) WrError(1110);
      else if (strcasecmp(ArgStr[1],"HL")==0)
       BEGIN
        if (NOT Memo("SUB")) WrError(1350);
        else
         BEGIN
          OpSize=1; DecodeAdr(ArgStr[2]);
          switch (AdrMode)
           BEGIN
            case ModAbs:
             BAsmCode[PrefixCnt]=0xed; BAsmCode[PrefixCnt+1]=0xd6;
             memcpy(BAsmCode+PrefixCnt+2,AdrVals,AdrCnt);
             CodeLen=PrefixCnt+2+AdrCnt;
             break;
            default: if (AdrMode!=ModNone) WrError(1350);
           END
         END
       END
      else if (strcasecmp(ArgStr[1],"SP")==0)
       BEGIN
        if (NOT Memo("SUB")) WrError(1350);
        else
         BEGIN
          OpSize=1; DecodeAdr(ArgStr[2]);
          switch (AdrMode)
           BEGIN
            case ModImm:
             BAsmCode[0]=0xed; BAsmCode[1]=0x92;
             memcpy(BAsmCode+2,AdrVals,AdrCnt);
             CodeLen=2+AdrCnt;
             break;
            default: if (AdrMode!=ModNone) WrError(1350);
           END
         END
       END
      else if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
      else
       BEGIN
        OpSize=0; DecodeAdr(ArgStr[2]);
        switch (AdrMode)
         BEGIN
          case ModReg8:
           CodeLen=PrefixCnt+1+AdrCnt;
           BAsmCode[PrefixCnt]=0x80+(ALUOrders[z].Code << 3)+AdrPart;
           memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
           break;
          case ModImm:
           if (NOT ImmIs8()) WrError(1320);
           else
            BEGIN
             CodeLen=2;
             BAsmCode[0]=0xc6+(ALUOrders[z].Code << 3);
             BAsmCode[1]=AdrVals[0];
            END
           break;
          default: if (AdrMode!=ModNone) WrError(1350);
         END
       END
      return True;
     END
    else if ((strncmp(ALUOrders[z].Name,OpPart,strlen(ALUOrders[z].Name))==0) AND (OpPart[strlen(OpPart)-1]=='W'))
     BEGIN
      if ((ArgCnt!=2) AND (ArgCnt!=1)) WrError(1110);
      else if (MomCPU<CPUZ380) WrError(1500);
      else if ((ArgCnt==2) AND (strcasecmp(ArgStr[1],"HL")!=0)) WrError(1350);
      else
       BEGIN
        OpSize=1; DecodeAdr(ArgStr[ArgCnt]);
        switch (AdrMode)
         BEGIN
          case ModReg16:
           if (PrefixCnt>0)      /* wenn Register, dann nie DDIR! */
            BEGIN
             BAsmCode[PrefixCnt]=0x87+(ALUOrders[z].Code << 3);
             CodeLen=1+PrefixCnt;
            END
           else if (AdrPart==3) WrError(1350);
           else
            BEGIN
             if (AdrPart==2) AdrPart=3;
             BAsmCode[0]=0xed; BAsmCode[1]=0x84+(ALUOrders[z].Code << 3)+AdrPart;
             CodeLen=2;
            END
           break;
          case ModReg8:
           if ((AdrPart!=6) OR (AdrCnt==0)) WrError(1350);
           else
            BEGIN
             BAsmCode[PrefixCnt]=0xc6+(ALUOrders[z].Code << 3);
             memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
             CodeLen=PrefixCnt+1+AdrCnt;
            END
           break;
          case ModImm:
           BAsmCode[0]=0xed; BAsmCode[1]=0x86+(ALUOrders[z].Code << 3);
           memcpy(BAsmCode+2,AdrVals,AdrCnt); CodeLen=2+AdrCnt;
           break;
          default: if (AdrMode!=ModNone) WrError(1350);
         END
       END
      return True;
     END

   if (Memo("ADD"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1]);
       switch (AdrMode)
        BEGIN
         case ModReg8:
          if (AdrPart!=7) WrError(1350);
          else
           BEGIN
            OpSize=0; DecodeAdr(ArgStr[2]);
            switch (AdrMode)
             BEGIN
              case ModReg8:
               CodeLen=PrefixCnt+1+AdrCnt;
               BAsmCode[PrefixCnt]=0x80+AdrPart;
               memcpy(BAsmCode+1+PrefixCnt,AdrVals,AdrCnt);
               break;
              case ModImm:
               CodeLen=PrefixCnt+1+AdrCnt;
               BAsmCode[PrefixCnt]=0xc6;
               memcpy(BAsmCode+1+PrefixCnt,AdrVals,AdrCnt);
               break;
              default: if (AdrMode!=ModNone) WrError(1350);
             END
           END
          break;
         case ModReg16:
          if (AdrPart==3) /* SP */
           BEGIN
            OpSize=1; DecodeAdr(ArgStr[2]);
            switch (AdrMode)
             BEGIN
              case ModImm:
               if (MomCPU<CPUZ380) WrError(1500);
               else
                BEGIN
                 BAsmCode[0]=0xed; BAsmCode[1]=0x82;
                 memcpy(BAsmCode+2,AdrVals,AdrCnt);
                 CodeLen=2+AdrCnt;
                END
               break;
              default: if (AdrMode!=ModNone) WrError(1350);
             END
           END
          else if (AdrPart!=2) WrError(1350);
          else
           BEGIN
            z=PrefixCnt; /* merkt, ob Indexregister */
            OpSize=1; DecodeAdr(ArgStr[2]);
            switch (AdrMode)
             BEGIN
              case ModReg16:
               if ((AdrPart==2) AND (PrefixCnt!=0) AND ((PrefixCnt!=2) OR (BAsmCode[0]!=BAsmCode[1]))) WrError(1350);
               else
                BEGIN
                 if (PrefixCnt==2) PrefixCnt--; CodeLen=1+PrefixCnt;
                 BAsmCode[PrefixCnt]=0x09+(AdrPart << 4);
                END
               break;
              case ModAbs:
               if (z!=0) WrError(1350);
               else if (MomCPU<CPUZ380) WrError(1500);
               else
                BEGIN
                 BAsmCode[PrefixCnt]=0xed; BAsmCode[PrefixCnt+1]=0xc2;
                 memcpy(BAsmCode+PrefixCnt+2,AdrVals,AdrCnt);
                 CodeLen=PrefixCnt+2+AdrCnt;
                END
               break;
              default: if (AdrMode!=ModNone) WrError(1350);
             END
           END
          break;
         default: if (AdrMode!=ModNone) WrError(1350);
        END
      END
     return True;
    END

   if (Memo("ADDW"))
    BEGIN
     if ((ArgCnt!=2) AND (ArgCnt!=1)) WrError(1110);
     else if (MomCPU<CPUZ380) WrError(1500);
     else if ((ArgCnt==2) AND (strcasecmp(ArgStr[1],"HL")!=0)) WrError(1350);
     else
      BEGIN
       OpSize=1; DecodeAdr(ArgStr[ArgCnt]);
       switch (AdrMode)
        BEGIN
         case ModReg16:
          if (PrefixCnt>0)      /* wenn Register, dann nie DDIR! */
           BEGIN
            BAsmCode[PrefixCnt]=0x87;
            CodeLen=1+PrefixCnt;
           END
          else if (AdrPart==3) WrError(1350);
          else
           BEGIN
            if (AdrPart==2) AdrPart=3;
            BAsmCode[0]=0xed; BAsmCode[1]=0x84+AdrPart;
            CodeLen=2;
           END
          break;
         case ModReg8:
          if ((AdrPart!=6) OR (AdrCnt==0)) WrError(1350);
          else
           BEGIN
            BAsmCode[PrefixCnt]=0xc6;
            memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
            CodeLen=PrefixCnt+1+AdrCnt;
           END
          break;
         case ModImm:
          BAsmCode[0]=0xed; BAsmCode[1]=0x86;
          memcpy(BAsmCode+2,AdrVals,AdrCnt); CodeLen=2+AdrCnt;
          break;
         default: if (AdrMode!=ModNone) WrError(1350);
        END
      END
     return True;
    END

   if ((Memo("ADC")) OR (Memo("SBC")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1]);
       switch (AdrMode)
        BEGIN
         case ModReg8:
          if (AdrPart!=7) WrError(1350);
          else
           BEGIN
            OpSize=0; DecodeAdr(ArgStr[2]);
            switch (AdrMode)
             BEGIN
              case ModReg8:
               CodeLen=PrefixCnt+1+AdrCnt;
               BAsmCode[PrefixCnt]=0x88+AdrPart;
               memcpy(BAsmCode+1+PrefixCnt,AdrVals,AdrCnt);
               break;
              case ModImm:
               CodeLen=PrefixCnt+1+AdrCnt;
               BAsmCode[PrefixCnt]=0xce;
               memcpy(BAsmCode+1+PrefixCnt,AdrVals,AdrCnt);
               break;
              default: if (AdrMode!=ModNone) WrError(1350);
             END
            if ((Memo("SBC")) AND (CodeLen!=0)) BAsmCode[PrefixCnt]+=0x10;
           END
          break; 
         case ModReg16:
          if ((AdrPart!=2) OR (PrefixCnt!=0)) WrError(1350);
          else
           BEGIN
            OpSize=1; DecodeAdr(ArgStr[2]);
            switch (AdrMode)
             BEGIN
              case ModReg16:
               if (PrefixCnt!=0) WrError(1350);
               else
                BEGIN
                 CodeLen=2; BAsmCode[0]=0xed;
                 BAsmCode[1]=0x42+(AdrPart << 4);
                 if (Memo("ADC")) BAsmCode[1]+=8;
                END
               break;
              default: if (AdrMode!=ModNone) WrError(1350);
             END
           END
          break;
         default: if (AdrMode!=ModNone) WrError(1350);
        END
      END
     return True;
    END

   if ((Memo("ADCW")) OR (Memo("SBCW")))
    BEGIN
     if ((ArgCnt!=2) AND (ArgCnt!=1)) WrError(1110);
     else if (MomCPU<CPUZ380) WrError(1500);
     else if ((ArgCnt==2) AND (strcasecmp(ArgStr[1],"HL")!=0)) WrError(1350);
     else
      BEGIN
       z=Ord(Memo("SBCW")) << 4;
       OpSize=1; DecodeAdr(ArgStr[ArgCnt]);
       switch (AdrMode)
        BEGIN
         case ModReg16:
          if (PrefixCnt>0)      /* wenn Register, dann nie DDIR! */
           BEGIN
            BAsmCode[PrefixCnt]=0x8f+z;
            CodeLen=1+PrefixCnt;
           END
          else if (AdrPart==3) WrError(1350);
          else
           BEGIN
            if (AdrPart==2) AdrPart=3;
            BAsmCode[0]=0xed; BAsmCode[1]=0x8c+z+AdrPart;
            CodeLen=2;
           END
          break;
         case ModReg8:
          if ((AdrPart!=6) OR (AdrCnt==0)) WrError(1350);
          else
           BEGIN
            BAsmCode[PrefixCnt]=0xce + z; /* ANSI :-0 */
            memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
            CodeLen=PrefixCnt+1+AdrCnt;
           END
          break;
         case ModImm:
          BAsmCode[0]=0xed; BAsmCode[1]=0x8e + z;
          memcpy(BAsmCode+2,AdrVals,AdrCnt); CodeLen=2+AdrCnt;
          break;
         default: if (AdrMode!=ModNone) WrError(1350);
        END
      END
     return True;
    END

   if ((Memo("INC")) OR (Memo("DEC")) OR (Memo("INCW")) OR (Memo("DECW")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       z=Ord((Memo("DEC")) OR (Memo("DECW")));
       DecodeAdr(ArgStr[1]);
       switch (AdrMode)
        BEGIN
         case ModReg8:
          if (OpPart[strlen(OpPart)-1]=='W') WrError(1350);
          else
           BEGIN
            CodeLen=PrefixCnt+1+AdrCnt;
            BAsmCode[PrefixCnt]=0x04+(AdrPart << 3)+z;
            memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
           END
          break;
         case ModReg16:
          CodeLen=1+PrefixCnt;
          BAsmCode[PrefixCnt]=0x03+(AdrPart << 4)+(z << 3);
          break;
         default: if (AdrMode!=ModNone) WrError(1350);
        END
      END
     return True;
    END

   for (z=0; z<ShiftOrderCnt; z++)
    if (Memo(ShiftOrders[z]))
     BEGIN
      if ((ArgCnt==0) OR (ArgCnt>2)) WrError(1110);
      else if ((z==6) AND (MomCPU!=CPUZ80U)) WrError(1500); /* SLIA undok. Z80 */
      else
       BEGIN
        OpSize=0;
        DecodeAdr(ArgStr[ArgCnt]);
        switch (AdrMode)
         BEGIN
          case ModReg8:
           if ((PrefixCnt>0) AND (AdrPart!=6)) WrError(1350); /* IXL..IYU verbieten */
           else
            BEGIN
             if (ArgCnt==1) OK=True;
             else if (MomCPU!=CPUZ80U)
              BEGIN
               WrError(1500); OK=False;
              END
             else if ((AdrPart!=6) OR (PrefixCnt!=1) OR (NOT DecodeReg8(ArgStr[1],&AdrPart)))
              BEGIN
               WrError(1350); OK=False;
              END
             else OK=True;
             if (OK)
              BEGIN
               CodeLen=PrefixCnt+1+AdrCnt+1;
               BAsmCode[PrefixCnt]=0xcb;
               memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
               BAsmCode[PrefixCnt+1+AdrCnt]=AdrPart+(z << 3);
               if ((AdrPart==7) AND (z<4)) WrError(10);
              END
            END
           break;
          default: if (AdrMode!=ModNone) WrError(1350);
         END
       END
      return True;
     END
    else if ((strncmp(OpPart,ShiftOrders[z],strlen(ShiftOrders[z]))==0) AND (OpPart[strlen(OpPart)-1]=='W'))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if ((MomCPU<CPUZ380) OR (z==6)) WrError(1500);
      else
       BEGIN
        OpSize=1; DecodeAdr(ArgStr[1]);
        switch (AdrMode)
         BEGIN
          case ModReg16:
           if (PrefixCnt>0)
            BEGIN
             BAsmCode[2]=0x04+(z << 3)+((BAsmCode[0] >> 5) & 1);
             BAsmCode[0]=0xed; BAsmCode[1]=0xcb;
             CodeLen=3;
            END
           else if (AdrPart==3) WrError(1350);
           else
            BEGIN
             if (AdrPart==2) AdrPart=3;
             BAsmCode[0]=0xed; BAsmCode[1]=0xcb;
             BAsmCode[2]=(z << 3)+AdrPart;
             CodeLen=3;
            END
           break;
          case ModReg8:
           if (AdrPart!=6) WrError(1350);
           else
            BEGIN
             if (AdrCnt==0)
              BEGIN
               BAsmCode[0]=0xed; PrefixCnt=1;
              END
             BAsmCode[PrefixCnt]=0xcb;
             memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
             BAsmCode[PrefixCnt+1+AdrCnt]=0x02+(z << 3);
             CodeLen=PrefixCnt+1+AdrCnt+1;
            END
           break;
          default: if (AdrMode!=ModNone) WrError(1350);
         END
       END
      return True;
     END

   for (z=0; z<BitOrderCnt; z++)
    if (Memo(BitOrders[z]))
     BEGIN
      if ((ArgCnt!=2) AND (ArgCnt!=3)) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[ArgCnt]);
        switch (AdrMode)
         BEGIN
          case ModReg8:
           if ((AdrPart!=6) AND (PrefixCnt!=0)) WrError(1350);
           else
            BEGIN
             AdrByte=EvalIntExpression(ArgStr[ArgCnt-1],UInt3,&OK);
             if (OK)
              BEGIN
               if (ArgCnt==2) OK=True;
               else if (MomCPU!=CPUZ80U)
                BEGIN
                 WrError(1500); OK=False;
                END
               else if ((AdrPart!=6) OR (PrefixCnt!=1) OR (z==0) OR (NOT DecodeReg8(ArgStr[1],&AdrPart)))
                BEGIN
                 WrError(1350); OK=False;
                END
               else OK=True;
               if (OK)
                BEGIN
                 CodeLen=PrefixCnt+2+AdrCnt;
                 BAsmCode[PrefixCnt]=0xcb;
                 memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
                 BAsmCode[PrefixCnt+1+AdrCnt]=AdrPart+(AdrByte << 3)+((z+1) << 6);
                END
              END
            END
           break;
          default: if (AdrMode!=ModNone) WrError(1350); 
         END
       END
      return True;
     END

   if (Memo("MLT"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPUZ180) WrError(1500);
     else
      BEGIN
       DecodeAdr(ArgStr[1]);
       if ((AdrMode!=ModReg16) OR (PrefixCnt!=0)) WrError(1350);
       else
        BEGIN
         BAsmCode[CodeLen]=0xed; BAsmCode[CodeLen+1]=0x4c+(AdrPart << 4);
         CodeLen=2;
        END
      END
     return True;
    END

   if ((Memo("DIVUW")) OR (Memo("MULTW")) OR (Memo("MULTUW")))
    BEGIN
     if (ArgCnt==1)
      BEGIN
       strcpy(ArgStr[2],ArgStr[1]); strmaxcpy(ArgStr[1],"HL",255); ArgCnt=2;
      END
     if (MomCPU<CPUZ380) WrError(1500);
     else if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"HL")!=0) WrError(1350);
     else
      BEGIN
       AdrByte=Ord(*OpPart=='D');
       z=Ord(OpPart[strlen(OpPart)-2]=='U');
       OpSize=1; DecodeAdr(ArgStr[ArgCnt]);
       switch (AdrMode)
        BEGIN
         case ModReg8:
          if ((AdrPart!=6) OR (PrefixCnt==0)) WrError(1350);
          else
           BEGIN
            BAsmCode[PrefixCnt]=0xcb;
            memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
            BAsmCode[PrefixCnt+1+AdrCnt]=0x92+(z << 3)+(AdrByte << 5);
            CodeLen=PrefixCnt+1+AdrCnt+1;
           END
          break;
         case ModReg16:
          if (AdrPart==3) WrError(1350);
          else if (PrefixCnt==0)
           BEGIN
            if (AdrPart==2) AdrPart=3;
            BAsmCode[0]=0xed; BAsmCode[1]=0xcb;
            BAsmCode[2]=0x90+AdrPart+(z << 3)+(AdrByte << 5);
            CodeLen=3;
           END
          else
           BEGIN
            BAsmCode[2]=0x94+((BAsmCode[0] >> 5) & 1)+(z << 3)+(AdrByte << 5);
            BAsmCode[0]=0xed; BAsmCode[1]=0xcb;
            CodeLen=3;
           END
          break;
         case ModImm:
          BAsmCode[0]=0xed; BAsmCode[1]=0xcb;
          BAsmCode[2]=0x97+(z << 3)+(AdrByte << 5);
          memcpy(BAsmCode+3,AdrVals,AdrCnt);
          CodeLen=3+AdrCnt;
          break;
         default: if (AdrMode!=ModNone) WrError(1350);
        END
      END
     return True;
    END

   if (Memo("TST"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPUZ180) WrError(1500);
     else
      BEGIN
       OpSize=0; DecodeAdr(ArgStr[1]);
       switch (AdrMode)
        BEGIN
         case ModReg8:
          if (PrefixCnt!=0) WrError(1350);
          else
           BEGIN
            BAsmCode[0]=0xed; BAsmCode[1]=4+(AdrPart << 3);
            CodeLen=2;
           END
          break;
         case ModImm:
          BAsmCode[0]=0xed; BAsmCode[1]=0x64; BAsmCode[2]=AdrVals[0];
          CodeLen=3;
          break;
         default: if (AdrMode!=ModNone) WrError(1350);
        END
      END
     return True;
    END

   if (Memo("SWAP"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPUZ380) WrError(1500);
     else
      BEGIN
       DecodeAdr(ArgStr[1]);
       switch (AdrMode)
        BEGIN
         case ModReg16:
          if (AdrPart==3) WrError(1350);
          else if (PrefixCnt==0)
           BEGIN
            if (AdrPart==2) AdrPart=3;
            BAsmCode[0]=0xed; BAsmCode[1]=0x0e + (AdrPart << 4); /*?*/
            CodeLen=2;
           END
          else
           BEGIN
            BAsmCode[PrefixCnt]=0x3e; CodeLen=PrefixCnt+1;
           END
          break;
         default: if (AdrMode!=ModNone) WrError(1350);
        END
      END
     return True;
    END

   return False;
END

        static void MakeCode_Z80(void)
BEGIN
   Boolean OK;
   LongWord AdrLong;
   LongInt AdrLInt;
   Byte AdrByte;
   int z;

   CodeLen=0; DontPrint=False; PrefixCnt=0; OpSize=0xff; MayLW=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   /* letzten Praefix umkopieren */

   LastPrefix=CurrPrefix; CurrPrefix=Pref_IN_N;

   /* evtl. Datenablage */

   if (DecodeIntelPseudo(False)) return;

/*--------------------------------------------------------------------------*/
/* Instruktionspraefix */

   if (Memo("DDIR"))
    BEGIN
     if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
     else if (MomCPU<CPUZ380) WrError(1500);
     else
      BEGIN
       OK=True;
       for (z=1; z<=ArgCnt; z++)
        if (OK)
         BEGIN
          NLS_UpString(ArgStr[z]);
          OK=ExtendPrefix(&CurrPrefix,ArgStr[z]);
          if (NOT OK) WrError(1135);
         END
       if (OK)
        BEGIN
         GetPrefixCode(CurrPrefix,BAsmCode+0,BAsmCode+1);
         CodeLen=2;
        END
      END
     return;
    END

/*--------------------------------------------------------------------------*/
/* mit Sicherheit am haeufigsten... */

   if ((Memo("LD")) OR (Memo("LDW")))
    BEGIN
     DecodeLD();
     return;
    END

/*--------------------------------------------------------------------------*/
/* ohne Operanden */

   for (z=0; z<FixedOrderCnt; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else if (MomCPU<FixedOrders[z].MinCPU) WrError(1500);
      else
       BEGIN
        if ((CodeLen=FixedOrders[z].Len)==2)
         BEGIN
          BAsmCode[0]=Hi(FixedOrders[z].Code);
          BAsmCode[1]=Lo(FixedOrders[z].Code);
         END
        else BAsmCode[0]=Lo(FixedOrders[z].Code);
       END;
      return;
     END

   /* nur Akku zugelassen */

   for (z=0; z<AccOrderCnt; z++)
    if (Memo(AccOrders[z].Name))
     BEGIN
      if (ArgCnt==0)
       BEGIN
        ArgCnt=1; strmaxcpy(ArgStr[1],"A",255);
       END
      if (ArgCnt!=1) WrError(1110);
      else if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
      else if (MomCPU<AccOrders[z].MinCPU) WrError(1500);
      else
       BEGIN
        if ((CodeLen=AccOrders[z].Len)==2)
         BEGIN
          BAsmCode[0]=Hi(AccOrders[z].Code);
          BAsmCode[1]=Lo(AccOrders[z].Code);
         END
        else BAsmCode[0]=Lo(AccOrders[z].Code);
       END
      return;
     END

   for (z=0; z<HLOrderCnt; z++)
    if (Memo(HLOrders[z].Name))
     BEGIN
      if (ArgCnt==0)
       BEGIN
        ArgCnt=1; strmaxcpy(ArgStr[1],"HL",255);
       END;
      if (ArgCnt!=1) WrError(1110);
      else if (strcasecmp(ArgStr[1],"HL")!=0) WrError(1350);
      else if (MomCPU<HLOrders[z].MinCPU) WrError(1500);
      else
       BEGIN
        if ((CodeLen=HLOrders[z].Len)==2)
         BEGIN
          BAsmCode[0]=Hi(HLOrders[z].Code);
          BAsmCode[1]=Lo(HLOrders[z].Code);
         END
        else BAsmCode[0]=Lo(HLOrders[z].Code);
       END
      return;
     END

/*-------------------------------------------------------------------------*/
/* Datentransfer */

   if ((Memo("PUSH")) OR (Memo("POP")))
    BEGIN
     z=Ord(Memo("PUSH")) << 2;
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"SR")==0)
      if (MomCPU<CPUZ380) WrError(1500);
      else
       BEGIN
        CodeLen=2; BAsmCode[0]=0xed; BAsmCode[1]=0xc1+z;
       END
     else
      BEGIN
       if (strcasecmp(ArgStr[1],"SP")==0) strmaxcpy(ArgStr[1],"A",255);
       if (strcasecmp(ArgStr[1],"AF")==0) strmaxcpy(ArgStr[1],"SP",255);
       OpSize=1; MayLW=True;
       DecodeAdr(ArgStr[1]);
       switch (AdrMode)
        BEGIN
         case ModReg16:
          CodeLen=1+PrefixCnt;
          BAsmCode[PrefixCnt]=0xc1+(AdrPart << 4)+z;
          break;
         case ModImm:
          if (MomCPU<CPUZ380) WrError(1500);
          else
           BEGIN
            BAsmCode[PrefixCnt]=0xfd; BAsmCode[PrefixCnt+1]=0xf5;
            memcpy(BAsmCode+PrefixCnt+2,AdrVals,AdrCnt);
            CodeLen=PrefixCnt+2+AdrCnt;
           END
          break;
         default: if (AdrMode!=ModNone) WrError(1350);
        END
      END
     return;
    END

   if (Memo("EX"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (ParPair("DE","HL"))
      BEGIN
       CodeLen=1; BAsmCode[0]=0xeb;
      END
     else if (ParPair("AF","AF\'"))
      BEGIN
       CodeLen=1; BAsmCode[0]=0x08;
      END
     else if (ParPair("AF","AF`"))
      BEGIN
       CodeLen=1; BAsmCode[0]=0x08;
      END
     else if (ParPair("(SP)","HL"))
      BEGIN
       CodeLen=1; BAsmCode[0]=0xe3;
      END
     else if (ParPair("(SP)","IX"))
      BEGIN
       CodeLen=2; BAsmCode[0]=IXPrefix; BAsmCode[1]=0xe3;
      END
     else if (ParPair("(SP)","IY"))
      BEGIN
       CodeLen=2; BAsmCode[0]=IYPrefix; BAsmCode[1]=0xe3;
      END
     else if (ParPair("(HL)","A"))
      BEGIN
       if (MomCPU<CPUZ380) WrError(1500);
       else
        BEGIN
         CodeLen=2; BAsmCode[0]=0xed; BAsmCode[1]=0x37;
        END
      END
     else
      BEGIN
       if (ArgStr[2][strlen(ArgStr[2])-1]=='\'')
        BEGIN
         OK=True; ArgStr[2][strlen(ArgStr[2])-1]='\0';
        END
       else OK=False;
       DecodeAdr(ArgStr[1]);
       switch (AdrMode)
        BEGIN
         case ModReg8:
          if ((AdrPart==6) OR (PrefixCnt!=0)) WrError(1350);
          else
           BEGIN
            AdrByte=AdrPart;
             DecodeAdr(ArgStr[2]);
             switch (AdrMode)
              BEGIN
               case ModReg8:
                if ((AdrPart==6) OR (PrefixCnt!=0)) WrError(1350);
                else if (MomCPU<CPUZ380) WrError(1500);
                else if ((AdrByte==7) AND (NOT OK))
                 BEGIN
                  BAsmCode[0]=0xed; BAsmCode[1]=0x07+(AdrPart << 3);
                  CodeLen=2;
                 END
                else if ((AdrPart==7) AND (NOT OK))
                 BEGIN
                  BAsmCode[0]=0xed; BAsmCode[1]=0x07+(AdrByte << 3);
                  CodeLen=2;
                 END
                else if ((OK) AND (AdrPart==AdrByte))
                 BEGIN
                  BAsmCode[0]=0xcb; BAsmCode[1]=0x30+AdrPart;
                  CodeLen=2;
                 END
                else WrError(1350);
                break;
               default: if (AdrMode!=ModNone) WrError(1350);
              END
           END
          break;
         case ModReg16:
          if (AdrPart==3) WrError(1350);
          else if (PrefixCnt==0) /* EX R16,... */
           BEGIN
            if (AdrPart==2) AdrByte=3; else AdrByte=AdrPart;
            DecodeAdr(ArgStr[2]);
            switch (AdrMode)
             BEGIN
              case ModReg16:
               if (AdrPart==3) WrError(1350);
               else if (MomCPU<CPUZ380) WrError(1500);
               else if (OK)
                BEGIN
                 if (AdrPart==2) AdrPart=3;
                 if ((PrefixCnt!=0) OR (AdrPart!=AdrByte)) WrError(1350);
                 else
                  BEGIN
                   CodeLen=3; BAsmCode[0]=0xed; BAsmCode[1]=0xcb;
                   BAsmCode[2]=0x30+AdrByte;
                  END
                END
               else if (PrefixCnt==0)
                BEGIN
                 if (AdrByte==0)
                  BEGIN
                   if (AdrPart==2) AdrPart=3;
                   BAsmCode[0]=0xed; BAsmCode[1]=0x01+(AdrPart << 2);
                   CodeLen=2;
                  END
                 else if (AdrPart==0)
                  BEGIN
                   BAsmCode[0]=0xed; BAsmCode[1]=0x01+(AdrByte << 2);
                   CodeLen=2;
                  END
                END
               else
                BEGIN
                 if (AdrPart==2) AdrPart=3;
                 BAsmCode[1]=0x03+((BAsmCode[0] >> 2) & 8)+(AdrByte << 4);
                 BAsmCode[0]=0xed;
                 CodeLen=2;
                END
               break;
              default: if (AdrMode!=ModNone) WrError(1350);
             END
           END
          else /* EX XY,... */
           BEGIN
            DecodeAdr(ArgStr[2]);
            switch (AdrMode)
             BEGIN
              case ModReg16:
               if (AdrPart==3) WrError(1350);
               else if (MomCPU<CPUZ380) WrError(1500);
               else if (OK)
                if ((PrefixCnt!=2) OR (BAsmCode[0]!=BAsmCode[1])) WrError(1350);
                else
                 BEGIN
                  CodeLen=3; BAsmCode[2]=((BAsmCode[0] >> 5) & 1)+0x34;
                  BAsmCode[0]=0xed; BAsmCode[1]=0xcb;
                 END
               else if (PrefixCnt==1)
                BEGIN
                 if (AdrPart==2) AdrPart=3; 
                 BAsmCode[1]=((BAsmCode[0] >> 2) & 8)+3+(AdrPart << 4);
                 BAsmCode[0]=0xed;
                 CodeLen=2;
                END
               else if (BAsmCode[0]==BAsmCode[1]) WrError(1350);
               else
                BEGIN
                 BAsmCode[0]=0xed; BAsmCode[1]=0x2b; CodeLen=2;
                END
               break;
              default: if (AdrMode!=ModNone) WrError(1350);
             END
           END
          break;
         default: if (AdrMode!=ModNone) WrError(1350);
        END
      END
     return;
    END

/*-------------------------------------------------------------------------*/
/* Arithmetik */

   if (CodeAri()) return;

/*-------------------------------------------------------------------------*/
/* Ein/Ausgabe */

   if (Memo("TSTI"))
    BEGIN
     if (MomCPU!=CPUZ80U) WrError(1500);
     else if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       BAsmCode[0]=0xed; BAsmCode[1]=0x70; CodeLen=2;
      END
     return;
    END

   if ((Memo("IN")) OR (Memo("OUT")))
    BEGIN
     if ((ArgCnt==1) AND (Memo("IN")))
      BEGIN
       if (MomCPU!=CPUZ80U) WrError(1500);
       else if (strcasecmp(ArgStr[1],"(C)")!=0) WrError(1350);
       else
        BEGIN
         BAsmCode[0]=0xed; BAsmCode[1]=0x70; CodeLen=2;
        END
      END
     else if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       if (Memo("OUT"))
        BEGIN
         strcpy(ArgStr[3],ArgStr[1]); strcpy(ArgStr[1],ArgStr[2]); strcpy(ArgStr[2],ArgStr[3]);
        END
       if (strcasecmp(ArgStr[2],"(C)")==0)
        BEGIN
         OpSize=0; DecodeAdr(ArgStr[1]);
         switch (AdrMode)
          BEGIN
           case ModReg8:
            if ((AdrPart==6) OR (PrefixCnt!=0)) WrError(1350);
            else
             BEGIN
              CodeLen=2;
              BAsmCode[0]=0xed; BAsmCode[1]=0x40+(AdrPart << 3);
              if (Memo("OUT")) BAsmCode[1]++;
             END
            break;
           case ModImm:
            if (Memo("IN")) WrError(1350);
            else if ((MomCPU==CPUZ80U) AND (AdrVals[0]==0))
             BEGIN
              BAsmCode[0]=0xed; BAsmCode[1]=0x71; CodeLen=2;
             END
            else if (MomCPU<CPUZ380) WrError(1500);
            else
             BEGIN
              BAsmCode[0]=0xed; BAsmCode[1]=0x71; BAsmCode[2]=AdrVals[0];
              CodeLen=3;
             END
            break;
           default: if (AdrMode!=ModNone) WrError(1350);
          END
        END
       else if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
       else
        BEGIN
         BAsmCode[1]=EvalIntExpression(ArgStr[2],UInt8,&OK);
         if (OK)
          BEGIN
           ChkSpace(SegIO);
           CodeLen=2;
           BAsmCode[0]=(Memo("OUT")) ? 0xd3 : 0xdb;
          END
        END
      END
     return;
    END

   if ((Memo("INW")) OR (Memo("OUTW")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPUZ380) WrError(1500);
     else
      BEGIN
       if (Memo("OUTW"))
        BEGIN
         strcpy(ArgStr[3],ArgStr[1]); strcpy(ArgStr[1],ArgStr[2]); strcpy(ArgStr[2],ArgStr[3]);
        END;
       if (strcasecmp(ArgStr[2],"(C)")!=0) WrError(1350);
       else
        BEGIN
         OpSize=1; DecodeAdr(ArgStr[1]);
         switch (AdrMode)
          BEGIN
           case ModReg16:
            if ((AdrPart==3) OR (PrefixCnt>0)) WrError(1350);
            else
             BEGIN
              switch (AdrPart)
               BEGIN
                case 1: AdrPart=2; break;
                case 2: AdrPart=7; break;
               END
              BAsmCode[0]=0xdd; BAsmCode[1]=0x40+(AdrPart << 3);
              if (Memo("OUTW")) BAsmCode[1]++;
              CodeLen=2;
             END
            break;
           case ModImm:
            if (Memo("INW")) WrError(1350);
            else
             BEGIN
              BAsmCode[0]=0xfd; BAsmCode[1]=0x79;
              memcpy(BAsmCode+2,AdrVals,AdrCnt);
              CodeLen=2+AdrCnt;
             END
            break;
           default: if (AdrMode!=ModNone) WrError(1350);
          END
        END
      END
     return;
    END

   if ((Memo("IN0")) OR (Memo("OUT0")))
    BEGIN
     if ((ArgCnt!=2) AND (ArgCnt!=1)) WrError(1110);
     else if ((ArgCnt==1) AND (Memo("OUT0"))) WrError(1110);
     else if (MomCPU<CPUZ180) WrError(1500);
     else
      BEGIN
       if (Memo("OUT0"))
        BEGIN
         strcpy(ArgStr[3],ArgStr[1]); strcpy(ArgStr[1],ArgStr[2]); strcpy(ArgStr[2],ArgStr[3]);
        END
       OpSize=0;
       if (ArgCnt==1)
        BEGIN
         AdrPart=6; OK=True;
        END
       else
        BEGIN
         DecodeAdr(ArgStr[1]);
         if ((AdrMode==ModReg8) AND (AdrPart!=6) AND (PrefixCnt==0)) OK=True;
         else
          BEGIN
           OK=False; if (AdrMode!=ModNone) WrError(1350);
          END
        END
       if (OK)
        BEGIN
         BAsmCode[2]=EvalIntExpression(ArgStr[ArgCnt],UInt8,&OK);
         if (OK)
          BEGIN
           BAsmCode[0]=0xed; BAsmCode[1]=AdrPart << 3;
           if (Memo("OUT0")) BAsmCode[1]++;
           CodeLen=3;
          END
        END
      END
     return;
    END

   if ((Memo("INA")) OR (Memo("INAW")) OR (Memo("OUTA")) OR (Memo("OUTAW")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPUZ380) WrError(1500);
     else
      BEGIN
       if (*OpPart=='O')
        BEGIN
         strcpy(ArgStr[3],ArgStr[1]); strcpy(ArgStr[1],ArgStr[2]); strcpy(ArgStr[2],ArgStr[3]);
        END
       OpSize=Ord(OpPart[strlen(OpPart)-1]=='W');
       if (((OpSize==0) AND (strcasecmp(ArgStr[1],"A")!=0))
       OR  ((OpSize==1) AND (strcasecmp(ArgStr[1],"HL")!=0))) WrError(1350);
       else
        BEGIN
         AdrLong=EvalIntExpression(ArgStr[2],ExtFlag?Int32:UInt8,&OK);
         if (OK)
          BEGIN
           ChkSpace(SegIO);
           if (AdrLong>0xffffff) ChangeDDPrefix("IW");
           else if (AdrLong>0xffff) ChangeDDPrefix("IB");
           BAsmCode[PrefixCnt]=0xed+(OpSize << 4);
           BAsmCode[PrefixCnt+1]=0xd3+(Ord(*OpPart=='I') << 3);
           BAsmCode[PrefixCnt+2]=AdrLong & 0xff;
           BAsmCode[PrefixCnt+3]=(AdrLong >> 8) & 0xff;
           CodeLen=PrefixCnt+4;
           if (AdrLong>0xffff)
            BAsmCode[CodeLen++]=(AdrLong >> 16) & 0xff;
           if (AdrLong>0xffffff)
            BAsmCode[CodeLen++]=(AdrLong >> 24) & 0xff;
          END
        END
      END
     return;
    END

   if (Memo("TSTIO"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPUZ180) WrError(1500);
     else
      BEGIN
       BAsmCode[2]=EvalIntExpression(ArgStr[1],Int8,&OK);
       if (OK)
        BEGIN
         BAsmCode[0]=0xed; BAsmCode[1]=0x74;
         CodeLen=3;
        END
      END
     return;
    END

/*-------------------------------------------------------------------------*/
/* Spruenge */

   if (Memo("RET"))
    BEGIN
     if (ArgCnt==0)
      BEGIN
       CodeLen=1; BAsmCode[0]=0xc9;
      END
     else if (ArgCnt!=1) WrError(1110);
     else if (NOT DecodeCondition(ArgStr[1],&z)) WrError(1360);
     else
      BEGIN
       CodeLen=1; BAsmCode[0]=0xc0+(z << 3);
      END
     return;
    END

   if (Memo("JP"))
    BEGIN
     if (ArgCnt==1)
      BEGIN
       if (strcasecmp(ArgStr[1],"(HL)")==0)
        BEGIN
         CodeLen=1; BAsmCode[0]=0xe9; OK=False;
        END
       else if (strcasecmp(ArgStr[1],"(IX)")==0)
        BEGIN
         CodeLen=2; BAsmCode[0]=IXPrefix; BAsmCode[1]=0xe9; OK=False;
        END
       else if (strcasecmp(ArgStr[1],"(IY)")==0)
        BEGIN
         CodeLen=2; BAsmCode[0]=IYPrefix; BAsmCode[1]=0xe9; OK=False;
        END
       else
        BEGIN
         z=1; OK=True;
        END
      END
     else if (ArgCnt==2)
      BEGIN
       OK=DecodeCondition(ArgStr[1],&z);
       if (OK) z<<=3; else WrError(1360);
      END
     else
      BEGIN
       WrError(1110); OK=False;
      END
     if (OK)
      BEGIN
       AdrLong=EvalAbsAdrExpression(ArgStr[ArgCnt],&OK);
       if (OK)
        BEGIN
#ifdef __STDC__
         if ((AdrLong & 0xffff0000u)==0)
#else
         if ((AdrLong & 0xffff0000)==0)
#endif
          BEGIN
           CodeLen=3; BAsmCode[0]=0xc2+z;
           BAsmCode[1]=Lo(AdrLong); BAsmCode[2]=Hi(AdrLong);
          END
#ifdef __STDC__
         else if ((AdrLong & 0xff000000u)==0)
#else
         else if ((AdrLong & 0xff000000)==0)
#endif
          BEGIN
           ChangeDDPrefix("IB");
           CodeLen=4+PrefixCnt; BAsmCode[PrefixCnt]=0xc2+z;
           BAsmCode[PrefixCnt+1]=Lo(AdrLong);
           BAsmCode[PrefixCnt+2]=Hi(AdrLong);
           BAsmCode[PrefixCnt+3]=Hi(AdrLong >> 8);
          END
         else
          BEGIN
           ChangeDDPrefix("IW");
           CodeLen=5+PrefixCnt; BAsmCode[PrefixCnt]=0xc2+z;
           BAsmCode[PrefixCnt+1]=Lo(AdrLong);
           BAsmCode[PrefixCnt+2]=Hi(AdrLong);
           BAsmCode[PrefixCnt+3]=Hi(AdrLong >> 8);
           BAsmCode[PrefixCnt+4]=Hi(AdrLong >> 16);
          END
        END
      END
     return;
    END

   if (Memo("CALL"))
    BEGIN
     if (ArgCnt==1)
      BEGIN
       z=9; OK=True;
      END
     else if (ArgCnt==2)
      BEGIN
       OK=DecodeCondition(ArgStr[1],&z);
       if (OK) z<<=3; else WrError(1360);
      END
     else
      BEGIN
       WrError(1110); OK=False;
      END
     if (OK)
      BEGIN
       AdrLong=EvalAbsAdrExpression(ArgStr[ArgCnt],&OK);
       if (OK)
        BEGIN
#ifdef __STDC__
         if ((AdrLong & 0xffff0000u)==0)
#else
         if ((AdrLong & 0xffff0000)==0)
#endif
          BEGIN
           CodeLen=3; BAsmCode[0]=0xc4+z;
           BAsmCode[1]=Lo(AdrLong); BAsmCode[2]=Hi(AdrLong);
          END
#ifdef __STDC__
         else if ((AdrLong & 0xff000000u)==0)
#else
         else if ((AdrLong & 0xff000000)==0)
#endif
          BEGIN
           ChangeDDPrefix("IB");
           CodeLen=4+PrefixCnt; BAsmCode[PrefixCnt]=0xc4+z;
           BAsmCode[PrefixCnt+1]=Lo(AdrLong);
           BAsmCode[PrefixCnt+2]=Hi(AdrLong);
           BAsmCode[PrefixCnt+3]=Hi(AdrLong >> 8);
          END
         else
          BEGIN
           ChangeDDPrefix("IW");
           CodeLen=5+PrefixCnt; BAsmCode[PrefixCnt]=0xc4+z;
           BAsmCode[PrefixCnt+1]=Lo(AdrLong);
           BAsmCode[PrefixCnt+2]=Hi(AdrLong);
           BAsmCode[PrefixCnt+3]=Hi(AdrLong >> 8);
           BAsmCode[PrefixCnt+4]=Hi(AdrLong >> 16);
          END
        END
      END
     return;
    END

   if (Memo("JR"))
    BEGIN
     if (ArgCnt==1)
      BEGIN
       z=3; OK=True;
      END
     else if (ArgCnt==2)
      BEGIN
       OK=DecodeCondition(ArgStr[1],&z);
       if ((OK) AND (z>3)) OK=False;
       if (OK) z+=4; else WrError(1360);
      END
     else
      BEGIN
       WrError(1110); OK=False;
      END
     if (OK)
      BEGIN
       AdrLInt=EvalAbsAdrExpression(ArgStr[ArgCnt],&OK);
       if (OK)
        BEGIN
         AdrLInt-=EProgCounter()+2;
         if ((AdrLInt<=0x7fl) AND (AdrLInt>=-0x80l))
          BEGIN
           CodeLen=2; BAsmCode[0]=z << 3;
           BAsmCode[1]=AdrLInt & 0xff;
          END
         else
          if (MomCPU<CPUZ380) WrError(1370);
          else
           BEGIN
            AdrLInt-=2;
            if ((AdrLInt<=0x7fffl) AND (AdrLInt>=-0x8000l))
             BEGIN
              CodeLen=4; BAsmCode[0]=0xdd; BAsmCode[1]=z << 3;
              BAsmCode[2]=AdrLInt & 0xff;
              BAsmCode[3]=(AdrLInt >> 8) & 0xff;
             END
            else
             BEGIN
              AdrLInt--;
              if ((AdrLInt<=0x7fffffl) AND (AdrLInt>=-0x800000l))
               BEGIN
                CodeLen=5; BAsmCode[0]=0xfd; BAsmCode[1]=z << 3;
                BAsmCode[2]=AdrLInt & 0xff;
                BAsmCode[3]=(AdrLInt >> 8) & 0xff;
                BAsmCode[4]=(AdrLInt >> 16) & 0xff;
               END
              else WrError(1370);
             END
           END
        END
      END
     return;
    END

   if (Memo("CALR"))
    BEGIN
     if (ArgCnt==1)
      BEGIN
       z=9; OK=True;
      END
     else if (ArgCnt==2)
      BEGIN
       OK=DecodeCondition(ArgStr[1],&z);
       if (OK) z<<=3; else WrError(1360);
      END
     else
      BEGIN
       WrError(1110); OK=False;
      END
     if (OK)
      BEGIN
       if (MomCPU<CPUZ380) WrError(1500);
       else
        BEGIN
         AdrLInt=EvalAbsAdrExpression(ArgStr[ArgCnt],&OK);
         if (OK)
          BEGIN
           AdrLInt-=EProgCounter()+3;
           if ((AdrLInt<=0x7fl) AND (AdrLInt>=-0x80l))
            BEGIN
             CodeLen=3; BAsmCode[0]=0xed; BAsmCode[1]=0xc4+z;
             BAsmCode[2]=AdrLInt & 0xff;
            END
           else
            BEGIN
             AdrLInt--;
             if ((AdrLInt<=0x7fffl) AND (AdrLInt>=-0x8000l))
              BEGIN
               CodeLen=4; BAsmCode[0]=0xdd; BAsmCode[1]=0xc4+z;
               BAsmCode[2]=AdrLInt & 0xff;
               BAsmCode[3]=(AdrLInt >> 8) & 0xff;
              END
             else
              BEGIN
               AdrLInt--;
               if ((AdrLInt<=0x7fffffl) AND (AdrLInt>=-0x800000l))
                BEGIN
                 CodeLen=5; BAsmCode[0]=0xfd; BAsmCode[1]=0xc4+z;
                 BAsmCode[2]=AdrLInt & 0xff;
                 BAsmCode[3]=(AdrLInt >> 8) & 0xff;
                 BAsmCode[4]=(AdrLInt >> 16) & 0xff;
                END
               else WrError(1370);
              END
            END
          END
        END
      END
     return;
    END

   if (Memo("DJNZ"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrLInt=EvalAbsAdrExpression(ArgStr[1],&OK);
       if (OK)
        BEGIN
         AdrLInt-=EProgCounter()+2;
         if ((AdrLInt<=0x7fl) AND (AdrLInt>=-0x80l))
          BEGIN
           CodeLen=2; BAsmCode[0]=0x10; BAsmCode[1]=Lo(AdrLInt);
          END
         else if (MomCPU<CPUZ380) WrError(1370);
         else
          BEGIN
           AdrLInt-=2;
           if ((AdrLInt<=0x7fffl) AND (AdrLInt>=-0x8000l))
            BEGIN
             CodeLen=4; BAsmCode[0]=0xdd; BAsmCode[1]=0x10;
             BAsmCode[2]=AdrLInt & 0xff;
             BAsmCode[3]=(AdrLInt >> 8) & 0xff;
            END
           else
            BEGIN
             AdrLInt--;
             if ((AdrLInt<=0x7fffffl) AND (AdrLInt>=-0x800000l))
              BEGIN
               CodeLen=5; BAsmCode[0]=0xfd; BAsmCode[1]=0x10;
               BAsmCode[2]=AdrLInt & 0xff;
               BAsmCode[3]=(AdrLInt >> 8) & 0xff;
               BAsmCode[4]=(AdrLInt >> 16) & 0xff;
              END
             else WrError(1370);
            END
          END
        END
      END
     return;
    END

   if (Memo("RST"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       FirstPassUnknown=False;
       AdrByte=EvalIntExpression(ArgStr[1],Int8,&OK);
       if (FirstPassUnknown) AdrByte=AdrByte & 0x38;  
       if (OK)
        BEGIN
         if ((AdrByte>0x38) OR ((AdrByte & 7)!=0)) WrError(1320);
         else
          BEGIN
           CodeLen=1; BAsmCode[0]=0xc7+AdrByte;
          END
        END
      END
     return;
    END

/*-------------------------------------------------------------------------*/
/* Sonderbefehle */

   if ((Memo("EI")) OR (Memo("DI")))
    BEGIN
     if (ArgCnt==0)
      BEGIN
       BAsmCode[0]=0xf3+(Ord(Memo("EI")) << 3);
       CodeLen=1;
      END
     else if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPUZ380) WrError(1500);
     else
      BEGIN
       BAsmCode[2]=EvalIntExpression(ArgStr[1],UInt8,&OK);
       if (OK)
        BEGIN
         BAsmCode[0]=0xdd; BAsmCode[1]=0xf3+(Ord(Memo("EI")) << 3);
         CodeLen=3;
        END
      END
     return;
    END

   if (Memo("IM"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrByte=EvalIntExpression(ArgStr[1],UInt2,&OK);
       if (OK)
        BEGIN
         if (AdrByte>3) WrError(1320);
         else if ((AdrByte==3) AND (MomCPU<CPUZ380)) WrError(1500);
         else
          BEGIN
           if (AdrByte==3) AdrByte=1;
           else if (AdrByte>=1) AdrByte++;
           CodeLen=2; BAsmCode[0]=0xed; BAsmCode[1]=0x46+(AdrByte << 3);
          END
        END
      END
     return;
    END

   if (Memo("LDCTL"))
    BEGIN
     OpSize=0;
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPUZ380) WrError(1500);
     else if (DecodeSFR(ArgStr[1],&AdrByte))
      BEGIN
       DecodeAdr(ArgStr[2]);
       switch (AdrMode)
        BEGIN
         case ModReg8:
          if (AdrPart!=7) WrError(1350);
          else
           BEGIN
            BAsmCode[0]=0xcd+((AdrByte & 3) << 4);
            BAsmCode[1]=0xc8+((AdrByte & 4) << 2);
            CodeLen=2;
           END
          break;
         case ModReg16:
          if ((AdrByte!=1) OR (AdrPart!=2) OR (PrefixCnt!=0)) WrError(1350);
          else
           BEGIN
            BAsmCode[0]=0xed; BAsmCode[1]=0xc8; CodeLen=2;
           END
          break;
         case ModImm:
          BAsmCode[0]=0xcd+((AdrByte & 3) << 4);
          BAsmCode[1]=0xca+((AdrByte & 4) << 2);
          BAsmCode[2]=AdrVals[0];
          CodeLen=3;
          break;
         default: if (AdrMode!=ModNone) WrError(1350);
        END
      END
     else if (DecodeSFR(ArgStr[2],&AdrByte))
      BEGIN
       DecodeAdr(ArgStr[1]);
       switch (AdrMode)
        BEGIN
         case ModReg8:
          if ((AdrPart!=7) OR (AdrByte==1)) WrError(1350);
          else
           BEGIN
            BAsmCode[0]=0xcd+((AdrByte & 3) << 4);
            BAsmCode[1]=0xd0;
            CodeLen=2;
           END
          break;
         case ModReg16:
          if ((AdrByte!=1) OR (AdrPart!=2) OR (PrefixCnt!=0)) WrError(1350);
          else
           BEGIN
            BAsmCode[0]=0xed; BAsmCode[1]=0xc0; CodeLen=2;
           END
          break;
         default: if (AdrMode!=ModNone) WrError(1350);
        END
      END
     else WrError(1350);
     return;
    END

   if ((Memo("RESC")) OR (Memo("SETC")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPUZ380) WrError(1500);
     else
      BEGIN
       AdrByte=0xff; NLS_UpString(ArgStr[1]);
       if (strcmp(ArgStr[1],"LW")==0) AdrByte=1;
       else if (strcmp(ArgStr[1],"LCK")==0) AdrByte=2;
       else if (strcmp(ArgStr[1],"XM")==0) AdrByte=3;
       else WrError(1440);
       if (AdrByte!=0xff)
        BEGIN
         CodeLen=2; BAsmCode[0]=0xcd+(AdrByte << 4);
         BAsmCode[1]=0xf7+(Ord(Memo("RESC")) << 3);
        END
      END
     return;
    END

   WrXError(1200,OpPart);
END

        static void InitCode_Z80(void)
BEGIN
   SaveInitProc();
   SetFlag(&ExtFlag,ExtFlagName,False);
   SetFlag(&LWordFlag,LWordFlagName,False);
END

        static Boolean IsDef_Z80(void)
BEGIN
   return Memo("PORT");
END

        static void SwitchFrom_Z80(void)
BEGIN
   DeinitFields(); ClearONOFF();
END

        static void SwitchTo_Z80(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=True;

   PCSymbol="$"; HeaderID=0x51; NOPCode=0x00;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode)+(1<<SegIO);
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;
   SegLimits[SegCode] = CodeEnd();
   Grans[SegIO  ]=1; ListGrans[SegIO  ]=1; SegInits[SegIO  ]=0;
   SegLimits[SegIO  ] = PortEnd();

   MakeCode=MakeCode_Z80; IsDef=IsDef_Z80;
   SwitchFrom=SwitchFrom_Z80; InitFields();

   /* erweiterte Modi nur bei Z380 */

   if (MomCPU>=CPUZ380)
    BEGIN
     AddONOFF("EXTMODE",   &ExtFlag   , ExtFlagName   ,False);
     AddONOFF("LWORDMODE", &LWordFlag , LWordFlagName ,False);
    END
   SetFlag(&ExtFlag,ExtFlagName,False);
   SetFlag(&LWordFlag,LWordFlagName,False);
END

        void codez80_init(void)
BEGIN
   CPUZ80 =AddCPU("Z80" ,SwitchTo_Z80);
   CPUZ80U=AddCPU("Z80UNDOC",SwitchTo_Z80);
   CPUZ180=AddCPU("Z180",SwitchTo_Z80);
   CPUZ380=AddCPU("Z380",SwitchTo_Z80);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_Z80;
END
