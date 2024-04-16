#ifndef _SYSDEFS_H
#define _SYSDEFS_H
/* sysdefs.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS Port                                                                   */
/*                                                                           */
/* system-specific definitions                                               */
/*                                                                           */
/* History:  2001-04-13 activated DRSEP for Win32 platform                   */
/*           2001-09-11 added MacOSX                                         */
/*           2001-10-13 added ARM/Linux                                      */
/*                                                                           */
/*****************************************************************************/

/* NOTE:
 *
 * when adding new platforms, " gcc -dM -E - <<<'' " might be helpful to
 * find out about predefined symbols
 *
 */

/*---------------------------------------------------------------------------*/
/* unify 68K platforms */

#ifdef __mc68020
#ifndef __m68k
#define __m68k
#endif
#endif

#ifdef m68000
#ifndef __m68k
#define __m68k
#endif
#endif

#ifdef __mc68000
# ifndef __m68k
#  define __m68k
# endif
#endif

/*---------------------------------------------------------------------------*/
/* ditto for i386 platforms */

/* MSDOS only runs on x86s... */

#ifdef __MSDOS__
#define __i386
#endif

/* For IBMC... */

#ifdef _M_I386
#define __i386
#endif

#ifdef __i386__
#ifndef __i386
#define __i386
#endif
#endif

/*---------------------------------------------------------------------------*/
/* ditto for VAX platforms */

#ifdef vax
#define __vax__
#endif

/*---------------------------------------------------------------------------*/
/* ditto for PPC platforms */

#ifdef __PPC
#ifndef _POWER
#define _POWER
#endif
#endif

#ifdef __ppc__
#ifndef _POWER
#define _POWER
#endif
#endif

#ifdef __PPC__
# ifndef _POWER
#  define _POWER
# endif
#endif

#ifdef __PPC64__
# ifndef _POWER4
#  define _POWER4
# endif
#endif

/*---------------------------------------------------------------------------*/
/* ditto for ARM platforms */

#ifdef __arm__
#ifndef __arm
#define __arm
#endif
#endif

/*---------------------------------------------------------------------------*/
/* If the compiler claims to be ANSI, we surely can use prototypes */

#ifdef __STDC__
#define __PROTOS__
#define UNUSED(x) (void)x
#else
#define UNUSED(x) {}
#endif

/*---------------------------------------------------------------------------*/
/* just a hack to allow distinguishing SunOS from Solaris on Sparcs... */

#ifdef sparc
#ifndef __sparc
#define __sparc
#endif
#endif

#ifdef __sparc
# ifndef __NetBSD__
#  ifndef __FreeBSD__
#   ifndef __linux__
#    ifndef __SVR4
#     define __sunos__
#    else /* __SVR4 */
#     define __solaris__
#    endif /* __SVR4 */
#   endif /* __linux__ */
#  endif /* __FreeBSD__ */
# endif /* __NetBSD */
#endif /* __sparc */

#ifdef __sparc__
#ifndef __sparc
#define __sparc
#endif
#endif

/*---------------------------------------------------------------------------*/
/* similar on Sun 3's... */

#ifdef __m68k
#ifndef __NetBSD__
#ifndef __MUNIX__
#ifndef __amiga
#define __sunos__
#endif
#endif
#endif
#endif

/*===========================================================================*/
/* 68K platforms */

#ifdef __m68k

/*---------------------------------------------------------------------------*/
/* SUN/3 with SunOS 4.x:

   see my SunOS quarrels in the Sparc section... */

#ifdef __sunos__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
#ifdef __GNUC__
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#else
#define NOLONGLONG
#endif
#define memmove(s1,s2,len) bcopy(s2,s1,len)
extern void bcopy();
#endif

/*---------------------------------------------------------------------------*/
/* SUN/3 with NetBSD 1.x:

   quite a normal 32-Bit-UNIX system */

#ifdef __NetBSD__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif

/*---------------------------------------------------------------------------*/
/* PCS/Cadmus:

   quite a bare system, lots of work required... */

#ifdef __MUNIX__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
#define NEEDS_STRSTR
typedef char Integ8;
typedef unsigned char Card8;
typedef short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
#define NOLONGLONG
#define memmove(s1,s2,len) bcopy(s2,s1,len)
extern double strtod();
extern char *getenv();
#endif

/*---------------------------------------------------------------------------*/
/* Linux/68K:

   quite a normal 32-Bit-UNIX system */

#ifdef __linux__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif

#endif /* __m68k */

/*===========================================================================*/
/* SPARC platforms */

#ifdef __sparc

/*---------------------------------------------------------------------------*/
/* SUN Sparc with SunOS 4.1.x:

   don't try cc, use gcc, it's hopeless without an ANSI-compliant compiler...
   SunOS does have NLS support, but it does not have D_FMT and T_FMT
   I should change this ...
   Though the manual pages claim that memmove and atexit exist, I could not
   find them in any library :-(  Fortunately, bcopy claims to be safe for
   overlapping arrays, we just have to reverse source and destination pointers.
   The sources themselves contain a switch to use on_exit instead of atexit
   (it uses a different callback scheme, so we cannot just make a #define here...)
   To get rid of most of the messages about missing prototypes, add
   -D__USE_FIXED_PROTOTYPES__ to your compiler flags!
   Apart from these few points, one could claim SunOS to be quite a normal
   32-bit-UNIX... */

#ifdef __sunos__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
#ifdef __GNUC__
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#else
#define NOLONGLONG
#endif
#define fpos_t long
#ifdef __STDC__
extern void bcopy();
#endif
#define memmove(s1,s2,len) bcopy(s2,s1,len)
#endif

/*---------------------------------------------------------------------------*/
/* SUN Sparc with Solaris 2.x:

   quite a normal 32-Bit-UNIX system */

#ifdef __solaris__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif

/*---------------------------------------------------------------------------*/
/* Sparc with NetBSD 1.x:

   quite a normal 32-Bit-UNIX system */

#ifdef __NetBSD__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif

/*---------------------------------------------------------------------------*/
/* Sparc with Linux                                                          */

#ifdef __linux__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif

#endif /* __sparc */

/*===========================================================================*/
/* Mips platforms */

#ifdef __mips

/*---------------------------------------------------------------------------*/
/* R3000 with Ultrix 4.3:

   nl_langinfo prototype is there, but no function in library ?!
   use long long only if you have gcc, c89 doesn't like them !
   cc isn't worth trying, believe me! */

#ifdef __ultrix
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
#define NEEDS_STRDUP
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
#ifdef __GNUC__
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#else
#define NOLONGLONG
#endif
#endif

/*---------------------------------------------------------------------------*/
/* R2000/3000 with NetBSD 1.2:

   quite a normal 32-Bit-UNIX system */

#ifdef __NetBSD__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif

/*---------------------------------------------------------------------------*/
/* R3000/4x00 with Irix 5.x:

  quite a normal 32-Bit-UNIX system
  seems also to work with 6.2... */

#ifdef __sgi
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif

#ifdef __linux__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif

#endif /* __mips */

/*===========================================================================*/
/* HP-PA platforms */

#ifdef __hppa

/*---------------------------------------------------------------------------*/
/* HP-PA 1.x with HP-UX: */

#ifdef __hpux
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif

#endif /* __hppa */

/*===========================================================================*/
/* POWER 64 bit platforms */

#ifdef _POWER64

/*---------------------------------------------------------------------------*/
/* POWER64 with Linux (Macintosh) */

#ifdef __linux__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long Integ64;
typedef unsigned long Card64;
#define HAS64
#endif

/*===========================================================================*/
/* POWER(32) platforms */

#elif defined _POWER

/*---------------------------------------------------------------------------*/
/* POWER with AIX 4.1: rs6000 */

#ifdef _AIX
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif

/*---------------------------------------------------------------------------*/
/* POWER with Linux (Macintosh) */

#ifdef __linux__

/* no long long data type if C89 is used */

#if (defined __STDC__) && (!defined __STDC_VERSION__)
# define NOLONGLONG
#endif

#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
#ifndef NOLONGLONG
typedef signed long long Integ64;
typedef unsigned long long Card64;
# define HAS64
#endif /* !NOLONGLONG */
#endif

/*---------------------------------------------------------------------------*/
/* POWER with OSX (Macintosh) */

#ifdef __APPLE__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif

#endif /* _POWER */

/*===========================================================================*/
/* VAX platforms */

#ifdef __vax__

/*---------------------------------------------------------------------------*/
/* VAX with Ultrix: */

#ifdef ultrix
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define VAXFLOAT
#define NEEDS_STRDUP
#define BKOKEN_SPRINTF
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
#define NOLONGLONG
#endif

/*---------------------------------------------------------------------------*/
/* VAX with NetBSD 1.x:

   quite a normal 32-Bit-UNIX system - apart from the float format... */

#ifdef __NetBSD__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define VAXFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif

#endif /* vax */

#ifdef __aarch64__

/*---------------------------------------------------------------------------*/
/* AArch64 with Linux and GCC: */

#ifdef __linux__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long Integ64;
typedef unsigned long Card64;
#define HAS64
#endif

#endif /* __aarch64__ */

/*===========================================================================*/
/* DEC Alpha platforms */

#ifdef __alpha

/*---------------------------------------------------------------------------*/
/* DEC Alpha with Digital UNIX and DEC C / GCC:

   Alpha is a 64 bit machine, so we do not need to use extra longs
   OSF has full NLS support */

#ifdef __osf__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long Integ64;
typedef unsigned long Card64;
#define HAS64
#endif

/*---------------------------------------------------------------------------*/
/* DEC Alpha with Linux and GCC:

   see OSF... */

#ifdef __linux__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long Integ64;
typedef unsigned long Card64;
#define HAS64
#endif

/*---------------------------------------------------------------------------*/
/* DEC Alpha with NetBSD and GCC:

   see OSF... */

#ifdef __NetBSD__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long Integ64;
typedef unsigned long Card64;
#define HAS64
#endif

#ifdef __FreeBSD__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long Integ64;
typedef unsigned long Card64;
#define HAS64
#endif

#endif /* __alpha */

/*===========================================================================*/
/* Intel i386 platforms */

#ifdef __i386

/*---------------------------------------------------------------------------*/
/* Intel i386 with NetBSD 1. and GCC: (tested on 1.5.3)

   principally, a normal 32-bit UNIX */

#ifdef __NetBSD__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif

/*---------------------------------------------------------------------------*/
/* Intel i386 with Linux and GCC:

   principally, a normal 32-bit *NIX */

#ifdef __linux__

/* no long long data type if C89 is used */

#if (defined __STDC__) && (!defined __STDC_VERSION__)
# define NOLONGLONG
#endif

#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
#ifndef NOLONGLONG
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif /* !NOLONGLONG */
#endif

/*---------------------------------------------------------------------------*/
/* Intel i386 with FreeBSD and GCC:

   principally, a normal 32-bit *NIX */

#ifdef __FreeBSD__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif

/*---------------------------------------------------------------------------*/
/* Intel i386 with Darwin and GCC:
   principally, a normal 32-bit *NIX */

#ifdef __APPLE__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif

/*---------------------------------------------------------------------------*/
/* Intel i386 with WIN32 and Cygnus GCC:

   well, not really a UNIX... */

#ifdef _WIN32

/* no long long data type if C89 is used */

#if (defined __STDC__) && (!defined __STDC_VERSION__)
# define NOLONGLONG
#endif

#define DEFSMADE
#define OPENRDMODE "rb"
#define OPENWRMODE "wb"
#define OPENUPMODE "rb+"
#define IEEEFLOAT
#define SLASHARGS
#define PATHSEP '\\'
#define SPATHSEP "\\"
#define DIRSEP ';'
#define SDIRSEP ";"
#define DRSEP ':'
#define SDRSEP ":"
#define NULLDEV "NUL"
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
#ifndef NOLONGLONG
typedef signed long long Integ64;
typedef unsigned long long Card64;
# define HAS64
#endif
#define W32_NLS
#endif

/*---------------------------------------------------------------------------*/
/* Intel i386 with OS/2 and emx-GCC:

   well, not really a UNIX... */

#ifdef __EMX__
#define DEFSMADE
#define OPENRDMODE "rb"
#define OPENWRMODE "wb"
#define OPENUPMODE "rb+"
#define IEEEFLOAT
#define SLASHARGS
#define PATHSEP '\\'
#define SPATHSEP "\\"
#define DIRSEP ';'
#define SDIRSEP ";"
#define DRSEP ':'
#define SDRSEP ":"
#define NULLDEV "NUL"
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#define OS2_NLS
#endif

/*---------------------------------------------------------------------------*/
/* Intel i386 with OS/2 and IBMC:

well, not really a UNIX... */

#ifdef __IBMC__
#define DEFSMADE
#define NODUP
#define OPENRDMODE "rb"
#define OPENWRMODE "wb"
#define OPENUPMODE "rb+"
#define IEEEFLOAT
#define SLASHARGS
#define PATHSEP '\\'
#define SPATHSEP "\\"
#define DRSEP ':'
#define SDRSEP ":"
#define NULLDEV "NUL"
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
#define NOLONGLONG
#define OS2_NLS
#endif

/*---------------------------------------------------------------------------*/
/* Intel x86 with MS-DOS and Borland-C:

   well, not really a UNIX...
   assure we get a usable memory model */

#ifdef __MSDOS__
#ifdef __TURBOC__
#ifndef __LARGE__
#error Wrong memory model - use large!
#endif
#define CKMALLOC
#define HEAPRESERVE 4096
#define DEFSMADE
#define OPENRDMODE "rb"
#define OPENWRMODE "wb"
#define OPENUPMODE "rb+"
#define IEEEFLOAT
#define SLASHARGS
#define PATHSEP '\\'
#define SPATHSEP "\\"
#define DIRSEP ';'
#define SDIRSEP ";"
#define DRSEP ':'
#define SDRSEP ":"
#define NULLDEV "NUL"
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed long Integ32;
#define PRIInteg32 "ld"
typedef unsigned long Card32;
#define NOLONGLONG
#define __PROTOS__
#undef UNUSED
#define UNUSED(x) (void)x
#endif
#endif

#endif /* __i386 */


/*===========================================================================*/
/* Intel x86_64 platforms */

#if  (defined __k8__) || (defined __x86_64) || (defined __x86_64__) || (defined _M_AMD64)

/*---------------------------------------------------------------------------*/
/* x86-64/amd64 with Linux/FreeBSD, OSX and GCC:

   Principally, a normal *NIX. */

#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)

/* no long long data type if C89 is used */

#if (defined __STDC__) && (!defined __STDC_VERSION__)
# define NOLONGLONG
#endif

#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long Integ64;
typedef unsigned long Card64;
#define HAS64

#endif /* __linux__ || __FreeBSD__ || __APPLE__ */

/*---------------------------------------------------------------------------*/
/* Intel i386 with WIN32 and MinGW:

   Well, not really a UNIX...note that in contrast to Unix-like systems,
   the size of 'long' remains 32 bit.  One still has to use 'long long' to
   get 64 bits. */

#ifdef _WIN32

/* no long long data type if C89 is used */

#if (defined __STDC__) && (!defined __STDC_VERSION__)
# define NOLONGLONG
#endif

#define DEFSMADE
#define OPENRDMODE "rb"
#define OPENWRMODE "wb"
#define OPENUPMODE "rb+"
#define IEEEFLOAT
#define SLASHARGS
#define PATHSEP '\\'
#define SPATHSEP "\\"
#define DIRSEP ';'
#define SDIRSEP ";"
#define DRSEP ':'
#define SDRSEP ":"
#define NULLDEV "NUL"
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
#ifndef NOLONGLONG
typedef signed long long Integ64;
typedef unsigned long long Card64;
# define HAS64
#endif

#endif /* _WIN32 */

#endif /* __k8__ || __x86_64 || __x86_64__ */

/*===========================================================================*/
/* ARM platform */

#ifdef __arm

/*---------------------------------------------------------------------------*/
/* ARM linux with GCC */

#ifdef __linux__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif /* __linux__ */

/*---------------------------------------------------------------------------*/
/* Psion PDA (ARM cpu) with SymbianOS Epoc32 rel.5 and installed epocemx-GCC:

   well, not really a UNIX... */

#ifdef __EPOC32__

#ifdef __EPOCEMX__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif /* __EPOCEMX__ */


#endif /* __EPOC32__ */

#endif /* __arm */

/*===========================================================================*/
/* Misc... */

/*---------------------------------------------------------------------------*/
/* Emscripten */

#ifdef __wasm__
#define DEFSMADE
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
#define PRIInteg32 "d"
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif

/*---------------------------------------------------------------------------*/
/* Just for curiosity, it won't work without 16 bit int's... */

#ifdef _CRAYMPP
#define OPENRDMODE "r"
#define OPENWRMODE "w"
#define OPENUPMODE "r+"
#define IEEEFLOAT
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ32;
#define PRIInteg32 "d"
typedef unsigned short Card32;
typedef signed int Integ64;
typedef unsigned int Card64;
#define HAS64
#endif

/*===========================================================================*/
/* Post-Processing: check for definition, add defaults */


#ifdef DEFSMADE
#ifndef PATHSEP
#define PATHSEP '/'
#define SPATHSEP "/"
#define DIRSEP ':'
#define SDIRSEP ":"
#endif
#ifndef NULLDEV
#define NULLDEV "/dev/null"
#endif
#else
#error "your platform so far is not included in AS's header files!"
#error "please edit sysdefs.h!"
#endif

#ifdef CKMALLOC
#define malloc(s) ckmalloc(s)
#define realloc(p,s) ckrealloc(p,s)

extern void *ckmalloc(size_t s);

extern void *ckrealloc(void *p, size_t s);
#endif
#endif /* _SYSDEFS_H */
