/* codekcpsm.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator xilinx kcpsm                                                */
/*                                                                           */
/* Historie: 27.02.2003 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/
/* $Id: codekcpsm.c,v 1.2 2004/05/29 11:33:03 alfred Exp $                   */
/*****************************************************************************
 * $Log: codekcpsm.c,v $
 * Revision 1.2  2004/05/29 11:33:03  alfred
 * - relocated DecodeIntelPseudo() into own module
 *
 * Revision 1.1  2003/11/06 02:49:23  alfred
 * - recreated
 *
 * Revision 1.4  2003/03/16 18:53:43  alfred
 * - created 807x
 *
 * Revision 1.3  2003/03/15 13:36:39  alfred
 * - set output to big endian
 *
 * Revision 1.2  2003/03/14 21:36:02  alfred
 * - cleanups for KCPSM
 *
 * Revision 1.1  2003/03/09 10:28:28  alfred
 * - added KCPSM
 *
 *****************************************************************************/

#include "stdinc.h"
#include "stdio.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "strutil.h"
#include "bpemu.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "intpseudo.h"
#include "codevars.h"
#include "headids.h"

#undef DEBUG_PRINTF

typedef struct
         {
          char *Name;
          Word Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          Word Code;
         } ALU2Order;

typedef struct
         {
          char *Name;
          Word Code;
         } IOop;

typedef struct
         {
          char *Name;
          Word Code;
         } Condition;


#define WorkOfs 0xe0


#define FixedOrderCnt 5
#define ALU2OrderCnt 7
#define ALU1OrderCnt 10
#define IOopCnt 2
#define CondCnt 5

#define ModNone  (-1)
#define ModWReg   0
#define MModWReg   (1 << ModWReg)
#define ModAbs    1
#define MModAbs    (1 << ModAbs)
#define ModImm    4
#define MModImm    (1 << ModImm)
#define ModIRReg  6
#define MModIRReg  (1 << ModIRReg)
#define ModInd    7
#define MModInd    (1 << ModInd)

static ShortInt AdrType;
static Word AdrMode,AdrIndex;

static FixedOrder *FixedOrders;
static ALU2Order *ALU2Orders;
static FixedOrder *ALU1Orders;
static IOop *IOops;
static Condition *Conditions;
static int TrueCond;

static CPUVar CPUKCPSM;

/*--------------------------------------------------------------------------*/

	static void AddFixed(char *NName, Word NCode)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
END

        static void AddALU2(char *NName, Word NCode)
BEGIN
   if (InstrZ>=ALU2OrderCnt) exit(255);
   ALU2Orders[InstrZ].Name=NName;
   ALU2Orders[InstrZ++].Code=NCode;
END

        static void AddALU1(char *NName, Word NCode)
BEGIN
   if (InstrZ>=ALU1OrderCnt) exit(255);
   ALU1Orders[InstrZ].Name=NName;
   ALU1Orders[InstrZ++].Code=NCode;
END

        static void AddIOop( Char *NName, Word NCode )
BEGIN
   if (InstrZ>=IOopCnt) exit(255);
   IOops[InstrZ].Name=NName;
   IOops[InstrZ++].Code=NCode;
END

        static void AddCondition(char *NName, Word NCode)
BEGIN
   if (InstrZ>=CondCnt) exit(255);
   Conditions[InstrZ].Name=NName;
   Conditions[InstrZ++].Code=NCode;
END
   
	static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("EI"     , 0x8030);  AddFixed("DI"     , 0x8010);
   AddFixed("RETIE"  , 0x80f0);  AddFixed("RETID"  , 0x80d0);
   AddFixed("NOP"    , 0xc000); //fake

   ALU2Orders=(ALU2Order *) malloc(sizeof(FixedOrder)*ALU2OrderCnt); InstrZ=0;
   AddALU2("ADD"   ,0x04);
   AddALU2("ADDCY" ,0x05);
   AddALU2("SUB"   ,0x06);
   AddALU2("SUBCY" ,0x07);
   AddALU2("OR"    ,0x02);
   AddALU2("AND"   ,0x01);
   AddALU2("XOR"   ,0x03); 

   ALU1Orders=(FixedOrder *) malloc(sizeof(FixedOrder)*ALU1OrderCnt); InstrZ=0;
   AddALU1("SR0" , 0x0e);
   AddALU1("SR1" , 0x0f);
   AddALU1("SRX" , 0x0a);
   AddALU1("SRA" , 0x08);
   AddALU1("RR"  , 0x0c);
   AddALU1("SL0" , 0x06);
   AddALU1("SL1" , 0x07);
   AddALU1("SLX" , 0x04);
   AddALU1("SLA" , 0x00);
   AddALU1("RL"  , 0x02);

   IOops=(IOop *) malloc(sizeof(IOop)*IOopCnt); InstrZ=0;
   AddIOop("INPUT"  , 0xa0);
   AddIOop("OUTPUT" , 0xe0);

   Conditions=(Condition *) malloc(sizeof(Condition)*CondCnt); InstrZ=0;
   TrueCond=InstrZ; AddCondition("T"  , 0);
   AddCondition("C"  , 6); AddCondition("NC" ,7);
   AddCondition("Z"  , 4); AddCondition("NZ" ,5);
END

	static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(ALU2Orders);
   free(ALU1Orders);
   free(Conditions);
END

/*--------------------------------------------------------------------------*/ 

	static Boolean IsWReg(char *Asc, Word *Erg)
BEGIN
   Boolean Err;
   Boolean retValue;
   char *s;

   if (FindRegDef(Asc, &s)) Asc = s;

   if ((strlen(Asc)<2) OR (toupper(*Asc)!='S')) 
     retValue = False;
   else
   BEGIN
     *Erg=ConstLongInt(Asc+1,&Err);
     if (NOT Err) 
       retValue = False;
     else 
       retValue = (*Erg<=15);
    END
#ifdef DEBUG_PRINTF
    fprintf( stderr, "IsWReg: %s %d\n", Asc, retValue );
#endif
    return retValue;
END

	static void ChkAdr(Byte Mask)
BEGIN
   if ((AdrType!=ModNone) AND ((Mask & (1 << AdrType))==0))
    BEGIN
     WrError(1350); AdrType=ModNone;
    END
END	

	static void DecodeAdr(char *Asc, Byte Mask, int Segment)
BEGIN
   Boolean OK;
   char  *p;

   AdrType=ModNone;

   /* immediate ? */

   if (*Asc=='#')
    BEGIN
     AdrMode=EvalIntExpression(Asc+1,UInt8,&OK);
     if (OK) AdrType=ModImm;
     ChkAdr(Mask); return;
    END;

   /* Register ? */

   if (IsWReg(Asc,&AdrMode))
    BEGIN
     AdrType=ModWReg; ChkAdr(Mask); return;
    END

   /* indiziert ? */

   if ((Asc[strlen(Asc)-1]==')') AND (strlen(Asc)>=4))
    BEGIN
     p=Asc+strlen(Asc)-1; *p='\0';
     while ((p>=Asc) AND (*p!='(')) p--;
     if (*p!='(') WrError(1300);
     else if (NOT IsWReg(p+1,&AdrMode)) WrXError(1445,p+1);
     else
      BEGIN
       *p='\0';
       AdrIndex=EvalIntExpression(Asc,UInt8,&OK);
       if (OK)
	BEGIN
	 AdrType=ModInd; ChkSpace(SegData);
	END
       ChkAdr(Mask); return;
      END
    END

   /* einfache direkte Adresse ? */

   AdrMode=EvalIntExpression(Asc,UInt8,&OK);
   if (OK)
    BEGIN
     AdrType=ModAbs;
     if (Segment != SegNone)
       ChkSpace(Segment);
     ChkAdr(Mask); return;
    END

   ChkAdr(Mask); 
END

/*---------------------------------------------------------------------*/

	static Boolean DecodePseudo(void)
BEGIN
   if (Memo("REG"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else AddRegDef(LabPart,ArgStr[1]);
     return True;
    END

   if (Memo("NAMEREG"))
   {
     if (ArgCnt != 2) WrError(1110);
     else AddRegDef(ArgStr[2], ArgStr[1]);
     return True;
   }

   if (Memo("CONSTANT"))
   {
     if (ArgCnt != 2) WrError(1110);
     else
     {
       TempResult t;
       Boolean OK;

       FirstPassUnknown = FALSE;
       t.Contents.Int = EvalIntExpression(ArgStr[2], Int32, &OK);
       if ((OK) && (!FirstPassUnknown))
       {
         t.Typ = TempInt;
         SetListLineVal(&t);
         PushLocHandle(-1);
         EnterIntSymbol(ArgStr[1], t.Contents.Int, SegNone, False);
         PopLocHandle();
       }
     }
     return True;
   }

   return False;
END

	static void MakeCode_KCPSM(void)
BEGIN
   int z;
   Word Save;

   CodeLen=0; DontPrint=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(True)) return;

   /* ohne Argument */
   for (z=0; z<FixedOrderCnt; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else
       BEGIN
        CodeLen=1; 
        WAsmCode[0]=FixedOrders[z].Code;
       END
      return;
     END

   /* Datentransfer */
   if (Memo("LOAD"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModWReg,SegNone);
       switch (AdrType)
        BEGIN
         case ModWReg:
 	  Save=AdrMode;
	  DecodeAdr(ArgStr[2],MModWReg+MModAbs+MModImm,SegNone);
	  switch (AdrType)
           BEGIN
	    case ModWReg:
#ifdef DEBUG_PRINTF
             fprintf( stderr, "LOAD-->ModWReg %d %d\n", AdrMode, Save );
#endif
             WAsmCode[0]=0xc000 | (Save << 8) | ( AdrMode << 4 );
	     CodeLen=1;
	     break;
	    case ModAbs:
#ifdef DEBUG_PRINTF
             fprintf( stderr, "LOAD-->ModAbs %d %d\n", AdrMode, Save );
#endif
	     WAsmCode[0]=0xc000 | (Save << 8) | ( AdrMode << 4 );
	     CodeLen=1;
	     break;
	    case ModImm:
#ifdef DEBUG_PRINTF
             fprintf( stderr, "LOAD-->ModImm %d %d\n", AdrMode, Save );
#endif
	     WAsmCode[0]=(Save << 8) | AdrMode;
	     CodeLen=1;
	     break;
	   END
	  break;
        END
      END
     return;
    END

   /* Arithmetik */
   for (z=0; z<ALU2OrderCnt; z++)
    if (Memo(ALU2Orders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModWReg,SegNone);
        switch (AdrType)
         BEGIN
  	  case ModWReg:
           Save=AdrMode;
           DecodeAdr(ArgStr[2],MModAbs+MModWReg+MModImm,SegNone);
           switch (AdrType)
            BEGIN
   	     case ModWReg:
              WAsmCode[0]=0xc000 | (Save << 8) | ( AdrMode << 4 ) | ALU2Orders[z].Code;
              CodeLen=1;
              break;
             case ModImm:
             case ModAbs:
              WAsmCode[0]=( ALU2Orders[z].Code << 12 ) | (Save << 8) | AdrMode;
              CodeLen=1;
              break;
            END
           break;
         END
       END
      return;
     END

   for (z=0; z<ALU1OrderCnt; z++)
    if (Memo(ALU1Orders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModWReg,SegNone);
        switch (AdrType)
         BEGIN
          case ModWReg:
           WAsmCode[0]=0xd000 | (AdrMode << 8) | ALU1Orders[z].Code; 
           CodeLen=1;
           break;
         END
       END
      return;
     END

  /* Spruenge */

   if (Memo("CALL"))
    BEGIN
     if( (ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
     else
      BEGIN
       if (ArgCnt==1) z=TrueCond;
       else
	BEGIN
	 z=0; NLS_UpString(ArgStr[1]);
	 while ((z<CondCnt) AND (strcmp(Conditions[z].Name,ArgStr[1])!=0)) z++;
	 if (z>=CondCnt) WrError(1360);
	END
       if (z<CondCnt)
        BEGIN
         DecodeAdr(ArgStr[ArgCnt],MModAbs+ModImm,SegCode);
         switch (AdrType)
          BEGIN
           case ModAbs:
 	   case ModImm:
            WAsmCode[0] = 0x8300 | (Conditions[z].Code << 10) | Lo(AdrMode);
	    CodeLen=1;
	    break;
          END
        END
      END
     return;
    END

   if (Memo("JUMP"))
    BEGIN
     if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
     else
      BEGIN
       if (ArgCnt==1) z=TrueCond;
       else
	BEGIN
	 z=0; NLS_UpString(ArgStr[1]);
	 while ((z<CondCnt) AND (strcmp(Conditions[z].Name,ArgStr[1])!=0)) z++;
	 if (z>=CondCnt) WrError(1360);
	END
       if (z<CondCnt)
	BEGIN
	 DecodeAdr(ArgStr[ArgCnt],MModAbs+MModImm,SegCode);
	 switch (AdrType)
          BEGIN
  	   case ModAbs:
	   case ModImm:
            WAsmCode[0] = 0x8100 | (Conditions[z].Code << 10) | Lo(AdrMode);
	    CodeLen=1;
	    break;
	  END
	END
      END
     return;
    END

   if (Memo("RETURN"))
    BEGIN
     if ((ArgCnt!=0) AND (ArgCnt!=1)) WrError(1110);
     else
      BEGIN
       if (ArgCnt==0) z=TrueCond;
       else
	BEGIN
	 z=0; NLS_UpString(ArgStr[1]);
	 while ((z<CondCnt) AND (strcmp(Conditions[z].Name,ArgStr[1])!=0)) z++;
	 if (z>=CondCnt) WrError(1360);
	END
       if (z<CondCnt)
	BEGIN
         WAsmCode[0] = 0x8080 | (Conditions[z].Code << 10);
	 CodeLen=1;
	END
      END
     return;
    END

   /* Sonderbefehle */
    /* INPUT/OUTPUT */
   for (z=0; z<IOopCnt; z++)
    if (Memo(IOops[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModWReg,SegNone);
        switch (AdrType)
         BEGIN
          case ModWReg:
           Save=AdrMode;
           DecodeAdr(ArgStr[2],MModInd+MModImm+MModAbs,SegData);
           switch (AdrType)
            BEGIN
 	     case ModInd:
              WAsmCode[0]=0x1000 | ((IOops[z].Code | Save) << 8) | ( AdrMode << 4 );
              CodeLen=1;
              break;
             case ModImm:
             case ModAbs:
              WAsmCode[0]=((IOops[z].Code | Save) << 8) | AdrMode;
              CodeLen=1;
              break;
            END
           break;
         END
       END
      return;
     END

   if (Memo("RETURNI"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       NLS_UpString(ArgStr[1]);      
       if ( strcmp(ArgStr[1], "ENABLE") == 0 )
        BEGIN
         WAsmCode[0]= 0x80f0;
         CodeLen=1;
        END
       if ( strcmp( ArgStr[1], "DISABLE") == 0 )
        BEGIN
         WAsmCode[0]= 0x80d0;
         CodeLen=1;
        END
      END
     return;
    END

   if (Memo("ENABLE"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       NLS_UpString(ArgStr[1]);      
       if ( strcmp(ArgStr[1], "INTERRUPT") == 0 )
        BEGIN
         WAsmCode[0]= 0x8030;
         CodeLen=1;
        END
      END
     return;
    END

   if (Memo("DISABLE"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       NLS_UpString(ArgStr[1]);      
       if ( strcmp(ArgStr[1], "INTERRUPT") == 0 )
        BEGIN
         WAsmCode[0]= 0x8010;
         CodeLen=1;
        END
      END
     return;
    END

   WrXError(1200,OpPart);
END

	static Boolean IsDef_KCPSM(void)
BEGIN
   return (Memo("REG")); 
END

        static void SwitchFrom_KCPSM(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_KCPSM(void)
BEGIN
   PFamilyDescr FoundDescr;

   FoundDescr = FindFamilyByName("KCPSM");

   TurnWords=True; ConstMode=ConstModeIntel; SetIsOccupied=False;

   PCSymbol="$"; HeaderID=FoundDescr->Id; NOPCode=0xc0; //nop = load s0,s0
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode)|(1<<SegData);
   Grans[SegCode]=2; ListGrans[SegCode]=2; SegInits[SegCode]=0;
   SegLimits[SegCode] = 0xff;
   Grans[SegData]=1; ListGrans[SegData]=1; SegInits[SegData]=0;
   SegLimits[SegData] = 0xff;

   MakeCode=MakeCode_KCPSM; IsDef=IsDef_KCPSM;
   SwitchFrom=SwitchFrom_KCPSM; InitFields();
END

	void codekcpsm_init(void)
BEGIN
   CPUKCPSM=AddCPU("KCPSM",SwitchTo_KCPSM);

   AddCopyright("XILINX KCPSM(Picoblaze)-Generator (C) 2003 Andreas Wassatsch");
END

