/* plist.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Anzeige des Inhalts einer Code-Datei                                      */
/*                                                                           */
/* Historie: 31. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include "endian.h"
#include "hex.h"
#include "bpemu.h"
#include "decodecmd.h"
#include "nls.h"
#include "stringutil.h"
#include "asmutils.h"

#define HeaderCnt 48
static Byte HeaderBytes[HeaderCnt]={
    0x01,0x05,0x09,0x11,0x12,0x13,0x14,
    0x19,0x21,0x29,0x31,0x39,0x3b,0x3c,0x41,
    0x42,0x49,0x4a,0x4c,0x51,0x52,0x53,
    0x54,0x55,0x56,0x61,0x62,0x63,
    0x64,0x65,0x66,0x68,0x69,0x6c,0x6f,0x70,
    0x71,0x72,0x73,0x74,0x75,0x76,
    0x77,0x78,0x79,0x7a,0x7b,0x7c};
static char *HeaderNames[HeaderCnt]={
    "680x0     ","MPC601    ","DSP56000  ","65xx      ","MELPS-4500","M16       ","M16C      ",
    "MELPS-7700","MCS-48    ","29xxx     ","MCS-(2)51 ","MCS-96    ","AVR       ","XA        ","8080/8085 ",
    "8086      ","TMS370xx  ","MSP430    ","80C166/167","Zx80      ","TLCS-900  ","TLCS-90   ",
    "TLCS-870  ","TLCS-47xx ","TLCS-9000 ","68xx      ","6805/HC08 ","6809      ",
    "6804      ","68HC16    ","68HC12    ","H8/300(H) ","H8/500    ","SH7000    ","COP8      ","16C8x     ",
    "16C5x     ","17C4x     ","TMS7000   ","TMS3201x  ","TMS3202x  ","TMS320C3x ",
    "TMS320C5x ","ST62xx    ","Z8        ","78(C)1x   ","75K0      ","78K0      "};


static char *SegNames[PCMax+1]={"NONE","CODE","DATA","IDATA","XDATA","YDATA",
                                "BDATA","IO"};

static FILE *ProgFile;
static String ProgName;
static Byte Header,Segment,Gran;
static LongWord StartAdr,Sums[PCMax+1];
static Word Len,ID,z;
static int Ch;
static Boolean HeadFnd;

#include "tools.rsc"
#include "plist.rsc"

	int main(int argc, char **argv)
BEGIN
   ParamCount=argc-1; ParamStr=argv;

   endian_init();
   hex_init();
   bpemu_init();
   stringutil_init();
   nls_init();
   decodecmd_init();
   asmutils_init();
 
   NLS_Initialize(); WrCopyRight("PLIST/C V1.41r5");

   if (ParamCount==0)
    BEGIN
     errno=0;
     printf("%s",MessFileRequest); fgets(ProgName,255,stdin);
     ChkIO(OutName);
    END
   else if (ParamCount==1 ) strmaxcpy(ProgName,ParamStr[1],255);
   else
    BEGIN
     errno=0; printf("%s%s%s\n",InfoMessHead1,GetEXEName(),InfoMessHead2); ChkIO(OutName);
     for (z=0; z<InfoMessHelpCnt; z++)
      BEGIN
       errno=0; printf("%s\n",InfoMessHelp[z]); ChkIO(OutName);
      END
     exit(1);
    END

   AddSuffix(ProgName,Suffix);

   if ((ProgFile=fopen(ProgName,"r"))==Nil) ChkIO(ProgName);

   if (NOT Read2(ProgFile,&ID)) ChkIO(ProgName);
   if (ID!=FileMagic) FormatError(ProgName,FormatInvHeaderMsg);

   errno=0; printf("\n"); ChkIO(OutName);
   errno=0; printf("%s\n",MessHeaderLine1); ChkIO(OutName);
   errno=0; printf("%s\n",MessHeaderLine2); ChkIO(OutName);

   for (z=0; z<=PCMax; Sums[z++]=0);

   do
    BEGIN
     ReadRecordHeader(&Header,&Segment,&Gran,ProgName,ProgFile);

     HeadFnd=False;

     if (Header==FileHeaderEnd)
      BEGIN
       errno=0; printf(MessGenerator); ChkIO(OutName);
       do 
        BEGIN
  	 errno=0; Ch=fgetc(ProgFile); ChkIO(ProgName);
         if (Ch!=EOF)
         BEGIN
	  errno=0; putchar(Ch); ChkIO(OutName);
         END
        END
       while (Ch!=EOF);
       errno=0; printf("\n"); ChkIO(OutName); HeadFnd=True;
      END

     else if (Header==FileHeaderStartAdr)
      BEGIN
       if (NOT Read4(ProgFile,&StartAdr)) ChkIO(ProgName);
       errno=0; printf("%s%s\n",MessEntryPoint,HexLong(StartAdr));
       ChkIO(OutName);
      END

     else
      BEGIN
       errno=0;
       for (z=0; z<HeaderCnt; z++)
        if ((Magic==0) AND (Header==HeaderBytes[z]))
	 BEGIN
	  printf("%s    ",HeaderNames[z]);
	  HeadFnd=True;
	 END
       if (NOT HeadFnd) printf("??????        ");
       ChkIO(OutName);

       errno=0; printf("%-5s   ",SegNames[Segment]);  ChkIO(OutName);

       if (NOT Read4(ProgFile,&StartAdr)) ChkIO(ProgName);
       errno=0; printf("%s          ",HexLong(StartAdr));  ChkIO(OutName);

       if (NOT Read2(ProgFile,&Len)) ChkIO(ProgName);
       errno=0; printf("%s      ",HexWord(Len));  ChkIO(OutName);

       if (Len!=0) StartAdr+=(Len/Gran)-1;
       else StartAdr--;
       errno=0; printf("%s\n",HexLong(StartAdr));  ChkIO(OutName);

       Sums[Segment]+=Len;

       if (ftell(ProgFile)+Len>=FileSize(ProgFile))
        FormatError(ProgName,FormatInvRecordLenMsg);
       else if (fseek(ProgFile,Len,SEEK_CUR)!=0) ChkIO(ProgName);
      END
    END
   while (Header!=0);

   errno=0; printf("\n"); ChkIO(OutName);
   errno=0; printf("%s",MessSum1); ChkIO(OutName);
   for (z=0; z<=PCMax; z++)
    if ((z==SegCode) OR (Sums[z]!=0))
     BEGIN
      errno=0;
      printf("%d%s%s\n%s",Sums[z],(Sums[z]==1)?MessSumSing:MessSumPlur,SegNames[z],Blanks(strlen(MessSum1)));
     END
   errno=0; printf("\n"); ChkIO(OutName);
   errno=0; printf("\n"); ChkIO(OutName);

   errno=0; fclose(ProgFile); ChkIO(ProgName);
   return 0;
END
