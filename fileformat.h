/* fileformat.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Definition von Konstanten fuer das P-Format                               */
/*                                                                           */
/* Historie: 3.12.1996 Grundsteinlegung                                      */
/*                                                                           */
/*****************************************************************************/

#ifndef _FILEFORMATS_H
#define _FILEFORMATS_H

#define FileMagic 0x1489

#define FileHeaderEnd      0x00
#define FileHeaderStartAdr 0x80
#define FileHeaderDataRec  0x81

#define SegNone  0
#define SegCode  1
#define SegData  2
#define SegIData 3
#define SegXData 4
#define SegYData 5
#define SegBData 6
#define SegIO    7
#define SegReg   8

#define PCMax SegReg

#endif
