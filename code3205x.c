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
} cmd_fixed_order[] = {{"ABS",    0xbe00},
		       {"ADCB",   0xbe11},
		       {"ADDB",   0xbe10},
		       {"ANDB",   0xbe12},
		       {"CMPL",   0xbe01},
		       {"CRGT",   0xbe1b},
		       {"CRLT",   0xbe1c},
		       {"EXAR",   0xbe1d},
		       {"LACB",   0xbe1f},
		       {"NEG",    0xbe02},
		       {"ORB",    0xbe13},
		       {"ROL",    0xbe0c},
		       {"ROLB",   0xbe14},
		       {"ROR",    0xbe0d},
		       {"RORB",   0xbe15},
		       {"SACB",   0xbe1e},
		       {"SATH",   0xbe5a},
		       {"SATL",   0xbe5b},
		       {"SBB",    0xbe18},
		       {"SBBB",   0xbe19},
		       {"SFL",    0xbe09},
		       {"SFLB",   0xbe16},
		       {"SFR",    0xbe0a},
		       {"SFRB",   0xbe17},
		       {"XORB",   0xbe1a},
		       {"ZAP",    0xbe59},
		       {"APAC",   0xbe04},
		       {"PAC",    0xbe03},
		       {"SPAC",   0xbe05},
		       {"ZPR",    0xbe58},
		       {"BACC",   0xbe20},
		       {"BACCD",  0xbe21},
		       {"CALA",   0xbe30},
		       {"CALAD",  0xbe3d},
		       {"NMI",    0xbe52},
		       {"RET",    0xef00},
		       {"RETD",   0xff00},
		       {"RETE",   0xbe3a},
		       {"RETI",   0xbe38},
		       {"TRAP",   0xbe51},
		       {"IDLE",   0xbe22},
		       {"NOP",    0x8b00},
		       {"POP",    0xbe32},
		       {"PUSH",   0xbe3c},
		       {"IDLE2",  0xbe23},
		       {NULL,     0}};

static const struct cmd_fixed_order
cmd_adr_order[] = {{"ADDC",   0x6000},
		   {"ADDS",   0x6200},
		   {"ADDT",   0x6300},
		   {"AND",    0x6e00},
		   {"LACL",   0x6900},
		   {"LACT",   0x6b00},
		   {"OR",     0x6d00},
		   {"SUBB",   0x6400},
		   {"SUBC",   0x0a00},
		   {"SUBS",   0x6600},
		   {"SUBT",   0x6700},
		   {"XOR",    0x6c00},
		   {"ZALR",   0x6800},
		   {"LDP",    0x0d00},
		   {"APL",    0x5a00},
		   {"CPL",    0x5b00},
		   {"OPL",    0x5900},
		   {"XPL",    0x5800},
		   {"MAR",    0x8b00},
		   {"LPH",    0x7500},
		   {"LT",     0x7300},
		   {"LTA",    0x7000},
		   {"LTD",    0x7200},
		   {"LTP",    0x7100},
		   {"LTS",    0x7400},
		   {"MADD",   0xab00},
		   {"MADS",   0xaa00},
		   {"MPY",    0x5400},
		   {"MPYA",   0x5000},
		   {"MPYS",   0x5100},
		   {"MPYU",   0x5500},
		   {"SPH",    0x8d00},
		   {"SPL",    0x8c00},
		   {"SQRA",   0x5200},
		   {"SQRS",   0x5300},
		   {"BLDP",   0x5700},
		   {"DMOV",   0x7700},
		   {"TBLR",   0xa600},
		   {"TBLW",   0xa700},
		   {"BITT",   0x6f00},
		   {"POPD",   0x8a00},
		   {"PSHD",   0x7600},
		   {"RPT",    0x0b00},
		   {NULL,     0}};


static const struct cmd_jmp_order {
	char *name;
	Word code;
	Boolean cond;
} cmd_jmp_order[] = {{"B",      0x7980,  False},
		     {"BD",     0x7d80,  False},
		     {"BANZ",   0x7b80,  False},
		     {"BANZD",  0x7f80,  False},
		     {"BCND",   0xe000,  True},
		     {"BCNDD",  0xf000,  True},
		     {"CALL",   0x7a80,  False},
		     {"CALLD",  0x7e80,  False},
		     {"CC",     0xe800,  True},
		     {"CCD",    0xf800,  True},
		     {NULL,     0,       False}};

static const struct cmd_fixed_order
cmd_plu_order[] = {{"APL",   0x5e00},
		   {"CPL",   0x5f00},
		   {"OPL",   0x5d00},
		   {"SPLK",  0xae00},
		   {"XPL",   0x5c00}};

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

static void decode_adr(char *arg, Integer aux, Boolean must1)
{
	Byte h;
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

static Word decode_cond(Integer argp)
{
	static const struct condition {
		char *name;
		Word codeand;
		Word codeor;
		Byte iszl;
		Byte isc;
		Byte isv;
		Byte istp;
	} cond_tab[] = {{"EQ",  0xf33, 0x088, 1, 0, 0, 0},
			{"NEQ", 0xf33, 0x008, 1, 0, 0, 0},
			{"LT",  0xf33, 0x044, 1, 0, 0, 0},
			{"LEQ", 0xf33, 0x0cc, 1, 0, 0, 0},
			{"GT",  0xf33, 0x004, 1, 0, 0, 0},
			{"GEQ", 0xf33, 0x08c, 1, 0, 0, 0},
			{"NC",  0xfee, 0x001, 0, 1, 0, 0},
			{"C",   0xfee, 0x011, 0, 1, 0, 0},
			{"NOV", 0xfdd, 0x002, 0, 0, 1, 0},
			{"OV",  0xfdd, 0x022, 0, 0, 1, 0},
			{"BIO", 0x0ff, 0x000, 0, 0, 0, 1},
			{"NTC", 0x0ff, 0x200, 0, 0, 0, 1},
			{"TC",  0x0ff, 0x100, 0, 0, 0, 1},
			{"UNC", 0x0ff, 0x300, 0, 0, 0, 1},
			{NULL,  0xfff, 0x000, 0, 0, 0, 0}};
	const struct condition *cndp;
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

static void pseudo_store(void (*callback)(Boolean *, Integer *, LongInt))
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
	const struct cmd_fixed_order *fo;
	const struct cmd_jmp_order *jo;
	static const struct bit_table {
		char *name;
		Word code;
	} bit_table[] = {{"OVM",  0xbe42 },
			 {"SXM",  0xbe46 },
			 {"HM",   0xbe48 },
			 {"TC",   0xbe4a },
			 {"C",    0xbe4e },
			 {"XF",   0xbe4c },
			 {"CNF",  0xbe44 },
			 {"INTM", 0xbe40 },
			 {NULL,   0      }};
	const struct bit_table *bitp;

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
}

/* ---------------------------------------------------------------------- */

void code3205x_init(void) {
	cpu_32050 = AddCPU("320C50", switch_to_3205x);
	cpu_32051 = AddCPU("320C51", switch_to_3205x);
	cpu_32053 = AddCPU("320C53", switch_to_3205x);

	AddCopyright("TMS320C5x-Generator (C) 1995/96 Thomas Sailer");
}


