/*
 * AS-Portierung 
 *
 * AS-Codegeneratormodul fuer die Texas Instruments TMS320C2x-Familie
 *
 * (C) 1996 Thomas Sailer <sailer@ife.ee.ethz.ch>
 *
 * 19.08.96: Erstellung
 * 18.01.97: Anpassungen fuer Case-Sensitivitaet
 */

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "stringutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "endian.h"

#include "code3202x.h"

/* ---------------------------------------------------------------------- */

static const struct cmd_fixed_order {
	char *name;
	Word code;
} cmd_fixed_order[] = {{"ABS",    0xce1b},
		       {"CMPL",   0xce27},
		       {"NEG",    0xce23},
		       {"ROL",    0xce34},
		       {"ROR",    0xce35},
		       {"SFL",    0xce18},
		       {"SFR",    0xce19},
		       {"ZAC",    0xca00},
		       {"APAC",   0xce15},
		       {"PAC",    0xce14},
		       {"SPAC",   0xce16},
		       {"BACC",   0xce25},
		       {"CALA",   0xce24},
		       {"RET",    0xce26},
		       {"RFSM",   0xce36},
		       {"RTXM",   0xce20},
		       {"RXF",    0xce0c},
		       {"SFSM",   0xce37},
		       {"STXM",   0xce21},
		       {"SXF",    0xce0d},
		       {"DINT",   0xce01},
		       {"EINT",   0xce00},
		       {"IDLE",   0xce1f},
		       {"NOP",    0x5500},
		       {"POP",    0xce1d},
		       {"PUSH",   0xce1c},
		       {"RC",     0xce30},
		       {"RHM",    0xce38},
		       {"ROVM",   0xce02},
		       {"RSXM",   0xce06},
		       {"RTC",    0xce32},
		       {"SC",     0xce31},
		       {"SHM",    0xce39},
		       {"SOVM",   0xce03},
		       {"SSXM",   0xce07},
		       {"STC",    0xce33},
		       {"TRAP",   0xce1e},
		       {NULL, 0}};


static const struct cmd_fixed_order 
cmd_jmp_order[] = {{"B",      0xff80},
		   {"BANZ",   0xfb80},
		   {"BBNZ",   0xf980},
		   {"BBZ",    0xf880},
		   {"BC",     0x5e80},
		   {"BGEZ",   0xf480},
		   {"BGZ",    0xf180},
		   {"BIOZ",   0xfa80},
		   {"BLEZ",   0xf280},
		   {"BLZ",    0xf380},
		   {"BNC",    0x5f80},
		   {"BNV",    0xf780},
		   {"BNZ",    0xf580},
		   {"BV",     0xf080},
		   {"BZ",     0xf680},
		   {"CALL",   0xfe80},
		   {NULL,     0}};



static const struct cmd_adr_order {
	char *name;
	Word code;
	Boolean must1;
} cmd_adr_order[] = {{"ADDC",   0x4300, False},
		     {"ADDH",   0x4800, False},
		     {"ADDS",   0x4900, False},
		     {"ADDT",   0x4a00, False},
		     {"AND",    0x4e00, False},
		     {"LACT",   0x4200, False},
		     {"OR",     0x4d00, False},
		     {"SUBB",   0x4f00, False},
		     {"SUBC",   0x4700, False},
		     {"SUBH",   0x4400, False},
		     {"SUBS",   0x4500, False},
		     {"SUBT",   0x4600, False},
		     {"XOR",    0x4c00, False},
		     {"ZALH",   0x4000, False},
		     {"ZALR",   0x7b00, False},
		     {"ZALS",   0x4100, False},
		     {"LDP",    0x5200, False},
		     {"MAR",    0x5500, False},
		     {"LPH",    0x5300, False},
		     {"LT",     0x3c00, False},
		     {"LTA",    0x3d00, False},
		     {"LTD",    0x3f00, False},
		     {"LTP",    0x3e00, False},
		     {"LTS",    0x5b00, False},
		     {"MPY",    0x3800, False},
		     {"MPYA",   0x3a00, False},
		     {"MPYS",   0x3b00, False},
		     {"MPYU",   0xcf00, False},
		     {"SPH",    0x7d00, False},
		     {"SPL",    0x7c00, False},
		     {"SQRA",   0x3900, False},
		     {"SQRS",   0x5a00, False},
		     {"DMOV",   0x5600, False},
		     {"TBLR",   0x5800, False},
		     {"TBLW",   0x5900, False},
		     {"BITT",   0x5700, False},
		     {"LST",    0x5000, False},
		     {"LST1",   0x5100, False},
		     {"POPD",   0x7a00, False},
		     {"PSHD",   0x5400, False},
		     {"RPT",    0x4b00, False},
		     {"SST",    0x7800, True},
		     {"SST1",   0x7900, True},
		     {NULL,     0,      False}};

static const struct cmd_adr_order 
cmd_adr_2ndadr_order[] = {{"BLKD",   0xfd00, False},
			  {"BLKP",   0xfc00, False},
			  {"MAC",    0x5d00, False},
			  {"MACD",   0x5c00, False},
			  {NULL,     0,      False}};

static const struct cmd_adr_shift_order {
	char *name;
	Word code;
	Word allow_shifts;
} cmd_adr_shift_order[] = {{"ADD",    0x0000, 0xf},
			   {"LAC",    0x2000, 0xf},
			   {"SACH",   0x6800, 0x7},
			   {"SACL",   0x6000, 0x7},
			   {"SUB",    0x1000, 0xf},
			   {"BIT",    0x9000, 0xf},
			   {NULL,     0,      0}};

static const struct cmd_imm_order {
	char *name;
	Word code;
	Integer Min;
	Integer Max;
	Word mask;
} cmd_imm_order[] = {{"ADDK",   0xcc00,     0,    255,   0xff},
		     {"LACK",   0xca00,     0,    255,   0xff},
		     {"SUBK",   0xcd00,     0,    255,   0xff},
		     {"ADRK",   0x7e00,     0,    255,   0xff},
		     {"SBRK",   0x7f00,     0,    255,   0xff},
		     {"RPTK",   0xcb00,     0,    255,   0xff},
		     {"MPYK",   0xa000, -4096,   4095, 0x1fff},
		     {"SPM",    0xce08,     0,      3,    0x3},
		     {"CMPR",   0xce50,     0,      3,    0x3},
		     {"FORT",   0xce0e,     0,      1,    0x1},
		     {"ADLK",   0xd002,     0, 0x7fff, 0xffff},
		     {"ANDK",   0xd004,     0, 0x7fff, 0xffff},
		     {"LALK",   0xd001,     0, 0x7fff, 0xffff},
		     {"ORK",    0xd005,     0, 0x7fff, 0xffff},
		     {"SBLK",   0xd003,     0, 0x7fff, 0xffff},
		     {"XORK",   0xd006,     0, 0x7fff, 0xffff},
		     {NULL,     0,          0, 0,      0}};

/* ---------------------------------------------------------------------- */

static Word adr_mode;
static Boolean adr_ok;

static CPUVar cpu_32025, cpu_32026, cpu_32028;

/* ---------------------------------------------------------------------- */

static Word eval_ar_expression(char *asc, Boolean *ok)
{
	*ok = True;
	if ((toupper(asc[0]) == 'A') && (toupper(asc[1]) == 'R') && (asc[2] >= '0') && 
	    (asc[2] <= '7') && (asc[3] == '\0'))
		return asc[2] - '0';
	return EvalIntExpression(asc, UInt3, ok);
}

/* ---------------------------------------------------------------------- */

static void decode_adr(char *arg, Integer aux, Boolean must1)
{
	static const struct adr_modes {
		char *name;
		Word mode;
	} adr_modes[] = {{ "*-",     0x90 },
			 { "*+",     0xa0 },
			 { "*BR0-",  0xc0 },
			 { "*0-",    0xd0 },
			 { "*AR0-",  0xd0 },
			 { "*0+",    0xe0 },
			 { "*AR0+",  0xe0 },
			 { "*BR0+",  0xf0 },
			 { "*",      0x80 },
			 { NULL,     0}};
	const struct adr_modes *am = adr_modes;
	Byte h;

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

static void pseudo_qxx(Integer num)
{
	Integer z;
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
	Integer z;
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

static void wr_code_byte(Boolean *ok, Integer *adr, LongInt val)
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

static void wr_code_word(Boolean *ok, Integer *adr, LongInt val)
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

static void wr_code_long(Boolean *ok, Integer *adr, LongInt val)
{
	WAsmCode[(*adr)++] = val & 0xffff;
	WAsmCode[(*adr)++] = val >> 16;
	CodeLen = *adr;
}

/* ---------------------------------------------------------------------- */

static void wr_code_byte_hilo(Boolean *ok, Integer *adr, LongInt val)
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

static void wr_code_byte_lohi(Boolean *ok, Integer *adr, LongInt val)
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
Boolean *, Integer *, LongInt
#endif
);

static void pseudo_store(tcallback callback)
{
	Boolean ok = True;
	Integer adr = 0;
	Integer z;
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
	Integer z,z2;
	unsigned char *cp;
	float flt;
	double dbl, mant;
	int exp;
	long lmant;
	Word w;

	if (Memo("PORT")) {
		CodeEquate(SegIO, 0, 15);
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

static void make_code_3202x(void) 
{
	Boolean ok;
	Word adr_word;
	LongInt adr_long;
	const struct cmd_fixed_order *fo;
	const struct cmd_adr_order *ao;
	const struct cmd_adr_shift_order *aso;
	const struct cmd_imm_order *io;

	CodeLen = 0; 
	DontPrint = False;

	/* zu ignorierendes */

	if(Memo("")) 
		return;

	/* Pseudoanweisungen */

	if(decode_pseudo()) 
		return;

	/* prozessorspezifische Befehle */

	if(Memo("CNFD")) {
		if(ArgCnt) {
			WrError(1110);
			return;
		}
		if(MomCPU == cpu_32026) {
			WrError(1500);
			return;
		}
		CodeLen = 1; 
		WAsmCode[0] = 0xce04; 
		return;
	}

	if(Memo("CNFP")) {
		if(ArgCnt) {
			WrError(1110);
			return;
		}
		if(MomCPU == cpu_32026) {
			WrError(1500);
			return;
		}
		CodeLen = 1; 
		WAsmCode[0] = 0xce05;
		return;
	}

	if(Memo("CONF")) {
		if(ArgCnt != 1) {
			WrError(1110);
			return;
		}
		if(MomCPU != cpu_32026) {
			WrError(1500);
			return;
		}
		WAsmCode[0] = 0xce3c|EvalIntExpression(ArgStr[1], UInt2, &ok);
		if(ok)
			CodeLen = 1;
		return;
	}

	/* kein Argument */
	
	for(fo = cmd_fixed_order; fo->name; fo++) {
		if (Memo(fo->name)) {
			if(ArgCnt) {
				WrError(1110);
				return;
			}
			CodeLen = 1;
			WAsmCode[0] = fo->code;
			return;
		}
	}

	/* Spruenge */

	for(fo = cmd_jmp_order; fo->name; fo++) {
		if (Memo(fo->name)) {
			if((ArgCnt < 1) || (ArgCnt > 3)) {
				WrError(1110);
				return;
			}
			adr_mode = 0;
			if(ArgCnt > 1) {
				decode_adr(ArgStr[2], 3, False);
				if(adr_mode < 0x80)
					WrError(1350);
			}
			WAsmCode[1] = EvalIntExpression(ArgStr[1], 
							Int16, &ok);
			if(ok) {
				CodeLen = 2; 
				WAsmCode[0] = fo->code | (adr_mode & 0x7f);
			}	
			return;
		}
	}

	/* nur Adresse */

	for(ao = cmd_adr_order; ao->name; ao++) {
		if (Memo(ao->name)) {
			if((ArgCnt < 1) || (ArgCnt > 2)) {
				WrError(1110);
				return;
			}
			decode_adr(ArgStr[1], 2, ao->must1);
			if(adr_ok) {
				CodeLen = 1;
				WAsmCode[0] = ao->code | adr_mode;
			}
			return;
		}
	}

	/* 2 Addressen */

	for(ao = cmd_adr_2ndadr_order; ao->name; ao++) {
		if (Memo(ao->name)) {
			if((ArgCnt < 2) || (ArgCnt > 3)) {
				WrError(1110);
				return;
			}
			WAsmCode[1] = EvalIntExpression(ArgStr[1], Int16, &ok);
			decode_adr(ArgStr[2], 3, ao->must1);
			if(ok && adr_ok) {
				CodeLen = 2; 
				WAsmCode[0] = ao->code | adr_mode;
			}
			return;
		}
	}

	/* Adresse & schieben */

	for(aso = cmd_adr_shift_order; aso->name; aso++) {
		if (Memo(aso->name)) {
			if((ArgCnt < 1) || (ArgCnt > 3)) {
				WrError(1110);
				return;
			}
			decode_adr(ArgStr[1], 3, False);
			if(!adr_ok) 
				return;
			if(ArgCnt < 2) {
				ok = True; 
				adr_word = 0;
			} else {
				adr_word = EvalIntExpression(ArgStr[2], Int4, 
							     &ok);
				if (ok && FirstPassUnknown) 
					adr_word = 0;
			}
			if(!ok) 
				return;
			if(aso->allow_shifts < adr_word) {
				WrError(1380);
				return;
			}
			CodeLen = 1; 
			WAsmCode[0] = aso->code | adr_mode | (adr_word << 8);
			return;
		}
	}

	/* Ein/Ausgabe */

	if((Memo("IN")) || (Memo("OUT"))) {		
		if((ArgCnt < 2) || (ArgCnt > 3)) {
			WrError(1110);
			return;
		}
		decode_adr(ArgStr[1], 3, False);
		if(!adr_ok)
			return;
		adr_word = EvalIntExpression(ArgStr[2], Int4, &ok);
		if(!ok)
			return;
		ChkSpace(SegIO);
		CodeLen = 1;
		WAsmCode[0] = ((Memo("OUT")) ? 0xe000 : 0x8000) | adr_mode | 
			(adr_word << 8);
		return;
	}

	/* konstantes Argument */

	for(io = cmd_imm_order; io->name; io++) {
		if (Memo(io->name)) {
			if((ArgCnt < 1) || (ArgCnt > 2) || 
			   ((ArgCnt == 2) && (io->mask != 0xffff))) {
				WrError(1110);
				return;
			}
			adr_long = EvalIntExpression(ArgStr[1], Int32, &ok);
			if(!ok)
				return;
			if(FirstPassUnknown)
				adr_long &= io->mask;
			if(io->mask == 0xffff) {
				if(adr_long < -32768) {
					WrError(1315);
					return;
				}
				if(adr_long > 65535) {
					WrError(1320);
					return;
				}
				adr_word = 0;
				ok = True;
				if(ArgCnt == 2) {
					adr_word = EvalIntExpression(ArgStr[2],
								     Int4, 
								     &ok);
					if(ok && FirstPassUnknown) 
						adr_word = 0;
				}
				if(!ok)
					return;                
				CodeLen = 2;
				WAsmCode[0] = io->code | (adr_word << 8);
				WAsmCode[1] = adr_long;
				return;
			}
			if(adr_long < io->Min) {
				WrError(1315);
				return;
			}
			if(adr_long > io->Max) {
				WrError(1320);
				return;
			}
			CodeLen = 1; 
			WAsmCode[0] = io->code | (adr_long & io->mask);
			return;
		}
	}

	/* mit Hilfsregistern */

	if(Memo("LARP")) {
		if(ArgCnt != 1) {
			WrError(1110);
			return;
		}
		adr_word = eval_ar_expression(ArgStr[1], &ok);
		if(!ok)
			return;
		CodeLen = 1; 
		WAsmCode[0] = 0x5588 | adr_word;
		return;
	}

	if((Memo("LAR")) OR (Memo("SAR"))) {
		if((ArgCnt < 2) || (ArgCnt > 3)) {
			WrError(1110);
			return;
		}
		adr_word = eval_ar_expression(ArgStr[1], &ok);
		if(!ok)
			return;
		decode_adr(ArgStr[2], 3, False);
		if(!adr_ok)
			return;	  
		CodeLen = 1;
		WAsmCode[0] = ((Memo("SAR")) ? 0x7000 : 0x3000) | adr_mode | 
			(adr_word << 8);
		return;
	}

	if(Memo("LARK")) {
		if(ArgCnt != 2) {
			WrError(1110);
			return;
		}
		adr_word = eval_ar_expression(ArgStr[1], &ok);
		if(!ok)
			return;
		WAsmCode[0] = EvalIntExpression(ArgStr[2], Int8, &ok) & 0xff;
		if(!ok)
			return;
		CodeLen = 1;
		WAsmCode[0] |= 0xc000 | (adr_word << 8);
		return;
	}

	if(Memo("LRLK")) {
		if(ArgCnt != 2) {
			WrError(1110);
			return;
		}
		adr_word = eval_ar_expression(ArgStr[1], &ok);
		if(!ok)
			return;
		WAsmCode[1] = EvalIntExpression(ArgStr[2], Int16, &ok);
		if(!ok)
			return;
		CodeLen = 2;
		WAsmCode[0] = 0xd000 | (adr_word << 8);
		return;
	}

	if(Memo("LDPK")) {
		if(ArgCnt != 1) {
			WrError(1110);
			return;
		}
		WAsmCode[0] = ConstIntVal(ArgStr[1], Int16, &ok);
		if(ok && (!(WAsmCode[0] & (~0x1ff)))) {  /* emulate Int9 */
			CodeLen = 1;
			WAsmCode[0] = (WAsmCode[0] & 0x1ff) | 0xc800;
			return;
		}
		WAsmCode[0] = EvalIntExpression(ArgStr[1], Int16, &ok);
		if(!ok)
			return;
		ChkSpace(SegData);
		CodeLen = 1;
		WAsmCode[0] = ((WAsmCode[0] >> 7) & 0x1ff) | 0xc800;
		return;
	}

	if(Memo("NORM")) {
		if((ArgCnt != 1)) {
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
		WAsmCode[0] = 0xce82 | (adr_mode & 0x70);
		return;
	}

	WrXError(1200, OpPart);
}

/* ---------------------------------------------------------------------- */

static Boolean check_pc_3202x(void)
{
	switch(ActPC) {
	case SegCode: 
	case SegData: 
		return (ProgCounter() <=0xffff);
	case SegIO: 
		return (ProgCounter() <=0xf);
	default:
		return False;
	}
}

/* ---------------------------------------------------------------------- */

static Boolean is_def_3202x(void)
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

static void switch_from_3202x(void)
{
}

/* ---------------------------------------------------------------------- */

static void switch_to_3202x(void)
{
	TurnWords = False; 
	ConstMode = ConstModeIntel; 
	SetIsOccupied = False;
	
	PCSymbol = "$";
	HeaderID = 0x75; 
	NOPCode = 0x5500;
	DivideChars = ",";
	HasAttrs = False;
	
	ValidSegs = (1 << SegCode) | (1 << SegData) | (1 << SegIO);
	Grans[SegCode] = 2; ListGrans[SegCode] = 2; SegInits[SegCode] = 0;
	Grans[SegData] = 2; ListGrans[SegData] = 2; SegInits[SegData] = 0;
	Grans[SegIO  ] = 2; ListGrans[SegIO  ] = 2; SegInits[SegIO  ] = 0;
	
	MakeCode = make_code_3202x; ChkPC = check_pc_3202x; 
	IsDef = is_def_3202x; SwitchFrom = switch_from_3202x;
}

/* ---------------------------------------------------------------------- */

void code3202x_init(void)
{
	cpu_32025 = AddCPU("320C25", switch_to_3202x);
	cpu_32026 = AddCPU("320C26", switch_to_3202x);
	cpu_32028 = AddCPU("320C28", switch_to_3202x);
	
	AddCopyright("TMS320C2x-Generator (C) 1994/96 Thomas Sailer");
}
