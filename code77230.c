/* code77230.c */
/*****************************************************************************/
/* Makroassembler AS                                                         */
/*                                                                           */
/* Codegenerator NEC uPD77230                                                */
/*                                                                           */
/* Historie: 13. 9.1998 Grundsteinlegung                                     */
/*           14. 9.1998 LDI/Destregs getestet                                */
/*           16. 9.1998 das instruktionelle Kleingemuese begonnen            */
/*           19. 9.1998 restliche CPU-Instruktionen                          */
/*           27. 9.1998 DW-Anweisung                                         */
/*           28. 9.1998 String-Argument fuer DW                              */
/*                      DS                                                   */
/*            2. 1.1999 ChkPC-Anpassung                                      */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*           14. 1.2001 silenced warnings about unused parameters            */
/*                                                                           */
/*****************************************************************************/
/* $Id: code77230.c,v 1.4 2005/09/08 16:53:42 alfred Exp $                   */
/*****************************************************************************
 * $Log: code77230.c,v $
 * Revision 1.4  2005/09/08 16:53:42  alfred
 * - use common PInstTable
 *
 * Revision 1.3  2005/05/21 16:35:05  alfred
 * - removed variables available globally
 *
 * Revision 1.2  2004/09/26 14:40:24  alfred
 * - fix error warning
 *
 * Revision 1.1  2003/11/06 02:49:21  alfred
 * - recreated
 *
 * Revision 1.2  2002/08/14 18:43:48  alfred
 * - warn null allocation, remove some warnings
 *
 *****************************************************************************/

/*---------------------------------------------------------------------------*/
/* Includes */

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "strutil.h"
#include "nls.h"
#include "endian.h"

#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"
#include "headids.h"
#include "codevars.h"            

/*---------------------------------------------------------------------------*/
/* Definitionen */

#define SrcRegCnt 32
#define DestRegCnt 32
#define ALUSrcRegCnt 4

#define JmpOrderCnt 32
#define ALU1OrderCnt 15
#define ALU2OrderCnt 10

#define CaseCnt 17

typedef struct
         {
          LongWord Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          LongWord Code;
         } Register;

enum {InstrLDI, InstrBranch,
      InstrALU, InstrMove, 
      InstrM0, InstrM1, InstrDP0, InstrDP1,
      InstrEA, InstrRP, InstrFC, InstrLC,
      InstrBASE0, InstrBASE1, InstrRPC,
      InstrP2, InstrP3, InstrEM, InstrBM, 
      InstrL, InstrRW, InstrWT, InstrNF, InstrWI,
      InstrFIS, InstrFD, InstrSHV, InstrRPS, InstrNAL, InstrCnt};

static LongWord CaseMasks[CaseCnt]=
                {(1l<<InstrLDI),
                 (1l<<InstrBranch)|(1l<<InstrMove),
                 (1l<<InstrALU)|(1l<<InstrMove)|(1l<<InstrM0)|(1l<<InstrM1)|(1l<<InstrDP0)|(1l<<InstrDP1),
                 (1l<<InstrALU)|(1l<<InstrMove)|(1l<<InstrEA)|(1l<<InstrDP0)|(1l<<InstrDP1),
                 (1l<<InstrALU)|(1l<<InstrMove)|(1l<<InstrRP)|(1l<<InstrM0)|(1l<<InstrDP0)|(1l<<InstrFC),
                 (1l<<InstrALU)|(1l<<InstrMove)|(1l<<InstrRP)|(1l<<InstrM1)|(1l<<InstrDP1)|(1l<<InstrFC),
                 (1l<<InstrALU)|(1l<<InstrMove)|(1l<<InstrRP)|(1l<<InstrM0)|(1l<<InstrM1)|(1l<<InstrL)|(1l<<InstrFC),
                 (1l<<InstrALU)|(1l<<InstrMove)|(1l<<InstrBASE0)|(1l<<InstrBASE1)|(1l<<InstrFC),
                 (1l<<InstrALU)|(1l<<InstrMove)|(1l<<InstrRPC)|(1l<<InstrL)|(1l<<InstrFC),
                 (1l<<InstrALU)|(1l<<InstrMove)|(1l<<InstrP3)|(1l<<InstrP2)|(1l<<InstrEM)|(1l<<InstrBM)|(1l<<InstrL)|(1l<<InstrFC),
                 (1l<<InstrALU)|(1l<<InstrMove)|(1l<<InstrRW)|(1l<<InstrL)|(1l<<InstrFC),
                 (1l<<InstrALU)|(1l<<InstrMove)|(1l<<InstrWT)|(1l<<InstrL)|(1l<<InstrFC),
                 (1l<<InstrALU)|(1l<<InstrMove)|(1l<<InstrNF)|(1l<<InstrWI)|(1l<<InstrL)|(1l<<InstrFC),
                 (1l<<InstrALU)|(1l<<InstrMove)|(1l<<InstrFIS)|(1l<<InstrFD)|(1l<<InstrL),
                 (1l<<InstrALU)|(1l<<InstrMove)|(1l<<InstrSHV),
                 (1l<<InstrALU)|(1l<<InstrMove)|(1l<<InstrRPS),
                 (1l<<InstrALU)|(1l<<InstrMove)|(1l<<InstrNAL)
                };

static CPUVar CPU77230;
static LongWord InstrMask;
static Boolean Error;
static LongWord *InstrComps,*InstrDefs;

static Register *SrcRegs,*ALUSrcRegs,*DestRegs;
static FixedOrder *JmpOrders,*ALU1Orders,*ALU2Orders;

/*---------------------------------------------------------------------------*/
/* Hilfsroutinen */

static int DiscCnt,SplittedArg;
static char *DiscPtr;

        static Boolean SplitArgs(int Count)
BEGIN
   char *p,*p1,*p2;

   SplittedArg=DiscCnt=Count;

   if (Count==0)
    BEGIN
     if (ArgCnt>0)
      BEGIN
       DiscPtr=ArgStr[1]-1;
       SplittedArg=1;
      END
     else DiscPtr=Nil;
     return True;
    END

   if (ArgCnt<Count) 
    BEGIN
     WrError(1110); Error=True; return False;
    END

   for (p=ArgStr[SplittedArg]; isspace(((usint)*p)&0xff); p++);
   p1=QuotPos(p,' ');
   p2=QuotPos(p,'\t');
   if ((p1==Nil) OR ((p2!=Nil) AND (p2<p1))) DiscPtr=p2;
   else DiscPtr=p1;
   if (DiscPtr!=Nil) *(DiscPtr)='\0';

   return True;
END

        static void DiscardArgs(void)
BEGIN
   char *p,*p2;
   int z;
   Boolean Eaten;

   if (DiscPtr!=Nil)
    BEGIN
     for (p=DiscPtr+1; isspace(((usint)*p)&0xff); p++)
      if (*p=='\0') break;
     for (p2=p; NOT isspace(((usint)*p2)&0xff); p2++)
      if (*p2=='\0') break;
     Eaten=(*p2=='\0'); *p2='\0';
     strcpy(OpPart,p); NLS_UpString(OpPart);
     if (Eaten)
      BEGIN
       for (z=1; z<ArgCnt; z++)
        strcpy(ArgStr[z],ArgStr[z+1]);
       ArgCnt--;
      END
     else
      BEGIN
       if (p2!=Nil)
        for (p2++; isspace(((usint)*p2)&0xff); p2++);
       strcpy(ArgStr[SplittedArg],p2);
      END
    END
   else *OpPart='\0';
   if (DiscCnt>0)
    BEGIN
     for (z=0; z<=ArgCnt-DiscCnt; z++)
      strcpy(ArgStr[z+1],ArgStr[z+DiscCnt]);
     ArgCnt-=DiscCnt-1;
    END
END

        static void AddComp(int Index, LongWord Value)
BEGIN
   if ((InstrMask&(1l<<Index))!=0)
    BEGIN
     WrError(1355); Error=True;
    END
   else
    BEGIN
     InstrMask|=(1l<<Index); InstrComps[Index]=Value;
    END;
END

        static Boolean DecodeReg(char *Asc, LongWord *Erg, Register *Regs, int Cnt)
BEGIN
   int z;

   for (z=0; z<Cnt; z++)
    if (strcasecmp(Asc,Regs[z].Name)==0)
     BEGIN
      *Erg=Regs[z].Code; return True;
     END
   return False;
END

/*---------------------------------------------------------------------------*/
/* Dekoder fuer Routinen */

        static void DecodeJmp(Word Index)
BEGIN
   FixedOrder *Op=JmpOrders+Index;
   int acnt=(Memo("RET")) ? 0 : 1;
   LongWord Adr;

   if (NOT SplitArgs(acnt)) return;
   if (acnt==0)
    BEGIN
     Adr=0; Error=True;
    END
   else Adr=EvalIntExpression(ArgStr[1],UInt13,&Error);
   Error=NOT Error;
   if (NOT Error) AddComp(InstrBranch,Op->Code+(Adr << 5));
   DiscardArgs();
END

        static void DecodeMOV(Word Index)
BEGIN
   LongWord DReg,SReg;
   UNUSED(Index);

   if (NOT SplitArgs(2)) return;
   if (NOT DecodeReg(ArgStr[1],&DReg,DestRegs,DestRegCnt))
    BEGIN
     WrXError(1445,ArgStr[1]); Error=True;
    END
   else if (NOT DecodeReg(ArgStr[2],&SReg,SrcRegs,SrcRegCnt))
    BEGIN
     WrXError(1445,ArgStr[2]); Error=True;
    END
   else AddComp(InstrMove,(SReg<<5)+DReg);
   DiscardArgs();
END

        static void DecodeLDI(Word Index)
BEGIN
   LongWord DReg,Src=0;
   UNUSED(Index);

   if (NOT SplitArgs(2)) return;
   if (NOT DecodeReg(ArgStr[1],&DReg,DestRegs,DestRegCnt))
    BEGIN
     WrXError(1445,ArgStr[1]); Error=True;
    END
   else Src=EvalIntExpression(ArgStr[2],Int24,&Error);
   Error=NOT Error;
   if (NOT Error) AddComp(InstrLDI,(Src<<5)+DReg);
   DiscardArgs();
END

        static void DecodeNOP(Word Index)
BEGIN
   UNUSED(Index);

   if (NOT SplitArgs(0)) return;
   AddComp(InstrALU,0);
   DiscardArgs();
END

        static void DecodeALU1(Word Index)
BEGIN
   FixedOrder *Op=ALU1Orders+Index;
   LongWord DReg;

   if (NOT SplitArgs(1)) return;
   if ((NOT DecodeReg(ArgStr[1],&DReg,DestRegs,DestRegCnt))
    OR (DReg<16) OR (DReg>23))
    BEGIN
     WrXError(1445,ArgStr[1]); Error=True;
    END
   else AddComp(InstrALU,(Op->Code<<17)+(DReg&7));
   DiscardArgs();
END

        static void DecodeALU2(Word Index)             
BEGIN                                                  
   FixedOrder *Op=ALU2Orders+Index;                    
   LongWord DReg,SReg;                                      
                                                       
   if (NOT SplitArgs(2)) return;                       
   if ((NOT DecodeReg(ArgStr[1],&DReg,DestRegs,DestRegCnt))
    OR (DReg<16) OR (DReg>23))                         
    BEGIN                                              
     WrXError(1445,ArgStr[1]); Error=True;             
    END                                                
   else if (NOT DecodeReg(ArgStr[2],&SReg,ALUSrcRegs,ALUSrcRegCnt))
    BEGIN                                              
     WrXError(1445,ArgStr[2]); Error=True;             
    END
   else AddComp(InstrALU,(Op->Code<<17)+(SReg<<3)+(DReg&7));
   DiscardArgs();                                      
END

        static void DecodeM0(Word Index)
BEGIN
   if (NOT SplitArgs(0)) return;
   AddComp(InstrM0,Index);
   DiscardArgs();
END

        static void DecodeM1(Word Index)
BEGIN
   if (NOT SplitArgs(0)) return;
   AddComp(InstrM1,Index);
   DiscardArgs();
END

        static void DecodeDP0(Word Index)
BEGIN
   if (NOT SplitArgs(0)) return;
   AddComp(InstrDP0,Index);
   DiscardArgs();
END  

        static void DecodeDP1(Word Index)
BEGIN
   if (NOT SplitArgs(0)) return;
   AddComp(InstrDP1,Index);
   DiscardArgs();
END

        static void DecodeEA(Word Index)
BEGIN
   if (NOT SplitArgs(0)) return;
   AddComp(InstrEA,Index);
   DiscardArgs();
END

        static void DecodeFC(Word Index)
BEGIN
   if (NOT SplitArgs(0)) return;
   AddComp(InstrFC,Index);
   DiscardArgs();
END

        static void DecodeRP(Word Index)
BEGIN
   if (NOT SplitArgs(0)) return;
   AddComp(InstrRP,Index);
   DiscardArgs();
END

        static void DecodeL(Word Index)
BEGIN
   if (NOT SplitArgs(0)) return;
   AddComp(InstrL,Index);
   DiscardArgs();
END

        static void DecodeBASE(Word Index)
BEGIN
   LongWord Value;

   if (NOT SplitArgs(1)) return;
   Value=EvalIntExpression(ArgStr[1],UInt3,&Error); Error=NOT Error;
   if (NOT Error) AddComp(Index,Value);
   DiscardArgs();
END

        static void DecodeRPC(Word Index)
BEGIN
   LongWord Value;
   UNUSED(Index);

   if (NOT SplitArgs(1)) return;
   FirstPassUnknown=False;
   Value=EvalIntExpression(ArgStr[1],UInt4,&Error);
   if (FirstPassUnknown) Value&=7;
   if (Value>9) Error=True; else Error=NOT Error;
   if (NOT Error) AddComp(InstrRPC,Value);
   DiscardArgs();
END

        static void DecodeP2(Word Index)
BEGIN
   if (NOT SplitArgs(0)) return;
   AddComp(InstrP2,Index);
   DiscardArgs();
END

        static void DecodeP3(Word Index)
BEGIN
   if (NOT SplitArgs(0)) return;
   AddComp(InstrP3,Index);
   DiscardArgs();
END

        static void DecodeBM(Word Index)
BEGIN
   /* Wenn EM-Feld schon da war, muss es EI gewesen sein */

   if (NOT SplitArgs(0)) return;
   if ((InstrMask&(1<<InstrEM))!=0)
    BEGIN
     Error=(InstrComps[InstrEM]==0);
     if (Error) WrError(1355);
     else AddComp(InstrBM,Index);
    END
   else AddComp(InstrBM,Index);
   DiscardArgs();
END

        static void DecodeEM(Word Index)
BEGIN
   /* Wenn BM-Feld schon da war, muss es EI sein */

   if (NOT SplitArgs(0)) return;
   if ((InstrMask&(1<<InstrBM))!=0)
    BEGIN
     Error=(Index==0);
     if (Error) WrError(1355);
     else AddComp(InstrEM,Index);
    END
   else
    BEGIN
     AddComp(InstrEM,Index);
     if (Index==0) InstrComps[InstrBM]=3;
    END
   DiscardArgs();
END

        static void DecodeRW(Word Index)
BEGIN
   if (NOT SplitArgs(0)) return;   
   AddComp(InstrRW,Index);
   DiscardArgs();
END

        static void DecodeWT(Word Index)
BEGIN
   if (NOT SplitArgs(0)) return; 
   AddComp(InstrWT,Index);
   DiscardArgs();
END

        static void DecodeNF(Word Index)
BEGIN
   if (NOT SplitArgs(0)) return; 
   AddComp(InstrNF,Index);
   DiscardArgs();
END

        static void DecodeWI(Word Index)
BEGIN
   if (NOT SplitArgs(0)) return; 
   AddComp(InstrWI,Index);
   DiscardArgs();
END

        static void DecodeFIS(Word Index)
BEGIN
   if (NOT SplitArgs(0)) return; 
   AddComp(InstrFIS,Index);
   DiscardArgs();
END

        static void DecodeFD(Word Index)
BEGIN
   if (NOT SplitArgs(0)) return; 
   AddComp(InstrFD,Index);
   DiscardArgs();
END

        static void DecodeSHV(Word Index)
BEGIN
   LongWord Value;

   if (NOT SplitArgs(1)) return;
   FirstPassUnknown=False;
   Value=EvalIntExpression(ArgStr[1],UInt6,&Error);
   if (FirstPassUnknown) Value&=31;
   if (Value>46) Error=True; else Error=NOT Error;
   if (NOT Error) AddComp(InstrSHV,(Index<<6)+Value);     
   DiscardArgs();
END

        static void DecodeRPS(Word Index)
BEGIN
   LongWord Value;
   UNUSED(Index);
   
   if (NOT SplitArgs(1)) return;
   Value=EvalIntExpression(ArgStr[1],UInt9,&Error);
   Error=NOT Error;
   if (NOT Error) AddComp(InstrRPS,Value);
   DiscardArgs();
END

        static void DecodeNAL(Word Index)
BEGIN
   LongWord Value;
   UNUSED(Index);
 
   if (NOT SplitArgs(1)) return;
   FirstPassUnknown=False;
   Value=EvalIntExpression(ArgStr[1],UInt13,&Error);
   if (FirstPassUnknown) Value=(Value&0x1ff)|(EProgCounter()&0x1e00);
   Error=NOT Error;
   if (NOT Error)
    BEGIN
     if ((NOT SymbolQuestionable) AND ((Value^EProgCounter())&0x1e00)) WrError(1910);
     else AddComp(InstrNAL,Value&0x1ff);
    END
   DiscardArgs();  
END

        static Boolean DecodePseudo(void)
BEGIN
   int z;
   Boolean OK;
   TempResult t;
   LongWord temp;
   LongInt sign,mant,expo,Size;
   char *cp;

   if (Memo("DW"))
    BEGIN
     if (ArgCnt<1) WrError(1110);
     else
      BEGIN
       z=1; OK=True;
       while ((OK) AND (z<=ArgCnt))
        BEGIN
         FirstPassUnknown=FALSE;
         EvalExpression(ArgStr[z],&t);
         switch(t.Typ)
          BEGIN
           case TempInt:
            if (NOT RangeCheck(t.Contents.Int,Int32))
             BEGIN
              WrError(1320); OK=False; break;
             END
            DAsmCode[CodeLen++]=t.Contents.Int;
            break;
           case TempFloat:
            if (NOT FloatRangeCheck(t.Contents.Float,Float32))
             BEGIN
              WrError(1320); OK=False; break;
             END
            Double_2_ieee4(t.Contents.Float,(Byte*) &temp,BigEndian);
            sign=(temp>>31)&1;
            expo=(temp>>23)&255;
            mant=temp&0x7fffff;
            if ((mant==0) AND (expo==0))
             DAsmCode[CodeLen++]=0x80000000;
            else
             BEGIN
              if (expo>0)
               BEGIN
                mant|=0x800000;
                expo-=127;
               END
              else expo-=126;
              if (mant>=0x800000)
               BEGIN
                mant=mant>>1; expo+=1;
               END
              if (sign==1) mant=((mant^0xffffff)+1);
              DAsmCode[CodeLen++]=((expo&0xff)<<24)|(mant&0xffffff);
             END
            break;
           case TempString:
            for (z=0,cp=t.Contents.Ascii; *cp!='\0'; cp++,z++)
             BEGIN
              DAsmCode[CodeLen]=(DAsmCode[CodeLen]<<8)+CharTransTable[((usint)*cp)&0xff];
              if ((z&3)==3) CodeLen++;
             END
            if ((z&3)!=0) 
            {
              DAsmCode[CodeLen] = (DAsmCode[CodeLen]) << ((4 - (z & 3)) << 3);
              CodeLen++;
            }
            break;
           default:
            OK=False;
          END
         z++;
        END
       if (NOT OK) CodeLen=0;
      END
     return True;
    END

   if (Memo("DS"))
    BEGIN
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
     return True;
    END

   return FALSE;
END

/*---------------------------------------------------------------------------*/
/* Codetabellenverwaltung */

        static void AddJmp(char *NName, LongWord NCode)
BEGIN
   if (InstrZ>=JmpOrderCnt) exit(255);
   JmpOrders[InstrZ].Code=NCode;
   AddInstTable(InstTable,NName,InstrZ++,DecodeJmp);
END

        static void AddALU1(char *NName, LongWord NCode)
BEGIN
   if (InstrZ>=ALU1OrderCnt) exit(255);
   ALU1Orders[InstrZ].Code=NCode;
   AddInstTable(InstTable,NName,InstrZ++,DecodeALU1);
END

        static void AddALU2(char *NName, LongWord NCode)
BEGIN
   if (InstrZ>=ALU2OrderCnt) exit(255);
   ALU2Orders[InstrZ].Code=NCode;
   AddInstTable(InstTable,NName,InstrZ++,DecodeALU2);
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

        static void AddDestReg(char *NName, LongWord NCode)
BEGIN
   if (InstrZ>=DestRegCnt) exit(255);
   DestRegs[InstrZ].Name=NName;
   DestRegs[InstrZ++].Code=NCode;
END

        static void InitFields(void)
BEGIN
   InstTable=CreateInstTable(201);

   AddInstTable(InstTable,"MOV",0,DecodeMOV);
   AddInstTable(InstTable,"LDI",0,DecodeLDI);
   AddInstTable(InstTable,"NOP",0,DecodeNOP);

   AddInstTable(InstTable,"SPCBP0",1,DecodeM0);
   AddInstTable(InstTable,"SPCIX0",2,DecodeM0);
   AddInstTable(InstTable,"SPCBI0",3,DecodeM0);

   AddInstTable(InstTable,"SPCBP1",1,DecodeM1);
   AddInstTable(InstTable,"SPCIX1",2,DecodeM1);
   AddInstTable(InstTable,"SPCBI1",3,DecodeM1);

   AddInstTable(InstTable,"INCBP0",1,DecodeDP0);
   AddInstTable(InstTable,"DECBP0",2,DecodeDP0);
   AddInstTable(InstTable,"CLRBP0",3,DecodeDP0);
   AddInstTable(InstTable,"STIX0" ,4,DecodeDP0);
   AddInstTable(InstTable,"INCIX0",5,DecodeDP0);
   AddInstTable(InstTable,"DECIX0",6,DecodeDP0);
   AddInstTable(InstTable,"CLRIX0",7,DecodeDP0);

   AddInstTable(InstTable,"INCBP1",1,DecodeDP1);
   AddInstTable(InstTable,"DECBP1",2,DecodeDP1);
   AddInstTable(InstTable,"CLRBP1",3,DecodeDP1);
   AddInstTable(InstTable,"STIX1" ,4,DecodeDP1);
   AddInstTable(InstTable,"INCIX1",5,DecodeDP1);
   AddInstTable(InstTable,"DECIX1",6,DecodeDP1);
   AddInstTable(InstTable,"CLRIX1",7,DecodeDP1);

   AddInstTable(InstTable,"INCAR" ,1,DecodeEA);
   AddInstTable(InstTable,"DECAR" ,2,DecodeEA);

   AddInstTable(InstTable,"XCHPSW",1,DecodeFC);

   AddInstTable(InstTable,"INCRP" ,1,DecodeRP);
   AddInstTable(InstTable,"DECRP" ,2,DecodeRP);
   AddInstTable(InstTable,"INCBRP",3,DecodeRP);

   AddInstTable(InstTable,"DECLC" ,1,DecodeL);

   AddInstTable(InstTable,"MCNBP0",InstrBASE0,DecodeBASE);
   AddInstTable(InstTable,"MCNBP1",InstrBASE1,DecodeBASE);

   AddInstTable(InstTable,"BITRP" ,0,DecodeRPC);

   AddInstTable(InstTable,"CLRP2" ,0,DecodeP2);
   AddInstTable(InstTable,"SETP2" ,1,DecodeP2);

   AddInstTable(InstTable,"CLRP3" ,0,DecodeP3);
   AddInstTable(InstTable,"SETP3" ,1,DecodeP3);

   AddInstTable(InstTable,"DI"    ,0,DecodeEM);
   AddInstTable(InstTable,"EI"    ,1,DecodeEM);
   AddInstTable(InstTable,"CLRBM" ,1,DecodeBM);
   AddInstTable(InstTable,"SETBM" ,2,DecodeBM);

   AddInstTable(InstTable,"RD"    ,1,DecodeRW);
   AddInstTable(InstTable,"WR"    ,2,DecodeRW);

   AddInstTable(InstTable,"WRBORD",1,DecodeWT);
   AddInstTable(InstTable,"WRBL24",2,DecodeWT);
   AddInstTable(InstTable,"WRBL23",3,DecodeWT);
   AddInstTable(InstTable,"WRBEL8",4,DecodeWT);
   AddInstTable(InstTable,"WRBL8E",5,DecodeWT);
   AddInstTable(InstTable,"WRBXCH",6,DecodeWT);
   AddInstTable(InstTable,"WRBBRV",7,DecodeWT);

   AddInstTable(InstTable,"TRNORM",2,DecodeNF);
   AddInstTable(InstTable,"RDNORM",4,DecodeNF);
   AddInstTable(InstTable,"FLTFIX",6,DecodeNF);
   AddInstTable(InstTable,"FIXMA" ,7,DecodeNF);

   AddInstTable(InstTable,"BWRL24",1,DecodeWI);
   AddInstTable(InstTable,"BWRORD",2,DecodeWI);

   AddInstTable(InstTable,"SPCPSW0",1,DecodeFIS);
   AddInstTable(InstTable,"SPCPSW1",2,DecodeFIS);
   AddInstTable(InstTable,"CLRPSW0",4,DecodeFIS);
   AddInstTable(InstTable,"CLRPSW1",5,DecodeFIS);
   AddInstTable(InstTable,"CLRPSW" ,6,DecodeFIS);

   AddInstTable(InstTable,"SPIE",1,DecodeFD);
   AddInstTable(InstTable,"IESP",2,DecodeFD); 

   AddInstTable(InstTable,"SETSVL",0,DecodeSHV);
   AddInstTable(InstTable,"SETSVR",1,DecodeSHV);

   AddInstTable(InstTable,"SPCRA",0,DecodeRPS);
   AddInstTable(InstTable,"JBLK" ,0,DecodeNAL);

   JmpOrders=(FixedOrder*) malloc(sizeof(FixedOrder)*JmpOrderCnt); InstrZ=0;
   AddJmp("JMP"   ,0x00); AddJmp("CALL"  ,0x01); AddJmp("RET"   ,0x02);
   AddJmp("JNZRP" ,0x03); AddJmp("JZ0"   ,0x04); AddJmp("JNZ0"  ,0x05);
   AddJmp("JZ1"   ,0x06); AddJmp("JNZ1"  ,0x07); AddJmp("JC0"   ,0x08);
   AddJmp("JNC0"  ,0x09); AddJmp("JC1"   ,0x0a); AddJmp("JNC1"  ,0x0b);
   AddJmp("JS0"   ,0x0c); AddJmp("JNS0"  ,0x0d); AddJmp("JS1"   ,0x0e);
   AddJmp("JNS1"  ,0x0f); AddJmp("JV0"   ,0x10); AddJmp("JNV0"  ,0x11);
   AddJmp("JV1"   ,0x12); AddJmp("JNV1"  ,0x13); AddJmp("JEV0"  ,0x14);
   AddJmp("JEV1"  ,0x15); AddJmp("JNFSI" ,0x16); AddJmp("JNESO" ,0x17);
   AddJmp("JIP0"  ,0x18); AddJmp("JIP1"  ,0x19); AddJmp("JNZIX0",0x1a);
   AddJmp("JNZIX1",0x1b); AddJmp("JNZBP0",0x1c); AddJmp("JNZBP1",0x1d);
   AddJmp("JRDY"  ,0x1e); AddJmp("JRQM"  ,0x1f);

   ALU1Orders=(FixedOrder*) malloc(sizeof(FixedOrder)*ALU1OrderCnt); InstrZ=0;
   AddALU1("INC"  ,0x01); AddALU1("DEC"  ,0x02); AddALU1("ABS"  ,0x03);
   AddALU1("NOT"  ,0x04); AddALU1("NEG"  ,0x05); AddALU1("SHLC" ,0x06);
   AddALU1("SHRC" ,0x07); AddALU1("ROL"  ,0x08); AddALU1("ROR"  ,0x09);
   AddALU1("SHLM" ,0x0a); AddALU1("SHRM" ,0x0b); AddALU1("SHRAM",0x0c);
   AddALU1("CLR"  ,0x0d); AddALU1("NORM" ,0x0e); AddALU1("CVT"  ,0x0f);

   ALU2Orders=(FixedOrder*) malloc(sizeof(FixedOrder)*ALU2OrderCnt); InstrZ=0;
   AddALU2("ADD"  ,0x10); AddALU2("SUB"  ,0x11); AddALU2("ADDC" ,0x12);
   AddALU2("SUBC" ,0x13); AddALU2("CMP"  ,0x14); AddALU2("AND"  ,0x15);
   AddALU2("OR"   ,0x16); AddALU2("XOR"  ,0x17); AddALU2("ADDF" ,0x18);
   AddALU2("SUBF" ,0x19);

   SrcRegs=(Register*) malloc(sizeof(Register)*SrcRegCnt); InstrZ=0;
   AddSrcReg("NON" ,0x00); AddSrcReg("RP"  ,0x01); AddSrcReg("PSW0" ,0x02);
   AddSrcReg("PSW1",0x03); AddSrcReg("SVR" ,0x04); AddSrcReg("SR"   ,0x05);
   AddSrcReg("LC"  ,0x06); AddSrcReg("STX" ,0x07); AddSrcReg("M"    ,0x08);
   AddSrcReg("ML"  ,0x09); AddSrcReg("ROM" ,0x0a); AddSrcReg("TR"   ,0x0b);
   AddSrcReg("AR"  ,0x0c); AddSrcReg("SI"  ,0x0d); AddSrcReg("DR"   ,0x0e);
   AddSrcReg("DRS" ,0x0f); AddSrcReg("WR0" ,0x10); AddSrcReg("WR1"  ,0x11);
   AddSrcReg("WR2" ,0x12); AddSrcReg("WR3" ,0x13); AddSrcReg("WR4"  ,0x14);
   AddSrcReg("WR5" ,0x15); AddSrcReg("WR6" ,0x16); AddSrcReg("WR7"  ,0x17);
   AddSrcReg("RAM0",0x18); AddSrcReg("RAM1",0x19); AddSrcReg("BP0"  ,0x1a);
   AddSrcReg("BP1" ,0x1b); AddSrcReg("IX0" ,0x1c); AddSrcReg("IX1"  ,0x1d);
   AddSrcReg("K"   ,0x1e); AddSrcReg("L"   ,0x1f);

   ALUSrcRegs=(Register*) malloc(sizeof(Register)*ALUSrcRegCnt); InstrZ=0;
   AddALUSrcReg("IB"  ,0x00); AddALUSrcReg("M"   ,0x01);
   AddALUSrcReg("RAM0",0x02); AddALUSrcReg("RAM1",0x03);

   DestRegs=(Register*) malloc(sizeof(Register)*DestRegCnt); InstrZ=0;
   AddDestReg("NON" ,0x00); AddDestReg("RP"  ,0x01); AddDestReg("PSW0" ,0x02);
   AddDestReg("PSW1",0x03); AddDestReg("SVR" ,0x04); AddDestReg("SR"   ,0x05);
   AddDestReg("LC"  ,0x06); AddDestReg("STK" ,0x07); AddDestReg("LKR0" ,0x08);
   AddDestReg("KLR1",0x09); AddDestReg("TRE" ,0x0a); AddDestReg("TR"   ,0x0b);
   AddDestReg("AR"  ,0x0c); AddDestReg("SO"  ,0x0d); AddDestReg("DR"   ,0x0e);
   AddDestReg("DRS" ,0x0f); AddDestReg("WR0" ,0x10); AddDestReg("WR1"  ,0x11);
   AddDestReg("WR2" ,0x12); AddDestReg("WR3" ,0x13); AddDestReg("WR4"  ,0x14);
   AddDestReg("WR5" ,0x15); AddDestReg("WR6" ,0x16); AddDestReg("WR7"  ,0x17);
   AddDestReg("RAM0",0x18); AddDestReg("RAM1",0x19); AddDestReg("BP0"  ,0x1a);
   AddDestReg("BP1" ,0x1b); AddDestReg("IX0" ,0x1c); AddDestReg("IX1"  ,0x1d);
   AddDestReg("K"   ,0x1e); AddDestReg("L"   ,0x1f);

   InstrComps=(LongWord*) malloc(sizeof(LongWord)*InstrCnt);
   InstrDefs=(LongWord*) malloc(sizeof(LongWord)*InstrCnt);
   for (InstrZ=0; InstrZ<InstrCnt; InstrDefs[InstrZ++]=0xffffffff);
   InstrDefs[InstrALU]=0;
   InstrDefs[InstrMove]=0;
   InstrDefs[InstrBM]=0;
   InstrDefs[InstrEM]=0;
   InstrDefs[InstrDP0]=0;
   InstrDefs[InstrDP1]=0;
   InstrDefs[InstrEA]=0;
   InstrDefs[InstrFC]=0;
   InstrDefs[InstrFD]=0;
   InstrDefs[InstrFIS]=0;
   InstrDefs[InstrL]=0;
   InstrDefs[InstrM0]=0; 
   InstrDefs[InstrM1]=0;
   InstrDefs[InstrNF]=0;
   InstrDefs[InstrRP]=0;
   InstrDefs[InstrRW]=0;
   InstrDefs[InstrWI]=0;
   InstrDefs[InstrWT]=0;
END

        static void DeinitFields(void)
BEGIN
   DestroyInstTable(InstTable);

   free(SrcRegs); free(ALUSrcRegs); free(DestRegs);

   free(JmpOrders); free(ALU1Orders); free(ALU2Orders);

   free(InstrComps); free(InstrDefs);
END

/*---------------------------------------------------------------------------*/
/* Callbacks */

        static void MakeCode_77230(void)
BEGIN
   int z,z2;
   LongWord Diff;

   /* Nullanweisung */

   if ((Memo("")) AND (*AttrPart=='\0') AND (ArgCnt==0)) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   /* solange dekodieren, bis keine Operanden mehr da oder Fehler */

   Error=False; InstrMask=0;
   memset(InstrComps,0,sizeof(LongWord)*InstrCnt);
   do
    BEGIN
     if (NOT LookupInstTable(InstTable,OpPart))
      BEGIN
       WrXError(1200,OpPart); Error=True;
      END
    END
   while ((NOT Error) AND (*OpPart!='\0'));

   /* passende Verknuepfung suchen */

   if (NOT Error)
    BEGIN
     for (z=0; z<CaseCnt; z++)
      BEGIN
       /* Bits ermitteln, die nur in einer Maske vorhanden sind */

       Diff=InstrMask^CaseMasks[z];

       /* Fall nur moeglich, wenn Bits im aktuellen Fall gesetzt sind, die
          der Fall nicht hat */

       if ((Diff&InstrMask)==0)
        BEGIN
         /* ist irgendein Feld unbenutzt, fuer das wir keinen Default haben? */

         for (z2=0; z2<InstrCnt; z2++)
          if (((Diff&(1l<<z2))!=0) AND (InstrDefs[z2]==0xffffffff)) break;
         if (z2==InstrCnt) break;
        END
      END

     switch (z)
      BEGIN
       case 0: /* nur LDI */
        DAsmCode[0]=0xe0000000+InstrComps[InstrLDI];
        CodeLen=1;
        break;
       case 1: /* JMP+MOV */
        DAsmCode[0]=0xd0000000+(InstrComps[InstrBranch]<<10)
                              +InstrComps[InstrMove];
        CodeLen=1;
        break;
       case 2: /* ALU+MOV+M0+M1+DP0+DP1 */
        DAsmCode[0]=(InstrComps[InstrALU]<<10)+InstrComps[InstrMove]
                   +(InstrComps[InstrDP1]<<15)+(InstrComps[InstrDP0]<<18)
                   +(InstrComps[InstrM1]<<21)+(InstrComps[InstrM0]<<23);
        CodeLen=1;
        break;
       case 3: /* ALU+MOV+EA+DP0+DP1 */
        DAsmCode[0]=(InstrComps[InstrALU]<<10)+InstrComps[InstrMove]
                   +(InstrComps[InstrDP1]<<15)+(InstrComps[InstrDP0]<<18)
                   +(InstrComps[InstrEA]<<21)+0x02000000;
        CodeLen=1;
        break;
       case 4: /* ALU+MOV+RP+M0+DP0+FC */
        DAsmCode[0]=(InstrComps[InstrALU]<<10)+InstrComps[InstrMove]
                   +(InstrComps[InstrRP]<<21)+(InstrComps[InstrFC]<<15)
                   +(InstrComps[InstrM0]<<19)+(InstrComps[InstrDP0]<<16)
                   +0x02800000;
        CodeLen=1;
        break;
       case 5: /* ALU+MOV+RP+M1+DP1+FC */
        DAsmCode[0]=(InstrComps[InstrALU]<<10)+InstrComps[InstrMove]
                   +(InstrComps[InstrRP]<<21)+(InstrComps[InstrFC]<<15)
                   +(InstrComps[InstrM1]<<19)+(InstrComps[InstrDP1]<<16)
                   +0x03000000;
        CodeLen=1;
        break;
       case 6: /* ALU+MOV+RP+M0+M1+L+FC */
        DAsmCode[0]=(InstrComps[InstrALU]<<10)+InstrComps[InstrMove]
                   +(InstrComps[InstrRP]<<21)+(InstrComps[InstrL]<<16)
                   +(InstrComps[InstrM0]<<19)+(InstrComps[InstrM1]<<17)
                   +(InstrComps[InstrFC]<<15)+0x03800000;
        CodeLen=1;
        break;
       case 7: /* ALU+MOV+BASE0+BASE1+FC */
        DAsmCode[0]=(InstrComps[InstrALU]<<10)+InstrComps[InstrMove]
                   +(InstrComps[InstrBASE0]<<19)+(InstrComps[InstrBASE1]<<16)
                   +(InstrComps[InstrFC]<<15)+0x04000000;
        CodeLen=1;
        break;
       case 8: /* ALU+MOV+RPC+L+FC */
        DAsmCode[0]=(InstrComps[InstrALU]<<10)+InstrComps[InstrMove]
                   +(InstrComps[InstrRPC]<<18)+(InstrComps[InstrL]<<16)
                   +(InstrComps[InstrFC]<<15)+0x04400000;
        CodeLen=1;
        break;
       case 9: /* ALU+MOV+P2+P3+EM+BM+L+FC */
        DAsmCode[0]=(InstrComps[InstrALU]<<10)+InstrComps[InstrMove]
                   +(InstrComps[InstrP2]<<20)+(InstrComps[InstrP3]<<21)
                   +(InstrComps[InstrEM]<<19)+(InstrComps[InstrBM]<<17)
                   +(InstrComps[InstrL]<<16)+(InstrComps[InstrFC]<<15)
                   +0x04800000;
        CodeLen=1;
        break;
       case 10: /* ALU+MOV+RW+L+FC */
        DAsmCode[0]=(InstrComps[InstrALU]<<10)+InstrComps[InstrMove]
                   +(InstrComps[InstrRW]<<20)+(InstrComps[InstrL]<<16)
                   +(InstrComps[InstrFC]<<15)+0x04c00000;
        CodeLen=1;
        break;
       case 11: /* ALU+MOV+WT+L+FC */
        DAsmCode[0]=(InstrComps[InstrALU]<<10)+InstrComps[InstrMove]
                   +(InstrComps[InstrWT]<<19)+(InstrComps[InstrL]<<16)
                   +(InstrComps[InstrFC]<<15)+0x05000000;
        CodeLen=1;
        break;
       case 12: /* ALU+MOV+NF+WI+L+FC */
        DAsmCode[0]=(InstrComps[InstrALU]<<10)+InstrComps[InstrMove]
                   +(InstrComps[InstrNF]<<19)+(InstrComps[InstrWI]<<17)
                   +(InstrComps[InstrL]<<16)+(InstrComps[InstrFC]<<15)
                   +0x05400000;
        CodeLen=1;
        break;
       case 13: /* ALU+MOV+FIS+FD+L */
        DAsmCode[0]=(InstrComps[InstrALU]<<10)+InstrComps[InstrMove]
                   +(InstrComps[InstrFIS]<<19)+(InstrComps[InstrFD]<<17)
                   +(InstrComps[InstrL]<<16)+0x05800000;
        CodeLen=1;
        break;
       case 14: /* ALU+MOV+SHV */
        DAsmCode[0]=(InstrComps[InstrALU]<<10)+InstrComps[InstrMove]
                   +(InstrComps[InstrSHV]<<15)+0x05c00000;
        CodeLen=1;
        break;
       case 15: /* ALU+MOV+RPS */
        DAsmCode[0]=(InstrComps[InstrALU]<<10)+InstrComps[InstrMove]
                   +(InstrComps[InstrRPS]<<15)+0x06000000;
        CodeLen=1;
        break;
       case 16: /* ALU+MOV+NAL */
        DAsmCode[0]=(InstrComps[InstrALU]<<10)+InstrComps[InstrMove]
                   +(InstrComps[InstrNAL]<<15)+0x07000000;
        CodeLen=1;
        break;
       default:
        WrError(1355);
      END
    END
END

        static Boolean IsDef_77230(void)
BEGIN
   return False;
END

        static void SwitchFrom_77230(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_77230(void)
BEGIN
   PFamilyDescr FoundDescr;

   FoundDescr=FindFamilyByName("77230");

   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=False;
   PCSymbol="$"; HeaderID=FoundDescr->Id; NOPCode=0x00000000;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode)|(1<<SegXData)|(1<<SegYData)|(1<<SegRData);
   Grans[SegCode ]=4; ListGrans[SegCode ]=4; SegInits[SegCode ]=0;
   SegLimits[SegCode ] = 0x1fff;
   Grans[SegXData]=4; ListGrans[SegXData]=4; SegInits[SegXData]=0;
   SegLimits[SegXData] = 0x1ff;
   Grans[SegYData]=4; ListGrans[SegYData]=4; SegInits[SegYData]=0;
   SegLimits[SegYData] = 0x1ff;  
   Grans[SegRData]=4; ListGrans[SegRData]=4; SegInits[SegRData]=0;
   SegLimits[SegRData] = 0x3ff;  

   MakeCode=MakeCode_77230; IsDef=IsDef_77230;
   SwitchFrom=SwitchFrom_77230;

   InitFields();
END

/*---------------------------------------------------------------------------*/
/* Initialisierung */

        void code77230_init(void)
BEGIN  
   CPU77230=AddCPU("77230",SwitchTo_77230);
END
