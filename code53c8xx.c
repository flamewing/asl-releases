/* code53c8xx.c */
/*****************************************************************************/
/* Makroassembler AS                                                         */
/*                                                                           */
/* Codegenerator SYM53C8xx                                                   */
/*                                                                           */
/* Historie: 30. 9.1998 angelegt                                             */
/*            3.10.1998 erste Befehle (NOP JUMP CALL RETURN INT INTFLY)      */
/*            4.10.1998 CHMOV CLEAR SET DISCONNECT LOAD STORE                */
/*            7.10.1998 MOVE begonnen                                        */
/*           15.11.1998 SELECT/RESELECT, WAIT                                */
/*            3. 1.1999 ChkPC-Anpassung                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"
#include "headids.h"
#include "strutil.h"

/*---------------------------------------------------------------------------*/

#define RegCnt 69

typedef struct
         {
          char *Name;
          LongWord Code;
          Word Mask;
         } TReg, *PReg;

static CPUVar CPU53C810, CPU53C860, CPU53C815, CPU53C825, CPU53C875,
              CPU53C895;

#define M_53C810 0x0001
#define M_53C860 0x0002
#define M_53C815 0x0004
#define M_53C825 0x0008
#define M_53C875 0x0010
#define M_53C895 0x0020

static PInstTable InstTable;
static PReg Regs;

/*---------------------------------------------------------------------------*/

	static Boolean IsInToken(char Inp)
BEGIN
   return ((Inp == '_') OR (isalnum(((usint) Inp) & 0xff)));
END

	static void GetToken(char *Src, char *Dest)
BEGIN
   char *p, *start, Save;

   /* Anfang suchen */

   for (p = Src; isspace(*p); p++)
    if (*p == '\0') break;
   if (*p == '\0')
    BEGIN
     *Dest = *Src = '\0'; return;
    END
   start = p;

   /* geklammerter Ausdruck ? */

   if (*p == '(')
    BEGIN
     p = QuotPos(p, ')');
     if (p == Nil)
      BEGIN
       strcpy(Dest, start); *Src = '\0';
      END
     else
      BEGIN
       Save = p[1]; p[1] = '\0';
       strcpy(Dest, start);
       p[1] = Save; strcpy(Src, p + 1);
      END
    END

   /* Spezialtoken ? */

   else if (NOT IsInToken(*p))
    BEGIN
     *Dest = *p; Dest[1] = '\0';
     strcpy(Src, p + 1);
    END

   /* Wort ? */

   else
    BEGIN
     for (; IsInToken(*p); p++)
      if (*p == '\0') break;
     Save = *p; *p = '\0';
     strcpy(Dest, start); 
     *p = Save; strcpy(Src, p);
    END
END

#if 0
	static void GetRToken(char *Src, char *Dest)
BEGIN
   char *p;

   KillPostBlanks(Src);

   p = Src + strlen(Src) - 1;

   /* geklammerter Ausdruck ? */ 

   if (*p == ')')                
    BEGIN                        
     p = RQuotPos(p, ')');
     if (p == Nil)
      BEGIN
       strcpy(Dest, Src); *Src = '\0';
      END
     else
      BEGIN
       strcpy(Dest, p); *p = '\0';
      END
    END

   /* Spezieltoken ? */

   else if (NOT IsInToken(*p))   
    BEGIN                        
     *Dest = *p; Dest[1] = '\0'; 
     *p = '\0';
    END

   else
    BEGIN
     for (; IsInToken(*p); p--)
      if (p <= Src) break;
     if (NOT IsInToken(*p)) p++;
     strcpy(Dest, p); *p = '\0';
    END
END
#endif

	static Boolean DecodePhase(char *Name, LongWord *Result)
BEGIN
   *Result = 8;
   if (strcasecmp(Name, "DATA_OUT") == 0) *Result = 0;
   else if (strcasecmp(Name, "DATA_IN") == 0) *Result = 1;
   else if (strcasecmp(Name, "COMMAND") == 0) *Result = 2;
   else if (strcasecmp(Name, "STATUS") == 0) *Result = 3;
   else if (strcasecmp(Name, "RES4") == 0) *Result = 4;
   else if (strcasecmp(Name, "RES5") == 0) *Result = 5;
   else if (strcasecmp(Name, "MESSAGE_OUT") == 0) *Result = 6;
   else if (strcasecmp(Name, "MESSAGE_IN") == 0) *Result = 7;
   return (*Result < 8);
END

	static Boolean DecodeReg(char *Name, LongWord *Result)
BEGIN
   int z;
   Integer Mask = 1 << (MomCPU - CPU53C810);
   PReg Reg;

   for (z = 0, Reg = Regs; z < RegCnt; z++, Reg++)
    if ((strcasecmp(Reg->Name, Name) == 0) AND (Mask & Reg->Mask))
     BEGIN
      *Result = Reg->Code; return True;
     END

   return False;
END

	static Boolean Err(int Num, char *Msg)
BEGIN
   WrXError(Num, Msg);
   return False;
END

	static Boolean DecodeCond(char *Src, LongWord *Dest)
BEGIN
   String Tok;
   Boolean PhaseATNUsed, DataUsed, CarryUsed, MaskUsed;
   LongWord Tmp;
   Boolean OK;

   /* IF/WHEN/Nix-Unterscheidung - TRUE fuer Nix setzen */

   GetToken(Src, Tok);
   if (*Tok == '\0')
    BEGIN
     *Dest |= 0x00080000;
     return True;
    END

   if (strcasecmp(Tok, "WHEN") == 0)
    *Dest |= 0x00010000;
   else if (strcasecmp(Tok, "IF") != 0)
    return Err(1135, Tok);

   /* Negierung? */

   GetToken(Src, Tok);  
   if (strcasecmp(Tok, "NOT") == 0) GetToken(Src, Tok);
   else *Dest |= 0x00080000;

   /* Bedingungen durchgehen */

   PhaseATNUsed = DataUsed = MaskUsed = CarryUsed = False;
   do
    BEGIN
     if (strcasecmp(Tok, "ATN") == 0)
      BEGIN
       if (PhaseATNUsed) return Err(1350, "2 x ATN/Phase");
       if (CarryUsed) return Err(1350, "Carry + ATN/Phase");
       if ((*Dest & 0x00010000) != 0) return Err(1350, "WHEN + ATN");
       PhaseATNUsed = True; *Dest |= 0x00020000;
      END
     else if (DecodePhase(Tok, &Tmp))
      BEGIN
       if (PhaseATNUsed) return Err(1350, "2 x ATN/Phase");
       if (CarryUsed) return Err(1350, "Carry + ATN/Phase");
       PhaseATNUsed = True; *Dest |= 0x00020000 + (Tmp << 24);
      END
     else if (strcasecmp(Tok, "CARRY") == 0)
      BEGIN
       if (CarryUsed) return Err(1350, "2 x Carry");
       if ((PhaseATNUsed) OR (DataUsed)) return Err(1350, "Carry + ...");
       CarryUsed = True; *Dest |= 0x00200000;
      END
     else if (strcasecmp(Tok, "MASK") == 0)
      BEGIN
       if (CarryUsed) return Err(1350, "Carry + Data");
       if (MaskUsed) return Err(1350, "2 x Mask");
       if (NOT DataUsed) return Err(1350, "Mask + !Data");
       GetToken(Src, Tok);
       Tmp = EvalIntExpression(Tok, UInt8, &OK);
       if (!OK) return False;
       MaskUsed = True; *Dest |= (Tmp << 8);
      END
     else
      BEGIN
       if (CarryUsed) return Err(1350, "Carry + Data");
       if (DataUsed) return Err(1350, "2 x Data");
       Tmp = EvalIntExpression(Tok, UInt8, &OK);
       if (!OK) return False;
       DataUsed = True; *Dest |= 0x00040000 + Tmp;
      END
     GetToken(Src, Tok);
     if (*Tok != '\0') 
      BEGIN
       if (strcasecmp(Tok, "AND") != 0) return Err(1350, Tok);
       GetToken(Src, Tok);
      END
    END
   while (*Tok != '\0');

   return True;
END

typedef enum {NONE, SFBR, REGISTER, IMM8} CompType;

	CompType DecodeComp(char *Inp, LongWord *Outp)
BEGIN
   Boolean OK;

   if (strcasecmp(Inp, "SFBR") == 0) return SFBR;
   else if (DecodeReg(Inp, Outp)) return REGISTER;
   else
    BEGIN
     *Outp = EvalIntExpression(Inp, Int8, &OK) & 0xff;
     return (OK) ?  IMM8 : NONE;
    END
END

/*---------------------------------------------------------------------------*/

static int InstrZ;

	static void DecodeFixed(Word Index)
BEGIN
   if (ArgCnt != 0) WrError(1110);
   else
    BEGIN
     DAsmCode[0] = ((LongWord) Index) << 24;
     DAsmCode[1] = 0x00000000;
     CodeLen = 8;
    END
END

	static void DecodeJmps(Word Index)
BEGIN
   LongWord Buf;
   LongInt Adr;
   int l;
   Boolean OK;

   if (ArgCnt == 0)
    if (Memo("INTFLY"))
     BEGIN
      ArgCnt = 1; strcpy(ArgStr[1], "0");
     END
    else if (Memo("RETURN"))
     BEGIN
      ArgCnt = 1; *ArgStr[1] = '\0';
     END
   if ((ArgCnt != 2) AND (ArgCnt!= 1)) WrError(1110);
   else
    BEGIN
     if (ArgCnt == 1)
      if (Memo("RETURN")) strcpy(ArgStr[2], ArgStr[1]);
      else *ArgStr[2] = '\0';
     Buf = 0;
     if (Memo("RETURN")) Adr = 0;
     else
      BEGIN
       l = strlen(ArgStr[1]);
       if ((strncasecmp(ArgStr[1], "REL(", 4) == 0) AND (ArgStr[1][l - 1] == ')'))
        BEGIN
         if (*OpPart == 'I') 
          BEGIN
           WrError(1350); OK = False;
          END
         Buf |= 0x00800000;
         strcpy(ArgStr[1], ArgStr[1] + 4); ArgStr[1][l - 5] = '\0';
        END
       Adr = EvalIntExpression(ArgStr[1], UInt32, &OK);
       if ((OK) AND (Buf != 0))
        BEGIN
         Adr -= EProgCounter() + 8;
         if ((NOT SymbolQuestionable) AND ((Adr > 0x7fffff) OR (Adr < -0x800000)))
          BEGIN
           WrError(1370); OK = False;
          END
        END
      END
     if ((OK) AND (DecodeCond(ArgStr[2], &Buf)))
      BEGIN
       DAsmCode[0] = 0x80000000 + (((LongWord) Index) << 27) + Buf;
       if (Memo("INTFLY")) DAsmCode[0] |= 0x00100000;
       DAsmCode[1] = Adr;
       CodeLen = 8;
      END
    END
END

	static void DecodeCHMOV(Word Index)
BEGIN
   String Token;
   char *Adr = Nil;
   LongWord Phase;
   Boolean OK;

   if ((MomCPU != CPU53C825) AND (MomCPU != CPU53C875) AND (MomCPU != CPU53C895)) WrError(1500);
   else if ((ArgCnt<2) OR (ArgCnt>3)) WrError(1110);
   else
    BEGIN
     GetToken(ArgStr[ArgCnt], Token);
     if (strcasecmp(Token, "WITH") == 0) DAsmCode[0] = 0x08000000;
     else if (strcasecmp(Token, "WHEN") == 0) DAsmCode[0] = 0x00000000;
     else
      BEGIN
       WrXError(1350, Token); return;
      END
     KillBlanks(ArgStr[ArgCnt]);
     if (NOT DecodePhase(ArgStr[ArgCnt], &Phase)) WrXError(1350, ArgStr[ArgCnt]);
     else
      BEGIN
       DAsmCode[0] |= Phase << 24;
       OK = False;
       if (ArgCnt == 2)
        BEGIN
         GetToken(ArgStr[1], Token);
         if (strcasecmp(Token, "FROM") != 0) WrError(1350);
         else
          BEGIN
           Adr = ArgStr[1];
           DAsmCode[0] |= 0x10000000;
           OK = True;
          END
        END
       else
        BEGIN
         Phase = EvalIntExpression(ArgStr[1], UInt24, &OK);
         if (OK)
          BEGIN
           DAsmCode[0] |= Phase;
           if (strncasecmp(ArgStr[2],"PTR", 3) == 0)
            BEGIN
             strcpy(ArgStr[2], ArgStr[2] + 4);
             DAsmCode[0] |= 0x20000000;
            END
           Adr = ArgStr[2];
          END
        END
       if (OK)
        BEGIN
         KillPrefBlanks(Adr);
         DAsmCode[1] = EvalIntExpression(Adr, UInt32, &OK);
         if (OK) CodeLen = 8;
        END
      END
    END
END

	static void DecodeFlags(Word Index)
BEGIN
   Boolean OK;
   String Token;

   if (ArgCnt != 1) WrError(1110);
   else
    BEGIN
     OK = True;
     DAsmCode[0] = ((LongWord) Index) << 24;
     DAsmCode[1] = 0;
     while ((OK) AND (*ArgStr[1] != '\0'))
      BEGIN
       GetToken(ArgStr[1], Token);
       if (strcasecmp(Token, "ACK") == 0) DAsmCode[0] |= 0x00000040;
       else if (strcasecmp(Token, "ATN") == 0) DAsmCode[0] |= 0x00000008;
       else if (strcasecmp(Token, "TARGET") == 0) DAsmCode[0] |= 0x00000200;
       else if (strcasecmp(Token, "CARRY") == 0) DAsmCode[0] |= 0x00000400;
       else
        BEGIN
         OK = False; WrXError(1350, Token);
        END
       if ((OK) AND (*ArgStr[1] != '\0'))
        BEGIN
         GetToken(ArgStr[1], Token);
         if (strcasecmp(Token, "AND") != 0)
          BEGIN
           OK = False; WrXError(1350, Token);
          END
        END
      END
     if (OK) CodeLen = 8;
    END
END

	static void DecodeRegTrans(Word Index)
BEGIN
   LongWord Reg, Cnt;
   Boolean OK;

   if (ArgCnt != 3) WrError(1110);
   else if (MomCPU == CPU53C815) WrError(1500);
   else if (NOT DecodeReg(ArgStr[1], &Reg)) WrXError(1445, ArgStr[1]);
   else
    BEGIN
     FirstPassUnknown = False;
     Cnt = EvalIntExpression(ArgStr[2], UInt3, &OK);
     if (FirstPassUnknown) Cnt = 1;
     if ((OK) AND (ChkRange(Cnt, 1, 4)))
      BEGIN
       DAsmCode[0] = 0xe0000000 + (((LongInt) Index) << 24) + (Reg << 16) + Cnt;
       if ((strncasecmp(ArgStr[3], "DSAREL(", 7) == 0)
       AND (ArgStr[3][strlen(ArgStr[3]) - 1] == ')'))
        BEGIN
         DAsmCode[0] |= 0x10000000;
         DAsmCode[1] = EvalIntExpression(ArgStr[3] + 6, SInt24, &OK) & 0xffffff;
        END
       else DAsmCode[1] = EvalIntExpression(ArgStr[3], UInt32, &OK);
       if (OK) CodeLen = 8;
      END
    END
END

	static void DecodeMOVE(Word Index)
BEGIN
   Boolean WithCarry, BigCPU;
   String Token;
   LongWord Tmp, DReg , AriOp = 0xff, ImmVal = 0x100;
   String Parts[8];
   Boolean OK;

   BigCPU = (MomCPU == CPU53C825) OR (MomCPU == CPU53C875) OR (MomCPU == CPU53C895);

   if ((ArgCnt > 3) OR (ArgCnt < 1)) WrError(1110);
   else if (ArgCnt == 1) /* MOVE Register */
    BEGIN
     ArgCnt = 0;
     do
      BEGIN
       GetToken(ArgStr[1], Parts[ArgCnt++]);
      END
     while ((*ArgStr[1] != '\0') AND (ArgCnt < 8));
     if ((ArgCnt > 1) AND (strcasecmp(Parts[ArgCnt - 1], "CARRY") == 0) AND (strcasecmp(Parts[ArgCnt - 1], "TO") == 0))
      BEGIN
       WithCarry = True; ArgCnt -= 2;
      END
     else WithCarry = False;
     DAsmCode[0] = 0x40000000;
     DAsmCode[1] = 0;
     if (ArgCnt == 3)
      BEGIN
       if (WithCarry) WrError(1350);
       else if (strcasecmp(Parts[1], "TO") == 0) /* MOVE */
        BEGIN
         switch (DecodeComp(Parts[0], &ImmVal))
          BEGIN
           case SFBR:
            switch (DecodeComp(Parts[2], &ImmVal))
             BEGIN
              case SFBR:
               ImmVal = 8;
              case REGISTER: /* --> 0x00 OR SFBR to reg */
               DAsmCode[0] += 0x2a000000 + (ImmVal << 16);
               CodeLen = 8;
               break;
              default:
               WrError(1350);
             END
            break;
           case REGISTER:
            DReg = ImmVal;
            switch (DecodeComp(Parts[2], &ImmVal))
             BEGIN
              case SFBR: /* --> 0x00 OR reg to SFBR */
               DAsmCode[0] += 0x32000000 + (DReg << 16);
               CodeLen = 8;
               break;
              case REGISTER:
               if (ImmVal != DReg) WrError(1350);
               else
                BEGIN
                 DAsmCode[0] += 0x3a000000 + (DReg << 16);
                 CodeLen = 8;
                END
               break;
              default:
               WrError(1350);
             END
            break;
           case IMM8:
            switch (DecodeComp(Parts[2], &DReg))
             BEGIN
              case SFBR:
               DReg = 8;
              case REGISTER: /* --> imm to reg */
               DAsmCode[0] += 0x38000000 + (DReg << 16) + (ImmVal << 8);
               CodeLen = 8;
               break;
              default:
               WrError(1350);
             END
            break;
           default:
            WrError(1350);
            break;
          END
        END /* ... TO ... */
       else if ((strcasecmp(Parts[1], "SHL") == 0) OR (strcasecmp(Parts[1], "SHR") == 0))
        BEGIN
         AriOp = 1 + (Ord(toupper(Parts[1][2]) == 'R') << 2);
         switch (DecodeComp(Parts[0], &DReg))
          BEGIN
            case SFBR:
             switch (DecodeComp(Parts[2], &DReg))
              BEGIN
               case SFBR:
                DReg = 8;
               case REGISTER:
                DAsmCode[0] += 0x28000000 + (AriOp << 24) + (DReg << 16);
                CodeLen = 8;
                break;
               default:
                WrError(1350);
              END
             break;
            case REGISTER:
             ImmVal = DReg;
             switch (DecodeComp(Parts[2], &DReg))
              BEGIN
               case SFBR:
                DAsmCode[0] += 0x30000000 + (AriOp << 24) + (ImmVal << 16);
                CodeLen = 8;
                break;
               case REGISTER:
                if (DReg != ImmVal) WrError(1350);
                else
                 BEGIN
                  DAsmCode[0] += 0x38000000 + (AriOp << 24) + (ImmVal << 16);
                  CodeLen = 8;
                 END
                break;
               default:
                WrError(1350);
              END
             break;
           default:
            WrError(1350);
          END
        END /* ... SHx ... */
      END /* ArgCnt == 3 */
     else if (ArgCnt == 5)
      BEGIN
       if (strcasecmp(Parts[3], "TO") != 0) WrError(1350);
       else
        BEGIN
         if (strcasecmp(Parts[1], "XOR") == 0) AriOp = 3;
         else if (strcasecmp(Parts[1], "OR") == 0) AriOp = 2;
         else if (strcasecmp(Parts[1], "AND") == 0) AriOp = 4;
         else if (strcmp(Parts[1], "+") == 0) AriOp = 6;
         if (WithCarry) AriOp = (AriOp == 6) ? 7 : 0xff;
         if (AriOp == 0xff) WrError(1350);
         else
          BEGIN
           DAsmCode[0] |= (AriOp << 24);
           switch (DecodeComp(Parts[0], &ImmVal))
            BEGIN
             case SFBR:
              switch (DecodeComp(Parts[2], &ImmVal))
               BEGIN
                case SFBR:
                 switch (DecodeComp(Parts[4], &ImmVal))
                  BEGIN
                   case SFBR:
                    ImmVal = 8;
                   case REGISTER:
                    if (NOT BigCPU) WrError(1500);
                    else
                     BEGIN
                      DAsmCode[0] |= 0x28800000 + (ImmVal << 16);
                      CodeLen = 8;
                     END
                    break;
                   default:
                    WrError(1350);
                  END
                 break;
                case IMM8:
                 switch (DecodeComp(Parts[4], &DReg))
                  BEGIN
                   case SFBR:
                    DReg = 8;
                   case REGISTER:
                    DAsmCode[0] |= 0x28000000 + (DReg << 16) + (ImmVal << 8);
                    CodeLen = 8;
                    break;
                   default:
                    WrError(1350);
                  END
                 break;
                default:
                 WrError(1350);
               END
              break;
             case REGISTER:
              DAsmCode[0] |= ImmVal << 16;
              DReg = ImmVal;
              switch (DecodeComp(Parts[2], &ImmVal))
               BEGIN
                case SFBR:
                 switch (DecodeComp(Parts[4], &ImmVal))
                  BEGIN
                   case SFBR:
                    if (NOT BigCPU) WrError(1500);
                    else
                     BEGIN
                      DAsmCode[0] |= 0x30800000;
                      CodeLen = 8;
                     END
                    break;
                   case REGISTER:
                    if (DReg != ImmVal) WrError(1350);
                    else if (NOT BigCPU) WrError(1500);
                    else
                     BEGIN
                      DAsmCode[0] |= 0x38800000;
                      CodeLen = 8;
                     END
                    break;
                   default:
                    WrError(1350);
                  END
                 break;
                case IMM8:
                 DAsmCode[0] |= (ImmVal << 8);
                 switch (DecodeComp(Parts[4], &Tmp))
                  BEGIN
                   case SFBR:
                    DAsmCode[0] |= 0x30000000;
                    CodeLen = 8;
                    break;
                   case REGISTER: 
                    if (DReg != Tmp) WrError(1350);
                    else
                     BEGIN
                      DAsmCode[0] |= 0x38000000;
                      CodeLen = 8;
                     END
                    break;
                   default:
                    WrError(1350);
                  END
                 break;
                default:
                 WrError(1350);
               END
              break;
             default:
              WrError(1350);
            END
          END
        END
      END  /* ArgCnt == 5 */
     else WrError(1350);
    END
   else if (ArgCnt == 2)
    BEGIN
     GetToken(ArgStr[1], Token);
     if (strcasecmp(Token, "FROM") != 0) WrError(1350);
     else
      BEGIN
       DAsmCode[0] = 0x00000000;
       DAsmCode[1] = EvalIntExpression(ArgStr[1], Int32, &OK);
       if (OK)
        BEGIN
         GetToken(ArgStr[2], Token);
         OK = True;
         if (strcasecmp(Token, "WHEN") == 0) DAsmCode[0] |= 0x08000000;
         else if (strcasecmp(Token, "WITH") != 0) OK = False;
         if (NOT OK) WrError(1350);
         else
          BEGIN
           KillBlanks(ArgStr[2]);
           if (NOT DecodePhase(ArgStr[2], &ImmVal)) WrXError(1350, ArgStr[2]);
           else
            BEGIN
             DAsmCode[0] |= ImmVal << 24;
             CodeLen = 8;
            END
          END
        END
      END
    END
   else if (ArgCnt == 3)
    BEGIN
     if (strncasecmp(ArgStr[1], "MEMORY", 6) == 0)
      BEGIN
       strcpy(ArgStr[1], ArgStr[1] + 7);
       if (strncasecmp(ArgStr[1], "NO FLUSH", 8) == 0)
        BEGIN
         DAsmCode[0] = 0xc1000000;
         strcpy(ArgStr[1], ArgStr[1] + 9);
        END
       else DAsmCode[0] = 0xc0000000;
       DAsmCode[0] |= EvalIntExpression(ArgStr[1], UInt24, &OK);
       if (OK)
        BEGIN
         DAsmCode[1] = EvalIntExpression(ArgStr[2], Int32, &OK);
         if (OK)
          BEGIN
           DAsmCode[2] = EvalIntExpression(ArgStr[3], Int32, &OK);
           if (OK) CodeLen = 12;
          END
        END
      END
     else
      BEGIN
       DAsmCode[0] = EvalIntExpression(ArgStr[1], UInt24, &OK);
       if (OK)
        BEGIN
         GetToken(ArgStr[3], Token);
         OK = True;
         if (strcasecmp(Token, "WHEN") == 0) DAsmCode[0] |= 0x08000000;
         else if (strcasecmp(Token, "WITH") != 0) OK = False;
         if (NOT OK) WrError(1350);
         else
          BEGIN
           KillBlanks(ArgStr[3]);
           if (NOT DecodePhase(ArgStr[3], &ImmVal)) WrXError(1350, ArgStr[2]);
           else
            BEGIN 
             DAsmCode[0] |= ImmVal << 24;
             if (strncasecmp(ArgStr[2], "PTR", 3) == 0)
              BEGIN
               strcpy(ArgStr[2], ArgStr[2] + 4);
               DAsmCode[0] |= 0x20000000;
              END
             DAsmCode[1] = EvalIntExpression(ArgStr[2], UInt32, &OK);
             if (OK) CodeLen = 8;
            END
          END
        END
      END
    END
   else WrError(1110);
END

	static void DecodeSELECT(Word MayATN)
BEGIN
   Boolean OK;
   LongInt Dist;
   int l;

   if (ArgCnt != 2) WrError(1110);
   else
    BEGIN
     DAsmCode[0] = 0x40000000; OK = True;
     if (strncasecmp(ArgStr[1], "ATN ", 4) == 0)
      BEGIN
       strcpy(ArgStr[1], ArgStr[1] + 4); KillPrefBlanks(ArgStr[1]);
       if (NOT MayATN) OK = False;
       else DAsmCode[0] |= 0x01000000;
      END
     if (NOT OK) WrError(1350);
     else
      BEGIN
       if (strncasecmp(ArgStr[1], "FROM ", 5) == 0)
        BEGIN
         strcpy(ArgStr[1], ArgStr[1] + 5); KillPrefBlanks(ArgStr[1]);
         DAsmCode[0] |= 0x02000000 + EvalIntExpression(ArgStr[1], UInt24, &OK);
        END
       else DAsmCode[0] |= EvalIntExpression(ArgStr[1], UInt4, &OK) << 16;
       if (OK)
        BEGIN
         l = strlen(ArgStr[2]);
         if ((strncasecmp(ArgStr[2], "REL(", 4) == 0) AND (ArgStr[2][l - 1] == ')'))
          BEGIN
           DAsmCode[0] |= 0x04000000; ArgStr[2][l - 1] = '\0';
           Dist = EvalIntExpression(ArgStr[2] + 4, UInt32, &OK) - (EProgCounter() + 8);
           if (OK)
            if ((NOT SymbolQuestionable) AND ((Dist > 0x7fffff) OR (Dist < -0x800000))) WrError(1370);
            else DAsmCode[1] = Dist & 0xffffff;
          END
         else DAsmCode[1] = EvalIntExpression(ArgStr[2], UInt32, &OK);
         if (OK) CodeLen = 8;
        END
      END
    END
END

	static void DecodeWAIT(Word Index)
BEGIN
   String Token;
   int l;
   Boolean OK;
   LongInt Dist;

   if (ArgCnt != 1) WrError(1110);
   else
    BEGIN
     GetToken(ArgStr[1], Token); KillPrefBlanks(ArgStr[1]);
     if (strcasecmp(Token, "DISCONNECT") == 0)
      BEGIN
       if (*ArgStr[1] != '\0') WrError(1350);
       else
        BEGIN
         DAsmCode[0] = 0x48000000;
         DAsmCode[1] = 0;
         CodeLen = 8;
        END
      END
     else if ((strcasecmp(Token, "RESELECT") == 0) OR (strcasecmp(Token, "SELECT") == 0))
      BEGIN
       l = strlen(ArgStr[1]);
       if ((strncasecmp(ArgStr[1], "REL(", 4) == 0) AND (ArgStr[1][l - 1] == ')'))
        BEGIN
         ArgStr[1][l - 1] = '\0';
         DAsmCode[0] = 0x54000000;
         Dist = EvalIntExpression(ArgStr[1] + 4, UInt32, &OK) - (EProgCounter() + 8);
         if (OK)
          if ((NOT SymbolQuestionable) AND ((Dist > 0x7fffff) OR (Dist < -0x800000))) WrError(1370);
          else DAsmCode[1] = Dist & 0xffffff;
        END
       else
        BEGIN
         DAsmCode[0] = 0x50000000;
         DAsmCode[1] = EvalIntExpression(ArgStr[1], UInt32, &OK);
        END
       if (OK)
        BEGIN
         if (toupper(*Token) == 'S') DAsmCode[0] |= 0x00000200;
         CodeLen = 8;
        END
      END
    END
END

/*---------------------------------------------------------------------------*/

static int InstrZ;

	static void AddReg(char *NName, LargeWord Adr, Word Mask)
BEGIN
   if (InstrZ >= RegCnt) exit(255);
   Regs[InstrZ].Name = NName;
   Regs[InstrZ].Code = Adr;
   Regs[InstrZ++].Mask = Mask;
END

	static void InitFields(void)
BEGIN
   InstTable = CreateInstTable(51);
   AddInstTable(InstTable, "NOP"     , 0x80, DecodeFixed);
   AddInstTable(InstTable, "DISCONNECT" , 0x48, DecodeFixed);
   AddInstTable(InstTable, "JUMP"    , 0,    DecodeJmps);
   AddInstTable(InstTable, "CALL"    , 1,    DecodeJmps);
   AddInstTable(InstTable, "RETURN"  , 2,    DecodeJmps);
   AddInstTable(InstTable, "INT"     , 3,    DecodeJmps);
   AddInstTable(InstTable, "INTFLY"  , 3,    DecodeJmps);
   AddInstTable(InstTable, "CHMOV"   , 0,    DecodeCHMOV);
   AddInstTable(InstTable, "CLEAR"   , 0x60, DecodeFlags);
   AddInstTable(InstTable, "SET"     , 0x58, DecodeFlags);
   AddInstTable(InstTable, "LOAD"    , 1,    DecodeRegTrans);
   AddInstTable(InstTable, "STORE"   , 0,    DecodeRegTrans);
   AddInstTable(InstTable, "MOVE"    , 0,    DecodeMOVE);
   AddInstTable(InstTable, "RESELECT", 0,    DecodeSELECT);
   AddInstTable(InstTable, "SELECT"  , 1,    DecodeSELECT);
   AddInstTable(InstTable, "WAIT"    , 0,    DecodeWAIT);

   Regs = (PReg) malloc(sizeof(TReg) * RegCnt); InstrZ = 0;
   AddReg("SCNTL0"   , 0x00, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SCNTL1"   , 0x01, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SCNTL2"   , 0x02, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SCNTL3"   , 0x03, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SCID"     , 0x04, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SXFER"    , 0x05, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SDID"     , 0x06, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("GPREG"    , 0x07, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SFBR"     , 0x08, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SOCL"     , 0x09, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SSID"     , 0x0a, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SBCL"     , 0x0b, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("DSTAT"    , 0x0c, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SSTAT0"   , 0x0d, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SSTAT1"   , 0x0e, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SSTAT2"   , 0x0f, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("DSA"      , 0x10, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("ISTAT"    , 0x14, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("CTEST0"   , 0x18, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("CTEST1"   , 0x19, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("CTEST2"   , 0x1a, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("CTEST3"   , 0x1b, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("TEMP"     , 0x1c, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("DFIFO"    , 0x20, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("CTEST4"   , 0x21, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("CTEST5"   , 0x22, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);  
   AddReg("CTEST6"   , 0x23, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("DBC"      , 0x24, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("DCMD"     , 0x27, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("DNAD"     , 0x28, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("DSP"      , 0x2c, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("DSPS"     , 0x30, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SCRATCHA" , 0x34, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("DMODE"    , 0x38, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("DIEN"     , 0x39, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SBR"      , 0x3a, M_53C810                       + M_53C860 + M_53C875 + M_53C895);
   AddReg("DWT"      , 0x3a,            M_53C815                                            );
   AddReg("DCNTL"    , 0x3b, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("ADDER"    , 0x3c, M_53C810            + M_53C825 + M_53C860            + M_53C895);
   AddReg("SIEN0"    , 0x40, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SIEN1"    , 0x41, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SIST0"    , 0x42, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SIST1"    , 0x43, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SLPAR"    , 0x44, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SWIDE"    , 0x45,                       M_53C825            + M_53C875 + M_53C895);
   AddReg("MACNTL"   , 0x46, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("GPCNTL"   , 0x47, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("STIME0"   , 0x48, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("STIME1"   , 0x49, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("RESPID"   , 0x4a, M_53C810 + M_53C815 + M_53C825 + M_53C860                      );
   AddReg("RESPID0"  , 0x4a,                                             M_53C875 + M_53C895);
   AddReg("RESPID1"  , 0x4b,                                             M_53C875 + M_53C895);
   AddReg("STEST0"   , 0x4c, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("STEST1"   , 0x4d, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("STEST2"   , 0x4e, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("STEST3"   , 0x4f, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SIDL"     , 0x50, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("STEST4"   , 0x52,                                                      + M_53C895);
   AddReg("SODL"     , 0x54, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SBDL"     , 0x58, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SCRATCHB" , 0x5c, M_53C810 + M_53C815 + M_53C825 + M_53C860 + M_53C875 + M_53C895);
   AddReg("SCRATCHC" , 0x60,                                             M_53C875 + M_53C895);
   AddReg("SCRATCHD" , 0x64,                                             M_53C875 + M_53C895);
   AddReg("SCRATCHE" , 0x68,                                             M_53C875 + M_53C895);
   AddReg("SCRATCHF" , 0x6c,                                             M_53C875 + M_53C895);
   AddReg("SCRATCHG" , 0x70,                                             M_53C875 + M_53C895);
   AddReg("SCRATCHH" , 0x74,                                             M_53C875 + M_53C895);
   AddReg("SCRATCHI" , 0x78,                                             M_53C875 + M_53C895);
   AddReg("SCRATCHJ" , 0x7c,                                             M_53C875 + M_53C895);
END

	static void DeinitFields(void)
BEGIN
   DestroyInstTable(InstTable);
   free(Regs);
END

/*---------------------------------------------------------------------------*/

	static void MakeCode_53c8xx(void)
BEGIN
   CodeLen=0; DontPrint=False;

   /* zu ignorierendes */   

   if (Memo("")) return; 

   if (NOT LookupInstTable(InstTable,OpPart)) WrXError(1200, OpPart);
END

	static Boolean IsDef_53c8xx(void)
BEGIN
   return False;
END

	static void SwitchFrom_53c8xx(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_53c8xx(void)
BEGIN
   PFamilyDescr FoundDescr;

   FoundDescr=FindFamilyByName("SYM53C8xx");

   TurnWords=False; ConstMode=ConstModeC; SetIsOccupied=True;
   PCSymbol="$"; HeaderID=FoundDescr->Id; NOPCode=0;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode);
   Grans[SegCode ]=1; ListGrans[SegCode ]=4; SegInits[SegCode ]=0;
#ifdef __STDC__
   SegLimits[SegCode] = 0xfffffffful;
#else
   SegLimits[SegCode] = 0xffffffffl;
#endif

   MakeCode=MakeCode_53c8xx; IsDef=IsDef_53c8xx;
   SwitchFrom=SwitchFrom_53c8xx;

   InitFields();
END

/*---------------------------------------------------------------------------*/

	void code53c8xx_init(void)
BEGIN
   CPU53C810 = AddCPU("SYM53C810", SwitchTo_53c8xx);
   CPU53C860 = AddCPU("SYM53C860", SwitchTo_53c8xx);
   CPU53C815 = AddCPU("SYM53C815", SwitchTo_53c8xx);
   CPU53C825 = AddCPU("SYM53C825", SwitchTo_53c8xx);
   CPU53C875 = AddCPU("SYM53C875", SwitchTo_53c8xx);
   CPU53C895 = AddCPU("SYM53C895", SwitchTo_53c8xx);
END
