# -------------------------------------------------------------------------
# choose your compiler (must be ANSI-compliant!) and linker command, plus
# any additionally needed flags

CC = gcc
LD = gcc
CFLAGS = -g -O3 -march=i586 -fomit-frame-pointer -Wall
LDFLAGS =
#                    ^^^^^
#                    |||||
# adapt this to your target cpu (386/486 or higher)
# note that older gcc versions require -m[34]86 or -mcpu=i[34]86 instead of -march=i[34]86.
# @GNU: why does this have to change every two years ?!

TARG_OBJEXTENSION = .o

HOST_OBJEXTENSION = $(TARG_OBJEXTENSION)

# -------------------------------------------------------------------------
# directories where binaries, includes, and manpages should go during
# installation

BINDIR = /usr/local/bin
INCDIR = /usr/local/include/asl
MANDIR = /usr/local/man
LIBDIR = /usr/local/lib/asl
DOCDIR = /usr/local/doc/asl

# -------------------------------------------------------------------------
# character encoding to use (choose one of them)

CHARSET = CHARSET_ISO8859_1
# CHARSET = CHARSET_ASCII7
# CHARSET = CHARSET_IBM437
