/* fileformat.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Definition von Konstanten fuer das P-Format                               */
/*                                                                           */
/* Historie: 3.12.1996 Grundsteinlegung                                      */
/*           11. 9.1998 ROMDATA-Segment hinzugenommen                        */
/*           12. 7.1999 RelocRec-Typ hinzugenommen                           */
/*                                                                           */
/*****************************************************************************/

#ifndef _FILEFORMATS_H
#define _FILEFORMATS_H

#define FileMagic 0x1489

#define FileHeaderEnd      0x00   /* Dateiende */
#define FileHeaderStartAdr 0x80   /* Einsprungadresse absolut */
#define FileHeaderDataRec  0x81   /* normaler Datenrecord */
#define FileHeaderRelocRec 0x82   /* normaler Datenrecord mit Relokationsinformationen */

#define SegNone  0
#define SegCode  1
#define SegData  2
#define SegIData 3
#define SegXData 4
#define SegYData 5
#define SegBData 6
#define SegIO    7
#define SegReg   8
#define SegRData 9

#define PCMax SegRData

enum {RelocNone, Reloc8, RelocL16, RelocM16, RelocL24, RelocM24,
                         RelocL32, RelocM32, RelocL64, RelocH64,
                         RelocVar = 0x80};

#endif
