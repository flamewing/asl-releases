/* headids.c */
/*****************************************************************************/
/* Makroassembler AS                                                         */
/*                                                                           */
/* Hier sind alle Prozessor-IDs mit ihren Eigenschaften gesammelt            */
/*                                                                           */
/* Historie: 29. 8.1998 angelegt                                             */
/*                      Intel i960                                           */
/*           30. 8.1998 NEC uPD7725                                          */
/*            6. 9.1998 NEC uPD77230                                         */
/*           30. 9.1998 Symbios SYM53C8xx                                    */
/*           29.11.1998 Intel 4004                                           */
/*            3.12.1998 Intel 8008                                           */
/*           25. 3.1999 National SC14xxx                                     */
/*            4. 7.1999 Fujitsu F2MC                                         */
/*           10. 8.1999 Fairchild ACE                                        */
/*           19.11.1999 Fujitsu F2MC16                                       */
/*           27. 1.2001 Intersil 1802                                        */
/*           2001-07-01 TI 320C54x                                           */
/*                                                                           */
/*****************************************************************************/
/* $Id: headids.c,v 1.9 2008/04/13 20:23:46 alfred Exp $                          */
/*****************************************************************************
 * $Log: headids.c,v $
 * Revision 1.9  2008/04/13 20:23:46  alfred
 * - add Atari Vecor Processor target
 *
 * Revision 1.8  2007/09/16 08:55:54  alfred
 * - preparations for KENBAK
 *
 * Revision 1.7  2006/07/08 10:32:55  alfred
 * - added RS08
 *
 * Revision 1.6  2006/04/06 20:26:54  alfred
 * - add COP4
 *
 * Revision 1.5  2005/12/09 14:48:06  alfred
 * - added 2650
 *
 * Revision 1.4  2005/09/11 18:10:51  alfred
 * - added XGATE
 *
 * Revision 1.3  2005/07/30 13:57:03  alfred
 * - add LatticeMico8
 *
 * Revision 1.2  2005/02/19 14:10:15  alfred
 * - added KCPSM3
 *
 * Revision 1.1  2003/11/06 02:49:24  alfred
 * - recreated
 *
 * Revision 1.6  2003/10/12 13:39:13  alfred
 * - added 78K2
 *
 * Revision 1.5  2003/08/16 16:38:39  alfred
 * - added eZ8
 *
 * Revision 1.4  2003/03/16 18:53:43  alfred
 * - created 807x
 *
 * Revision 1.3  2003/03/09 10:28:28  alfred
 * - added KCPSM
 *
 * Revision 1.2  2003/03/09 09:28:45  alfred
 * - added KPCSM
 *
 *****************************************************************************/

#include "stdinc.h"

#include <string.h>

#include "headids.h"

/*---------------------------------------------------------------------------*/

static TFamilyDescr Descrs[]=
       {
        {"680x0"     , 0x0001, MotoS   },
        {"DSP56000"  , 0x0009, MotoS   },
        {"MPC601"    , 0x0005, MotoS   },
        {"M-CORE"    , 0x0003, MotoS   },
        {"XGATE"     , 0x0004, MotoS   },
        {"68xx"      , 0x0061, MotoS   },
        {"6805/HC08" , 0x0062, MotoS   },
        {"6809"      , 0x0063, MotoS   },
        {"68HC12"    , 0x0066, MotoS   },
        {"68HC16"    , 0x0065, MotoS   },
        {"68RS08"    , 0x005e, MotoS   },
        {"H8/300(H}" , 0x0068, MotoS   },
        {"H8/500"    , 0x0069, MotoS   },
        {"SH7x00"    , 0x006c, MotoS   },
        {"65xx"      , 0x0011, MOSHex  },
        {"MELPS-7700", 0x0019, MOSHex  },
        {"MELPS-4500", 0x0012, IntHex  },
        {"M16"       , 0x0013, IntHex32},
        {"M16C"      , 0x0014, IntHex  },
        {"MCS-48"    , 0x0021, IntHex  },
        {"MCS-(2)51" , 0x0031, IntHex  },
        {"MCS-96/196", 0x0039, IntHex  },
        {"4004/4040" , 0x003f, IntHex  },
        {"8008"      , 0x003e, IntHex  },
        {"8080/8085" , 0x0041, IntHex  },
        {"8086"      , 0x0042, IntHex16},
        {"i960"      , 0x002a, IntHex32},
        {"8X30x"     , 0x003a, IntHex  },
        {"2650"      , 0x0037, MotoS   },
        {"XA"        , 0x003c, IntHex16},
        {"AVR"       , 0x003b, Atmel   },
        {"29xxx"     , 0x0029, IntHex32},
        {"80C166/167", 0x004c, IntHex16},
        {"Zx80"      , 0x0051, IntHex  },
        {"Z8"        , 0x0079, IntHex  },
        {"eZ8"       , 0x0059, IntHex  },
        {"KCPSM"     , 0x006b, IntHex  },
        {"KCPSM3"    , 0x005b, IntHex  },
        {"Mico8"     , 0x005c, IntHex  },
        {"TLCS-900"  , 0x0052, MotoS   },
        {"TLCS-90"   , 0x0053, IntHex  },
        {"TLCS-870"  , 0x0054, IntHex  },
        {"TLCS-47xx" , 0x0055, IntHex  },
        {"TLCS-9000 ", 0x0056, MotoS   },
        {"16C8x"     , 0x0070, IntHex  },
        {"16C5x"     , 0x0071, IntHex  },
        {"17C4x"     , 0x0072, IntHex  },
        {"ST6"       , 0x0078, IntHex  },
        {"ST7"       , 0x0033, IntHex  },
        {"ST9"       , 0x0032, IntHex  },
        {"6804"      , 0x0064, MotoS   },
        {"TMS3201x"  , 0x0074, TiDSK   },
        {"TMS3202x"  , 0x0075, TiDSK   },
        {"TMS320C3x" , 0x0076, IntHex32},
        {"TMS320C5x" , 0x0077, TiDSK   },
        {"TMS320C54x", 0x004b, TiDSK   },
        {"TMS320C6x" , 0x0047, IntHex32},
        {"TMS9900"   , 0x0048, IntHex  },
        {"TMS7000"   , 0x0073, IntHex  },
        {"TMS370xx"  , 0x0049, IntHex  },
        {"MSP430"    , 0x004a, IntHex  },
        {"SC/MP"     , 0x006e, IntHex  },
        {"807x"      , 0x006a, IntHex  },
        {"COP4"      , 0x005f, IntHex  },
        {"COP8"      , 0x006f, IntHex  },
        {"SC14XXX"   , 0x006d, IntHex  },
        {"ACE"       , 0x0067, IntHex  },
        {"78(C)1x"   , 0x007a, IntHex  },
        {"75K0"      , 0x007b, IntHex  },
        {"78K0"      , 0x007c, IntHex  },
        {"78K2"      , 0x0060, IntHex16},
        {"7720"      , 0x007d, IntHex  },
        {"7725"      , 0x007e, IntHex  },
        {"77230"     , 0x007f, IntHex  },
        {"SYM53C8xx" , 0x0025, IntHex  },
        {"F2MC8"     , 0x0015, IntHex  },
        {"F2MC16"    , 0x0016, IntHex  },
        {"1802"      , 0x0038, IntHex  },
        {"KENBAK"    , 0x0027, IntHex  },
        {"ATARI_VECTOR", 0x0002,IntHex },
        {Nil         , 0xffff, Default }
       };

/*---------------------------------------------------------------------------*/

PFamilyDescr FindFamilyByName(char *Name)
{
  PFamilyDescr pRun;

  for (pRun = Descrs; pRun->Name != NULL; pRun++)
    if (!strcmp(Name, pRun->Name))
      return pRun;

  return NULL;
}

PFamilyDescr FindFamilyById(Word Id)
{
  PFamilyDescr pRun;       

  for (pRun = Descrs; pRun->Name != NULL; pRun++)
    if (Id == pRun->Id)
      return pRun;

  return NULL;
}

/*---------------------------------------------------------------------------*/

void headids_init(void)
{
}
