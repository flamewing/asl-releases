/***************************************************************************/
/*                                                                         */
/* Project   :       as1750 -- Mil-Std-1750 Assembler and Linker           */
/*                                                                         */
/* Component :          as1750.c -- 1750 instruction assembly              */
/*                                                                         */
/* Copyright :        (C) Daimler-Benz Aerospace AG, 1994-1997             */
/*                                                                         */
/* Author    :       Oliver M. Kellogg, Dornier Satellite Systems,         */
/*                       Dept. RST13, D-81663 Munich, Germany.             */
/* Contact   :             oliver.kellogg@space.otn.dasa.de                */
/*                                                                         */
/* Disclaimer:                                                             */
/*                                                                         */
/*  This program is free software; you can redistribute it and/or modify   */
/*  it under the terms of the GNU General Public License as published by   */
/*  the Free Software Foundation; either version 2 of the License, or      */
/*  (at your option) any later version.                                    */
/*                                                                         */
/*  This program is distributed in the hope that it will be useful,        */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/*  GNU General Public License for more details.                           */
/*                                                                         */
/*  You should have received a copy of the GNU General Public License      */
/*  along with this program; if not, write to the Free Software            */
/*  Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.   */
/*                                                                         */
/***************************************************************************/

#ifdef AS1750
#include "common.h"
#include "utils.h"
#else /* ASL */
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include "datatypes.h"
#include "stringutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "as1750.h"
#define bool      char
#define ushort    unsigned short
#define ulong     unsigned long
#define status    unsigned
#define dtoi(ascii_char) (ascii_char - '0')
#endif


#ifndef AS1750
static
#endif
  status			/* Output an error text (printf style). */
error (char *layout,...)	/*  Return the ERROR status code. */
{
  va_list vargu;
  char output_line[132];

  va_start (vargu, layout);
  vsprintf (output_line, layout, vargu);
#ifdef AS1750
  fprintf (stderr, "%s line%5d: %s\n", nopath (file[curr_file].name),
	   file[curr_file].line_number, output_line);
#else /* ASL */
  WrErrorString (output_line, "\0", 0, 0);
#endif
  va_end (vargu);
  return ERROR;
}


/* Get a number. Return pointer to first position in s after the end of
 * the number, or NULL for error.
 * Will also read character constants of the form: 'x', and two-character
 * packed strings of the form "xy" (where x=>highbyte,y=>lowbyte.)
 */

#ifdef AS1750

char *
get_num (char *s, int *outnum)
{
  bool is_neg = FALSE, intel = FALSE, c_lang = FALSE, tld = FALSE;
  char *start;

  *outnum = 0;
  if (*s == '-')
    {
      is_neg = TRUE;
      ++s;
    }
  else if (*s == '+')
    ++s;
  /* determine if Intel format */
  if (isdigit (*s))
    {
      char *p = s;
      while (isxdigit (*++p))
	;
      if (upcase (*p) == 'H')
	intel = TRUE;
    }
  if (intel
      || (c_lang = (*s == '0' && upcase (*(s + 1)) == 'X'))
      || (tld = strncmp (s, "16#", 3) == 0))
    {
      s += c_lang ? 2 : tld ? 3 : 0;
      start = s;
      while (isxdigit (*s))
	{
	  *outnum = (*outnum << 4) | xtoi (*s);
	  ++s;
	}
      if (s - start > 4)
	{
	  error ("get_num -- number at '%s' too large", start);
	  return NULL;
	}
      if (intel)
	s++;
      else if (tld)
	{
	  if (*s != '#')
	    {
	      error ("get_num -- expecting '#' at end of number");
	      return NULL;
	    }
	  s++;
	}
    }
  else if (*s == '0' || (tld = (*s == '8' && *(s + 1) == '#')))
    {
      s += tld ? 2 : 1;
      start = s;
      while (*s >= '0' && *s <= '7')
	{
	  *outnum = (*outnum << 3) | (*s - '0');
	  ++s;
	}
      if (s - start > 6)
	{
	  error ("get_num -- number at '%s' too large", start);
	  return NULL;
	}
      if (tld)
	{
	  if (*s != '#')
	    {
	      error ("get_num -- expecting '#' at end of number");
	      return NULL;
	    }
	  ++s;
	}
    }
  else if (*s == '@' || (tld = (*s == '2' && *(s + 1) == '#')))
    {
      s += (tld ? 2 : 1);
      start = s;
      while (*s == '0' || *s == '1')
	{
	  *outnum = (*outnum << 1) | (*s - '0');
	  ++s;
	}
      if (s - start > 16)
	{
	  error ("get_num -- number at '%s' too large", start);
	  return NULL;
	}
      if (tld)
	{
	  if (*s != '#')
	    {
	      error ("get_num -- expecting '#' at end of number");
	      return NULL;
	    }
	  ++s;
	}
    }
  else if (isdigit (*s))
    {
      start = s;
      while (isdigit (*s))
	{
	  *outnum = (*outnum * 10) + dtoi (*s);
	  ++s;
	}
      if (s - start > 5)
	{
	  error ("get_num -- number at '%s' too large", start);
	  return NULL;
	}
    }
  else if (*s == '\'')
    {
      start = s;
      if (*++s == '\\')
	{
	  switch (*++s)
	    {
	    case 't':
	      *outnum = 9;
	      break;
	    case 'n':
	      *outnum = 10;
	      break;
	    case 'r':
	      *outnum = 13;
	      break;
	    default:
	      error ("get_num -- unknown escape '\\%c'", *s);
	      return NULL;
	    }
	}
      else
        *outnum = (int) *s & 0xFF;
      if (*++s != '\'')
	{
	  error ("get_num -- character constant incorrectly terminated", start);
	  return NULL;
	}
      ++s;
    }
  else if (*s == '"')
    {
      start = s;
      *outnum = ((int) *++s & 0xFF) << 8;
      *outnum |= (int) *++s & 0xFF;
      if (*++s != '"')
	{
	  error ("get_num -- character tuple incorrectly terminated", start);
	  return NULL;
	}
      ++s;
    }
  else
    return NULL;
  if (is_neg)
    *outnum = -*outnum;
  return s;
}

/* Get a constant symbol (previously defined by EQU) or a number.
 * Return pointer to first character after the symbol or number consumed,
 * or NULL if reading the symbol or number was unsuccessful.
 */
char *
get_sym_num (char *s, int *outnum)
{
  char *p, c;
  symbol_t sym;

  if ((p = get_num (s, outnum)) != NULL)
    return p;

  /* Didn't find a raw number; try symbol. */
  if (!issymstart (*s))
    {
      error ("expecting symbol at '%s'", s);
      return NULL;
    }
  p = s;
  while (issymchar (*p))
    p++;
  c = *p;
  *p = '\0';
  if ((sym = find_symbol (s)) == SNULL)
    {
      error ("unidentified symbol at '%s'", s);
      return NULL;
    }
  *p = c;
  if (!sym->is_constant)
    {
      error ("symbol must be constant in this context");
      return NULL;
    }
  *outnum = (int) sym->value;
  return p;
}


extern int curr_frag;		/* imported from main.c */

/* Enter a 16-bit word into the object space */
void
add_word (ushort word)
{
  struct objblock *obj = &objblk[curr_frag];

  if (obj->data == (ushort *) 0)
    {
      obj->n_allocated = 256;
      obj->data = (ushort *) malloc (obj->n_allocated * sizeof (ushort));
      obj->line = (struct linelist **) malloc
	(obj->n_allocated * sizeof (struct linelist *));
    }
  else if (obj->n_used == obj->n_allocated)
    {
      obj->n_allocated *= 2;
      obj->data = (ushort *) realloc (obj->data,
				      obj->n_allocated * sizeof (ushort));
      obj->line = (struct linelist **) realloc (obj->line,
			     obj->n_allocated * sizeof (struct linelist *));
    }
  if (obj->data == (ushort *) 0 || obj->line == (struct linelist **) 0)
    problem ("request for object space refused by OS");
  obj->data[obj->n_used] = word;
  obj->line[obj->n_used] = line;
  obj->n_used++;
}

void
add_reloc (symbol_t sym)	/* auxiliary to parse_addr() */
{
  struct reloc *rel;

  if (relblk.data == (struct reloc *) 0)
    {
      relblk.n_allocated = 256;
      relblk.data = (struct reloc *) malloc
	(relblk.n_allocated * sizeof (struct reloc));
    }
  else if (relblk.n_used == relblk.n_allocated)
    {
      relblk.n_allocated *= 2;
      relblk.data = (struct reloc *) realloc (relblk.data,
				relblk.n_allocated * sizeof (struct reloc));
    }
  if (relblk.data == (struct reloc *) 0)
    problem ("request for relocation space refused by OS");
  rel = &relblk.data[relblk.n_used];
  rel->done = FALSE;		/* default initializations */
  rel->reltype = Word_Reloc;
  rel->frag_index = curr_frag;
  rel->obj_index = objblk[curr_frag].n_used;
  rel->sym = sym;
  relblk.n_used++;
}


/* Parse the address expression at s into object space.
   Returns OKAY or ERROR. */
status
parse_addr (char *s)
{
  int num;
  char *p, c, c1;
  symbol_t sym, sym1;

  if (issymstart (*s))
    {
      p = skip_symbol (s);
      c = *p;			/* prepare for symbol lookup */
      *p = '\0';
      if ((sym = find_or_enter (s)) == SNULL)
	return ERROR;
      sym->is_referenced = TRUE;
      *p = c;
      s = p++;
      if (c == '\0')
	{
	  if (sym->is_constant)
	    add_word ((ushort) sym->value);
	  else
	    {
	      add_reloc (sym);
	      add_word (0);
	    }
	}
      else if (c != '+' && c != '-')
	return error ("error after symbolname at: %s", s);
      else if (issymstart (*p))
	{
	  s = skip_symbol (p);
	  if (*s != '\0' && *s != ',')
	    return error ("address expression too complex");
	  c1 = *s;		/* prepare for symbol lookup */
	  *s = '\0';
	  if ((sym1 = find_or_enter (p)) == SNULL)
	    return ERROR;
	  sym1->is_referenced = TRUE;
	  *s = c1;
	  if (c == '+')
	    {
	      if (sym->is_constant)
		{
		  if (sym1->is_constant)
		    {
		      long sum = sym->value + sym1->value;
		      if (sum < -0x8000L)
			return error ("negative overflow in symbol addition");
		      else if (sum > 0xFFFFL)
			return error ("positive overflow in symbol addition");
		      add_word ((ushort) sum);
		    }
		  else
		    {
		      add_reloc (sym1);
		      add_word ((ushort) sym->value);
		    }
		}
	      else
		{
		  if (sym1->is_constant)
		    {
		      add_reloc (sym);
		      add_word ((ushort) sym1->value);
		    }
		  else
		    return error ("cannot add relocatable symbols");
		}
	    }
	  else
	    /* subtraction */
	    {
	      if (sym->is_constant)
		{
		  if (sym1->is_constant)
		    {
		      long dif = sym->value - sym1->value;
		      if (dif < -0x8000L)
			return error ("negative overflow in symbol subtraction");
		      else if (dif > 0xFFFFL)
			return error ("positive overflow symbol subtraction");
		      add_word ((ushort) dif);
		    }
		  else
		    error ("cannot subtract relocatable symbol from constant symbol");
		}
	      else
		{
		  if (sym1->is_constant)
		    {
		      add_reloc (sym);
		      add_word ((ushort) - sym1->value);
		    }
		  else
		    {
		      if (objblk[sym->frag_index].section !=
			  objblk[sym1->frag_index].section)
			return error
			  ("cannot subtract relocatable symbols from different fragments");
		      if (sym->value < sym1->value)
			error ("warning: strange subtraction of relocatable symbols");
		      add_word ((ushort) (sym->value - sym1->value));
		    }
		}
	    }
	}
      else
	{
	  if (get_num (s, &num) == NULL)
	    return ERROR;
	  if (sym->is_constant)
	    {
	      long sum = sym->value + (long) num;
	      if (sum < -32768L)
		return error ("neg. overflow in symbolic expression");
	      else if (sum > 65535L)
		return error ("overflow in symbolic expression");
	      add_word ((ushort) sum);
	    }
	  else
	    {
	      add_reloc (sym);
	      add_word ((ushort) num);
	    }
	}
    }
  else if ((s = get_num (s, &num)) == NULL)
    return ERROR;
  else
    {
      if (*s == '\0')
	add_word ((ushort) num);
      else if (*s != '+')
	return error ("expected '+' in address expression at: %s", s);
      else
	{
	  ++s;
	  if (!issymstart (*s))
	    return error ("expected symbolname in address expression at: %s", s);
	  p = skip_symbol (s);
	  if (*p != '\0' && *p != ',')
	    return error ("illegal characters after symbolname at: %s", s);
	  c = *p;		/* prepare for symbol lookup */
	  *p = '\0';
	  if ((sym = find_or_enter (s)) == SNULL)
	    return ERROR;
	  sym->is_referenced = TRUE;
	  *p = c;
	  s = p;
	  if (sym->is_constant)
	    add_word ((ushort) (sym->value + (long) num));
	  else
	    {
	      add_reloc (sym);
	      add_word ((ushort) num);
	    }
	}
    }
  return OKAY;
}

#else /* ASL */

static char *
get_sym_num (char *s, int *outnum)
{
  Boolean okay;
  *outnum = (int) EvalIntExpression (s, Int16, &okay);
  if (!okay)
    return NULL;
  return (s + strlen (s));	/* Any non-NULL value will do here. */
}


#define add_word(word)  WAsmCode[CodeLen++]=word

static status
parse_addr (char *s)
{
  int value;
  Boolean okay;
  value = (int) EvalIntExpression (s, Int16, &okay);
  if (!okay)
    return ERROR;
  add_word ((ushort) value);
  return OKAY;
}

#endif /* from #else of #ifdef AS1750 */

/* From here on, everything is identical between as1750 and ASL. */

static ushort
get_num_bounded (char *s, int lowlim, int highlim)
{
  int num;
  if (get_sym_num (s, &num) == NULL)
    return ERROR;
  if (num < lowlim || num > highlim)
    return error ("number not in range %d..%d", lowlim, highlim);
  return (ushort) num;
}

static ushort
get_regnum (char *s)
{
  ushort regnum;
  *s = toupper (*s);
  if (*s != 'R')
    return error ("expecting register at '%s'", s);
  ++s;
  if (!isdigit (*s))
    return error ("expecting register at '%s'", s);
  regnum = dtoi (*s);
  ++s;
  if (isdigit (*s))
    {
      regnum = regnum * 10 + dtoi (*s);
      if (regnum > 15)
	return error ("register number out of range");
      ++s;
    }
  return regnum;
}


/**********************************************************************/
/* Functions to process opcode arguments according to addressing mode */

static int n_args;
static char *arg[4];

static status
check_indexreg (void)
{
  if (n_args > 2)
    {
      ushort rx;

      if ((rx = get_regnum (arg[2])) == ERROR)
	return ERROR;
      if (rx == 0)
	return error ("R0 not an index register");
#ifdef AS1750
      objblk[curr_frag].data[objblk[curr_frag].n_used - 2] |= rx;
#else /* ASL */
      WAsmCode[0] |= rx;
#endif
    }
  return OKAY;
}


static ushort
as_r (ushort oc)
{
  ushort ra, rb;
  if (n_args != 2)
    return error ("incorrect number of operands");
  if ((ra = get_regnum (arg[0])) == ERROR)
    return ERROR;
  if ((rb = get_regnum (arg[1])) == ERROR)
    return ERROR;

  add_word (oc | (ra << 4) | rb);
  return 1;
}

static ushort
as_mem (ushort oc)
{
  ushort ra;
  if (n_args < 2)
    return error ("insufficient number of operands");
  if ((ra = get_regnum (arg[0])) == ERROR)
    return ERROR;
  add_word (oc | (ra << 4));
  if (parse_addr (arg[1]))
    return ERROR;
  if (check_indexreg ())
    return ERROR;
  return 2;
}

static ushort
as_addr (ushort oc)		/* LST and LSTI */
{
  if (n_args < 1)
    return error ("insufficient number of operands");
  add_word (oc);
  if (parse_addr (arg[0]))
    return ERROR;
  n_args++;			/* cheat check_indexreg() */
  arg[2] = arg[1];
  if (check_indexreg ())
    return ERROR;
  return 2;
}

static ushort
as_xmem (ushort oc)		/* MIL-STD-1750B extended mem. addr. */
{
  ushort ra;

  if (n_args < 2)
    return error ("insufficient number of operands");
  if ((ra = get_regnum (arg[0])) == ERROR)
    return ERROR;
  add_word (oc | (ra << 4));
  if (parse_addr (arg[1]))
    return ERROR;
  if (n_args > 2)
    {
      ushort rx;
      if ((rx = get_regnum (arg[2])) == ERROR)
	return ERROR;
#ifdef AS1750
      objblk[curr_frag].data[objblk[curr_frag].n_used - 2] |= rx;
#else /* ASL */
      WAsmCode[0] |= rx;
#endif
    }
  return 2;
}

static ushort
as_im_0_15 (ushort oc)		/* for STC, LM, TB,SB,RB,TSB */
{
  ushort ra;
  if (n_args < 2)
    return error ("insufficient number of operands");
  if ((ra = get_num_bounded (arg[0], 0, 15)) == ERROR)
    return ERROR;
  add_word (oc | (ra << 4));
  if (parse_addr (arg[1]))
    return ERROR;
  if (check_indexreg ())
    return ERROR;
  return 2;
}

static ushort
as_im_1_16 (ushort oc)
{
  ushort ra;
  if (n_args < 2)
    return error ("insufficient number of operands");
  if ((ra = get_num_bounded (arg[0], 1, 16)) == ERROR)
    return ERROR;
  add_word (oc | ((ra - 1) << 4));
  if (parse_addr (arg[1]))
    return ERROR;
  if (check_indexreg ())
    return ERROR;
  return 2;
}

static ushort
as_im_ocx (ushort oc)
{
  ushort ra;
  if (n_args != 2)
    return error ("incorrect number of operands");
  if ((ra = get_regnum (arg[0])) == ERROR)
    return ERROR;
  add_word (oc | (ra << 4));	/* oc has OCX in LSnibble */
  if (parse_addr (arg[1]))
    return ERROR;
  return 2;
}

static ushort
as_is (ushort oc)
{
  ushort ra, i;
  if (n_args != 2)
    return error ("incorrect number of operands");
  if ((ra = get_regnum (arg[0])) == ERROR)
    return ERROR;
  if ((i = get_num_bounded (arg[1], 1, 16)) == ERROR)
    return ERROR;
  add_word (oc | (ra << 4) | (i - 1));
  return 1;
}

static ushort
as_icr (ushort oc)
{
#ifdef AS1750
  struct objblock *obj = &objblk[curr_frag];
  int last = obj->n_used;
#endif
  if (n_args != 1)
    return error ("incorrect number of operands");
  if (parse_addr (arg[0]))
    return ERROR;
#ifdef AS1750
  /* If symbol relocation, then set the relocation type to Byte_Reloc */
  if (relblk.n_used > 0)
    {
      struct reloc *rel = &relblk.data[relblk.n_used - 1];
      if (rel->frag_index == curr_frag && rel->obj_index == last)
	rel->reltype = Byte_Reloc;
    }
  obj->data[last] &= 0x00FF;
  obj->data[last] |= oc;
#else /* ASL */
  {
    const short target = (short) WAsmCode[0];
    const long curr_pc = (long) EProgCounter () & 0xFFFFL;
    const long diff = (long) target - curr_pc;
    if (diff < -128L || diff > 127L)
      return error
	("address distance too large in Instruction Counter Relative operation");
    WAsmCode[0] = oc | (ushort) (diff & 0xFFL);
  }
#endif
  return 1;
}

static ushort
as_b (ushort oc)
{
  char r;
  ushort br, lobyte;
  if (n_args != 2)
    return error ("incorrect number of operands");
  r = toupper (*arg[0]);
  if (r != 'B' && r != 'R')
    return error ("expecting base register");
  if ((br = get_num_bounded (arg[0] + 1, 12, 15)) == ERROR)
    return ERROR;
  if ((lobyte = get_num_bounded (arg[1], 0, 255)) == ERROR)
    return ERROR;
  add_word (oc | ((br - 12) << 8) | lobyte);
  return 1;
}

static ushort
as_bx (ushort oc)
{
  char r;
  ushort br, rx = 0;
  if (n_args != 2)
    return error ("incorrect number of operands");
  r = toupper (*arg[0]);
  if ((r != 'B' && r != 'R')
      || (br = get_num_bounded (arg[0] + 1, 12, 15)) == ERROR)
    return error ("expecting base register");
  if ((rx = get_regnum (arg[1])) == ERROR)
    return ERROR;
  if (rx == 0)
    return error ("R0 not an index register");
  add_word (oc | ((br - 12) << 8) | rx);
  return 1;
}

static ushort
as_r_imm (ushort oc)		/* for shifts with immediate shiftcount */
{
  ushort rb, n;
  if (n_args != 2)
    return error ("incorrect number of operands");
  if ((rb = get_regnum (arg[0])) == ERROR)
    return ERROR;
  if ((n = get_num_bounded (arg[1], 1, 16)) == ERROR)
    return ERROR;
  add_word (oc | ((n - 1) << 4) | rb);
  return 1;
}

static ushort
as_imm_r (ushort oc)		/* for test/set/reset-bit in reg. */
{
  ushort n, rb;
  if (n_args != 2)
    return error ("incorrect number of operands");
  if ((n = get_num_bounded (arg[0], 0, 15)) == ERROR)
    return ERROR;
  if ((rb = get_regnum (arg[1])) == ERROR)
    return ERROR;
  add_word (oc | (n << 4) | rb);
  return 1;
}

static ushort
as_jump (ushort oc)
{
  ushort condcode = 0xFFFF;
  static const struct
    {
      char *name;
      ushort value;
    }
  cond[] =
  {				/* CPZN */
    { "LT",  0x1 },		/* 0001 */
    { "LZ",  0x1 },		/* 0001 */
    { "EQ",  0x2 },		/* 0010 */
    { "EZ",  0x2 },		/* 0010 */
    { "LE",  0x3 },		/* 0011 */
    { "LEZ", 0x3 },		/* 0011 */
    { "GT",  0x4 },		/* 0100 */
    { "GTZ", 0x4 },		/* 0100 */
    { "NE",  0x5 },		/* 0101 */
    { "NZ",  0x5 },		/* 0101 */
    { "GE",  0x6 },		/* 0110 */
    { "GEZ", 0x6 },		/* 0110 */
    { "ALL", 0x7 },		/* 0111 */
    { "CY",  0x8 },		/* 1000 */
    { "CLT", 0x9 },		/* 1001 */
    { "CEQ", 0xA },		/* 1010 */
    { "CEZ", 0xA },		/* 1010 */
    { "CLE", 0xB },		/* 1011 */
    { "CGT", 0xC },		/* 1100 */
    { "CNZ", 0xD },		/* 1101 */
    { "CGE", 0xE },		/* 1110 */
    { "UC",  0xF }		/* 1111 */
  };
  if (n_args < 2)
    return error ("insufficient number of operands");
  if (isalpha (*arg[0]))
    {
      int i;
      for (i = 0; i < 22; i++)
	if (!strcasecmp (arg[0], cond[i].name))
	  {
	    condcode = cond[i].value;
	    break;
	  }
    }
  if (condcode == 0xFFFF)
    if ((condcode = get_num_bounded (arg[0], 0, 15)) == ERROR)
      return ERROR;
  add_word (oc | (condcode << 4));
  if (parse_addr (arg[1]))
    return ERROR;
  if (check_indexreg ())
    return ERROR;
  return 2;
}

static ushort
as_s (ushort oc)		/* For the moment, BEX only. */
{
  ushort lsnibble;
  if (n_args != 1)
    return error ("incorrect number of operands");
  if ((lsnibble = get_num_bounded (arg[0], 0, 15)) == ERROR)
    return ERROR;
  add_word (oc | lsnibble);
  return 1;
}

static ushort
as_sr (ushort oc)		/* XBR and URS. */
{
  ushort hlp;
  if (n_args != 1)
    return error ("incorrect number of operands");
  if ((hlp = get_regnum (arg[0])) == ERROR)
    return ERROR;
  add_word (oc | (hlp << 4));
  return 1;
}

static ushort
as_xio (ushort oc)
{
  ushort ra;
  int cmdfld = -1;
  static const struct
    {
      char *mnem;
      ushort cmd;
    }
  xio[] =
  {
    { "SMK",  0x2000 },
    { "CLIR", 0x2001 },
    { "ENBL", 0x2002 },
    { "DSBL", 0x2003 },
    { "RPI",  0x2004 },
    { "SPI",  0x2005 },
    { "OD",   0x2008 },
    { "RNS",  0x200A },
    { "WSW",  0x200E },
    { "CO",   0x4000 },
    { "CLC",  0x4001 },
    { "MPEN", 0x4003 },
    { "ESUR", 0x4004 },
    { "DSUR", 0x4005 },
    { "DMAE", 0x4006 },
    { "DMAD", 0x4007 },
    { "TAS",  0x4008 },
    { "TAH",  0x4009 },
    { "OTA",  0x400A },
    { "GO",   0x400B },
    { "TBS",  0x400C },
    { "TBH",  0x400D },
    { "OTB",  0x400E },
    { "LMP",  0x5000 },
    { "WIPR", 0x5100 },
    { "WOPR", 0x5200 },
    { "RMP",  0xD000 },
    { "RIPR", 0xD100 },
    { "ROPR", 0xD200 },
    { "RMK",  0xA000 },
    { "RIC1", 0xA001 },
    { "RIC2", 0xA002 },
    { "RPIR", 0xA004 },
    { "RDOR", 0xA008 },
    { "RDI",  0xA009 },
    { "TPIO", 0xA00B },
    { "RMFS", 0xA00D },
    { "RSW",  0xA00E },
    { "RCFR", 0xA00F },
    { "CI",   0xC000 },
    { "RCS",  0xC001 },
    { "ITA",  0xC00A },
    { "ITB",  0xC00E },
    { "", 0xFFFF }
  };

  if (n_args < 2)
    return error ("incorrect number of operands");
  if ((ra = get_regnum (arg[0])) == ERROR)
    return ERROR;
  add_word (oc | (ra << 4));
  /* Get the XIO command field. */
  if (isalpha (*arg[1]))
    {
      int i;
      for (i = 0; xio[i].cmd != 0xFFFF; i++)
	if (!strcasecmp (arg[1], xio[i].mnem))
	  break;
      if (xio[i].cmd != 0xFFFF)
	cmdfld = xio[i].cmd;
    }
  if (cmdfld == -1)
    if (get_sym_num (arg[1], &cmdfld) == NULL)
      return ERROR;
  add_word ((ushort) cmdfld);
  if (check_indexreg ())
    return ERROR;
  return 2;
}

static ushort
as_none (ushort oc)
{
  add_word (oc);
  return 1;
}

/* end of argument assembly functions */

/***********************************************************************/

static struct
  {
    char *mnemon;
    ushort opcode;
    ushort (*as_args) (ushort);
  } optab[] =
 {
  { "AISP", 0xA200, as_is },		/* Sorted by beginning letter. */
  { "AIM",  0x4A01, as_im_ocx },	/* Within each beginning letter, */
  { "AR",   0xA100, as_r },		/* sorted by approximate */
  { "A",    0xA000, as_mem },		/* instruction frequency. */
  { "ANDM", 0x4A07, as_im_ocx },
  { "ANDR", 0xE300, as_r },
  { "AND",  0xE200, as_mem },
  { "ABS",  0xA400, as_r },
  { "AB",   0x1000, as_b },
  { "ANDB", 0x3400, as_b },
  { "ABX",  0x4040, as_bx },
  { "ANDX", 0x40E0, as_bx },
  { "BEZ",  0x7500, as_icr },
  { "BNZ",  0x7A00, as_icr },
  { "BGT",  0x7900, as_icr },
  { "BLE",  0x7800, as_icr },
  { "BGE",  0x7B00, as_icr },
  { "BLT",  0x7600, as_icr },
  { "BR",   0x7400, as_icr },
  { "BEX",  0x7700, as_s },
  { "BPT",  0xFFFF, as_none },
  { "BIF",  0x4F00, as_s },
  { "CISP", 0xF200, as_is },
  { "CIM",  0x4A0A, as_im_ocx },
  { "CR",   0xF100, as_r },
  { "C",    0xF000, as_mem },
  { "CISN", 0xF300, as_is },
  { "CB",   0x3800, as_b },
  { "CBL",  0xF400, as_mem },
  { "CBX",  0x40C0, as_bx },
  { "DLR",  0x8700, as_r },
  { "DL",   0x8600, as_mem },
  { "DST",  0x9600, as_mem },
  { "DSLL", 0x6500, as_r_imm },
  { "DSRL", 0x6600, as_r_imm },
  { "DSRA", 0x6700, as_r_imm },
  { "DSLC", 0x6800, as_r_imm },
  { "DSLR", 0x6D00, as_r },
  { "DSAR", 0x6E00, as_r },
  { "DSCR", 0x6F00, as_r },
  { "DECM", 0xB300, as_im_1_16 },
  { "DAR",  0xA700, as_r },
  { "DA",   0xA600, as_mem },
  { "DSR",  0xB700, as_r },
  { "DS",   0xB600, as_mem },
  { "DMR",  0xC700, as_r },
  { "DM",   0xC600, as_mem },
  { "DDR",  0xD700, as_r },
  { "DD",   0xD600, as_mem },
  { "DCR",  0xF700, as_r },
  { "DC",   0xF600, as_mem },
  { "DLB",  0x0400, as_b },
  { "DSTB", 0x0C00, as_b },
  { "DNEG", 0xB500, as_r },
  { "DABS", 0xA500, as_r },
  { "DR",   0xD500, as_r },
  { "D",    0xD400, as_mem },
  { "DISP", 0xD200, as_is },
  { "DIM",  0x4A05, as_im_ocx },
  { "DISN", 0xD300, as_is },
  { "DVIM", 0x4A06, as_im_ocx },
  { "DVR",  0xD100, as_r },
  { "DV",   0xD000, as_mem },
  { "DLI",  0x8800, as_mem },
  { "DSTI", 0x9800, as_mem },
  { "DB",   0x1C00, as_b },
  { "DBX",  0x4070, as_bx },
  { "DLBX", 0x4010, as_bx },
  { "DSTX", 0x4030, as_bx },
  { "DLE",  0xDF00, as_xmem },
  { "DSTE", 0xDD00, as_xmem },
  { "EFL",  0x8A00, as_mem },
  { "EFST", 0x9A00, as_mem },
  { "EFCR", 0xFB00, as_r },
  { "EFC",  0xFA00, as_mem },
  { "EFAR", 0xAB00, as_r },
  { "EFA",  0xAA00, as_mem },
  { "EFSR", 0xBB00, as_r },
  { "EFS",  0xBA00, as_mem },
  { "EFMR", 0xCB00, as_r },
  { "EFM",  0xCA00, as_mem },
  { "EFDR", 0xDB00, as_r },
  { "EFD",  0xDA00, as_mem },
  { "EFLT", 0xEB00, as_r },
  { "EFIX", 0xEA00, as_r },
  { "FAR",  0xA900, as_r },
  { "FA",   0xA800, as_mem },
  { "FSR",  0xB900, as_r },
  { "FS",   0xB800, as_mem },
  { "FMR",  0xC900, as_r },
  { "FM",   0xC800, as_mem },
  { "FDR",  0xD900, as_r },
  { "FD",   0xD800, as_mem },
  { "FCR",  0xF900, as_r },
  { "FC",   0xF800, as_mem },
  { "FABS", 0xAC00, as_r },
  { "FIX",  0xE800, as_r },
  { "FLT",  0xE900, as_r },
  { "FNEG", 0xBC00, as_r },
  { "FAB",  0x2000, as_b },
  { "FABX", 0x4080, as_bx },
  { "FSB",  0x2400, as_b },
  { "FSBX", 0x4090, as_bx },
  { "FMB",  0x2800, as_b },
  { "FMBX", 0x40A0, as_bx },
  { "FDB",  0x2C00, as_b },
  { "FDBX", 0x40B0, as_bx },
  { "FCB",  0x3C00, as_b },
  { "FCBX", 0x40D0, as_bx },
  { "INCM", 0xA300, as_im_1_16 },
  { "JC",   0x7000, as_jump },
  { "J",    0x7400, as_icr },	/* TBD (GAS) */
  { "JEZ",  0x7500, as_icr },	/* TBD (GAS) */
  { "JLE",  0x7800, as_icr },	/* TBD (GAS) */
  { "JGT",  0x7900, as_icr },	/* TBD (GAS) */
  { "JNZ",  0x7A00, as_icr },	/* TBD (GAS) */
  { "JGE",  0x7B00, as_icr },	/* TBD (GAS) */
  { "JLT",  0x7600, as_icr },	/* TBD (GAS) */
  { "JCI",  0x7100, as_jump },
  { "JS",   0x7200, as_mem },
  { "LISP", 0x8200, as_is },
  { "LIM",  0x8500, as_mem },
  { "LR",   0x8100, as_r },
  { "L",    0x8000, as_mem },
  { "LISN", 0x8300, as_is },
  { "LB",   0x0000, as_b },
  { "LBX",  0x4000, as_bx },
  { "LSTI", 0x7C00, as_addr },
  { "LST",  0x7D00, as_addr },
  { "LI",   0x8400, as_mem },
  { "LM",   0x8900, as_im_0_15 },
  { "LUB",  0x8B00, as_mem },
  { "LLB",  0x8C00, as_mem },
  { "LUBI", 0x8D00, as_mem },
  { "LLBI", 0x8E00, as_mem },
  { "LE",   0xDE00, as_xmem },
  { "MISP", 0xC200, as_is },
  { "MSIM", 0x4A04, as_im_ocx },
  { "MSR",  0xC100, as_r },
  { "MS",   0xC000, as_mem },
  { "MISN", 0xC300, as_is },
  { "MIM",  0x4A03, as_im_ocx },
  { "MR",   0xC500, as_r },
  { "M",    0xC400, as_mem },
  { "MOV",  0x9300, as_r },
  { "MB",   0x1800, as_b },
  { "MBX",  0x4060, as_bx },
  { "NEG",  0xB400, as_r },
  { "NOP",  0xFF00, as_none },
  { "NIM",  0x4A0B, as_im_ocx },
  { "NR",   0xE700, as_r },
  { "N",    0xE600, as_mem },
  { "ORIM", 0x4A08, as_im_ocx },
  { "ORR",  0xE100, as_r },
  { "OR",   0xE000, as_mem },
  { "ORB",  0x3000, as_b },
  { "ORBX", 0x40F0, as_bx },
  { "PSHM", 0x9F00, as_r },
  { "POPM", 0x8F00, as_r },
  { "RBR",  0x5400, as_imm_r },
  { "RVBR", 0x5C00, as_r },
  { "RB",   0x5300, as_im_0_15 },
  { "RBI",  0x5500, as_im_0_15 },
  { "ST",   0x9000, as_mem },
  { "STC",  0x9100, as_im_0_15 },
  { "SISP", 0xB200, as_is },
  { "SIM",  0x4A02, as_im_ocx },
  { "SR",   0xB100, as_r },
  { "S",    0xB000, as_mem },
  { "SLL",  0x6000, as_r_imm },
  { "SRL",  0x6100, as_r_imm },
  { "SRA",  0x6200, as_r_imm },
  { "SLC",  0x6300, as_r_imm },
  { "SLR",  0x6A00, as_r },
  { "SAR",  0x6B00, as_r },
  { "SCR",  0x6C00, as_r },
  { "SJS",  0x7E00, as_mem },
  { "STB",  0x0800, as_b },
  { "SBR",  0x5100, as_imm_r },
  { "SB",   0x5000, as_im_0_15 },
  { "SVBR", 0x5A00, as_r },
  { "SOJ",  0x7300, as_mem },
  { "SBB",  0x1400, as_b },
  { "STBX", 0x4020, as_bx },
  { "SBBX", 0x4050, as_bx },
  { "SBI",  0x5200, as_im_0_15 },
  { "STZ",  0x9100, as_addr },
  { "STCI", 0x9200, as_im_0_15 },
  { "STI",  0x9400, as_mem },
  { "SFBS", 0x9500, as_r },
  { "SRM",  0x9700, as_mem },
  { "STM",  0x9900, as_im_0_15 },
  { "STUB", 0x9B00, as_mem },
  { "STLB", 0x9C00, as_mem },
  { "SUBI", 0x9D00, as_mem },
  { "SLBI", 0x9E00, as_mem },
  { "STE",  0xDC00, as_xmem },
  { "TBR",  0x5700, as_imm_r },
  { "TB",   0x5600, as_im_0_15 },
  { "TBI",  0x5800, as_im_0_15 },
  { "TSB",  0x5900, as_im_0_15 },
  { "TVBR", 0x5E00, as_r },
  { "URS",  0x7F00, as_sr },
  { "UAR",  0xAD00, as_r },
  { "UA",   0xAE00, as_mem },
  { "USR",  0xBD00, as_r },
  { "US",   0xBE00, as_mem },
  { "UCIM", 0xF500, as_im_ocx },
  { "UCR",  0xFC00, as_r },
  { "UC",   0xFD00, as_mem },
  { "VIO",  0x4900, as_mem },
  { "XORR", 0xE500, as_r },
  { "XORM", 0x4A09, as_im_ocx },
  { "XOR",  0xE400, as_mem },
  { "XWR",  0xED00, as_r },
  { "XBR",  0xEC00, as_sr },
  { "XIO",  0x4800, as_xio },
  { "", 0, as_none }			/* end-of-array marker */
};

/* Offset table with indexes into optab: indexed by starting letter
   (A => 0, B => 1, ..., Z => 25), returns the starting index
   and number of entries for that letter in optab[].
   start_index is -1 if no entries for that starting letter in optab[]. */

static struct
  {
    int start_index;
    int n_entries;
  }
ofstab[26];

/* Initialize ofstab[]. This must be called before as1750() can be used. */
void
init_as1750 ()
{
  int optab_ndx, ofstab_ndx = 1;
  char c;

  ofstab[0].start_index = 0;
  ofstab[0].n_entries = 1;
  for (optab_ndx = 1; (c = *optab[optab_ndx].mnemon) != '\0'; optab_ndx++)
    {
      if (c != *optab[optab_ndx - 1].mnemon)
	{
	  while (ofstab_ndx != c - 'A')
	    ofstab[ofstab_ndx++].start_index = -1;
	  ofstab[ofstab_ndx].start_index = optab_ndx;
	  ofstab[ofstab_ndx++].n_entries = 1;
	}
      else
	ofstab[ofstab_ndx - 1].n_entries++;
    }
}


/************************** HERE'S THE BEEF: ****************************/

ushort
as1750 (char *operation, int n_operands, char *operand[])
{
  int i, optab_ndx, firstletter;
  bool found = FALSE;

  if (n_operands < 0 || n_operands > 3 || *operation < 'A' || *operation > 'X')
    return NO_OPCODE;
  n_args = n_operands;
  for (i = 0; i < n_args; i++)
    arg[i] = operand[i];
  firstletter = (int) (*operation - 'A');
  if ((optab_ndx = ofstab[firstletter].start_index) < 0)
    return NO_OPCODE;
  for (i = 0; i < ofstab[firstletter].n_entries; i++, optab_ndx++)
    if (!strcmp (operation + 1, optab[optab_ndx].mnemon + 1))
      {
	found = TRUE;
	break;
      }
  if (!found)
    return NO_OPCODE;
  return (*optab[optab_ndx].as_args) (optab[optab_ndx].opcode);
}

