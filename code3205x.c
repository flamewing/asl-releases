/*
 * AS-Portierung 
 *
 * AS-Codegeneratormodul fuer die Texas Instruments TMS320C5x-Familie
 *
 * (C) 1996 Thomas Sailer <sailer@ife.ee.ethz.ch>
 *
 * 20.08.96: Erstellung
 */

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "strutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "endian.h"

#include "code3202x.h"

/* ---------------------------------------------------------------------- */

typedef struct {
	char *name;
	Word code;
} cmd_fixed;
typedef struct {
	char *name;
	Word code;
	Boolean cond;
} cmd_jmp;
typedef struct {
	char *name;
	Word mode;
} adr_mode_t;
typedef struct {
	char *name;
	Word codeand;
	Word codeor;
	Byte iszl;
	Byte isc;
	Byte isv;
	Byte istp;
} condition;
typedef struct {
	char *name;
	Word code;
} bit_table_t;

static cmd_fixed *cmd_fixed_order;
#define cmd_fixed_cnt 46	
static cmd_fixed *cmd_adr_order;
#define cmd_adr_cnt 44	
static cmd_jmp *cmd_jmp_order;
#define cmd_jmp_cnt 11	
static cmd_fixed *cmd_plu_order;
#define cmd_plu_cnt 6
static adr_mode_t *adr_modes;
#define adr_mode_cnt 10
static condition *cond_tab;
#define cond_cnt 15
static bit_table_t *bit_table;
#define bit_cnt 9

static int instrz;

static void addfixed(char *nname, Word ncode)
{
	if (instrz>=cmd_fixed_cnt) exit(255);
	cmd_fixed_order[instrz].name=nname;
	cmd_fixed_order[instrz++].code=ncode;
}

static void addadr(char *nname, Word ncode)
{
	if (instrz>=cmd_adr_cnt) exit(255);
	cmd_adr_order[instrz].name=nname;
	cmd_adr_order[instrz++].code=ncode;
}

static void addjmp(char *nname, Word ncode, Boolean ncond)
{
	if (instrz>=cmd_jmp_cnt) exit(255);
	cmd_jmp_order[instrz].name=nname;
	cmd_jmp_order[instrz].code=ncode;
	cmd_jmp_order[instrz++].cond=ncond;
}

static void addplu(char *nname, Word ncode)
{
	if (instrz>=cmd_plu_cnt) exit(255);
	cmd_plu_order[instrz].name=nname;
	cmd_plu_order[instrz++].code=ncode;
}

static void addadrmode(char *nname, Word nmode)
{
	if (instrz>=adr_mode_cnt) exit(255);
	adr_modes[instrz].name=nname;
	adr_modes[instrz++].mode=nmode;
}

static void addcond(char *nname, Word ncodeand, Word ncodeor, Byte niszl,
		    Byte nisc, Byte nisv, Byte nistp)
{
	if (instrz>=cond_cnt) exit(255);
	cond_tab[instrz].name=nname;
	cond_tab[instrz].codeand=ncodeand;
	cond_tab[instrz].codeor=ncodeor;
	cond_tab[instrz].iszl=niszl;
	cond_tab[instrz].isc=nisc;
	cond_tab[instrz].isv=nisv;
	cond_tab[instrz++].istp=nistp;
}

static void addbit(char *nname, Word ncode)
{
	if (instrz>=bit_cnt) exit(255);
	bit_table[instrz].name=nname;
	bit_table[instrz++].code=ncode;
}

static void initfields(void)
{
	cmd_fixed_order=(cmd_fixed *) malloc(sizeof(cmd_fixed)*cmd_fixed_cnt); instrz=0;
	addfixed("ABS",    0xbe00); addfixed("ADCB",   0xbe11);
	addfixed("ADDB",   0xbe10); addfixed("ANDB",   0xbe12);
	addfixed("CMPL",   0xbe01); addfixed("CRGT",   0xbe1b);
	addfixed("CRLT",   0xbe1c); addfixed("EXAR",   0xbe1d);
	addfixed("LACB",   0xbe1f); addfixed("NEG",    0xbe02);
	addfixed("ORB",    0xbe13); addfixed("ROL",    0xbe0c);
	addfixed("ROLB",   0xbe14); addfixed("ROR",    0xbe0d);
	addfixed("RORB",   0xbe15); addfixed("SACB",   0xbe1e);
	addfixed("SATH",   0xbe5a); addfixed("SATL",   0xbe5b);
	addfixed("SBB",    0xbe18); addfixed("SBBB",   0xbe19);
	addfixed("SFL",    0xbe09); addfixed("SFLB",   0xbe16);
	addfixed("SFR",    0xbe0a); addfixed("SFRB",   0xbe17);
	addfixed("XORB",   0xbe1a); addfixed("ZAP",    0xbe59);
	addfixed("APAC",   0xbe04); addfixed("PAC",    0xbe03);
	addfixed("SPAC",   0xbe05); addfixed("ZPR",    0xbe58);
	addfixed("BACC",   0xbe20); addfixed("BACCD",  0xbe21);
	addfixed("CALA",   0xbe30); addfixed("CALAD",  0xbe3d);
	addfixed("NMI",    0xbe52); addfixed("RET",    0xef00);
	addfixed("RETD",   0xff00); addfixed("RETE",   0xbe3a);
	addfixed("RETI",   0xbe38); addfixed("TRAP",   0xbe51);
	addfixed("IDLE",   0xbe22); addfixed("NOP",    0x8b00);
	addfixed("POP",    0xbe32); addfixed("PUSH",   0xbe3c);
	addfixed("IDLE2",  0xbe23); addfixed(NULL,     0);

	cmd_adr_order=(cmd_fixed *) malloc(sizeof(cmd_fixed)*cmd_adr_cnt); instrz=0;
	addadr("ADDC",   0x6000); addadr("ADDS",   0x6200);
	addadr("ADDT",   0x6300); addadr("AND",    0x6e00);
	addadr("LACL",   0x6900); addadr("LACT",   0x6b00);
	addadr("OR",     0x6d00); addadr("SUBB",   0x6400);
	addadr("SUBC",   0x0a00); addadr("SUBS",   0x6600);
	addadr("SUBT",   0x6700); addadr("XOR",    0x6c00);
	addadr("ZALR",   0x6800); addadr("LDP",    0x0d00);
	addadr("APL",    0x5a00); addadr("CPL",    0x5b00);
	addadr("OPL",    0x5900); addadr("XPL",    0x5800);
	addadr("MAR",    0x8b00); addadr("LPH",    0x7500);
	addadr("LT",     0x7300); addadr("LTA",    0x7000);
	addadr("LTD",    0x7200); addadr("LTP",    0x7100);
	addadr("LTS",    0x7400); addadr("MADD",   0xab00);
	addadr("MADS",   0xaa00); addadr("MPY",    0x5400);
	addadr("MPYA",   0x5000); addadr("MPYS",   0x5100);
	addadr("MPYU",   0x5500); addadr("SPH",    0x8d00);
	addadr("SPL",    0x8c00); addadr("SQRA",   0x5200);
	addadr("SQRS",   0x5300); addadr("BLDP",   0x5700);
	addadr("DMOV",   0x7700); addadr("TBLR",   0xa600);
	addadr("TBLW",   0xa700); addadr("BITT",   0x6f00);
	addadr("POPD",   0x8a00); addadr("PSHD",   0x7600);
	addadr("RPT",    0x0b00); addadr(NULL,     0);

	cmd_jmp_order=(cmd_jmp *) malloc(sizeof(cmd_jmp)*cmd_jmp_cnt); instrz=0;
	addjmp("B",      0x7980,  False); addjmp("BD",     0x7d80,  False);
	addjmp("BANZ",   0x7b80,  False); addjmp("BANZD",  0x7f80,  False);
	addjmp("BCND",   0xe000,  True);  addjmp("BCNDD",  0xf000,  True);
	addjmp("CALL",   0x7a80,  False); addjmp("CALLD",  0x7e80,  False);
	addjmp("CC",     0xe800,  True);  addjmp("CCD",    0xf800,  True);
	addjmp(NULL,     0,       False);

	cmd_plu_order=(cmd_fixed *) malloc(sizeof(cmd_fixed)*cmd_plu_cnt); instrz=0;
	addplu("APL",   0x5e00); addplu("CPL",   0x5f00);
	addplu("OPL",   0x5d00); addplu("SPLK",  0xae00);
	addplu("XPL",   0x5c00); addplu(NULL,    0     );

	adr_modes=(adr_mode_t *) malloc(sizeof(adr_mode_t)*adr_mode_cnt); instrz=0;
	addadrmode( "*-",     0x90 ); addadrmode( "*+",     0xa0 );
	addadrmode( "*BR0-",  0xc0 ); addadrmode( "*0-",    0xd0 );
	addadrmode( "*AR0-",  0xd0 ); addadrmode( "*0+",    0xe0 );
	addadrmode( "*AR0+",  0xe0 ); addadrmode( "*BR0+",  0xf0 );
	addadrmode( "*",      0x80 ); addadrmode( NULL,     0);

	cond_tab=(condition *) malloc(sizeof(condition)*cond_cnt); instrz=0;
	addcond("EQ",  0xf33, 0x088, 1, 0, 0, 0);
	addcond("NEQ", 0xf33, 0x008, 1, 0, 0, 0);
	addcond("LT",  0xf33, 0x044, 1, 0, 0, 0);
	addcond("LEQ", 0xf33, 0x0cc, 1, 0, 0, 0);
	addcond("GT",  0xf33, 0x004, 1, 0, 0, 0);
	addcond("GEQ", 0xf33, 0x08c, 1, 0, 0, 0);
	addcond("NC",  0xfee, 0x001, 0, 1, 0, 0);
	addcond("C",   0xfee, 0x011, 0, 1, 0, 0);
	addcond("NOV", 0xfdd, 0x002, 0, 0, 1, 0);
	addcond("OV",  0xfdd, 0x022, 0, 0, 1, 0);
	addcond("BIO", 0x0ff, 0x000, 0, 0, 0, 1);
	addcond("NTC", 0x0ff, 0x200, 0, 0, 0, 1);
	addcond("TC",  0x0ff, 0x100, 0, 0, 0, 1);
	addcond("UNC", 0x0ff, 0x300, 0, 0, 0, 1);
	addcond(NULL,  0xfff, 0x000, 0, 0, 0, 0);
	
	bit_table=(bit_table_t *) malloc(sizeof(bit_table_t)*bit_cnt); instrz=0;
	addbit("OVM",  0xbe42 ); addbit("SXM",  0xbe46 );
	addbit("HM",   0xbe48 ); addbit("TC",   0xbe4a );
	addbit("C",    0xbe4e ); addbit("XF",   0xbe4c );
	addbit("CNF",  0xbe44 ); addbit("INTM", 0xbe40 );
	addbit(NULL,   0     );
}

static void deinitfields(void)
{
	free(cmd_fixed_order);
	free(cmd_adr_order);
	free(cmd_jmp_order);
	free(cmd_plu_order);
	free(adr_modes);
	free(cond_tab);
	free(bit_table);
}

/* ---------------------------------------------------------------------- */

static Word adr_mode;
static Boolean adr_ok;

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

static void decode_adr(char *arg, int aux, Boolean must1)
{
	Byte h;
	adr_mode_t *am = adr_modes;

	adr_ok = False;
	while (am->name && strcasecmp(am->name, arg))
		am++;
	if (!am->name) {
		if (aux <= ArgCnt) {
			WrError(1110);
			return;
		}
		h = EvalIntExpression(arg, Int16, &adr_ok);
		if (!adr_ok)
			return;
		if (must1 && (h >= 0x80) && (!FirstPassUnknown)) {
			WrError(1315); 
			adr_ok = False;
			return;
		}
		adr_mode = h & 0x7f; 
		ChkSpace(SegData);
		return;
	}
	adr_mode = am->mode;
	if (aux <= ArgCnt) {
		h = eval_ar_expression(ArgStr[aux], &adr_ok);
		if (adr_ok) 
			adr_mode |= 0x8 | h;
	} else
		adr_ok = True;
}

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
	if((cnttp > 1) || (cntzl > 1) || (cntv > 1) || (cntc > 1)) 
		WrError(1200); /* invalid condition */
	return ret;
}

/* ---------------------------------------------------------------------- */

static void pseudo_qxx(Integer num)
{
	int z;
	Boolean ok;
	double res;

	if (!ArgCnt) {
		WrError(1110);
		return;
	}
	for(z = 1; z <= ArgCnt; z++) {
		res = ldexp(EvalFloatExpression(ArgStr[z], Float64, &ok), num);
		if (!ok) {
			CodeLen = 0;
			return;
		}
		if ((res > 32767.49) || (res < -32768.49)) {
			CodeLen = 0;
			WrError(1320);
			return;
		}
		WAsmCode[CodeLen++] = res;
	}
}

/* ---------------------------------------------------------------------- */

static void pseudo_lqxx(Integer num)
{
	int z;
	Boolean ok;
	double res;
	LongInt resli;

	if (!ArgCnt) {
		WrError(1110);
		return;
	}
	for(z = 1; z <= ArgCnt; z++) {
		res = ldexp(EvalFloatExpression(ArgStr[z], Float64, &ok), num);
		if (!ok) {
			CodeLen = 0;
			return;
		}
		if ((res > 2147483647.49) || (res < -2147483647.49)) {
			CodeLen = 0;
			WrError(1320);
			return;
		}
		resli = res;
		WAsmCode[CodeLen++] = resli & 0xffff;
		WAsmCode[CodeLen++] = resli >> 16;
	}
}

/* ---------------------------------------------------------------------- */

static void define_untyped_label(void)
{
	if (LabPart[0]) {
		PushLocHandle(-1);
		EnterIntSymbol(LabPart, EProgCounter(), SegNone, False);
		PopLocHandle();
	}
}

/* ---------------------------------------------------------------------- */

static void wr_code_byte(Boolean *ok, int *adr, LongInt val)
{
	if ((val < -128) || (val > 0xff)) {
		WrError(1320);
		*ok = False;
		return;
	}
	WAsmCode[(*adr)++] = val & 0xff;
	CodeLen = *adr;
}

/* ---------------------------------------------------------------------- */

static void wr_code_word(Boolean *ok, int *adr, LongInt val)
{
	if ((val < -32768) || (val > 0xffff)) {
		WrError(1320);
		*ok = False;
		return;
	}
	WAsmCode[(*adr)++] = val;
	CodeLen = *adr;
}

/* ---------------------------------------------------------------------- */

static void wr_code_long(Boolean *ok, int *adr, LongInt val)
{
	WAsmCode[(*adr)++] = val & 0xffff;
	WAsmCode[(*adr)++] = val >> 16;
	CodeLen = *adr;
}

/* ---------------------------------------------------------------------- */

static void wr_code_byte_hilo(Boolean *ok, int *adr, LongInt val)
{
	if ((val < -128) || (val > 0xff)) {
		WrError(1320);
		*ok = False;
		return;
	}
	if ((*adr) & 1) 
		WAsmCode[((*adr)++)/2] |= val & 0xff;
	else 
		WAsmCode[((*adr)++)/2] = val << 8;
	CodeLen = ((*adr)+1)/2;
}

/* ---------------------------------------------------------------------- */

static void wr_code_byte_lohi(Boolean *ok, int *adr, LongInt val)
{
	if ((val < -128) || (val > 0xff)) {
		WrError(1320);
		*ok = False;
		return;
	}
	if ((*adr) & 1) 
		WAsmCode[((*adr)++)/2] |= val << 8;
	else 
		WAsmCode[((*adr)++)/2] = val & 0xff;
	CodeLen = ((*adr)+1)/2;
}

/* ---------------------------------------------------------------------- */

typedef void (*tcallback)(
#ifdef __PROTOS__
Boolean *, int *, LongInt
#endif
);

static void pseudo_store(tcallback callback)
{
	Boolean ok = True;
	int adr = 0;
	int z;
	TempResult t;
	unsigned char *cp;

	if (!ArgCnt) {
		WrError(1110);
		return;
	}
	define_untyped_label();
	for(z = 1; z <= ArgCnt; z++) {
		if (!ok)
			return;
		EvalExpression(ArgStr[z], &t);
		switch(t.Typ) {
		case TempInt:
			callback(&ok, &adr, t.Contents.Int);
			break;
		case TempFloat:
			WrError(1135);
			return;
		case TempString:
			cp = (unsigned char *)t.Contents.Ascii;
			while (*cp) 
				callback(&ok, &adr, CharTransTable[*cp++]);
			break;
		default:
			WrError(1135);
			return;
		}
	}
}

/* ---------------------------------------------------------------------- */

static Boolean decode_pseudo(void)
{
	Word size;
	Boolean ok;
	TempResult t;
	int z,z2;
	unsigned char *cp;
	float flt;
	double dbl, mant;
	int exp;
	long lmant;
	Word w;

	if(Memo("PORT")) {
		CodeEquate(SegIO,0,65535);
		return True;
	}

	if (Memo("RES") || Memo("BSS")) {
		if (ArgCnt != 1) {
			WrError(1110);
			return True;
		}
		if (Memo("BSS"))
			define_untyped_label();
		FirstPassUnknown = False;
		size = EvalIntExpression(ArgStr[1], Int16, &ok);
		if (FirstPassUnknown) {
			WrError(1820);
			return True;
		}
		if (!ok) 
			return True;
		DontPrint = True;
		CodeLen = size;
		if (MakeUseList)
			if (AddChunk(SegChunks+ActPC, ProgCounter(),
				     CodeLen, ActPC == SegCode))
				WrError(90);
		return True;
	}

	if(Memo("DATA")) {
		if (!ArgCnt) {
			WrError(1110);
			return True;
		}
		ok = True;
		for(z = 1; (z <= ArgCnt) && ok; z++) {
			EvalExpression(ArgStr[z], &t);
			switch(t.Typ) {
			case TempInt:  
				if((t.Contents.Int < -32768) || 
				   (t.Contents.Int > 0xffff)) {
					WrError(1320); 
					ok = False;
				} else
					WAsmCode[CodeLen++] = t.Contents.Int;
				break;
			default:
			case TempFloat:
				WrError(1135); 
				ok = False;
				break;
			case TempString:
				z2 = 0;
				cp = (unsigned char *)t.Contents.Ascii;
				while (*cp) {
					if (z2 & 1)
						WAsmCode[CodeLen++] |= 
							(CharTransTable[*cp++]
							 << 8);
					else
						WAsmCode[CodeLen] = 
							CharTransTable[*cp++];
					z2++;
				}
				if (z2 & 1)
					CodeLen++;
				break;
			}
		}
		if (!ok)
			CodeLen = 0;
		return True;
	}

	if(Memo("STRING")) {
		pseudo_store(wr_code_byte_hilo); 
		return True;
	}
	if(Memo("RSTRING")) {
		pseudo_store(wr_code_byte_lohi); 
		return True;
	}
	if(Memo("BYTE")) {
		pseudo_store(wr_code_byte); 
		return True;
	}
	if(Memo("WORD")) {
		pseudo_store(wr_code_word); 
		return True;
	}
	if(Memo("LONG")) {
		pseudo_store(wr_code_long); 
		return True;
	}

	/* Qxx */

	if((OpPart[0] == 'Q') && (OpPart[1] >= '0') && (OpPart[1] <= '9') &&
	   (OpPart[2] >= '0') && (OpPart[2] <= '9') && (OpPart[3] == '\0')) {
		pseudo_qxx(10*(OpPart[1]-'0')+OpPart[2]-'0');
		return True;
	}

	/* LQxx */

	if((OpPart[0] == 'L') && (OpPart[1] == 'Q') && (OpPart[2] >= '0') && 
	   (OpPart[2] <= '9') && (OpPart[3] >= '0') && (OpPart[3] <= '9') && 
	   (OpPart[4] == '\0')) {
		pseudo_lqxx(10*(OpPart[2]-'0')+OpPart[3]-'0');
		return True;
	}

	/* Floating point definitions */

	if(Memo("FLOAT")) {
		if (!ArgCnt) {
			WrError(1110);
			return True;
		}
		define_untyped_label();
		ok = True;
		for(z = 1; (z <= ArgCnt) && ok; z++) {
			flt = EvalFloatExpression(ArgStr[z], Float32, &ok);
			memcpy(WAsmCode+CodeLen, &flt, sizeof(float));
			if (BigEndian) {
				w = WAsmCode[CodeLen];
				WAsmCode[CodeLen] = WAsmCode[CodeLen+1];
				WAsmCode[CodeLen+1] = w;
			}
			CodeLen += sizeof(float)/2;
		}
		if(!ok)
			CodeLen = 0;
		return True;
	}

	if(Memo("DOUBLE")) {
		if (!ArgCnt) {
			WrError(1110);
			return True;
		}
		define_untyped_label();
		ok = True;
		for(z = 1; (z <= ArgCnt) && ok; z++) {
			dbl = EvalFloatExpression(ArgStr[z], Float64, &ok);
			memcpy(WAsmCode+CodeLen, &dbl, sizeof(dbl));
			if (BigEndian) {
				w = WAsmCode[CodeLen];
				WAsmCode[CodeLen] = WAsmCode[CodeLen+3];
				WAsmCode[CodeLen+3] = w;
				w = WAsmCode[CodeLen+1];
				WAsmCode[CodeLen+1] = WAsmCode[CodeLen+2];
				WAsmCode[CodeLen+2] = w;
			}
			CodeLen += sizeof(dbl)/2;
		}
		if(!ok)
			CodeLen = 0;
		return True;
	}

	if(Memo("EFLOAT")) {
		if (!ArgCnt) {
			WrError(1110);
			return True;
		}
		define_untyped_label();
		ok = True;
		for(z = 1; (z <= ArgCnt) && ok; z++) {
			dbl = EvalFloatExpression(ArgStr[z], Float64, &ok);
			mant = frexp(dbl, &exp);
			WAsmCode[CodeLen++] = ldexp(mant, 15);
			WAsmCode[CodeLen++] = exp-1;
		}
		if(!ok)
			CodeLen = 0;
		return True;
	}

	if(Memo("BFLOAT")) {
		if (!ArgCnt) {
			WrError(1110);
			return True;
		}
		define_untyped_label();
		ok = True;
		for(z = 1; (z <= ArgCnt) && ok; z++) {
			dbl = EvalFloatExpression(ArgStr[z], Float64, &ok);
			mant = frexp(dbl, &exp);
			lmant = ldexp(mant, 31);
			WAsmCode[CodeLen++] = (lmant & 0xffff);
			WAsmCode[CodeLen++] = (lmant >> 16);
			WAsmCode[CodeLen++] = exp-1;
		}
		if(!ok)
			CodeLen = 0;
		return True;
	}

	if(Memo("TFLOAT")) {
		if (!ArgCnt) {
			WrError(1110);
			return True;
		}
		define_untyped_label();
		ok = True;
		for(z = 1; (z <= ArgCnt) && ok; z++) {
			dbl = EvalFloatExpression(ArgStr[z], Float64, &ok);
			mant = frexp(dbl, &exp);
			mant = modf(ldexp(mant, 15), &dbl);
			WAsmCode[CodeLen+3] = dbl;
			mant = modf(ldexp(mant, 16), &dbl);
			WAsmCode[CodeLen+2] = dbl;
			mant = modf(ldexp(mant, 16), &dbl);
			WAsmCode[CodeLen+1] = dbl;
			mant = modf(ldexp(mant, 16), &dbl);
			WAsmCode[CodeLen] = dbl;
			CodeLen += 4;
			WAsmCode[CodeLen++] = ((exp - 1) & 0xffff);
			WAsmCode[CodeLen++] = ((exp - 1) >> 16);
		}
		if(!ok)
			CodeLen = 0;
		return True;
	}
	return False;
}

/* ---------------------------------------------------------------------- */

static void make_code_3205x(void)
{
	Boolean ok;
	Word adr_word;
	LongInt adr_long;
	const cmd_fixed *fo;
	const cmd_jmp *jo;
	bit_table_t *bitp;

	CodeLen = 0; 
	DontPrint = False;

	/* zu ignorierendes */
	   
	if(Memo(""))
		return;

	/* Pseudoanweisungen */
	
	if(decode_pseudo()) 
		return;

	/* prozessorspezifische Befehle */

	/* kein Argument */
	
	for(fo = cmd_fixed_order; fo->name; fo++) {
		if(Memo(fo->name)) {
			if(ArgCnt) {
				WrError(1110);
				return;
			}
			CodeLen = 1; 
			WAsmCode[0] = fo->code;
			return;
		}
	}

	/* Immediate */

	if(Memo("MPY") && (ArgCnt == 1) && (ArgStr[1][0] == '#')) {
		adr_long = EvalIntExpression((ArgStr[1])+1, Int16, &ok);
		if(!ok)
			return;
		if(FirstPassUnknown || (adr_long < -4096) || 
		   (adr_long > 4095)) {
			CodeLen = 2;
			WAsmCode[0] = 0xbe80;
			WAsmCode[1] = adr_long;
		} else {
			CodeLen = 1;
			WAsmCode[0] = 0xc000 | (adr_long & 0x1fff);
		}
		return;
	}

	if((ArgCnt >= 2) && (ArgCnt <= 3) && (ArgStr[1][0] == '#')) {
		for(fo = cmd_plu_order; fo->name; fo++) {
			if(Memo(fo->name)) {
				decode_adr(ArgStr[2], 3, False);
				WAsmCode[1] = EvalIntExpression((ArgStr[1])+1,
								Int16, &ok);
				if((!ok) || (!adr_ok))
					return;        
				CodeLen = 2;
				WAsmCode[0] = fo->code | adr_mode;
				return;
			}
		}
	}

	if(Memo("LDP") && (ArgCnt == 1) && (ArgStr[1][0] == '#')) {
		adr_word = EvalIntExpression((ArgStr[1])+1, UInt16, &ok);
		if((adr_word >= 0x200) && (!FirstPassUnknown)) 
			WrError(1200); /* out of range */
		if(!ok)
			return;      
		CodeLen = 1;
		WAsmCode[0] = (adr_word & 0x1ff) | 0xbc00;	
		return;
	}

	if((Memo("AND") || Memo("OR") || Memo("XOR")) && (ArgCnt >= 1) && 
	   (ArgCnt <= 2) && (ArgStr[1][0] == '#')) {
		WAsmCode[1] = EvalIntExpression((ArgStr[1])+1, Int16, &ok);
		adr_word = 0;
		adr_ok = True;
		if(ArgCnt >= 2) 
			adr_word = EvalIntExpression(ArgStr[2], UInt5, 
						     &adr_ok);
		if((!ok) || (!adr_ok))
			return;
		if((adr_word>16) && (!FirstPassUnknown)) 
			WrError(1200); /* invalid shift */
		CodeLen = 2;
		if (adr_word >= 16) {
			if(Memo("AND"))
				WAsmCode[0] = 0xbe81;
			else if(Memo("OR"))
				WAsmCode[0] = 0xbe82;
			else 
				WAsmCode[0] = 0xbe83;
			return;
		}
		if(Memo("AND"))
			WAsmCode[0] = 0xbfb0;
		else if(Memo("OR"))
			WAsmCode[0] = 0xbfc0;
		else 
			WAsmCode[0] = 0xbfd0;
		WAsmCode[0] |= adr_word & 0xf;
		return;
	}

	if(Memo("LACL") && (ArgCnt == 1) && (ArgStr[1][0] == '#')) {
		adr_word = EvalIntExpression((ArgStr[1])+1, UInt8, &ok);
		if(!ok)
			return;
		CodeLen = 1;
		WAsmCode[0] = (adr_word & 0xff) | 0xb900;	
		return;
	}

	if(Memo("RPT") && (ArgCnt == 1) && (ArgStr[1][0] == '#')) {
		adr_long = EvalIntExpression((ArgStr[1])+1, Int16, &ok)
			& 0xffff;
		if(!ok)
			return;
		if(FirstPassUnknown || (adr_long > 255)) {
			CodeLen = 2;
			WAsmCode[0] = 0xbec4;
			WAsmCode[1] = adr_long;
			return;
		}
		CodeLen = 1;
		WAsmCode[0] = 0xbb00 | (adr_long & 0xff);            
		return;
	}

	/* nur Adresse */

	for(fo = cmd_adr_order; fo->name; fo++) {
		if(Memo(fo->name)) {
			if((ArgCnt < 1) || (ArgCnt > 2)) {
				WrError(1110);
				return;
			}
			decode_adr(ArgStr[1], 2, False);
			if(!adr_ok)
				return;	
			CodeLen = 1; 
			WAsmCode[0] = fo->code | adr_mode;
			return;
		}
	}

	/* Adresse, spezial */

	if(Memo("LAMM") || Memo("SAMM")) {
		if((ArgCnt < 1) || (ArgCnt > 2)) {
			WrError(1110);
			return;
		}
		decode_adr(ArgStr[1], 2, True);
		if(!adr_ok)
			return;
		CodeLen = 1; 
		WAsmCode[0] = ((Memo("SAMM")) ? 0x8800 : 0x0800) | adr_mode;
		return;
	}

	if(Memo("LST") || Memo("SST")) {
		if((ArgCnt < 2) || (ArgCnt > 3)) {
			WrError(1110);
			return;
		}
		if(ArgStr[1][0] != '#') {
			WrError(1200); /* invalid instruction */
			return;
		}
		adr_word = EvalIntExpression((ArgStr[1])+1, UInt1, &ok);
		decode_adr(ArgStr[2], 3, Memo("SST"));
		if((!ok) || (!adr_ok))
			return;
		CodeLen = 1;
		WAsmCode[0] = ((Memo("SST")) ? 0x8e00 : 0x0e00) | 
			((adr_word & 1) << 8) | adr_mode;
		return;
	}

	/* Spezial: Set/Reset Mode Flags */
	if(Memo("CLRC") OR Memo("SETC")) {
		if(ArgCnt != 1) {
			WrError(1110);
			return;
		}
		CodeLen = 1;
		WAsmCode[0] = !!(Memo("SETC")); NLS_UpString(ArgStr[1]);
		for(bitp = bit_table; bitp->name; bitp++)
			if(!strcmp(ArgStr[1], bitp->name)) {
				WAsmCode[0] |= bitp->code;
				return;
			}
		WrError(1200); /* invalid instruction */
		return;
	}

	/* Spruenge */

	for(jo = cmd_jmp_order; jo->name; jo++) {
		if(Memo(jo->name)) {
			if((ArgCnt < 1) || ((ArgCnt > 3) && (!jo->cond))) {
				WrError(1110);
				return;
			}
			adr_mode  =  0;
			if(jo->cond)
				adr_mode = decode_cond(2);
			else if(ArgCnt > 1) {
				decode_adr(ArgStr[2], 3, False);
				if(adr_mode < 0x80) {
					WrError(1350);
					return;
				}
				adr_mode &= 0x7f;
			}
			WAsmCode[1] = EvalIntExpression(ArgStr[1], Int16, &ok);
			if(!ok)
				return;         
			CodeLen = 2; 
			WAsmCode[0] = jo->code | adr_mode;
			return;
		}
	}

	if(Memo("XC")) {
		if(ArgCnt < 1) { 
			WrError(1110);
			return;
		}
		adr_mode = EvalIntExpression(ArgStr[1], UInt2, &ok);
		if((adr_mode != 1) && (adr_mode != 2) && (!FirstPassUnknown))
			WrError(1315);
		if(!ok)
			return;
		CodeLen = 1;
		WAsmCode[0] = ((adr_mode == 2) ? 0xf400 : 0xe400) | 
			decode_cond(2);
		return;
	}

	if(Memo("RETC")) {
		CodeLen = 1;
		WAsmCode[0] = 0xec00 | decode_cond(1);
		return;
	}

	if(Memo("RETCD")) {
		CodeLen = 1;
		WAsmCode[0] = 0xfc00 | decode_cond(1);
		return;
	}

	if(Memo("INTR")) {
		if((ArgCnt != 1)) {
			WrError(1110);
			return;
		}
		WAsmCode[0] = EvalIntExpression(ArgStr[1], UInt5, &ok) & 0x1f;
		if(!ok)
			return;
		WAsmCode[0] |= 0xbe60;
		CodeLen = 1;
		return;
	}

	/* IO und Datenmemorybefehle */

	if(Memo("BLDD")) {
		if((ArgCnt < 2) || (ArgCnt > 3)) {
			WrError(1110);
			return;
		}
		if(!strcasecmp(ArgStr[1], "BMAR")) {
			decode_adr(ArgStr[2], 3, False);
			if(!adr_ok)
				return;
			CodeLen = 1;
			WAsmCode[0] = 0xac00 | adr_mode;		
			return;
		}
		if(!strcasecmp(ArgStr[2], "BMAR")) {
			decode_adr(ArgStr[1], 3, False);
			if(!adr_ok)
				return;
			CodeLen = 1;
			WAsmCode[0] = 0xad00 | adr_mode;
	                return;
		}
		if(ArgStr[1][0] == '#') {
			WAsmCode[1] = EvalIntExpression((ArgStr[1])+1, Int16,
							&ok);
			decode_adr(ArgStr[2], 3, False);
			if((!adr_ok) || (!ok))
				return;
			CodeLen = 2;
			WAsmCode[0] = 0xa800 | adr_mode;          
			return;
		}
		if(ArgStr[2][0] == '#') {
			WAsmCode[1] = EvalIntExpression((ArgStr[2])+1, Int16, 
							&ok);
			decode_adr(ArgStr[1], 3, False);
			if((!adr_ok) || (!ok))
				return;
			CodeLen = 2;
			WAsmCode[0] = 0xa900 | adr_mode;
			return ;
		}
		WrError(1200); /* invalid instruction */
		return;
	}

	if(Memo("BLPD")) {
		if((ArgCnt < 2) || (ArgCnt > 3)) {
			WrError(1110);
			return;
		}
		if(!strcasecmp(ArgStr[1], "BMAR")) {
			decode_adr(ArgStr[2], 3, False);
			if(!adr_ok)
				return;
			CodeLen = 1;
			WAsmCode[0] = 0xa400 | adr_mode;		
			return;
		}
		if(ArgStr[1][0] == '#') {
			WAsmCode[1] = EvalIntExpression((ArgStr[1])+1, Int16,
							&ok);
			decode_adr(ArgStr[2], 3, False);
			if((!adr_ok) || (!ok))
				return;
			CodeLen = 2;
			WAsmCode[0] = 0xa500 | adr_mode;          
			return;
		}
		WrError(1200); /* invalid instruction */
		return;
	}

	if(Memo("LMMR") || Memo("SMMR")) {
		if((ArgCnt < 2) || (ArgCnt > 3)) {
			WrError(1110);
			return;
		}
		if(ArgStr[2][0] != '#') {
			WrError(1200); /*invalid parameter*/
			return;
		}
		WAsmCode[1] = EvalIntExpression((ArgStr[2])+1, Int16, &ok);
		decode_adr(ArgStr[1], 3, True);
		if((!adr_ok) || (!ok))
			return;
		CodeLen = 2;
		WAsmCode[0] = ((Memo("LMMR")) ? 0x8900 : 0x0900) | adr_mode;
		return;
	}

	if((Memo("IN")) || (Memo("OUT"))) {
		if((ArgCnt < 2) || (ArgCnt > 3)) {
			WrError(1110);
			return;
		}
		decode_adr(ArgStr[1],3,False);
		if(!adr_ok)
			return;
		WAsmCode[1] = EvalIntExpression(ArgStr[2], Int16, &ok);
		ChkSpace(SegIO);
		CodeLen = 2;
		WAsmCode[0] = ((Memo("IN")) ? 0xaf00 : 0x0c00) | adr_mode;
		return;
	}

	/* spezialbefehle */

	if(Memo("CMPR") || Memo("SPM")) {
		if(ArgCnt != 1) {
			WrError(1110);
			return;
		}
		WAsmCode[0] = EvalIntExpression(ArgStr[1], UInt2, &ok) & 3;
		if(!ok)
			return;
		CodeLen = 1;
		WAsmCode[0] |= ((Memo("CMPR")) ? 0xbf44 : 0xbf00);
		return;
	}

	if(Memo("MAC") || Memo("MACD")) {
		if((ArgCnt < 2) || (ArgCnt > 3)) {
			WrError(1110);
			return;
		}
		WAsmCode[1] = EvalIntExpression(ArgStr[1], Int16, &ok);
		decode_adr(ArgStr[2], 3, False);
		if((!adr_ok) || (!ok))
			return;
		CodeLen = 2;
		WAsmCode[0] = ((Memo("MACD")) ? 0xa300 : 0xa200) | adr_mode;
		return;
	}

	if(Memo("SACL") || Memo("SACH")) {
		if((ArgCnt < 1) || (ArgCnt > 3)) {
			WrError(1110);
			return;
		}
		ok = True;
		adr_word = 0;
		if(ArgCnt >= 2) 
			adr_word = EvalIntExpression(ArgStr[2], UInt3, &ok);
		decode_adr(ArgStr[1],3,False);
		if((!adr_ok) || (!ok))
			return;
		CodeLen = 1;
		WAsmCode[0] = ((Memo("SACH")) ? 0x9800 : 0x9000) | adr_mode | 
			((adr_word & 7) << 8);
		return;
	}

	/* Auxregisterbefehle */
	
	if(Memo("ADRK") || Memo("SBRK")) {
		if(ArgCnt != 1) {
			WrError(1110);
			return;
		}
		if(ArgStr[1][0] != '#') {
			WrError(1200); /*invalid parameter*/
			return;
		}
		adr_word = EvalIntExpression((ArgStr[1])+1, UInt8, &ok);
		if(!ok)
			return;
		CodeLen = 1;
		WAsmCode[0] = ((Memo("SBRK")) ? 0x7c00 : 0x7800) | 
			(adr_word & 0xff);
		return;
	}

	if(Memo("SAR")) {
		if((ArgCnt < 2) || (ArgCnt > 3)) {
			WrError(1110);
			return;
		}
		adr_word = eval_ar_expression(ArgStr[1], &ok);
		decode_adr(ArgStr[2], 3, False);
		if((!adr_ok) || (!ok))
			return;
		CodeLen = 1;
		WAsmCode[0] = 0x8000 | ((adr_word & 7) << 8) | adr_mode;
		return;
	}

	if(Memo("LAR")) {
		if((ArgCnt < 2) || (ArgCnt > 3)) {
			WrError(1110);
			return;
		}
		adr_word = eval_ar_expression(ArgStr[1], &ok);
		if(ArgStr[2][0] == '#') {
			if(ArgCnt > 2) 
				WrError(1110);
			adr_long = EvalIntExpression((ArgStr[2])+1, Int16,
						     &adr_ok) & 0xffff;
			if((!adr_ok) || (!ok))
				return;
			if(FirstPassUnknown || (adr_long > 255)) {
				CodeLen = 2;
				WAsmCode[0] = 0xbf08 | (adr_word & 7);
				WAsmCode[1] = adr_long;
				return;
			}
			CodeLen = 1;
			WAsmCode[0] = 0xb000 | ((adr_word & 7) << 8) |
				(adr_long & 0xff);
			return;
		}
		decode_adr(ArgStr[2], 3, False);
		if((!adr_ok) || (!ok))
			return;
		CodeLen = 1;
		WAsmCode[0] = 0x0000 | ((adr_word & 7) << 8) | adr_mode;
		return;
	}

	if(Memo("NORM")) {
		if((ArgCnt < 1) || (ArgCnt > 2)) {
			WrError(1110);
			return;
		}
		decode_adr(ArgStr[1], 2, False);
		if(!adr_ok)
			return;
		if(adr_mode < 0x80) {
			WrError(1350);
			return;
		}
		CodeLen = 1;
		WAsmCode[0] = 0xa080 | (adr_mode & 0x7f);
		return;
	}

	/* add/sub */

	if(Memo("ADD") || Memo("SUB")) {
		if((ArgCnt < 1) || (ArgCnt > 3)) {
			WrError(1110);
			return;
		}
		if(ArgStr[1][0] == '#') {
			if(ArgCnt > 2) 
				WrError(1110);
			adr_long = EvalIntExpression((ArgStr[1])+1, Int16, 
						     &ok);
			adr_word = 0;
			adr_ok = True;
			if((ArgCnt > 1) || FirstPassUnknown || (adr_long < 0) 
			   || (adr_long > 255)) {
				if(ArgCnt > 1) 
					adr_word = EvalIntExpression(ArgStr[2],
								     UInt4,
								     &adr_ok);
				if((!adr_ok) || (!ok))
					return;
				CodeLen = 2;
				WAsmCode[0] = ((Memo("SUB")) ? 0xbfa0 : 
					       0xbf90) | (adr_word & 0xf);
				WAsmCode[1] = adr_long;
				return;
			}				
			if(!ok)
				return;
			CodeLen = 1;
			WAsmCode[0] = ((Memo("SUB")) ? 0xba00 : 
				       0xb800) | (adr_long & 0xff);
			return;			
		}
		adr_word = 0;
		ok = True;
		decode_adr(ArgStr[1], 3, False);
		if(ArgCnt >= 2) 
			adr_word = EvalIntExpression(ArgStr[2], UInt5, &ok);
		if(!ok)
			return;          
		if((adr_word > 16) && (!FirstPassUnknown)) {
			WrError(1200); /* shift out of range */
			return;          
		}
		CodeLen = 1;
		if (adr_word >= 16) {
			WAsmCode[0] = ((Memo("SUB")) ? 0x6500 : 0x6100) | 
				adr_mode;
			return;
		}
		WAsmCode[0] = ((Memo("SUB")) ? 0x3000 : 0x2000) | 
			((adr_word & 0xf) << 8) | adr_mode;
		return;
	}

	if(Memo("LACC")) {
		if((ArgCnt < 1) || (ArgCnt > 3)) {
			WrError(1110);
			return;
		}
		if(ArgStr[1][0] == '#') {
			if(ArgCnt > 2) 
				WrError(1110);
			adr_long = EvalIntExpression((ArgStr[1])+1, Int16, 
						     &ok);
			adr_word = 0;
			adr_ok = True;
			if(ArgCnt > 1) 
				adr_word = EvalIntExpression(ArgStr[2], UInt4,
							     &adr_ok);
			if((!adr_ok) || (!ok))
				return;
			CodeLen = 2;
			WAsmCode[0] = 0xbf80 | (adr_word & 0xf);
			WAsmCode[1] = adr_long;
			return;
		}
		adr_word = 0;
		ok = True;
		decode_adr(ArgStr[1], 3, False);
		if(ArgCnt >= 2) 
			adr_word = EvalIntExpression(ArgStr[2], UInt5, &ok);
		if(!ok)
			return;
		if((adr_word > 16) && (!FirstPassUnknown)) {
			WrError(1200); /* shift out of range */
			return;
		}
		CodeLen = 1;
		if(adr_word >= 16) {
			WAsmCode[0] = 0x6a00 | adr_mode;
			return;
		}
		WAsmCode[0] = 0x1000 | ((adr_word & 0xf) << 8) | adr_mode;
		return;
	}
	
	if(Memo("BSAR")) {
		if(ArgCnt != 1) {
			WrError(1110);
			return;
		}
		adr_word = EvalIntExpression(ArgStr[1], UInt5, &ok);
		if(adr_word < 1) {
			WrError(1315);
			return;
		}
		if(adr_word > 16) {
			WrError(1320);
			return;
		}
		if(!ok)
			return;
		CodeLen = 1;
		WAsmCode[0] = 0xbfe0 | ((adr_word-1) & 0xf);
		return;
	}

	if(Memo("BIT")) {
		if((ArgCnt < 2) || (ArgCnt > 3)) {
			WrError(1110);
			return;
		}
		adr_word = EvalIntExpression(ArgStr[2], UInt4, &ok);
		decode_adr(ArgStr[1], 3, False);
		if((!adr_ok) || (!ok))
			return;
		CodeLen = 1;
		WAsmCode[0] = 0x4000 | adr_mode | ((adr_word & 0xf) << 8);
		return;
	}

	/* repeat commands */

	if(Memo("RPTB")) {
		if(ArgCnt != 1) {
			WrError(1110);
			return;
		}
		WAsmCode[1] = EvalIntExpression(ArgStr[1], Int16, &ok);
		if(!ok)
			return;
		CodeLen = 2;
		WAsmCode[0] = 0xbec6;
		return;
	}

	if(Memo("RPTZ")) {
		if(ArgCnt != 1) {
			WrError(1110);
			return;
		}
		if(ArgStr[1][0] != '#') {
			WrError(1200); /* not const */
			return;
		}
		WAsmCode[1] = EvalIntExpression((ArgStr[1])+1, Int16, &ok);
		if(!ok)
			return;
		CodeLen = 2;
		WAsmCode[0] = 0xbec5;
		return;
	}

	WrXError(1200, OpPart);
}

/* ---------------------------------------------------------------------- */

static Boolean chk_pc_3205x(void)
{
	switch(ActPC) {
	case SegCode: 
	case SegData: 
	case SegIO:
		return (ProgCounter() <= 0xffff);
	default:
		return False;
	}
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
	Grans[SegData] = 2; ListGrans[SegData] = 2; SegInits[SegData] = 0;
	Grans[SegIO  ] = 2; ListGrans[SegIO  ] = 2; SegInits[SegIO  ] = 0;
	
	MakeCode = make_code_3205x; ChkPC = chk_pc_3205x; 
	IsDef = is_def_3205x; SwitchFrom = switch_from_3205x;
	initfields();
}

/* ---------------------------------------------------------------------- */

void code3205x_init(void)
{
	cpu_32050 = AddCPU("320C50", switch_to_3205x);
	cpu_32051 = AddCPU("320C51", switch_to_3205x);
	cpu_32053 = AddCPU("320C53", switch_to_3205x);

	AddCopyright("TMS320C5x-Generator (C) 1995/96 Thomas Sailer");
}


