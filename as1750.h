#ifndef _AS1750_H
#define _AS1750_H

/* as1750.h  --  exports of as1750.c */

extern void init_as1750 ();
extern unsigned short as1750 (char *operation,
			      int n_operands, char *operand[]);

#ifdef AS1750
extern void add_word (ushort word);
extern void add_reloc (symbol_t sym);
extern char *get_num (char *s, int *outnum);
extern char *get_sym_num (char *s, int *outnum);
extern status parse_addr (char *s);
extern status error (char *layout, ...);
#else  /* ASL */
#define OKAY      0
#define ERROR     0xFFFD
#define NO_OPCODE 0xFFFE
#endif

#endif /* _AS1750_H */
