/* codeallg.pas */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* von allen Codegeneratoren benutzte Pseudobefehle                          */
/*                                                                           */
/* Historie:  10. 5.1996 Grundsteinlegung                                    */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>


#include "nls.h"
#include "stringutil.h"
#include "stringlists.h"
#include "bpemu.h"
#include "decodecmd.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmmac.h"
#include "asmcode.h"
#include "codepseudo.h"


	void SetCPU(CPUVar NewCPU, Boolean NotPrev)
BEGIN
   LongInt HCPU;
   char *z,*dest;
   Boolean ECPU;
   char s[11];
   PCPUDef Lauf;

   Lauf=FirstCPUDef;
   while ((Lauf!=Nil) AND (Lauf->Number!=NewCPU)) Lauf=Lauf->Next;
   if (Lauf==Nil) return;

   strmaxcpy(MomCPUIdent,Lauf->Name,11);
   MomCPU=Lauf->Orig;
   MomVirtCPU=Lauf->Number;
   strmaxcpy(s,MomCPUIdent,11);
   for (z=dest=s; *z!='\0'; z++)
    if (((*z>='0') AND (*z<='9')) OR ((*z>='A') AND (*z<='F')))
     *(dest++)=*z;
   *dest='\0';
   for (z=s; *z!='\0'; z++)
    if ((*z>='0') AND (*z<='9')) break;
   if (*z!='\0') strcpy(s,z);
   strprep(s,"$");
   HCPU=ConstLongInt(s,&ECPU);
   if (ParamCount!=0)
    BEGIN
     EnterIntSymbol(MomCPUName,HCPU,SegNone,True);
     EnterStringSymbol(MomCPUIdentName,MomCPUIdent,True);
    END

   InternSymbol=Default_InternSymbol;
   if (NOT NotPrev) SwitchFrom(); 
   Lauf->SwitchProc();

   DontPrint=True;
END


	char *IntLine(LongInt Inp)
BEGIN
   static String s;

   switch (ConstMode)
    BEGIN
     case ConstModeIntel:
      sprintf(s,"%sH",HexString(Inp,0));
      if (*s>'9') strmaxprep(s,"0",255);
      break;
     case ConstModeMoto:
      sprintf(s,"$%s",HexString(Inp,0));
      break;
     case ConstModeC:
      sprintf(s,"0x%s",HexString(Inp,0));
      break;
    END

   return s;
END


	static void CodeSECTION(void)
BEGIN
   PSaveSection Neu;

   if (ArgCnt!=1) WrError(1110);
   else if (ExpandSymbol(ArgStr[1]))
    if (NOT ChkSymbName(ArgStr[1])) WrXError(1020,ArgStr[1]);
    else if ((PassNo==1) AND (GetSectionHandle(ArgStr[1],False,MomSectionHandle)!=-2)) WrError(1483);
    else
     BEGIN
      Neu=(PSaveSection) malloc(sizeof(TSaveSection));
      Neu->Next=SectionStack;
      Neu->Handle=MomSectionHandle;
      Neu->LocSyms=Nil; Neu->GlobSyms=Nil; Neu->ExportSyms=Nil;
      MomSectionHandle=GetSectionHandle(ArgStr[1],True,MomSectionHandle);
      SectionStack=Neu;
     END
END


	static void CodeENDSECTION_ChkEmptList(PForwardSymbol *Root)
BEGIN
   PForwardSymbol Tmp;

   while (*Root!=Nil)
    BEGIN
     WrXError(1488,(*Root)->Name);
     free((*Root)->Name);
     Tmp=*Root; *Root=Tmp->Next; free(Tmp);
    END
END

	static void CodeENDSECTION(void)
BEGIN
   PSaveSection Tmp;

   if (ArgCnt>1) WrError(1110);
   else if (SectionStack==Nil) WrError(1487);
   else if ((ArgCnt==0) OR (ExpandSymbol(ArgStr[1])))
    if ((ArgCnt==1) AND (GetSectionHandle(ArgStr[1],False,SectionStack->Handle)!=MomSectionHandle)) WrError(1486);
    else
     BEGIN
      Tmp=SectionStack; SectionStack=Tmp->Next;
      CodeENDSECTION_ChkEmptList(&(Tmp->LocSyms));
      CodeENDSECTION_ChkEmptList(&(Tmp->GlobSyms));
      CodeENDSECTION_ChkEmptList(&(Tmp->ExportSyms));
      if (ArgCnt==0)
       sprintf(ListLine,"[%s]",GetSectionName(MomSectionHandle));
      MomSectionHandle=Tmp->Handle;
      free(Tmp);
     END
END


	static void CodeCPU(void)
BEGIN
   PCPUDef Lauf;

   if (ArgCnt!=1) WrError(1110);
   else if (*AttrPart!='\0') WrError(1100);
   else
    BEGIN
     NLS_UpString(ArgStr[1]);
     Lauf=FirstCPUDef;
     while ((Lauf!=Nil) AND (strcmp(ArgStr[1],Lauf->Name)!=0))
      Lauf=Lauf->Next;
     if (Lauf==Nil) WrXError(1430,ArgStr[1]);
     else
      BEGIN
       SetCPU(Lauf->Number,False); ActPC=SegCode;
      END
    END
END


	static void CodeSETEQU(void)
BEGIN
   TempResult t;
   Boolean MayChange;
   Integer DestSeg;

   FirstPassUnknown=False;
   MayChange=((NOT Memo("EQU")) AND (NOT Memo("=")));
   if (*AttrPart!='\0') WrError(1100);
   else if ((ArgCnt<1) OR (ArgCnt>2)) WrError(1110);
   else
    BEGIN
     EvalExpression(ArgStr[1],&t);
     if (NOT FirstPassUnknown)
      BEGIN
       if (ArgCnt==1) DestSeg=SegNone;
       else
        BEGIN
         NLS_UpString(ArgStr[2]);
         if (strcmp(ArgStr[2],"MOMSEGMENT")==0) DestSeg=ActPC;
         else
          BEGIN
           DestSeg=0;
            while ((DestSeg<=PCMax) AND
                   (strcmp(ArgStr[2],SegNames[DestSeg])!=0))
             DestSeg++;
          END
        END
       if (DestSeg>PCMax) WrXError(1961,ArgStr[2]);
       else
        BEGIN
         SetListLineVal(&t);
         PushLocHandle(-1);
         switch (t.Typ)
          BEGIN
           case TempInt   : EnterIntSymbol   (LabPart,t.Contents.Int,DestSeg,MayChange); break;
           case TempFloat : EnterFloatSymbol (LabPart,t.Contents.Float,MayChange); break;
           case TempString: EnterStringSymbol(LabPart,t.Contents.Ascii,MayChange); break;
           case TempNone  : break;
          END
         PopLocHandle();
        END
      END
    END
END


	static void CodeORG(void)
BEGIN
   LargeInt HVal;
   Boolean ValOK;

   FirstPassUnknown=False;
   if (*AttrPart!='\0') WrError(1100);
   else if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
#ifndef HAS64
     HVal=EvalIntExpression(ArgStr[1],UInt32,&ValOK);
#else
     HVal=EvalIntExpression(ArgStr[1],Int64,&ValOK);
#endif
     if (FirstPassUnknown) WrError(1820);
     if ((ValOK) AND (NOT FirstPassUnknown))
      BEGIN
       PCs[ActPC]=HVal; DontPrint=True;
      END
    END
END


   	static void CodeSHARED_BuildComment(char *c)
BEGIN
   switch (ShareMode)
    BEGIN
     case 1: sprintf(c,"(* %s *)",CommPart); break;
     case 2: sprintf(c,"/* %s */",CommPart); break;
     case 3: sprintf(c,"; %s",CommPart); break;
   END
END

	static void CodeSHARED(void)
BEGIN
   Integer z;
   Boolean ValOK;
   LargeInt HVal;
   Double FVal;
   String s,c;

   if (ShareMode==0) WrError(30);
   else if ((ArgCnt==0) AND (*CommPart!='\0'))
    BEGIN
     CodeSHARED_BuildComment(c); 
     errno=0; fprintf(ShareFile,"%s\n",c); ChkIO(10004);
    END
   else
    for (z=1; z<=ArgCnt; z++)
     BEGIN
      if (IsSymbolString(ArgStr[z]))
       BEGIN
	ValOK=GetStringSymbol(ArgStr[z],s);
	if (ShareMode==1) 
         BEGIN
          strmaxprep(s,"\'",255); strmaxcat(s,"\'",255);
         END
	else
         BEGIN
          strmaxprep(s,"\"",255); strmaxcat(s,"\"",255);
         END
       END
      else if (IsSymbolFloat(ArgStr[z]))
       BEGIN
	ValOK=GetFloatSymbol(ArgStr[z],&FVal);
	sprintf(s,"%0.17g",FVal);
       END
      else
       BEGIN
	ValOK=GetIntSymbol(ArgStr[z],&HVal);
	switch (ShareMode)
         BEGIN
	  case 1: sprintf(s,"$%s",HexString(HVal,0)); break;
          case 2: sprintf(s,"0x%s",HexString(HVal,0)); break;
	  case 3: strmaxcpy(s,IntLine(HVal),255); break;
	 END
       END
      if (ValOK)
       BEGIN
        if ((z==1) AND (*CommPart!='\0'))
         BEGIN
          CodeSHARED_BuildComment(c); strmaxprep(c," ",255);
         END
        else *c='\0';
	errno=0;
	switch (ShareMode)
         BEGIN
          case 1: 
           fprintf(ShareFile,"%s = %s;%s\n",ArgStr[z],s,c); break;
          case 2: 
           fprintf(ShareFile,"#define %s %s%s\n",ArgStr[z],s,c); break;
          case 3:
           strmaxprep(s,IsSymbolChangeable(ArgStr[z])?"set ":"equ ",255);
           fprintf(ShareFile,"%s %s%s\n",ArgStr[z],s,c); break;
	 END
	ChkIO(10004);
       END
      else if (PassNo==1)
       BEGIN
        Repass=True;
	if ((MsgIfRepass) AND (PassNo>=PassNoForMessage)) WrXError(170,ArgStr[z]);
       END
     END
END


	static void CodePAGE(void)
BEGIN
   Integer LVal,WVal;
   Boolean ValOK;

   if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
   else if (*AttrPart!='\0') WrError(1100);
   else
    BEGIN
     LVal=EvalIntExpression(ArgStr[1],UInt8,&ValOK);
     if (ValOK)
      BEGIN
       if ((LVal<5) AND (LVal!=0)) LVal=5;
       if (ArgCnt==1)
        BEGIN
         WVal=0; ValOK=True;
        END
       else WVal=EvalIntExpression(ArgStr[2],UInt8,&ValOK);
       if (ValOK)
        BEGIN
         if ((WVal<5) AND (WVal!=0)) WVal=5;
         PageLength=LVal; PageWidth=WVal;
        END
      END
    END
END


	static void CodeNEWPAGE(void)
BEGIN
   ShortInt HVal8;
   Boolean ValOK;

   if (ArgCnt>1) WrError(1110);
   else if (*AttrPart!='\0') WrError(1100);
   else
    BEGIN
     if (ArgCnt==0)
      BEGIN
       HVal8=0; ValOK=True;
      END
     else HVal8=EvalIntExpression(ArgStr[1],Int8,&ValOK);
     if ((ValOK) OR (ArgCnt==0))
      BEGIN
       if (HVal8>ChapMax) HVal8=ChapMax;
       else if (HVal8<0) HVal8=0;
       NewPage(HVal8,True);
      END
    END
END


	static void CodeString(char *erg)
BEGIN
   String tmp;
   Boolean OK;

   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     EvalStringExpression(ArgStr[1],&OK,tmp);
     if (NOT OK) WrError(1970); else strmaxcpy(erg,tmp,255);
    END
END


	static void CodePHASE(void)
BEGIN
   Boolean OK;
   LongInt HVal;

   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     HVal=EvalIntExpression(ArgStr[1],Int32,&OK);
     if (OK) Phases[ActPC]=HVal-ProgCounter();
    END
END


	static void CodeDEPHASE(void)
BEGIN
   if (ArgCnt!=0) WrError(1110);
   else Phases[ActPC]=0;
END


	static void CodeWARNING(void)
BEGIN
   String mess;
   Boolean OK;

   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     EvalStringExpression(ArgStr[1],&OK,mess);
     if (NOT OK) WrError(1970);
     else WrErrorString(mess,"",True,False);
    END
END


	static void CodeMESSAGE(void)
BEGIN
   String mess;
   Boolean OK;

   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     EvalStringExpression(ArgStr[1],&OK,mess);
     if (NOT OK) WrError(1970);
     printf("%s%s\n",mess,ClrEol);
     if (strcmp(LstName,"/dev/null")!=0) WrLstLine(mess);
    END
END


	static void CodeERROR(void)
BEGIN
   String mess;
   Boolean OK;

   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     EvalStringExpression(ArgStr[1],&OK,mess);
     if (NOT OK) WrError(1970);
     else WrErrorString(mess,"",False,False);
    END
END


	static void CodeFATAL(void)
BEGIN
   String mess;
   Boolean OK;

   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     EvalStringExpression(ArgStr[1],&OK,mess);
     if (NOT OK) WrError(1970);
     else WrErrorString(mess,"",False,True);
    END
END


	static void CodeCHARSET(void)
BEGIN
   Byte w1,w2,w3;
   Integer ch;
   Boolean OK;

   if ((ArgCnt<2) OR (ArgCnt>3)) WrError(1110);
   else
    BEGIN
     w1=EvalIntExpression(ArgStr[1],Int8,&OK);
     if (OK)
      BEGIN
       w3=EvalIntExpression(ArgStr[ArgCnt],Int8,&OK);
       if (OK)
	BEGIN
         if (ArgCnt==2)
          BEGIN
           w2=w1; OK=True;
          END
	 else w2=EvalIntExpression(ArgStr[2],Int8,&OK);
         if (OK)
          BEGIN
	   if (w1>w2) WrError(1320);
	   else
            for (ch=w1; ch<=w2; ch++)
	     CharTransTable[ch]=ch-w1+w3;
	  END
	END
      END
    END
END


	static void CodeFUNCTION(void)
BEGIN
   String FName;
   Boolean OK;
   Integer z;

   if (ArgCnt<2) WrError(1110);
   else
    BEGIN
     OK=True; z=1;
     do
      BEGIN
       OK=(OK AND ChkMacSymbName(ArgStr[z]));
       if (NOT OK) WrXError(1020,ArgStr[z]);
       z++;
      END
     while ((z<ArgCnt) AND (OK));
     if (OK)
      BEGIN
       strmaxcpy(FName,ArgStr[ArgCnt],255);
       for (z=1; z<ArgCnt; z++)
	CompressLine(ArgStr[z],z,FName);
       EnterFunction(LabPart,FName,ArgCnt-1);
      END
    END
END


	static void CodeSAVE(void)
BEGIN
   PSaveState Neu;

   if (ArgCnt!=0) WrError(1110);
   else
    BEGIN
     Neu=(PSaveState) malloc(sizeof(TSaveState));
     Neu->Next=FirstSaveState;
     Neu->SaveCPU=MomCPU;
     Neu->SavePC=ActPC;
     Neu->SaveListOn=ListOn;
     Neu->SaveLstMacroEx=LstMacroEx;
     FirstSaveState=Neu;
    END
END


	static void CodeRESTORE(void)
BEGIN
   PSaveState Old;

   if (ArgCnt!=0) WrError(1110);
   else if (FirstSaveState==Nil) WrError(1450);
   else
    BEGIN
     Old=FirstSaveState; FirstSaveState=Old->Next;
     if (Old->SavePC!=ActPC)
      BEGIN
       ActPC=Old->SavePC; DontPrint=True;
      END
     if (Old->SaveCPU!=MomCPU) SetCPU(Old->SaveCPU,False);
     EnterIntSymbol(ListOnName,ListOn=Old->SaveListOn,0,True); 
     SetFlag(&LstMacroEx,LstMacroExName,Old->SaveLstMacroEx);
     free(Old);
    END
END


	static void CodeSEGMENT(void)
BEGIN
   Byte SegZ;
   Word Mask;
   Boolean Found;

   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     Found=False; NLS_UpString(ArgStr[1]);
     for (SegZ=1,Mask=2; SegZ<=PCMax; SegZ++,Mask<<=1)
      if (((ValidSegs&Mask)!=0) AND (strcmp(ArgStr[1],SegNames[SegZ])==0))
       BEGIN
        Found=True;
        if (ActPC!=SegZ)
 	 BEGIN
	  ActPC=SegZ;
	  if (NOT PCsUsed[ActPC]) PCs[ActPC]=SegInits[ActPC];
	  PCsUsed[ActPC]=True;
	  DontPrint=True;
	 END
       END
     if (NOT Found) WrXError(1961,ArgStr[1]);
    END
END


	static void CodeLABEL(void)
BEGIN
   LongInt Erg;
   Boolean OK;

   FirstPassUnknown=False;
   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     Erg=EvalIntExpression(ArgStr[1],Int32,&OK);
     if ((OK) AND (NOT FirstPassUnknown))
      BEGIN
       PushLocHandle(-1);
       EnterIntSymbol(LabPart,Erg,SegCode,False);
       sprintf(ListLine,"=%s",IntLine(Erg));
       PopLocHandle();
       END
    END
END


	static void CodeREAD(void)
BEGIN
   String Exp;
   TempResult Erg;
   Boolean OK;
   LongInt SaveLocHandle;

   if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
   else
    BEGIN
     if (ArgCnt==2) EvalStringExpression(ArgStr[1],&OK,Exp);
     else
      BEGIN
       sprintf(Exp,"Read %s ? ",ArgStr[1]); OK=True;
      END
     if (OK)
      BEGIN
       setbuf(stdout,Nil); printf("%s",Exp); 
       fgets(Exp,255,stdin); UpString(Exp);
       FirstPassUnknown=False;
       EvalExpression(Exp,&Erg);
       if (OK)
	BEGIN
	 SetListLineVal(&Erg);
         SaveLocHandle=MomLocHandle; MomLocHandle=-1;
	 if (FirstPassUnknown) WrError(1820);
	 else switch (Erg.Typ)
          BEGIN
   	   case TempInt   : EnterIntSymbol(ArgStr[ArgCnt],Erg.Contents.Int,SegNone,True); break;
   	   case TempFloat : EnterFloatSymbol(ArgStr[ArgCnt],Erg.Contents.Float,True); break;
   	   case TempString: EnterStringSymbol(ArgStr[ArgCnt],Erg.Contents.Ascii,True); break;
           case TempNone  : break;
	  END
         MomLocHandle=SaveLocHandle;
	END
      END
    END
END


	static void CodeALIGN(void)
BEGIN
   Word Dummy;
   Boolean OK;
   LongInt NewPC;

   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     FirstPassUnknown=False;
     Dummy=EvalIntExpression(ArgStr[1],Int16,&OK);
     if (OK)
      if (FirstPassUnknown) WrError(1820);
      else
       BEGIN
	NewPC=ProgCounter()+Dummy-1;
	NewPC-=NewPC%Dummy;
	CodeLen=NewPC-ProgCounter();
	DontPrint=(CodeLen!=0);
	if ((MakeUseList) AND (DontPrint))
	 if (AddChunk(SegChunks+ActPC,ProgCounter(),CodeLen,ActPC==SegCode)) WrError(90);
       END
    END
END


	static void CodeENUM(void)
BEGIN
   Integer z;
   char *p=Nil;
   Boolean OK;
   LongInt Counter,First=0,Last=0;
   String SymPart;

   Counter=0;
   if (ArgCnt==0) WrError(1110);
   else
    for (z=1; z<=ArgCnt; z++)
     BEGIN
      p=QuotPos(ArgStr[z],'=');
      if (p!=Nil)
       BEGIN
        strmaxcpy(SymPart,p+1,255);
	FirstPassUnknown=False;
	Counter=EvalIntExpression(SymPart,Int32,&OK);
	if (NOT OK) return;
	if (FirstPassUnknown)
	 BEGIN
	  WrXError(1820,SymPart); return;
	 END
        *p='\0';
       END
      EnterIntSymbol(ArgStr[z],Counter,SegNone,False);
      if (z==1) First=Counter;
      else if (z==ArgCnt) Last=Counter;
      Counter++;
     END
   sprintf(ListLine,"=%s",IntLine(First));
   if (ArgCnt!=1)
    BEGIN
     strmaxcat(ListLine,"..",255);
     strmaxcat(ListLine,IntLine(Last),255);
    END
END


	static void CodeEND(void)
BEGIN
   LongInt HVal;
   Boolean OK;

   if (ArgCnt>1) WrError(1110);
   else
    BEGIN
     if (ArgCnt==1)
      BEGIN
       FirstPassUnknown=False;
       HVal=EvalIntExpression(ArgStr[1],Int32,&OK);
       if ((OK) AND (NOT FirstPassUnknown))
        BEGIN
         ChkSpace(SegCode);
         StartAdr=HVal;
         StartAdrPresent=True;
        END
      END
     ENDOccured=True;
    END
END


	static void CodeLISTING(void)
BEGIN
   Byte Value=0xff;
   Boolean OK;

   if (ArgCnt!=1) WrError(1110);
   else if (*AttrPart!='\0') WrError(1100);
   else
    BEGIN
     OK=True; NLS_UpString(ArgStr[1]);
     if (strcmp(ArgStr[1],"OFF")==0) Value=0;
     else if (strcmp(ArgStr[1],"ON")==0) Value=1;
     else if (strcmp(ArgStr[1],"NOSKIPPED")==0) Value=2;
     else if (strcmp(ArgStr[1],"PURECODE")==0) Value=3;
     else OK=False;
     if (NOT OK) WrError(1520);
     else EnterIntSymbol(ListOnName,ListOn=Value,0,True);
    END
END

        static void CodeBINCLUDE(void)
BEGIN
   FILE *F;
   LongInt Len=-1;
   LongWord Ofs=0,Curr,Rest;
   Word RLen;
   Boolean OK,SaveTurnWords;
   String Name;

   if ((ArgCnt<1) OR (ArgCnt>3)) WrError(1110);
   else if (ActPC!=SegCode) WrError(1940);
   else
    BEGIN
     if (ArgCnt==1) OK=True;
     else
      BEGIN
       FirstPassUnknown=False;
       Ofs=EvalIntExpression(ArgStr[2],Int32,&OK);
       if (FirstPassUnknown)
        BEGIN
         WrError(1820); OK=False;
        END
       if (OK)
        if (ArgCnt==2) Len=-1;
        else
         BEGIN
          Len=EvalIntExpression(ArgStr[3],Int32,&OK);
          if (FirstPassUnknown)
           BEGIN
            WrError(1820); OK=False;
           END
         END
      END
     if (OK)
      BEGIN
       strmaxcpy(Name,ArgStr[1],255);
       if (*Name=='"') strcpy(Name,Name+1);
       if (Name[strlen(Name)-1]=='"') Name[strlen(Name)-1]='\0';
       strmaxcpy(ArgStr[1],Name,255);
       strmaxcpy(Name,FExpand(FSearch(Name,IncludeList)),255);
       if (Name[strlen(Name)-1]=='/') strmaxcat(Name,ArgStr[1],255);
       F=fopen(Name,"r"); ChkIO(10001);
       if (Len==-1)
        BEGIN
         Len=FileSize(F); ChkIO(10003);
        END
       fseek(F,SEEK_SET,Ofs); ChkIO(10003);
       Rest=Len; SaveTurnWords=TurnWords; TurnWords=False;
       do
        BEGIN
         if (Rest<MaxCodeLen) Curr=Rest; else Curr=MaxCodeLen;
         RLen=fread(BAsmCode,1,Curr,F); ChkIO(10003);
         CodeLen=RLen; WriteBytes();
         Rest-=RLen;
        END
       while ((Rest!=0) AND (RLen==Curr));
       if (Rest!=0) WrError(1600);
       TurnWords=SaveTurnWords;
       DontPrint=True; CodeLen=Len-Rest;
       fclose(F);
      END
    END
END

        static void CodePUSHV(void)
BEGIN
   Integer z;

   if (ArgCnt<2) WrError(1110);
   else
    BEGIN
     if (NOT CaseSensitive) NLS_UpString(ArgStr[1]);
     for (z=2; z<=ArgCnt; z++)
      PushSymbol(ArgStr[z],ArgStr[1]);
    END
END

        static void CodePOPV(void)
BEGIN
   Integer z;

   if (ArgCnt<2) WrError(1110);
   else
    BEGIN
     if (NOT CaseSensitive) NLS_UpString(ArgStr[1]);
     for (z=2; z<=ArgCnt; z++)
      PopSymbol(ArgStr[z],ArgStr[1]);
    END
END

	static PForwardSymbol CodePPSyms_SearchSym(PForwardSymbol Root, char *Comp)
BEGIN
   PForwardSymbol Lauf=Root;

   while ((Lauf!=Nil) AND (strcmp(Lauf->Name,Comp)!=0)) Lauf=Lauf->Next;
   return Lauf;
END


	static void CodePPSyms(PForwardSymbol *Orig,
                               PForwardSymbol *Alt1,
                               PForwardSymbol *Alt2)
BEGIN
   PForwardSymbol Lauf;
   Integer z;
   String Sym,Section;

   if (ArgCnt==0) WrError(1110);
   else
    for (z=1; z<=ArgCnt; z++)
     BEGIN
      SplitString(ArgStr[z],Sym,Section,QuotPos(ArgStr[z],':'));
      if (NOT ExpandSymbol(Sym)) return;
      if (NOT ExpandSymbol(Section)) return;
      if (NOT CaseSensitive) NLS_UpString(Sym);
      Lauf=CodePPSyms_SearchSym(*Alt1,Sym);
      if (Lauf!=Nil) WrXError(1489,ArgStr[z]);
      else
       BEGIN
	Lauf=CodePPSyms_SearchSym(*Alt2,Sym);
	if (Lauf!=Nil) WrXError(1489,ArgStr[z]);
	else
	 BEGIN
	  Lauf=CodePPSyms_SearchSym(*Orig,Sym);
	  if (Lauf==Nil)
	   BEGIN
	    Lauf=(PForwardSymbol) malloc(sizeof(TForwardSymbol)); 
            Lauf->Next=*Orig; *Orig=Lauf;
	    Lauf->Name=strdup(Sym);
	   END
	  IdentifySection(Section,&(Lauf->DestSection));
	 END
       END
     END
END

#define ONOFFAllgCount 2
static ONOFFRec ONOFFAllgs[ONOFFAllgCount]=
                {{"MACEXP" , &LstMacroEx,  LstMacroExName},
                 {"RELAXED", &RelaxedMode, RelaxedName}};

typedef struct
         {
          char *Name;
          void (*Proc)(void);
         } PseudoOrder;
static PseudoOrder Pseudos[]=
                   {{"ALIGN",      CodeALIGN     },
                    {"BINCLUDE",   CodeBINCLUDE  },
                    {"CHARSET",    CodeCHARSET   },
                    {"CPU",        CodeCPU       },
                    {"DEPHASE",    CodeDEPHASE   },
                    {"END",        CodeEND       },
                    {"ENDSECTION", CodeENDSECTION},
                    {"ENUM",       CodeENUM      },
                    {"ERROR",      CodeERROR     },
                    {"FATAL",      CodeFATAL     },
                    {"FUNCTION",   CodeFUNCTION  },
                    {"LABEL",      CodeLABEL     },
		    {"LISTING",    CodeLISTING   },
                    {"MESSAGE",    CodeMESSAGE   },
                    {"NEWPAGE",    CodeNEWPAGE   },
                    {"ORG",        CodeORG       },
                    {"PAGE",       CodePAGE      },
                    {"PHASE",      CodePHASE     },
                    {"POPV",       CodePOPV      },
                    {"PUSHV",      CodePUSHV     },
                    {"READ",       CodeREAD      },
                    {"RESTORE",    CodeRESTORE   },
                    {"SAVE",       CodeSAVE      },
                    {"SECTION",    CodeSECTION   },
                    {"SEGMENT",    CodeSEGMENT   },
                    {"SHARED",     CodeSHARED    },
                    {"WARNING",    CodeWARNING   },
                    {""       ,    Nil           }};

typedef struct
         { 
          char *Name;
          char *p;
         } PseudoStrOrder;
static PseudoStrOrder PseudoStrs[]=
                      {{"PRTINIT",   PrtInitString},
                       {"PRTEXIT",   PrtExitString},
                       {"TITLE",     PrtTitleString},
                       {"",          Nil           }};


	Boolean CodeGlobalPseudo(void)
BEGIN
   PseudoOrder *POrder;
   PseudoStrOrder *PSOrder;

   if ((Memo("EQU"))
    OR (Memo("="))
    OR (Memo(":="))
    OR (((NOT SetIsOccupied) AND (Memo("SET"))))
    OR (((SetIsOccupied) AND (Memo("EVAL")))))
     BEGIN
      CodeSETEQU(); return True;
     END

   if (CodeONOFF(ONOFFAllgs,ONOFFAllgCount)) return True;

   POrder=Pseudos;
   while ((POrder->Proc!=Nil) AND (strcmp(POrder->Name,OpPart)<=0))
    BEGIN
     if (strcmp(POrder->Name,OpPart)==0)
      BEGIN
       POrder->Proc(); return True;
      END
     POrder++;
    END

   for (PSOrder=PseudoStrs; PSOrder->p!=Nil; PSOrder++)
    if (Memo(PSOrder->Name))
     BEGIN
      CodeString(PSOrder->p); return True;
     END

   if (SectionStack!=Nil)
    BEGIN
     if (Memo("FORWARD"))
      BEGIN
       if (PassNo<=MaxSymPass)
	CodePPSyms(&(SectionStack->LocSyms),
		   &(SectionStack->GlobSyms),
		   &(SectionStack->ExportSyms));
       return True;
      END
     if (Memo("PUBLIC"))
      BEGIN
       CodePPSyms(&(SectionStack->GlobSyms),
		  &(SectionStack->LocSyms),
		  &(SectionStack->ExportSyms));
       return True;
      END
     if (Memo("GLOBAL"))
      BEGIN
       CodePPSyms(&(SectionStack->ExportSyms),
		  &(SectionStack->LocSyms),
		  &(SectionStack->GlobSyms));
       return True;
      END
    END

   return False;
END


	void codeallg_init(void)
BEGIN
END
