/*
 * AS-Portierung 
 *
 * AS-Codegeneratormodul fuer die Texas Instruments TMS320C5x-Familie
 *
 * (C) 1996 Thomas Sailer <sailer@ife.ee.ethz.ch>
 *
 * 20.08.96: Erstellung
 *  7.07.1998 Fix Zugriffe auf CharTransTable wg. signed chars
 * 18.08.1998 BookKeeping-Aufruf in RES
 *  9. 1.1999 ChkPC jetzt ueber SegLimits
 * 30. 5.1999 Erweiterung auf C203 abgeschlossen, Hashtabelle fuer
 *            Prozessorbefehle erledigt
 *  9. 3.2000 'ambiguous else'-Warnungen beseitigt
 * 14. 1.2001 silenced warnings about unused parameters
 * 2001-11-11 use DecodeTIPSeudo
 */
/* $Id: code3205x.c,v 1.4 2005/09/08 16:53:39 alfred Exp $                   */
/*****************************************************************************
 * $Log: code3205x.c,v $
 * Revision 1.4  2005/09/08 16:53:39  alfred
 * - use common PInstTable
 *
 * Revision 1.3  2004/09/26 14:42:44  alfred
 * - remove warning
 *
 * Revision 1.2  2004/05/29 12:18:05  alfred
 * - relocated DecodeTIPseudo() to separate module
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "bpemu.h"
#include "strutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"
#include "codepseudo.h"
#include "codevars.h"
#include "tipseudo.h"
#include "endian.h"

#include "code3205x.h"

/* ---------------------------------------------------------------------- */

typedef struct {
        char *name;
        CPUVar mincpu;
        Word code;
} cmd_fixed;
typedef struct {
        char *name;
        CPUVar mincpu;
        Word code;
        Boolean cond;
} cmd_jmp;
typedef struct {
        char *name;
        CPUVar mincpu;
        Word mode;
} adr_mode_t;
typedef struct {
        char *name;
        CPUVar mincpu;
        Word codeand;
        Word codeor;
        Byte iszl;
        Byte isc;
        Byte isv;
        Byte istp;
} condition;
typedef struct {
        char *name;
        CPUVar mincpu;
        Word code;
} bit_table_t;

static cmd_fixed *cmd_fixed_order;
#define cmd_fixed_cnt 45
static cmd_fixed *cmd_adr_order;
#define cmd_adr_cnt 32
static cmd_jmp *cmd_jmp_order;
#define cmd_jmp_cnt 11
static cmd_fixed *cmd_plu_order;
#define cmd_plu_cnt 7
static adr_mode_t *adr_modes;
#define adr_mode_cnt 10
static condition *cond_tab;
#define cond_cnt 15
static bit_table_t *bit_table;
#define bit_cnt 9

static Word adr_mode;
static Boolean adr_ok;

static CPUVar cpu_320203;
static CPUVar cpu_32050;
static CPUVar cpu_32051;
static CPUVar cpu_32053;

/* ---------------------------------------------------------------------- */

static Word eval_ar_expression(char *asc, Boolean *ok)
{
        *ok = True;

        if ((toupper(asc[0]) == 'A') && (toupper(asc[1]) == 'R') && (asc[2] >= '0') && 
            (asc[2] <= '7') && (asc[3] <= '\0'))
                return asc[2] - '0';
        return EvalIntExpression(asc, UInt3, ok);
}

/* ---------------------------------------------------------------------- */

        static Boolean decode_adr(char *arg, int aux, Boolean must1)
{
  Word h;
  adr_mode_t *am = adr_modes;

  /* Annahme: nicht gefunden */

  adr_ok = False;

  /* Adressierungsmodus suchen */

  while (am->name && strcasecmp(am->name, arg))
   am++;

  /* nicht gefunden: dann absolut */

  if (!am->name)
   BEGIN
    /* ARn-Register darf dann nicht vorhanden sein */
    if (aux <= ArgCnt)
     BEGIN
      WrError(1110); return FALSE;
     END

    /* Adresse berechnen */
    h = EvalIntExpression(arg, Int16, &adr_ok);
    if (!adr_ok) return FALSE;

    /* Adresslage pruefen */
    if (must1 && (h >= 0x80) && (!FirstPassUnknown))
     BEGIN
      WrError(1315); 
      adr_ok = False;
      return FALSE;
     END

    /* nur untere 7 Bit gespeichert */
    adr_mode = h & 0x7f; 
    ChkSpace(SegData);
   END

  /* ansonsten evtl. noch Adressregister dazu */

  else
   BEGIN
    /* auf dieser CPU nicht erlaubter Modus ? */

    if (am->mincpu > MomCPU)
     BEGIN
      WrError(1505); return FALSE;
     END

    adr_mode = am->mode;
    if (aux <= ArgCnt)
     BEGIN
      h = eval_ar_expression(ArgStr[aux], &adr_ok);
      if (adr_ok) adr_mode |= 0x8 | h;
     END
    else
     adr_ok = True;
   END

  return adr_ok;
END

/* ---------------------------------------------------------------------- */

static Word decode_cond(int argp)
{
        condition *cndp;
        Byte cntzl = 0, cntc = 0, cntv = 0, cnttp = 0;
        Word ret = 0x300;

        while(argp <= ArgCnt) {
                for(cndp = cond_tab; 
                    cndp->name && strcasecmp(cndp->name, ArgStr[argp]); cndp++);
                if (!cndp->name) {
                        WrError(1360);
                        return ret;
                }
                ret &= cndp->codeand;
                ret |= cndp->codeor;
                cntzl += cndp->iszl;
                cntc += cndp->isc;
                cntv += cndp->isv;
                cnttp += cndp->istp;
                argp++;
        }
        if ((cnttp > 1) || (cntzl > 1) || (cntv > 1) || (cntc > 1)) 
                WrXError(1200, ArgStr[argp]); /* invalid condition */
        return ret;
}

/* ---------------------------------------------------------------------- */

        static Word DecodeShift(char *arg, Boolean *ok)
BEGIN
   Word Shift;

   FirstPassUnknown = False;
   Shift = EvalIntExpression(arg, UInt5, ok);
   if (*ok)
    BEGIN
     if (FirstPassUnknown) Shift &= 15;
     *ok = ChkRange(Shift, 0, 16);
    END
   return Shift;
END

/* ---------------------------------------------------------------------- */

        static void DecodeFixed(Word Index)
BEGIN
   cmd_fixed *fo = cmd_fixed_order + Index;

   if (ArgCnt != 0)
    BEGIN
     WrError(1110);
     return;
    END

   if (fo->mincpu > MomCPU)
    BEGIN
     WrError(1500);
     return;
    END

   CodeLen = 1; 
   WAsmCode[0] = fo->code;
END

        static void DecodeCmdAdr(Word Index)
BEGIN
   cmd_fixed *fo = cmd_adr_order + Index;

   if ((ArgCnt < 1) OR (ArgCnt > 2))
    BEGIN
     WrError(1110);
     return;
    END

   if (MomCPU < fo->mincpu)
    BEGIN
     WrError(1500);
     return;
    END

   decode_adr(ArgStr[1], 2, False);
   if (adr_ok)
    BEGIN
     CodeLen = 1; 
     WAsmCode[0] = fo->code | adr_mode;
    END
END

        static void DecodeCmdJmp(Word Index)
BEGIN
   cmd_jmp *jo = cmd_jmp_order + Index;
   Boolean ok;

   if (MomCPU < jo->mincpu)
    BEGIN
     WrError(1500);
     return;
    END

   if ((ArgCnt < 1) || ((ArgCnt > 3) && (!jo->cond)))
    BEGIN
     WrError(1110);
     return;
    END

   adr_mode  =  0;
   if (jo->cond)
    adr_mode = decode_cond(2);
   else if (ArgCnt > 1)
    BEGIN
     decode_adr(ArgStr[2], 3, False);
     if (adr_mode < 0x80)
      BEGIN
       WrError(1350);
       return;
      END
     adr_mode &= 0x7f;
    END

   WAsmCode[1] = EvalIntExpression(ArgStr[1], Int16, &ok);
   if (!ok) return;         

   CodeLen = 2; 
   WAsmCode[0] = jo->code | adr_mode;
END

        static void DecodeCmdPlu(Word Index)
BEGIN
   Boolean ok;

   cmd_fixed *fo = cmd_plu_order + Index;

   if (MomCPU < fo->mincpu) WrError(1500);
   else if (*ArgStr[1] == '#')
    BEGIN
     if ((ArgCnt < 2) OR (ArgCnt > 3)) WrError(1110);
     else
      BEGIN
       decode_adr(ArgStr[2], 3, False);
       WAsmCode[1] = EvalIntExpression(ArgStr[1] + 1, Int16, &ok);
       if ((ok) AND (adr_ok)) 
        BEGIN
         CodeLen = 2;
         WAsmCode[0] = fo->code | 0x0400 | adr_mode;
        END
      END
    END
   else if (strlen(OpPart) == 4) WrError(1120);
   else
    BEGIN
     if ((ArgCnt < 1) OR (ArgCnt > 2)) WrError(1110);
     else
      BEGIN
       decode_adr(ArgStr[1], 2, False);
       if (adr_ok)
        BEGIN
         CodeLen = 1;
         WAsmCode[0] = fo->code | adr_mode;
        END
      END
    END
END

        static void DecodeADDSUB(Word Index)
BEGIN
   Word Shift;
   LongInt adr_long;
   Boolean ok;

   if ((ArgCnt < 1) || (ArgCnt > 3)) WrError(1110);
   else
    BEGIN
     if (*ArgStr[1] == '#')
      BEGIN
       if (ArgCnt > 2) WrError(1110);
       else
        BEGIN
         ok = True;
         if (ArgCnt == 1) Shift = 0;
         else Shift = EvalIntExpression(ArgStr[2], UInt4, &adr_ok);
         if (ok)
          BEGIN
           adr_long = EvalIntExpression(ArgStr[1] + 1, UInt16, &ok);
           if (ok)
            BEGIN
             if ((Shift == 0) && (Hi(adr_long) == 0))
              BEGIN
               CodeLen = 1;
               WAsmCode[0] = (Index << 9) | 0xb800 | (adr_long & 0xff);
              END
             else
              BEGIN
               CodeLen = 2;
               WAsmCode[0] = ((Index << 4) + 0xbf90) | (Shift & 0xf);
               WAsmCode[1] = adr_long;
              END
            END
          END
        END
      END
     else
      BEGIN
       decode_adr(ArgStr[1], 3, False);
       if (adr_ok)
        BEGIN
         ok = True;
         if (ArgCnt >= 2)
          Shift = DecodeShift(ArgStr[2], &ok);
         else
          Shift = 0;
         if (ok)
          BEGIN
           CodeLen = 1;
           if (Shift == 16)
            WAsmCode[0] = ((Index << 10) | 0x6100) | adr_mode;
           else
            WAsmCode[0] = ((Index << 12) | 0x2000) | ((Shift & 0xf) << 8) | adr_mode;
          END
        END
      END
    END
END

        static void DecodeADRSBRK(Word Index)
BEGIN
   Word adr_word;
   Boolean ok;

   if (ArgCnt != 1)
    BEGIN
     WrError(1110);
     return;
    END

   if (*ArgStr[1] != '#')
    BEGIN
     WrError(1120); /*invalid parameter*/
     return;
    END

   adr_word = EvalIntExpression(ArgStr[1] + 1, UInt8, &ok);
   if (ok)
    BEGIN
     CodeLen = 1;
     WAsmCode[0] = (Index << 10)| 0x7800 | (adr_word & 0xff);
    END
END

        static void DecodeLogic(Word Index)
BEGIN
   Boolean adr_ok, ok;
   Word Shift;

   if ((ArgCnt != 1) AND (ArgCnt != 2))
    BEGIN
     WrError(1110);
     return;
    END

   if (*ArgStr[1] == '#')
    BEGIN
     WAsmCode[1] = EvalIntExpression((ArgStr[1])+1, UInt16, &ok);
     Shift = 0;
     adr_ok = True;
     if (ArgCnt >= 2) 
      Shift = DecodeShift(ArgStr[2], &adr_ok);
     if ((ok) AND (adr_ok))
      BEGIN
       CodeLen = 2;
       if (Shift >= 16)
        WAsmCode[0] = 0xbe80 | Lo(Index);
       else
        WAsmCode[0] = 0xbfa0 + ((Index & 3) << 4) + (Shift & 0xf);
      END
    END
   else
    BEGIN
     if (decode_adr(ArgStr[1], 2, False))
      BEGIN
       CodeLen = 1; 
       WAsmCode[0] = (Index & 0xff00) | adr_mode;
      END
    END
END

        static void DecodeBIT(Word Index)
BEGIN
   Word bit;
   Boolean ok;
   UNUSED(Index);

   if ((ArgCnt < 2) || (ArgCnt > 3))
    BEGIN
     WrError(1110);
     return;
    END

   bit = EvalIntExpression(ArgStr[2], UInt4, &ok);

   decode_adr(ArgStr[1], 3, False);

   if ((adr_ok) AND (ok))
    BEGIN
     CodeLen = 1;
     WAsmCode[0] = 0x4000 | adr_mode | ((bit & 0xf) << 8);
    END
END

        static void DecodeBLDD(Word Index)
BEGIN
   Boolean ok;
   UNUSED(Index);

   if ((ArgCnt < 2) || (ArgCnt > 3))
    BEGIN
     WrError(1110);
     return;
    END

   if (!strcasecmp(ArgStr[1], "BMAR"))
    BEGIN
     if (MomCPU < cpu_32050) WrError(1500);
     else
      BEGIN
       decode_adr(ArgStr[2], 3, False);
       if (!adr_ok) return;
       CodeLen = 1;
       WAsmCode[0] = 0xac00 | adr_mode;
      END
     return;
    END

   if (!strcasecmp(ArgStr[2], "BMAR"))
    BEGIN
     if (MomCPU < cpu_32050) WrError(1500);
     else
      BEGIN
       decode_adr(ArgStr[1], 3, False);
       if (!adr_ok) return;
       CodeLen = 1;
       WAsmCode[0] = 0xad00 | adr_mode;
      END
     return;
    END

   if (*ArgStr[1] == '#')
    BEGIN
     WAsmCode[1] = EvalIntExpression((ArgStr[1])+1, Int16, &ok);
     decode_adr(ArgStr[2], 3, False);
     if ((!adr_ok) || (!ok)) return;
     CodeLen = 2;
     WAsmCode[0] = 0xa800 | adr_mode;          
     return;
    END

   if (*ArgStr[2] == '#')
    BEGIN
     WAsmCode[1] = EvalIntExpression((ArgStr[2])+1, Int16, &ok);
     decode_adr(ArgStr[1], 3, False);
     if ((!adr_ok) || (!ok)) return;
     CodeLen = 2;
     WAsmCode[0] = 0xa900 | adr_mode;
     return;
    END

   WrError(1350); /* invalid addr mode */
END

        static void DecodeBLPD(Word Index)
BEGIN
   Boolean ok;
   UNUSED(Index);

   if ((ArgCnt < 2) || (ArgCnt > 3))
    BEGIN
     WrError(1110);
     return;
    END

   if (!strcasecmp(ArgStr[1], "BMAR"))
    BEGIN
     if (MomCPU < cpu_32050) WrError(1500);
     else
      BEGIN
       decode_adr(ArgStr[2], 3, False);
       if (!adr_ok) return;
       CodeLen = 1;
       WAsmCode[0] = 0xa400 | adr_mode;         
      END
     return;
    END

   if (*ArgStr[1] == '#')
    BEGIN
     WAsmCode[1] = EvalIntExpression((ArgStr[1])+1, Int16, &ok);
     decode_adr(ArgStr[2], 3, False);
     if ((!adr_ok) || (!ok)) return;
     CodeLen = 2;
     WAsmCode[0] = 0xa500 | adr_mode;          
     return;
    END

   WrError(1350); /* invalid addressing mode */
END

        static void DecodeCLRSETC(Word Index)
BEGIN
  bit_table_t *bitp;

  if (ArgCnt != 1)
   BEGIN
    WrError(1110);
    return;
   END

  WAsmCode[0] = Index; NLS_UpString(ArgStr[1]);

  for(bitp = bit_table; bitp->name; bitp++)
   if (!strcmp(ArgStr[1], bitp->name))
    BEGIN
     if (bitp->mincpu > MomCPU) WrError(1500);
     else
      BEGIN
       WAsmCode[0] |= bitp->code;
       CodeLen = 1;
      END
     return;
    END

  WrXError(1445, ArgStr[1]); /* invalid instruction */
END

        static void DecodeCMPRSPM(Word Index)
BEGIN
  Boolean ok;

  if (ArgCnt != 1)
   BEGIN
    WrError(1110);
    return;
   END

  WAsmCode[0] = Index | (EvalIntExpression(ArgStr[1], UInt2, &ok) & 3);
  if (!ok) return;
  CodeLen = 1;
END

        static void DecodeIO(Word Index)
BEGIN
  Boolean ok;

  if ((ArgCnt < 2) || (ArgCnt > 3))
   BEGIN
    WrError(1110);
    return;
   END

  decode_adr(ArgStr[1],3,False);
  if (!adr_ok) return;

  WAsmCode[1] = EvalIntExpression(ArgStr[2], UInt16, &ok);
  if (!ok) return;
  ChkSpace(SegIO);

  CodeLen = 2;
  WAsmCode[0] = Index | adr_mode;
END

        static void DecodeINTR(Word Index)
BEGIN
   Boolean ok;
   UNUSED(Index);

   if (ArgCnt != 1)
    BEGIN
     WrError(1110);
     return;
    END

   WAsmCode[0] = EvalIntExpression(ArgStr[1], UInt5, &ok) | 0xbe60;
   if (!ok) return;
   CodeLen = 1;
END

        static void DecodeLACC(Word Index)
BEGIN
  Boolean ok;
  LongWord adr_long;
  Word Shift;
  UNUSED(Index);

  if ((ArgCnt < 1) || (ArgCnt > 3))
   BEGIN
    WrError(1110);
    return;
   END

  if (*ArgStr[1] == '#')
   BEGIN
    if (ArgCnt > 2) WrError(1110);
    else
     BEGIN
      adr_long = EvalIntExpression(ArgStr[1] + 1, Int16, &ok);
      if (ok)
       BEGIN
        Shift = 0;
        adr_ok = True;
        if (ArgCnt > 1) 
         Shift = EvalIntExpression(ArgStr[2], UInt4, &adr_ok);
        if (adr_ok)
         BEGIN
          CodeLen = 2;
          WAsmCode[0] = 0xbf80 | (Shift & 0xf);
          WAsmCode[1] = adr_long;
         END
       END
     END
   END
  else
   BEGIN
    decode_adr(ArgStr[1], 3, False);
    if (adr_ok)
     BEGIN
      Shift = 0; ok = True;
      if (ArgCnt >= 2)
       Shift = DecodeShift(ArgStr[2], &ok);
      if (ok)
       BEGIN
        CodeLen = 1;
        if (Shift >= 16)
         WAsmCode[0] = 0x6a00 | adr_mode;
        else
         WAsmCode[0] = 0x1000 | ((Shift & 0xf) << 8) | adr_mode;
       END
     END
   END
END

        static void DecodeLACL(Word Index)
BEGIN
   Boolean ok;
   UNUSED(Index);

   if (*ArgStr[1] == '#')
    BEGIN
     if (ArgCnt != 1) WrError(1110);
     else
      BEGIN
       WAsmCode[0] = EvalIntExpression(ArgStr[1] + 1, UInt8, &ok);
       if (!ok) return;
       CodeLen = 1;
       WAsmCode[0] |= 0xb900;   
      END
    END
   else
    BEGIN
     if ((ArgCnt < 1) OR (ArgCnt > 2)) WrError(1110);
     else
      BEGIN
       decode_adr(ArgStr[1], 2, False);
       if (adr_ok)
        BEGIN
         WAsmCode[0] = 0x6900 | adr_mode;
         CodeLen = 1;
        END
      END
    END
END

        static void DecodeLAR(Word Index)
BEGIN
   Word Reg;
   LongWord adr_long;
   Boolean ok;
   UNUSED(Index);

   if ((ArgCnt < 2) || (ArgCnt > 3))
    BEGIN
     WrError(1110);
     return;
    END

   Reg = eval_ar_expression(ArgStr[1], &ok);
   if (!ok) return;

   if (*ArgStr[2] == '#')
    BEGIN
     if (ArgCnt > 2) WrError(1110);
     adr_long = EvalIntExpression(ArgStr[2] + 1, Int16,
                                     &adr_ok) & 0xffff;
     if (adr_ok)
      BEGIN
       if (adr_long > 255)
        BEGIN
         CodeLen = 2;
         WAsmCode[0] = 0xbf08 | (Reg & 7);
         WAsmCode[1] = adr_long;
        END
       else
        BEGIN
         CodeLen = 1;
         WAsmCode[0] = 0xb000 | ((Reg & 7) << 8) | (adr_long & 0xff);
        END
      END
    END
   else
    BEGIN
     decode_adr(ArgStr[2], 3, False);
     if (adr_ok)
      BEGIN
       CodeLen = 1;
       WAsmCode[0] = 0x0000 | ((Reg & 7) << 8) | adr_mode;
      END
    END
END

        static void DecodeLDP(Word Index)
BEGIN
   Word konst;
   Boolean ok;
   UNUSED(Index);

   if (*ArgStr[1] == '#')
    BEGIN
     if (ArgCnt != 1) WrError(1110);
     else
      BEGIN
       konst = EvalIntExpression(ArgStr[1] + 1, UInt9, &ok);
       if (ok)
        BEGIN
         CodeLen = 1;
         WAsmCode[0] = (konst & 0x1ff) | 0xbc00;        
        END
      END
    END
   else
    BEGIN
     if ((ArgCnt != 1) AND (ArgCnt != 2)) WrError(1110);
     else
      BEGIN
       decode_adr(ArgStr[1], 2, False);
       if (adr_ok)
        BEGIN
         CodeLen = 1;
         WAsmCode[0] = 0x0d00 | adr_mode;
        END
      END
    END
END

        static void DecodeLSST(Word Index)
BEGIN
  Word konst;
  Boolean ok;

  if ((ArgCnt < 2) || (ArgCnt > 3))
   BEGIN
    WrError(1110);
    return;
   END

  if (*ArgStr[1] != '#')
   BEGIN
    WrError(1120); /* invalid instruction */
   END

  konst = EvalIntExpression(ArgStr[1] + 1, UInt1, &ok);
  decode_adr(ArgStr[2], 3, Index);
  if ((ok) AND (adr_ok))
   BEGIN
    CodeLen = 1;
    WAsmCode[0] = 0x0e00 | (Index << 15) | ((konst & 1) << 8) | adr_mode;
   END
END

        static void DecodeMAC(Word Index)
BEGIN
   Boolean ok;

   if ((ArgCnt < 2) OR (ArgCnt > 3))
    BEGIN
     WrError(1110);
     return;
    END

   WAsmCode[1] = EvalIntExpression(ArgStr[1], Int16, &ok);
   ChkSpace(SegCode);

   decode_adr(ArgStr[2], 3, False);

   if ((adr_ok) AND (ok))
    BEGIN
     CodeLen = 2;
     WAsmCode[0] = 0xa200 | (Index << 8) | adr_mode;
    END
END

        static void DecodeMPY(Word Index)
BEGIN
   LongInt Imm;
   Boolean ok;

   if (*ArgStr[1] == '#')
    BEGIN
     if (ArgCnt != 1) WrError(1110);
     else
      BEGIN
       FirstPassUnknown = FALSE;
       Imm = EvalIntExpression(ArgStr[1] + 1, SInt16, &ok);
       if (FirstPassUnknown) Imm &= 0xfff;
       if (ok)
        BEGIN
         if ((Imm < -4096) || (Imm > 4095))
          BEGIN
           if (MomCPU < cpu_32050) WrError(1500);
           else
            BEGIN
             CodeLen = 2;              /* What does that mean? */
             WAsmCode[0] = 0xbe80;
             WAsmCode[1] = Imm; 
            END
          END
         else
          BEGIN
           CodeLen = 1;
           WAsmCode[0] = 0xc000 | (Imm & 0x1fff);
          END
        END
      END
    END
   else
    BEGIN
     if ((ArgCnt < 1) OR (ArgCnt > 2)) WrError(1110);
     else
      BEGIN
       decode_adr(ArgStr[1], 2, Index);
       if (adr_ok)
        BEGIN
         CodeLen = 1;
         WAsmCode[0] = 0x5400 | adr_mode;
        END
      END
    END
END

        static void DecodeNORM(Word Index)
BEGIN
  UNUSED(Index);

  if ((ArgCnt < 1) || (ArgCnt > 2))
   BEGIN
    WrError(1110);
    return;
   END

  decode_adr(ArgStr[1], 2, False);
  if (adr_ok)
   BEGIN
    if (adr_mode < 0x80) WrError(1350);
    else
     BEGIN
      CodeLen = 1;
      WAsmCode[0] = 0xa080 | (adr_mode & 0x7f);
     END
   END
END

        static void DecodeRETC(Word Index)
BEGIN
   if (ArgCnt < 1) WrError(1110);
   else if ((Memo("RETCD")) AND (MomCPU < cpu_32050)) WrError(1500);
   else
    BEGIN
     CodeLen = 1;
     WAsmCode[0] = 0xec00 | (Index << 12) | decode_cond(1);
    END
END

        static void DecodeRPT(Word Index)
BEGIN
   Word Imm;
   Boolean ok;
   UNUSED(Index);

   if (*ArgStr[1] == '#')
    BEGIN
     if (ArgCnt != 1) WrError(1110);
     else
      BEGIN
       Imm = EvalIntExpression(ArgStr[1] + 1, (MomCPU >= cpu_32050) ? UInt16 : UInt8, &ok);
       if (ok)
        BEGIN
         if (Imm > 255)
          BEGIN
           CodeLen = 2;
           WAsmCode[0] = 0xbec4;
           WAsmCode[1] = Imm;
          END
         else
          BEGIN
           CodeLen = 1;
           WAsmCode[0] = 0xbb00 | (Imm & 0xff);
          END
        END
      END
    END
   else
    BEGIN
     if ((ArgCnt != 1) AND (ArgCnt != 2)) WrError(1110);
     else
      BEGIN
       decode_adr(ArgStr[1], 2, False);
       if (adr_ok)
        BEGIN
         CodeLen = 1;
         WAsmCode[0] = 0x0b00 | adr_mode;
        END
      END
    END
END

        static void DecodeSAC(Word Index)
BEGIN
  Boolean ok;
  Word Shift;

  if ((ArgCnt < 1) || (ArgCnt > 3))
   BEGIN
    WrError(1110);
    return;
   END

  ok = True;
  Shift = 0;
  if (ArgCnt >= 2) 
   Shift = EvalIntExpression(ArgStr[2], UInt3, &ok);

  decode_adr(ArgStr[1], 3, False);

  if ((adr_ok) AND (ok))
   BEGIN
    CodeLen = 1;
    WAsmCode[0] = (Index << 11) | 0x9000 | adr_mode | ((Shift & 7) << 8);
   END
END

        static void DecodeSAR(Word Index)
BEGIN
  Word Reg;
  Boolean ok;
  UNUSED(Index);

  if ((ArgCnt < 2) || (ArgCnt > 3))
   BEGIN
    WrError(1110);
    return;
   END

  Reg = eval_ar_expression(ArgStr[1], &ok);
  decode_adr(ArgStr[2], 3, False);

  if ((adr_ok) AND (ok))
   BEGIN
    CodeLen = 1;
    WAsmCode[0] = 0x8000 | ((Reg & 7) << 8) | adr_mode;
   END
END

        static void DecodeBSAR(Word Index)
BEGIN
   Word Shift;
   Boolean ok;
   UNUSED(Index);

   if (ArgCnt != 1) WrError(1110);
   else if (MomCPU < cpu_32050) WrError(1500);
   else
    BEGIN
     FirstPassUnknown = False;
     Shift = EvalIntExpression(ArgStr[1], UInt5, &ok);
     if (FirstPassUnknown) Shift = 1;
     if (ok)
      if (ChkRange(Shift, 1, 16))
       BEGIN
        CodeLen = 1;
        WAsmCode[0] = 0xbfe0 | ((Shift - 1) & 0xf);
       END
    END
END

        static void DecodeLSAMM(Word Index)
BEGIN
   if ((ArgCnt < 1) OR (ArgCnt > 2)) WrError(1110);
   else if (MomCPU < cpu_32050) WrError(1500);
   else
    BEGIN
     decode_adr(ArgStr[1], 2, True);
     if (adr_ok)
      BEGIN
       CodeLen = 1;
       WAsmCode[0] = 0x0800 | (Index << 15) | adr_mode;
      END
    END
END

        static void DecodeLSMMR(Word Index)
BEGIN
   Boolean ok;

   if ((ArgCnt < 2) OR (ArgCnt > 3)) WrError(1110);
   else if (MomCPU < cpu_32050) WrError(1500);
   else if (ArgStr[2][0] != '#') WrError(1120);
   else
    BEGIN
     WAsmCode[1] = EvalIntExpression(ArgStr[2] + 1, Int16, &ok);
     decode_adr(ArgStr[1], 3, True);
     if ((adr_ok) AND (ok))
      BEGIN
       CodeLen = 2;
       WAsmCode[0] = 0x0900 | (Index << 15) | adr_mode;
      END
    END
END

        static void DecodeRPTB(Word Index)
BEGIN
   Boolean ok;
   UNUSED(Index);

   if (ArgCnt != 1) WrError(1110);
   else if (MomCPU < cpu_32050) WrError(1500);
   else
    BEGIN
     WAsmCode[1] = EvalIntExpression(ArgStr[1], Int16, &ok);
     if (ok)
      BEGIN
       CodeLen = 2;
       WAsmCode[0] = 0xbec6;
      END
    END
END

        static void DecodeRPTZ(Word Index)
BEGIN
   Boolean ok;
   UNUSED(Index);

   if (ArgCnt != 1) WrError(1110);
   else if (MomCPU < cpu_32050) WrError(1500);
   else if (*ArgStr[1] != '#') WrError(1120);
   else
    BEGIN
     WAsmCode[1] = EvalIntExpression(ArgStr[1] + 1, Int16, &ok);
     if (ok)
      BEGIN
       CodeLen = 2;
       WAsmCode[0] = 0xbec5;
      END
    END
END

        static void DecodeXC(Word Index)
BEGIN
   Word Mode; 
   Boolean ok;
   UNUSED(Index);

   if (ArgCnt < 2) WrError(1110);
   else if (MomCPU < cpu_32050) WrError(1500);
   else
    BEGIN
     FirstPassUnknown = False;
     Mode = EvalIntExpression(ArgStr[1], UInt2, &ok);
     if (ok)
      BEGIN
       if ((Mode != 1) && (Mode != 2) && (!FirstPassUnknown)) WrError(1315);
       else
        BEGIN
         CodeLen = 1;
         WAsmCode[0] = (0xd400 + (Mode << 12)) | decode_cond(2);
        END
      END
    END
END

/* ---------------------------------------------------------------------- */

static int instrz;

static void addfixed(char *nname, CPUVar mincpu, Word ncode)
{
        if (instrz>=cmd_fixed_cnt) exit(255);
        cmd_fixed_order[instrz].name = nname;
        cmd_fixed_order[instrz].mincpu = mincpu;
        cmd_fixed_order[instrz].code = ncode;
        AddInstTable(InstTable, nname, instrz++, DecodeFixed);
}

static void addadr(char *nname, CPUVar mincpu, Word ncode)
{
        if (instrz>=cmd_adr_cnt) exit(255);
        cmd_adr_order[instrz].name = nname;
        cmd_adr_order[instrz].mincpu = mincpu;
        cmd_adr_order[instrz].code = ncode;
        AddInstTable(InstTable, nname, instrz++, DecodeCmdAdr);
}

static void addjmp(char *nname, CPUVar mincpu, Word ncode, Boolean ncond)
{
        if (instrz>=cmd_jmp_cnt) exit(255);
        cmd_jmp_order[instrz].name = nname;
        cmd_jmp_order[instrz].mincpu = mincpu;
        cmd_jmp_order[instrz].code = ncode;
        cmd_jmp_order[instrz].cond = ncond;
        AddInstTable(InstTable, nname, instrz++, DecodeCmdJmp);
}

static void addplu(char *nname, CPUVar mincpu, Word ncode)
{
        if (instrz>=cmd_plu_cnt) exit(255);
        cmd_plu_order[instrz].name = nname;
        cmd_plu_order[instrz].mincpu = mincpu;
        cmd_plu_order[instrz].code = ncode;
        AddInstTable(InstTable, nname, instrz++, DecodeCmdPlu);
}

static void addadrmode(char *nname, CPUVar mincpu, Word nmode)
{
        if (instrz>=adr_mode_cnt) exit(255);
        adr_modes[instrz].name = nname;
        adr_modes[instrz].mincpu = mincpu;
        adr_modes[instrz++].mode= nmode;
}

static void addcond(char *nname, CPUVar mincpu, Word ncodeand, Word ncodeor, Byte niszl,
                    Byte nisc, Byte nisv, Byte nistp)
{
        if (instrz>=cond_cnt) exit(255);
        cond_tab[instrz].name = nname;
        cond_tab[instrz].mincpu = mincpu;
        cond_tab[instrz].codeand = ncodeand;
        cond_tab[instrz].codeor = ncodeor;
        cond_tab[instrz].iszl = niszl;
        cond_tab[instrz].isc = nisc;
        cond_tab[instrz].isv = nisv;
        cond_tab[instrz++].istp = nistp;
}

static void addbit(char *nname, CPUVar mincpu, Word ncode)
{
        if (instrz>=bit_cnt) exit(255);
        bit_table[instrz].name = nname;
        bit_table[instrz].mincpu = mincpu;
        bit_table[instrz++].code = ncode;
}

static void initfields(void)
{
        InstTable = CreateInstTable(203);

        cmd_fixed_order=(cmd_fixed *) malloc(sizeof(cmd_fixed)*cmd_fixed_cnt); instrz = 0;
        addfixed("ABS",    cpu_320203, 0xbe00); addfixed("ADCB",   cpu_32050 , 0xbe11);
        addfixed("ADDB",   cpu_32050 , 0xbe10); addfixed("ANDB",   cpu_32050 , 0xbe12);
        addfixed("CMPL",   cpu_320203, 0xbe01); addfixed("CRGT",   cpu_32050 , 0xbe1b);
        addfixed("CRLT",   cpu_32050 , 0xbe1c); addfixed("EXAR",   cpu_32050 , 0xbe1d);
        addfixed("LACB",   cpu_32050 , 0xbe1f); addfixed("NEG",    cpu_320203, 0xbe02);
        addfixed("ORB",    cpu_32050 , 0xbe13); addfixed("ROL",    cpu_320203, 0xbe0c);
        addfixed("ROLB",   cpu_32050 , 0xbe14); addfixed("ROR",    cpu_320203, 0xbe0d);
        addfixed("RORB",   cpu_32050 , 0xbe15); addfixed("SACB",   cpu_32050 , 0xbe1e);
        addfixed("SATH",   cpu_32050 , 0xbe5a); addfixed("SATL",   cpu_32050 , 0xbe5b);
        addfixed("SBB",    cpu_32050 , 0xbe18); addfixed("SBBB",   cpu_32050 , 0xbe19);
        addfixed("SFL",    cpu_320203, 0xbe09); addfixed("SFLB",   cpu_32050 , 0xbe16);
        addfixed("SFR",    cpu_320203, 0xbe0a); addfixed("SFRB",   cpu_32050 , 0xbe17);
        addfixed("XORB",   cpu_32050 , 0xbe1a); addfixed("ZAP",    cpu_32050 , 0xbe59);
        addfixed("APAC",   cpu_320203, 0xbe04); addfixed("PAC",    cpu_320203, 0xbe03);
        addfixed("SPAC",   cpu_320203, 0xbe05); addfixed("ZPR",    cpu_32050 , 0xbe58);
        addfixed("BACC",   cpu_320203, 0xbe20); addfixed("BACCD",  cpu_32050 , 0xbe21);
        addfixed("CALA",   cpu_320203, 0xbe30); addfixed("CALAD",  cpu_32050 , 0xbe3d);
        addfixed("NMI",    cpu_320203, 0xbe52); addfixed("RET",    cpu_320203, 0xef00);
        addfixed("RETD",   cpu_32050 , 0xff00); addfixed("RETE",   cpu_32050 , 0xbe3a);
        addfixed("RETI",   cpu_32050 , 0xbe38); addfixed("TRAP",   cpu_320203, 0xbe51);
        addfixed("IDLE",   cpu_320203, 0xbe22); addfixed("NOP",    cpu_320203, 0x8b00);
        addfixed("POP",    cpu_320203, 0xbe32); addfixed("PUSH",   cpu_320203, 0xbe3c);
        addfixed("IDLE2",  cpu_32050 , 0xbe23);

        cmd_adr_order = (cmd_fixed *) malloc(sizeof(cmd_fixed)*cmd_adr_cnt); instrz = 0;
        addadr("ADDC",   cpu_320203, 0x6000); addadr("ADDS",   cpu_320203, 0x6200);
        addadr("ADDT",   cpu_320203, 0x6300); addadr("LACT",   cpu_320203, 0x6b00);
        addadr("SUBB",   cpu_320203, 0x6400); addadr("SUBC",   cpu_320203, 0x0a00);
        addadr("SUBS",   cpu_320203, 0x6600); addadr("SUBT",   cpu_320203, 0x6700);
        addadr("ZALR",   cpu_320203, 0x6800); addadr("MAR",    cpu_320203, 0x8b00);
        addadr("LPH",    cpu_320203, 0x7500); addadr("LT",     cpu_320203, 0x7300);
        addadr("LTA",    cpu_320203, 0x7000); addadr("LTD",    cpu_320203, 0x7200);
        addadr("LTP",    cpu_320203, 0x7100); addadr("LTS",    cpu_320203, 0x7400);
        addadr("MADD",   cpu_32050 , 0xab00); addadr("MADS",   cpu_32050 , 0xaa00);
        addadr("MPYA",   cpu_320203, 0x5000); addadr("MPYS",   cpu_320203, 0x5100);
        addadr("MPYU",   cpu_320203, 0x5500); addadr("SPH",    cpu_320203, 0x8d00);
        addadr("SPL",    cpu_320203, 0x8c00); addadr("SQRA",   cpu_320203, 0x5200);
        addadr("SQRS",   cpu_320203, 0x5300); addadr("BLDP",   cpu_32050 , 0x5700);
        addadr("DMOV",   cpu_320203, 0x7700); addadr("TBLR",   cpu_320203, 0xa600);
        addadr("TBLW",   cpu_320203, 0xa700); addadr("BITT",   cpu_320203, 0x6f00);
        addadr("POPD",   cpu_320203, 0x8a00); addadr("PSHD",   cpu_320203, 0x7600);

        cmd_jmp_order=(cmd_jmp *) malloc(sizeof(cmd_jmp)*cmd_jmp_cnt); instrz=0;
        addjmp("B",      cpu_320203, 0x7980,  False);
        addjmp("BD",     cpu_32050 , 0x7d80,  False);
        addjmp("BANZ",   cpu_320203, 0x7b80,  False);
        addjmp("BANZD",  cpu_32050 , 0x7f80,  False);
        addjmp("BCND",   cpu_320203, 0xe000,  True);
        addjmp("BCNDD",  cpu_32050 , 0xf000,  True);
        addjmp("CALL",   cpu_320203, 0x7a80,  False);
        addjmp("CALLD",  cpu_32050 , 0x7e80,  False);
        addjmp("CC",     cpu_320203, 0xe800,  True);
        addjmp("CCD",    cpu_32050 , 0xf800,  True);

        cmd_plu_order=(cmd_fixed *) malloc(sizeof(cmd_fixed)*cmd_plu_cnt); instrz=0;
        addplu("APL",   cpu_32050 , 0x5a00); addplu("CPL",   cpu_32050 , 0x5b00);
        addplu("OPL",   cpu_32050 , 0x5900); addplu("SPLK",  cpu_320203, 0xaa00);
        addplu("XPL",   cpu_32050 , 0x5800);

        adr_modes=(adr_mode_t *) malloc(sizeof(adr_mode_t)*adr_mode_cnt); instrz=0;
        addadrmode( "*-",     cpu_320203, 0x90 ); addadrmode( "*+",     cpu_320203, 0xa0 );
        addadrmode( "*BR0-",  cpu_320203, 0xc0 ); addadrmode( "*0-",    cpu_320203, 0xd0 );
        addadrmode( "*AR0-",  cpu_32050 , 0xd0 ); addadrmode( "*0+",    cpu_320203, 0xe0 );
        addadrmode( "*AR0+",  cpu_32050 , 0xe0 ); addadrmode( "*BR0+",  cpu_320203, 0xf0 );
        addadrmode( "*",      cpu_320203, 0x80 ); addadrmode( NULL,     cpu_32050 , 0);

        cond_tab=(condition *) malloc(sizeof(condition)*cond_cnt); instrz=0;
        addcond("EQ",  cpu_32050 , 0xf33, 0x088, 1, 0, 0, 0);
        addcond("NEQ", cpu_32050 , 0xf33, 0x008, 1, 0, 0, 0);
        addcond("LT",  cpu_32050 , 0xf33, 0x044, 1, 0, 0, 0);
        addcond("LEQ", cpu_32050 , 0xf33, 0x0cc, 1, 0, 0, 0);
        addcond("GT",  cpu_32050 , 0xf33, 0x004, 1, 0, 0, 0);
        addcond("GEQ", cpu_32050 , 0xf33, 0x08c, 1, 0, 0, 0);
        addcond("NC",  cpu_32050 , 0xfee, 0x001, 0, 1, 0, 0);
        addcond("C",   cpu_32050 , 0xfee, 0x011, 0, 1, 0, 0);
        addcond("NOV", cpu_32050 , 0xfdd, 0x002, 0, 0, 1, 0);
        addcond("OV",  cpu_32050 , 0xfdd, 0x022, 0, 0, 1, 0);
        addcond("BIO", cpu_32050 , 0x0ff, 0x000, 0, 0, 0, 1);
        addcond("NTC", cpu_32050 , 0x0ff, 0x200, 0, 0, 0, 1);
        addcond("TC",  cpu_32050 , 0x0ff, 0x100, 0, 0, 0, 1);
        addcond("UNC", cpu_32050 , 0x0ff, 0x300, 0, 0, 0, 1);
        addcond(NULL,  cpu_32050 , 0xfff, 0x000, 0, 0, 0, 0);
        
        bit_table=(bit_table_t *) malloc(sizeof(bit_table_t)*bit_cnt); instrz=0;
        addbit("OVM",  cpu_320203, 0xbe42 ); addbit("SXM",  cpu_320203, 0xbe46 );
        addbit("HM",   cpu_32050 , 0xbe48 ); addbit("TC",   cpu_320203, 0xbe4a );
        addbit("C",    cpu_320203, 0xbe4e ); addbit("XF",   cpu_320203, 0xbe4c );
        addbit("CNF",  cpu_320203, 0xbe44 ); addbit("INTM", cpu_320203, 0xbe40 );
        addbit(NULL,   cpu_32050 , 0     );

        AddInstTable(InstTable, "ADD"  , 0, DecodeADDSUB);
        AddInstTable(InstTable, "SUB"  , 1, DecodeADDSUB);
        AddInstTable(InstTable, "ADRK" , 0, DecodeADRSBRK);
        AddInstTable(InstTable, "SBRK" , 1, DecodeADRSBRK);
        AddInstTable(InstTable, "AND"  , 0x6e01, DecodeLogic);
        AddInstTable(InstTable, "OR"   , 0x6d02, DecodeLogic);
        AddInstTable(InstTable, "XOR"  , 0x6c03, DecodeLogic);
        AddInstTable(InstTable, "BIT"  , 0, DecodeBIT);
        AddInstTable(InstTable, "BLDD" , 0, DecodeBLDD);
        AddInstTable(InstTable, "BLPD" , 0, DecodeBLPD);
        AddInstTable(InstTable, "CLRC" , 0, DecodeCLRSETC);
        AddInstTable(InstTable, "SETC" , 1, DecodeCLRSETC);
        AddInstTable(InstTable, "CMPR" , 0xbf44, DecodeCMPRSPM);
        AddInstTable(InstTable, "SPM"  , 0xbf00, DecodeCMPRSPM);
        AddInstTable(InstTable, "IN"   , 0xaf00, DecodeIO);
        AddInstTable(InstTable, "OUT"  , 0x0c00, DecodeIO);
        AddInstTable(InstTable, "INTR" , 0, DecodeINTR);
        AddInstTable(InstTable, "LACC" , 0, DecodeLACC);
        AddInstTable(InstTable, "LACL" , 0, DecodeLACL);
        AddInstTable(InstTable, "LAR"  , 0, DecodeLAR);
        AddInstTable(InstTable, "LDP"  , 0, DecodeLDP);
        AddInstTable(InstTable, "SST"  , 1, DecodeLSST);
        AddInstTable(InstTable, "LST"  , 0, DecodeLSST);
        AddInstTable(InstTable, "MAC"  , 0, DecodeMAC);
        AddInstTable(InstTable, "MACD" , 1, DecodeMAC);
        AddInstTable(InstTable, "MPY"  , 0, DecodeMPY);
        AddInstTable(InstTable, "NORM" , 0, DecodeNORM);
        AddInstTable(InstTable, "RETC" , 0, DecodeRETC);
        AddInstTable(InstTable, "RETCD", 1, DecodeRETC);
        AddInstTable(InstTable, "RPT"  , 0, DecodeRPT);
        AddInstTable(InstTable, "SACL" , 0, DecodeSAC);
        AddInstTable(InstTable, "SACH" , 1, DecodeSAC);
        AddInstTable(InstTable, "SAR"  , 0, DecodeSAR);
        AddInstTable(InstTable, "BSAR" , 0, DecodeBSAR);
        AddInstTable(InstTable, "LAMM" , 0, DecodeLSAMM);
        AddInstTable(InstTable, "SAMM" , 1, DecodeLSAMM);
        AddInstTable(InstTable, "LMMR" , 1, DecodeLSMMR);
        AddInstTable(InstTable, "SMMR" , 0, DecodeLSMMR);
        AddInstTable(InstTable, "RPTB" , 0, DecodeRPTB);
        AddInstTable(InstTable, "RPTZ" , 0, DecodeRPTZ);
        AddInstTable(InstTable, "XC"   , 0, DecodeXC);
}

static void deinitfields(void)
{
        DestroyInstTable(InstTable);
        free(cmd_fixed_order);
        free(cmd_adr_order);
        free(cmd_jmp_order);
        free(cmd_plu_order);
        free(adr_modes);
        free(cond_tab);
        free(bit_table);
}

/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */

#if 0
static void define_untyped_label(void)
{
        if (LabPart[0]) {
                PushLocHandle(-1);
                EnterIntSymbol(LabPart, EProgCounter(), SegNone, False);
                PopLocHandle();
        }
}
#endif

/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */

static Boolean decode_pseudo(void)
{
  if (Memo("PORT"))
  {
    CodeEquate(SegIO,0,65535);
    return True;
  }

  return False;
}

/* ---------------------------------------------------------------------- */

        static void make_code_3205x(void)
BEGIN
  CodeLen = 0; 
  DontPrint = False;

  /* zu ignorierendes */
     
  if (Memo("")) return;

  /* Pseudoanweisungen */

  if (decode_pseudo()) return;

  if (DecodeTIPseudo()) return;

  /* per Hash-Tabelle */

  if (NOT LookupInstTable(InstTable, OpPart)) WrXError(1200, OpPart);
}

/* ---------------------------------------------------------------------- */

static Boolean is_def_3205x(void)
{
        static const char *defs[] = { "BSS", "PORT", "STRING", "RSTRING", 
                                              "BYTE", "WORD", "LONG", "FLOAT",
                                              "DOUBLE", "EFLOAT", "BFLOAT", 
                                              "TFLOAT", NULL }; 
        const char **cp = defs;

        while(*cp) {
                if (Memo(*cp))
                        return True;
                cp++;
        }
        return False;
}

/* ---------------------------------------------------------------------- */

static void switch_from_3205x(void)
{
        deinitfields();
}

/* ---------------------------------------------------------------------- */

static void switch_to_3205x(void)
{
        TurnWords = False;
        ConstMode = ConstModeIntel; 
        SetIsOccupied = False;

        PCSymbol = "$";
        HeaderID = 0x77; 
        NOPCode = 0x8b00;
        DivideChars = ",";
        HasAttrs = False;

        ValidSegs = (1 << SegCode) | (1 << SegData) | (1 << SegIO);
        Grans[SegCode] = 2; ListGrans[SegCode] = 2; SegInits[SegCode] = 0;
        SegLimits[SegCode] = 0xffff;
        Grans[SegData] = 2; ListGrans[SegData] = 2; SegInits[SegData] = 0;
        SegLimits[SegData] = 0xffff;
        Grans[SegIO  ] = 2; ListGrans[SegIO  ] = 2; SegInits[SegIO  ] = 0;
        SegLimits[SegIO  ] = 0xffff;
        
        MakeCode = make_code_3205x;
        IsDef = is_def_3205x; SwitchFrom = switch_from_3205x;
        initfields();
}

/* ---------------------------------------------------------------------- */

void code3205x_init(void)
{
        cpu_320203 = AddCPU("320C203", switch_to_3205x);
        cpu_32050  = AddCPU("320C50",  switch_to_3205x);
        cpu_32051  = AddCPU("320C51",  switch_to_3205x);
        cpu_32053  = AddCPU("320C53",  switch_to_3205x);

        AddCopyright("TMS320C5x-Generator (C) 1995/96 Thomas Sailer");
}
