/* endian.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Little/Big-Endian-Routinen                                                */
/*                                                                           */
/* Historie: 30. 5.1996 Grundsteinlegung                                     */
/*            6. 7.1997 Dec32BlankString dazu                                */
/*                                                                           */
/*****************************************************************************/

extern Boolean BigEndian;

extern char *Integ16Format,*Integ32Format,*Integ64Format;
extern char *IntegerFormat,*LongIntFormat,*QuadIntFormat;
extern char *LargeIntFormat;


extern void WSwap(void *Field, int Cnt);

extern void DSwap(void *Field, int Cnt);

extern void QSwap(void *Field, int Cnt);

extern void DWSwap(void *Field, int Cnt);

extern void QWSwap(void *Field, int Cnt);


extern void Double_2_ieee4(Double inp, Byte *dest, Boolean NeedsBig);

extern void Double_2_ieee8(Double inp, Byte *dest, Boolean NeedsBig);

extern void Double_2_ieee10(Double inp, Byte *dest, Boolean NeedsBig);


extern Boolean Read2(FILE *file, void *Ptr);

extern Boolean Read4(FILE *file, void *Ptr);

extern Boolean Read8(FILE *file, void *Ptr);


extern Boolean Write2(FILE *file, void *Ptr);

extern Boolean Write4(FILE *file, void *Ptr);

extern Boolean Write8(FILE *file, void *Ptr);


extern char *Dec32BlankString(LongInt number, int Stellen);


extern void endian_init(void);
