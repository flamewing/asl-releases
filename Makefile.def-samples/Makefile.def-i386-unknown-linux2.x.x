# -------------------------------------------------------------------------
# choose your compiler (must be ANSI-compliant!) and linker command, plus
# any additionally needed flags

CC = gcc
LD = gcc
CFLAGS = -O3 -m486 -fomit-frame-pointer -g -Wall
LDFLAGS =
#            ^^^^^
#            |||||
# adapt this to your target cpu (386/486 or higher)
# -------------------------------------------------------------------------
# directories where binaries, includes, and manpages should go during
# installation

BINDIR = /usr/local/bin
INCDIR = /usr/local/include/asl
MANDIR = /usr/local/man

# -------------------------------------------------------------------------
# language AS will speak to you

LANGRSC = lang_EN

# -------------------------------------------------------------------------
# character encoding to use (choose one of them)

CHARSET = CHARSET_ISO8859_1
# CHARSET = CHARSET_ASCII7
# CHARSET = CHARSET_IBM437
