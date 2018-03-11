/* config.h.  Generated from config.h.in by configure.  */
#ifndef _CONFIG_H
#define _CONFIG_H

#define HAVE_SYS_TYPES_H 1
#define HAVE_STDINT_H 1

/* #undef int8_t */
/* #undef int16_t */
/* #undef int32_t */
/* #undef int64_t */
/* #undef uint8_t */
/* #undef uint16_t */
/* #undef uint32_t */
/* #undef uint64_t */

#define HAVE_LONGLONG 1
#define HAVE_UINT16 1
#define HAVE_UINT64 1

#define HAVE_MEMMOVE 1

#define HAVE_FOPEN_B 1

#define HAVE_STRING_STRCASECMP 1
/* #undef HAVE_STRINGS_STRCASECMP */

#if (!defined HAVE_STRING_STRCASECMP) && (!defined HAVE_STRINGS_STRCASECMP)
# define NEEDS_CASECMP
#endif

#define HOST_FLOAT_FORMAT IEEE_FLOAT_FORMAT

#define HAVE_LOCALE_LIB 1

/* #undef HAVE_BROKEN_SPRINTF */

#define ARCHPRNAME "x86_64"
#define ARCHSYSNAME "pc-linux-gnu"

#define PATHSEP '/'
#define DIRSEP ':'
/* #undef DRSEP */
#define NULLDEV "/dev/null"
/* #undef SLASHARGS */

/* #undef CKMALLOC */
/* #undef HEAPRESERVE */

#endif /* _CONFIG_H */
