/* code8x30x.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator Signetics 8X30x                                             */
/*                                                                           */
/* Historie: 25.6.1997 Grundsteinlegung                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "chunks.h"
#include "bpemu.h"
#include "stringutil.h"

#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"

/*****************************************************************************/

#define AriOrderCnt 4

typedef struct
         {
          char *Name;
          Word Code; 
         } FixedOrder;

static CPUVar CPU8x300,CPU8x305;
static FixedOrder *AriOrders;

/*-------------------------------------------------------------------------*/

static int InstrZ;

        static void AddAri(char *NName, Word NCode)
BEGIN
   if (InstrZ>=AriOrderCnt) exit(255);
   AriOrders[InstrZ].Name=NName;
   AriOrders[InstrZ++].Code=NCode;
END

	static void InitFields(void)
BEGIN
   AriOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*AriOrderCnt); InstrZ=0;
   AddAri("MOVE",0); AddAri("ADD",1); AddAri("AND",2); AddAri("XOR",3);
END

	static void DeinitFields(void)
BEGIN
   free(AriOrders);
END

/*-------------------------------------------------------------------------*/

        static Boolean DecodeReg(char *Asc, Word *Erg, ShortInt *ErgLen)
BEGIN
   Boolean OK;
   Word Acc;
   LongInt Adr;
   char *z;

   *ErgLen=(-1);

   if (strcasecmp(Asc,"AUX")==0)
    BEGIN
     *Erg=0; return True;
    END

   if (strcasecmp(Asc,"OVF")==0)
    BEGIN
     *Erg=8; return True;
    END

   if (strcasecmp(Asc,"IVL")==0)
    BEGIN
     *Erg=7; return True;
    END

   if (strcasecmp(Asc,"IVR")==0)
    BEGIN
     *Erg=15; return True;
    END

   if ((toupper(*Asc)=='R') AND (strlen(Asc)>1) AND (strlen(Asc)<4))
    BEGIN
     Acc=0; OK=True;
     for (z=Asc+1; *z!='\0'; z++)
      if (OK)
       if ((*z<'0') OR (*z>'7')) OK=False;
       else Acc=(Acc << 3)+(*z-'0');
     if ((OK) AND (Acc<32))
      BEGIN
       if ((MomCPU==CPU8x300) AND (Acc>9) AND (Acc<15))
        BEGIN
         WrXError(1445,Asc); return False;
        END
       else *Erg=Acc;
       return True;
      END
    END

   if ((strlen(Asc)==4) AND (strncasecmp(Asc+1,"IV",2)==0) AND (Asc[3]>='0') AND (Asc[3]<='7'))
    if (toupper(*Asc)=='L')
     BEGIN
      *Erg=Asc[3]-'0'+0x10; return True;
     END
    else if (toupper(*Asc)=='R')
     BEGIN
      *Erg=Asc[3]-'0'+0x18; return True;
     END

   /* IV - Objekte */

   Adr=EvalIntExpression(Asc,UInt24,&OK);
   if (OK)
    BEGIN
     *ErgLen=Adr & 7;
     *Erg=0x10+((Adr & 0x10) >> 1)+((Adr & 0x700) >> 8);
     return True;
    END
   else return False;
END

        static char *HasDisp(char *Asc)
BEGIN
   Integer Lev;
   char *z;
   int l=strlen(Asc);

   if (Asc[l-1]==')')
    BEGIN
     z=Asc+l-2; Lev=0;
     while ((z>=Asc) AND (Lev!=-1))
      BEGIN
       switch (*z)
        BEGIN
         case '(': Lev--; break;
         case ')': Lev++; break;
        END
       if (Lev!=-1) z--;
      END
     if (Lev!=-1)
      BEGIN
       WrError(1300); return Nil;
      END
    END
   else z=Nil;

   return z;
END

        static Boolean GetLen(char *Asc, Word *Erg)
BEGIN
   Boolean OK;

   FirstPassUnknown=False;
   *Erg=EvalIntExpression(Asc,UInt4,&OK); if (NOT OK) return False;
   if (FirstPassUnknown) *Erg=8;
   if (NOT ChkRange(*Erg,1,8)) return False;
   *Erg&=7; return True;
END

/*-------------------------------------------------------------------------*/

/* Symbol: 00AA0ORL */

	static Boolean DecodePseudo(void)
BEGIN
   LongInt Adr,Ofs,Erg;
   Word Len;
   Boolean OK;

   if ((Memo("LIV")) OR (Memo("RIV")))
    BEGIN
     Erg=0x10*Ord(Memo("RIV"));
     if (ArgCnt!=3) WrError(1110);
     else
      BEGIN
       Adr=EvalIntExpression(ArgStr[1],UInt8,&OK);
       if (OK)
        BEGIN
         Ofs=EvalIntExpression(ArgStr[2],UInt3,&OK);
         if (OK)
          if (GetLen(ArgStr[3],&Len))
           BEGIN
            PushLocHandle(-1);
            EnterIntSymbol(LabPart,Erg+(Adr << 16)+(Ofs << 8)+(Len & 7),SegNone,False);
            PopLocHandle();
           END
        END
      END
     return True;
    END

   return False;
END

        static void MakeCode_8x30X(void)
BEGIN
   Boolean OK;
   Word SrcReg,DestReg;
   ShortInt SrcLen,DestLen;
   LongInt Op;
   Word Rot,Adr;
   Integer z;
   char *p;
   String tmp;

   CodeLen=0; DontPrint=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   /* eingebaute Makros */

   if (Memo("NOP"))     /* NOP = MOVE AUX,AUX */
    BEGIN
     if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       WAsmCode[0]=0x0000; CodeLen=1;
      END
     return;
    END

   if (Memo("HALT"))      /* HALT = JMP * */
    BEGIN
     if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       WAsmCode[0]=0xe000+(EProgCounter() & 0x1fff); CodeLen=1;
      END
     return;
    END

   if ((Memo("XML")) OR (Memo("XMR")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU8x305) WrError(1500);
     else
      BEGIN
       Adr=EvalIntExpression(ArgStr[1],Int8,&OK);
       if (OK)
        BEGIN
         WAsmCode[0]=0xca00+(Ord(Memo("XER")) << 8)+(Adr & 0xff);
         CodeLen=1;
        END
      END
     return;
    END

   /* Datentransfer */

   if (Memo("SEL"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       Op=EvalIntExpression(ArgStr[1],UInt24,&OK);
       if (OK)
        BEGIN
         WAsmCode[0]=0xc700+((Op & 0x10) << 7)+((Op >> 16) & 0xff);
         CodeLen=1;
        END
      END
     return;
    END

   if (Memo("XMIT"))
    BEGIN
     if ((ArgCnt!=2) AND (ArgCnt!=3)) WrError(1110);
     else if (DecodeReg(ArgStr[2],&SrcReg,&SrcLen))
      if (SrcReg<16)
       BEGIN
        if (ArgCnt!=2) WrError(1110);
        else
         BEGIN
          Adr=EvalIntExpression(ArgStr[1],Int8,&OK);
          if (OK)
           BEGIN
            WAsmCode[0]=0xc000+(SrcReg << 8)+(Adr & 0xff);
            CodeLen=1;
           END
         END
       END
      else
       BEGIN
        if (ArgCnt==2)
	 BEGIN
	  Rot=0xffff; OK=True;
         END
        else OK=GetLen(ArgStr[3],&Rot);
        if (OK)
         BEGIN
          if (Rot==0xffff)
           Rot=(SrcLen==-1) ? 0 : SrcLen;
          if ((SrcLen!=-1) AND (Rot!=SrcLen)) WrError(1131);
          else
           BEGIN
            Adr=EvalIntExpression(ArgStr[1],Int5,&OK);
            if (OK)
             BEGIN
              WAsmCode[0]=0xc000+(SrcReg << 8)+(Rot << 5)+(Adr & 0x1f);
              CodeLen=1;
             END
           END
         END
       END
     return;
    END

   /* Arithmetik */

   for (z=0; z<AriOrderCnt; z++)
    if (Memo(AriOrders[z].Name))
     BEGIN
      if ((ArgCnt!=2) AND (ArgCnt!=3)) WrError(1110);
      else if (DecodeReg(ArgStr[ArgCnt],&DestReg,&DestLen))
       if (DestReg<16)         /* Ziel Register */
        BEGIN
         if (ArgCnt==2)        /* wenn nur zwei Operanden und Ziel Register... */
          BEGIN
           p=HasDisp(ArgStr[1]); /* kann eine Rotation dabei sein */
           if (p!=Nil)
            BEGIN                 /* jau! */
             strcpy(tmp,p+1); tmp[strlen(tmp)-1]='\0';
             Rot=EvalIntExpression(tmp,UInt3,&OK);
             if (OK)
              BEGIN
               *p='\0';
               if (DecodeReg(ArgStr[1],&SrcReg,&SrcLen))
                if (SrcReg>=16) WrXError(1445,ArgStr[1]);
                else
                 BEGIN
                  WAsmCode[0]=(AriOrders[z].Code << 13)+(SrcReg << 8)+(Rot << 5)+DestReg;
                  CodeLen=1;
                 END
              END
            END
           else                   /* noi! */
            BEGIN
             if (DecodeReg(ArgStr[1],&SrcReg,&SrcLen))
              BEGIN
               WAsmCode[0]=(AriOrders[z].Code << 13)+(SrcReg << 8)+DestReg;
               if ((SrcReg>=16) AND (SrcLen!=-1)) WAsmCode[0]+=SrcLen << 5;
               CodeLen=1;
              END
            END
          END
         else                     /* 3 Operanden --> Quelle ist I/O */
          BEGIN
           if (GetLen(ArgStr[2],&Rot))
            if (DecodeReg(ArgStr[1],&SrcReg,&SrcLen))
             if (SrcReg<16) WrXError(1445,ArgStr[1]);
             else if ((SrcLen!=-1) AND (SrcLen!=Rot)) WrError(1131);
             else
              BEGIN
               WAsmCode[0]=(AriOrders[z].Code << 13)+(SrcReg << 8)+(Rot << 5)+DestReg;
               CodeLen=1;
              END
          END
        END
       else                       /* Ziel I/O */
        BEGIN
         if (ArgCnt==2)           /* 2 Argumente: Laenge=Laenge Ziel */
          BEGIN
           Rot=DestLen; OK=True;
          END
         else                     /* 3 Argumente: Laenge=Laenge Ziel+Angabe */
          BEGIN
           OK=GetLen(ArgStr[2],&Rot);
           if (OK)
            BEGIN
             if (FirstPassUnknown) Rot=DestLen;
             if (DestLen==-1) DestLen=Rot;
             OK=Rot==DestLen;
             if (NOT OK) WrError(1131);
            END
          END
         if (OK)
          if (DecodeReg(ArgStr[1],&SrcReg,&SrcLen))
           BEGIN
            if ((Rot==0xffff))
             Rot=((SrcLen==-1)) ? 0 : SrcLen;
            if ((DestReg>=16) AND (SrcLen!=-1) AND (SrcLen!=Rot)) WrError(1131);
            else
             BEGIN
              WAsmCode[0]=(AriOrders[z].Code << 13)+(SrcReg << 8)+(Rot << 5)+DestReg;
              CodeLen=1;
             END
           END
        END
      return;
     END

   if (Memo("XEC"))
    BEGIN
     if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
     else
      BEGIN
       p=HasDisp(ArgStr[1]);
       if (p==Nil) WrError(1350);
       else 
        BEGIN
         strcpy(tmp,p+1); tmp[strlen(tmp)-1]='\0';
         if (DecodeReg(tmp,&SrcReg,&SrcLen))
          BEGIN
           *p='\0';
           if (SrcReg<16)
            BEGIN
             if (ArgCnt!=1) WrError(1110);
             else
              BEGIN
               WAsmCode[0]=EvalIntExpression(ArgStr[1],UInt8,&OK);
               if (OK)
                BEGIN
                 WAsmCode[0]+=0x8000+(SrcReg << 8);
                 CodeLen=1;
                END
              END
            END
           else
            BEGIN
             if (ArgCnt==1)
	      BEGIN
	       Rot=0xffff; OK=True;
              END
             else OK=GetLen(ArgStr[2],&Rot);
             if (OK)
              BEGIN
               if (Rot==0xffff)
                Rot=(SrcLen==-1) ? 0 : SrcLen; 
               if ((SrcLen!=-1) AND (Rot!=SrcLen)) WrError(1131);
               else
                BEGIN
                 WAsmCode[0]=EvalIntExpression(ArgStr[1],UInt5,&OK);
                 if (OK)
                  BEGIN
                   WAsmCode[0]+=0x8000+(SrcReg << 8)+(Rot << 5);
                   CodeLen=1;
                  END
                END
              END
            END
          END
        END
      END
     return;
    END

   /* Spruenge */

   if (Memo("JMP"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       WAsmCode[0]=EvalIntExpression(ArgStr[1],UInt13,&OK);
       if (OK)
        BEGIN
         WAsmCode[0]+=0xe000; CodeLen=1;
        END
      END
     return;
    END

   if (Memo("NZT"))
    BEGIN
     if ((ArgCnt!=2) AND (ArgCnt!=3)) WrError(1110);
     else if (DecodeReg(ArgStr[1],&SrcReg,&SrcLen))
      if (SrcReg<16)
       BEGIN
        if (ArgCnt!=2) WrError(1110);
        else
         BEGIN
          Adr=EvalIntExpression(ArgStr[2],UInt13,&OK);
          if (OK)
           if ((NOT SymbolQuestionable) AND ((Adr >> 8)!=(EProgCounter() >> 8))) WrError(1910);
           else
            BEGIN
             WAsmCode[0]=0xa000+(SrcReg << 8)+(Adr & 0xff);
             CodeLen=1;
            END
         END
       END
      else
       BEGIN
        if (ArgCnt==2)
	 BEGIN
	  Rot=0xffff; OK=True;
         END
        else OK=GetLen(ArgStr[2],&Rot);
        if (OK)
         BEGIN
          if (Rot==0xffff)
           Rot=(SrcLen==-1) ? 0 : SrcLen;
          if ((SrcLen!=-1) AND (Rot!=SrcLen)) WrError(1131);
          else
           BEGIN
            Adr=EvalIntExpression(ArgStr[ArgCnt],UInt13,&OK);
            if (OK)
             if ((NOT SymbolQuestionable) AND ((Adr >> 5)!=(EProgCounter() >> 5))) WrError(1910);
             else
              BEGIN
               WAsmCode[0]=0xa000+(SrcReg << 8)+(Rot << 5)+(Adr & 0x1f);
               CodeLen=1;
              END
           END
         END
       END
     return;
    END;

   WrXError(1200,OpPart);
END

        static Boolean ChkPC_8x30X(void)
BEGIN
   switch (ActPC)
    BEGIN
     case SegCode  : return (ProgCounter() <=0x1fff);
     default       : return False;
    END
END

        static Boolean IsDef_8x30X(void)
BEGIN
   return (Memo("LIV") OR Memo("RIV"));
END

        static void SwitchFrom_8x30X()
BEGIN
   DeinitFields();
END

        static void SwitchTo_8x30X(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeMoto; SetIsOccupied=False;

   PCSymbol="*"; HeaderID=0x3a; NOPCode=0x0000;
   DivideChars=","; HasAttrs=False;

   ValidSegs=1<<SegCode;
   Grans[SegCode]=2; ListGrans[SegCode]=2; SegInits[SegCode]=0;

   MakeCode=MakeCode_8x30X; ChkPC=ChkPC_8x30X; IsDef=IsDef_8x30X;
   SwitchFrom=SwitchFrom_8x30X; InitFields();
END

	void code8x30x_init(void)
BEGIN
   CPU8x300=AddCPU("8x300",SwitchTo_8x30X);
   CPU8x305=AddCPU("8x305",SwitchTo_8x30X);
END

