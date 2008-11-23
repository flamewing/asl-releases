/* code7720.c */
/*****************************************************************************/
/* Makroassembler AS                                                         */
/*                                                                           */
/* Codegenerator NEC uPD772x                                                 */
/*                                                                           */
/* Historie: 30. 8.1998 Grundsteinlegung                                     */
/*           31. 8.1998 RET-Anweisung                                        */
/*            2. 9.1998 Verallgemeinerung auf 77C25                          */
/*            5. 9.1998 Pseudo-Anweisungen                                   */
/*           11. 9.1998 ROMData-Segment angelegt                             */
/*           24. 9.1998 Korrekturen fuer DOS-Compiler                        */
/*            2. 1.1999 ChkPC-Anpassung                                      */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*           14. 1.2001 silenced warnings about unused parameters            */
/*                                                                           */
/*****************************************************************************/
/* $Id: code7720.c,v 1.5 2008/11/23 10:39:17 alfred Exp $                    */
/***************************************************************************** 
 * $Log: code7720.c,v $
 * Revision 1.5  2008/11/23 10:39:17  alfred
 * - allow strings with NUL characters
 *
 * Revision 1.4  2007/11/24 22:48:05  alfred
 * - some NetBSD changes
 *
 * Revision 1.3  2005/09/08 16:53:42  alfred
 * - use common PInstTable
 *
 * Revision 1.2  2005/05/21 16:35:05  alfred
 * - removed variables available globally
 *
 * Revision 1.1  2003/11/06 02:49:21  alfred
 * - recreated
 *
 * Revision 1.4  2003/05/02 21:23:11  alfred
 * - strlen() updates
 *
 * Revision 1.3  2002/08/14 18:43:48  alfred
 * - warn null allocation, remove some warnings
 *
 * Revision 1.2  2002/07/14 18:39:58  alfred
 * - fixed TempAll-related warnings
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>
#include "strutil.h"
#include "nls.h"
#include "bpemu.h"

#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmcode.h"
#include "asmitree.h"
#include "headids.h"
#include "codevars.h"            

/*---------------------------------------------------------------------------*/

#define JmpOrderCnt 36
#define ALU2OrderCnt 8
#define ALU1OrderCnt 7

#define DestRegCnt 16
#define SrcRegCnt 16
#define ALUSrcRegCnt 4

typedef struct
         {
          LongWord Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          LongWord Code;
         } TReg;

typedef enum {MoveField,ALUField,DPLField,DPHField,RPField,RetField} OpComps;

static CPUVar CPU7720,CPU7725;

static LongWord ActCode;
static Boolean InOp;
static Byte UsedOpFields;

static Byte TypePos,ImmValPos,AddrPos,ALUPos,DPLPos,AccPos,ALUSrcPos;
static IntType MemInt;
static Word ROMEnd,DROMEnd,RAMEnd;

static FixedOrder *JmpOrders,*ALU2Orders,*ALU1Orders;

static TReg *DestRegs,*SrcRegs,*ALUSrcRegs;

static PInstTable OpTable;

/*---------------------------------------------------------------------------*/
/* Hilfsroutinen */

        static Boolean DecodeReg(char *Asc, LongWord *Code, TReg *Regs, int Cnt)
BEGIN
   int z;

   for (z=0; z<Cnt; z++)
    if (strcasecmp(Asc,Regs[z].Name)==0) break;

   if (z<Cnt) *Code=Regs[z].Code;
   return z<Cnt;
END

        static Boolean ChkOpPresent(OpComps Comp)
BEGIN
   if ((UsedOpFields&(1l<<Comp))!=0)
    BEGIN
     WrError(1355); return False;
    END
   else
    BEGIN
     UsedOpFields|=1l<<Comp; return True;
    END
END

/*---------------------------------------------------------------------------*/
/* Dekoder */                               

        static void DecodeJmp(Word Index)
BEGIN
   Word Dest;
   Boolean OK;
   FixedOrder *Op=JmpOrders+Index;

   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     Dest=EvalIntExpression(ArgStr[1],MemInt,&OK);
     if (OK)
      BEGIN
       DAsmCode[0]=(2l<<TypePos)+(Op->Code<<13)+(Dest<<AddrPos);
       CodeLen=1;
      END
    END
END

static void DecodeDATA(Word Index)
{
  LongInt MinV,MaxV;
  TempResult t;
  int z,Max;
  Boolean OK;
  UNUSED(Index);

  if (ActPC == SegCode)
    MaxV = (MomCPU >= CPU7725) ? 16777215 : 8388607;
  else
    MaxV = 65535;
  MinV = (-((MaxV + 1) >> 1));
  if (ArgCnt==0) WrError(1110);
  else
  {
    OK = True; z = 1;
    while ((OK) & (z <= ArgCnt))
    {
      FirstPassUnknown = False;
      EvalExpression(ArgStr[z], &t);
      if ((FirstPassUnknown) && (t.Typ == TempInt))
        t.Contents.Int &= MaxV;

      switch (t.Typ)
      {
        case TempInt:
          if ((OK = ChkRange(t.Contents.Int, MinV, MaxV)))
          {
            if (ActPC == SegCode) DAsmCode[CodeLen++] = t.Contents.Int;
            else WAsmCode[CodeLen++] = t.Contents.Int;
          }
          break;
        case TempFloat:
          WrError(1135); OK = False;
          break;
        case TempString:
        {
          unsigned z2;
          LongInt Trans;
          int Pos;

          Max = ((ActPC == SegCode) && (MomCPU >= CPU7725)) ? 3 : 2; Pos = 0;
          for (z2 = 0; z2 < t.Contents.Ascii.Length; z2++)
          {
            Trans = CharTransTable[((usint) t.Contents.Ascii.Contents[z2]) & 0xff];
            if (ActPC == SegCode)
              DAsmCode[CodeLen] = (Pos == 0) ? Trans : (DAsmCode[CodeLen] << 8) | Trans;
            else
              WAsmCode[CodeLen] = (Pos == 0) ? Trans : (WAsmCode[CodeLen] << 8) | Trans;
            if (++Pos == Max)
            {
              Pos = 0; CodeLen++;
            }
          }
          if (Pos != 0) CodeLen++;
          break;
        }
        default:
          OK = False;
      }
      z++;
    }
  }
}

        static void DecodeRES(Word Index)
BEGIN
   Word Size;
   Boolean OK;
   UNUSED(Index);

   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     FirstPassUnknown=False;
     Size=EvalIntExpression(ArgStr[1],Int16,&OK);
     if (FirstPassUnknown) WrError(1820);
     if ((OK) AND (NOT FirstPassUnknown))
      BEGIN
       DontPrint=True;
       if (!Size) WrError(290);
       CodeLen=Size;
       BookKeeping();
      END
    END
END

        static void DecodeALU2(Word Index)
BEGIN
   LongWord Acc=0xff,Src;
   FixedOrder *Op=ALU2Orders+Index;
   char ch;
   
   if (NOT ChkOpPresent(ALUField)) return;

   if (ArgCnt!=2) WrError(1110);
   else if (NOT DecodeReg(ArgStr[2],&Src,ALUSrcRegs,ALUSrcRegCnt)) WrXError(1445,ArgStr[2]);
   else
    BEGIN
     if ((strlen(ArgStr[1])==4) AND (strncasecmp(ArgStr[1],"ACC",3)==0))
      BEGIN
       ch=mytoupper(ArgStr[1][3]);
       if ((ch>='A') AND (ch<='B')) Acc=ch-'A';
      END
     if (Acc==0xff) WrXError(1445,ArgStr[1]);
     else ActCode|=(Op->Code<<ALUPos)+(Acc<<AccPos)+(Src<<ALUSrcPos);
    END
END

        static void DecodeALU1(Word Index)
BEGIN
   LongWord Acc=0xff;
   FixedOrder *Op=ALU1Orders+Index;
   char ch;
   
   if (NOT ChkOpPresent(ALUField)) return;

   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     if ((strlen(ArgStr[1])==4) AND (strncasecmp(ArgStr[1],"ACC",3)==0))
      BEGIN
       ch=mytoupper(ArgStr[1][3]);
       if ((ch>='A') AND (ch<='B')) Acc=ch-'A';
      END
     if (Acc==0xff) WrXError(1445,ArgStr[1]);
     else ActCode|=(Op->Code<<ALUPos)+(Acc<<AccPos);
    END
END

        static void DecodeNOP(Word Index)
BEGIN
   UNUSED(Index);

   if (NOT ChkOpPresent(ALUField)) return;
END

        static void DecodeDPL(Word Index)
BEGIN
   if (NOT ChkOpPresent(DPLField)) return;

   if (ArgCnt!=0) WrError(1110);
   else ActCode|=(((LongWord)Index)<<DPLPos);
END

        static void DecodeDPH(Word Index)
BEGIN
   if (NOT ChkOpPresent(DPHField)) return;

   if (ArgCnt!=0) WrError(1110);
   else ActCode|=(((LongWord)Index)<<9);
END

        static void DecodeRP(Word Index)
BEGIN
   if (NOT ChkOpPresent(RPField)) return;

   if (ArgCnt!=0) WrError(1110);
   else ActCode|=(((LongWord)Index)<<8);
END

        static void DecodeRET(Word Index)
BEGIN
   UNUSED(Index);

   if (NOT ChkOpPresent(RetField)) return;

   if (ArgCnt!=0) WrError(1110);
   else ActCode|=(1l<<TypePos);
END

        static void DecodeLDI(Word Index)
BEGIN
   LongWord Value;
   LongWord Reg;
   Boolean OK;
   UNUSED(Index);

   if (ArgCnt!=2) WrError(1110);
   else if (NOT DecodeReg(ArgStr[1],&Reg,DestRegs,DestRegCnt)) WrXError(1445,ArgStr[1]);
   else
    BEGIN
     Value=EvalIntExpression(ArgStr[2],Int16,&OK);
     if (OK)
      BEGIN
       DAsmCode[0]=(3l<<TypePos)+Reg+(Value<<ImmValPos);
       CodeLen=1;
      END
    END
END

        static void DecodeOP(Word Index)
BEGIN
   char *p;
   int z;
   UNUSED(Index);

   UsedOpFields=0; ActCode=0;

   if (ArgCnt>1)
    BEGIN
     p=FirstBlank(ArgStr[1]);
     if (p!=NULL)
      BEGIN
       *p='\0'; strmaxcpy(OpPart,ArgStr[1],255);
       NLS_UpString(OpPart);
       strcpy(ArgStr[1],p+1); KillPrefBlanks(ArgStr[1]);
      END
     else
      BEGIN
       strmaxcpy(OpPart,ArgStr[1],255);
       for (z=1; z<ArgCnt; z++)
        strcpy(ArgStr[z],ArgStr[z+1]);
       ArgCnt--;
      END
     if (NOT LookupInstTable(OpTable,OpPart)) WrXError(1200,OpPart);
    END

    DAsmCode[0]=ActCode; CodeLen=1;
END

        static void DecodeMOV(Word Index)
BEGIN
   LongWord Dest,Src;
   UNUSED(Index);

   if (NOT ChkOpPresent(MoveField)) return;

   if (ArgCnt!=2) WrError(1110);
   else if (NOT DecodeReg(ArgStr[1],&Dest,DestRegs,DestRegCnt)) WrXError(1445,ArgStr[1]);
   else if (NOT DecodeReg(ArgStr[2],&Src,SrcRegs,SrcRegCnt)) WrXError(1445,ArgStr[2]);
   else ActCode|=Dest+(Src<<4);
END

/*---------------------------------------------------------------------------*/
/* Tabellenverwaltung */

        static void AddJmp(char *NName, LongWord NCode)
BEGIN
   if (InstrZ>=JmpOrderCnt) exit(255);
   if ((MomCPU<CPU7725) AND (Odd(NCode))) return;
   JmpOrders[InstrZ].Code=(MomCPU==CPU7725) ? NCode : NCode>>1;
   AddInstTable(InstTable, NName, InstrZ++, DecodeJmp);
END

        static void AddALU2(char *NName, LongWord NCode)
BEGIN
   if (InstrZ>=ALU2OrderCnt) exit(255);
   ALU2Orders[InstrZ].Code=NCode;
   AddInstTable(OpTable, NName, InstrZ++, DecodeALU2);
END

        static void AddALU1(char *NName, LongWord NCode)
BEGIN
   if (InstrZ>=ALU1OrderCnt) exit(255);
   ALU1Orders[InstrZ].Code=NCode;
   AddInstTable(OpTable, NName, InstrZ++, DecodeALU1);
END

        static void AddDestReg(char *NName, LongWord NCode)
BEGIN
   if (InstrZ>=DestRegCnt) exit(255);
   DestRegs[InstrZ].Name=NName;
   DestRegs[InstrZ++].Code=NCode;
END

        static void AddSrcReg(char *NName, LongWord NCode)
BEGIN
   if (InstrZ>=SrcRegCnt) exit(255);
   SrcRegs[InstrZ].Name=NName;
   SrcRegs[InstrZ++].Code=NCode;
END

        static void AddALUSrcReg(char *NName, LongWord NCode)
BEGIN
   if (InstrZ>=ALUSrcRegCnt) exit(255);
   ALUSrcRegs[InstrZ].Name=NName;
   ALUSrcRegs[InstrZ++].Code=NCode;
END

        static void InitFields(void)
BEGIN
   InstTable=CreateInstTable(101);
   OpTable=CreateInstTable(79);

   AddInstTable(InstTable,"LDI",0,DecodeLDI);
   AddInstTable(InstTable,"LD",0,DecodeLDI);
   AddInstTable(InstTable,"OP",0,DecodeOP);
   AddInstTable(InstTable,"DATA",0,DecodeDATA);
   AddInstTable(InstTable,"RES",0,DecodeRES);
   AddInstTable(OpTable,"MOV",0,DecodeMOV);
   AddInstTable(OpTable,"NOP",0,DecodeNOP);
   AddInstTable(OpTable,"DPNOP",0,DecodeDPL);
   AddInstTable(OpTable,"DPINC",1,DecodeDPL);
   AddInstTable(OpTable,"DPDEC",2,DecodeDPL);
   AddInstTable(OpTable,"DPCLR",3,DecodeDPL);
   AddInstTable(OpTable,"M0",0,DecodeDPH);   
   AddInstTable(OpTable,"M1",1,DecodeDPH);   
   AddInstTable(OpTable,"M2",2,DecodeDPH);   
   AddInstTable(OpTable,"M3",3,DecodeDPH);   
   AddInstTable(OpTable,"M4",4,DecodeDPH);   
   AddInstTable(OpTable,"M5",5,DecodeDPH);   
   AddInstTable(OpTable,"M6",6,DecodeDPH);   
   AddInstTable(OpTable,"M7",7,DecodeDPH);   
   if (MomCPU>=CPU7725)
    BEGIN
     AddInstTable(OpTable,"M8",8,DecodeDPH);   
     AddInstTable(OpTable,"M9",9,DecodeDPH);   
     AddInstTable(OpTable,"MA",10,DecodeDPH);   
     AddInstTable(OpTable,"MB",11,DecodeDPH);   
     AddInstTable(OpTable,"MC",12,DecodeDPH);   
     AddInstTable(OpTable,"MD",13,DecodeDPH);   
     AddInstTable(OpTable,"ME",14,DecodeDPH);   
     AddInstTable(OpTable,"MF",15,DecodeDPH);   
    END
   AddInstTable(OpTable,"RPNOP",0,DecodeRP);   
   AddInstTable(OpTable,"RPDEC",1,DecodeRP);
   AddInstTable(OpTable,"RET",1,DecodeRET);

   JmpOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*JmpOrderCnt); InstrZ=0;
   AddJmp("JMP"   , 0x100); AddJmp("CALL"  , 0x140);
   AddJmp("JNCA"  , 0x080); AddJmp("JCA"   , 0x082);
   AddJmp("JNCB"  , 0x084); AddJmp("JCB"   , 0x086);
   AddJmp("JNZA"  , 0x088); AddJmp("JZA"   , 0x08a);
   AddJmp("JNZB"  , 0x08c); AddJmp("JZB"   , 0x08e);
   AddJmp("JNOVA0", 0x090); AddJmp("JOVA0" , 0x092);
   AddJmp("JNOVB0", 0x094); AddJmp("JOVB0" , 0x096);
   AddJmp("JNOVA1", 0x098); AddJmp("JOVA1" , 0x09a);
   AddJmp("JNOVB1", 0x09c); AddJmp("JOVB1" , 0x09e);
   AddJmp("JNSA0" , 0x0a0); AddJmp("JSA0"  , 0x0a2);
   AddJmp("JNSB0" , 0x0a4); AddJmp("JSB0"  , 0x0a6);
   AddJmp("JNSA1" , 0x0a8); AddJmp("JSA1"  , 0x0aa);
   AddJmp("JNSB1" , 0x0ac); AddJmp("JSB1"  , 0x0ae);
   AddJmp("JDPL0" , 0x0b0); AddJmp("JDPLF" , 0x0b2);
   AddJmp("JNSIAK", 0x0b4); AddJmp("JSIAK" , 0x0b6);
   AddJmp("JNSOAK", 0x0b8); AddJmp("JSOAK" , 0x0ba);
   AddJmp("JNRQM" , 0x0bc); AddJmp("JRQM"  , 0x0be);
   AddJmp("JDPLN0", 0x0b1); AddJmp("JDPLNF" , 0x0b3);

   ALU2Orders=(FixedOrder *) malloc(sizeof(FixedOrder)*ALU2OrderCnt); InstrZ=0;
   AddALU2("OR"  , 1); AddALU2("AND" , 2); AddALU2("XOR" , 3);
   AddALU2("SUB" , 4); AddALU2("ADD" , 5); AddALU2("SBB" , 6);
   AddALU2("ADC" , 7); AddALU2("CMP" ,10);

   ALU1Orders=(FixedOrder *) malloc(sizeof(FixedOrder)*ALU1OrderCnt); InstrZ=0;
   AddALU1("DEC" , 8); AddALU1("INC" , 9); AddALU1("SHR1",11);
   AddALU1("SHL1",12); AddALU1("SHL2",13); AddALU1("SHL4",14);
   AddALU1("XCHG",15);

   DestRegs=(TReg *) malloc(sizeof(TReg)*DestRegCnt); InstrZ=0;
   AddDestReg("@NON",  0); AddDestReg("@A"  ,  1);
   AddDestReg("@B"  ,  2); AddDestReg("@TR" ,  3);
   AddDestReg("@DP" ,  4); AddDestReg("@RP" ,  5);
   AddDestReg("@DR" ,  6); AddDestReg("@SR" ,  7);
   AddDestReg("@SOL",  8); AddDestReg("@SOM",  9);
   AddDestReg("@K"  , 10); AddDestReg("@KLR", 11);
   AddDestReg("@KLM", 12); AddDestReg("@L"  , 13);
   AddDestReg((MomCPU==CPU7725)?"@TRB":"",14);
   AddDestReg("@MEM", 15);
   

   SrcRegs=(TReg *) malloc(sizeof(TReg)*SrcRegCnt); InstrZ=0;
   AddSrcReg("NON" ,  0); AddSrcReg("A"   ,  1);
   AddSrcReg("B"   ,  2); AddSrcReg("TR"  ,  3);
   AddSrcReg("DP"  ,  4); AddSrcReg("RP"  ,  5);
   AddSrcReg("RO"  ,  6); AddSrcReg("SGN" ,  7);
   AddSrcReg("DR"  ,  8); AddSrcReg("DRNF",  9);
   AddSrcReg("SR"  , 10); AddSrcReg("SIM" , 11);
   AddSrcReg("SIL" , 12); AddSrcReg("K"   , 13);
   AddSrcReg("L"   , 14); AddSrcReg("MEM" , 15);

   ALUSrcRegs=(TReg *) malloc(sizeof(TReg)*ALUSrcRegCnt); InstrZ=0;
   AddALUSrcReg("RAM", 0); AddALUSrcReg("IDB", 1);
   AddALUSrcReg("M"  , 2); AddALUSrcReg("N"  , 3);
END

        static void DeinitFields(void)
BEGIN
   DestroyInstTable(InstTable);
   DestroyInstTable(OpTable);
   free(JmpOrders);
   free(ALU2Orders);
   free(ALU1Orders);
   free(DestRegs);
   free(SrcRegs);
   free(ALUSrcRegs);
END

/*---------------------------------------------------------------------------*/
/* Callbacks */

        static void MakeCode_7720(void)
BEGIN
   Boolean NextOp;

   /* Nullanweisung */

   if ((Memo("")) AND (*AttrPart=='\0') AND (ArgCnt==0)) return;

   /* direkte Anweisungen */

   NextOp=Memo("OP");
   if (LookupInstTable(InstTable,OpPart))
    BEGIN
     InOp=NextOp; return;
    END

   /* wenn eine parallele Op-Anweisung offen ist, noch deren Komponenten testen */

   if ((InOp) AND (LookupInstTable(OpTable,OpPart)))
    BEGIN
     RetractWords(1);
     DAsmCode[0]=ActCode; CodeLen=1;
     return;
    END

   /* Hae??? */

   WrXError(1200,OpPart);
END

        static Boolean IsDef_7720(void)
BEGIN
   return False;
END

        static void SwitchFrom_7720(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_7720(void)
BEGIN
   PFamilyDescr FoundDescr;

   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=False;

   if (MomCPU==CPU7725)
    BEGIN
     FoundDescr=FindFamilyByName("7725");
     MemInt=UInt11;
     ROMEnd=0x7ff; DROMEnd=0x3ff; RAMEnd=0xff;
     TypePos=22;
     ImmValPos=6;
     AddrPos=2;
     ALUPos=16;
     DPLPos=13;
     AccPos=15;
     ALUSrcPos=20;
    END
   else
    BEGIN
     FoundDescr=FindFamilyByName("7720");
     MemInt=UInt9;
     ROMEnd=0x1ff; DROMEnd=0x1ff; RAMEnd=0x7f;
     TypePos=21;
     ImmValPos=5;
     AddrPos=4;
     ALUPos=15;
     DPLPos=12;
     AccPos=14;
     ALUSrcPos=19;
    END
 
   PCSymbol="$"; HeaderID=FoundDescr->Id; NOPCode=0x000000;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1l<<SegCode)|(1l<<SegData)|(1l<<SegRData);
   Grans[SegCode ]=4; ListGrans[SegCode ]=4; SegInits[SegCode ]=0;
   SegLimits[SegCode ] = ROMEnd;
   Grans[SegData ]=2; ListGrans[SegData ]=2; SegInits[SegData ]=0;
   SegLimits[SegData ] = RAMEnd;
   Grans[SegRData]=2; ListGrans[SegRData]=2; SegInits[SegRData]=0;
   SegLimits[SegRData] = DROMEnd;

   MakeCode=MakeCode_7720; IsDef=IsDef_7720;
   SwitchFrom=SwitchFrom_7720;

   InOp=False; UsedOpFields=0; ActCode=0;
   InitFields();
END

/*---------------------------------------------------------------------------*/
/* Initialisierung */

        void code7720_init(void)
BEGIN
   CPU7720=AddCPU("7720",SwitchTo_7720);
   CPU7725=AddCPU("7725",SwitchTo_7720);
END
