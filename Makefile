include Makefile.def

AS_SRCS = endian.c bpemu.c nls.c stringutil.c stdhandl.c stringlists.c insttree.c chunks.c decodecmd.c ioerrors.c \
          asmdef.c asmsub.c asmpars.c asmmac.c asmcode.c \
          filenums.c includelist.c asmdebug.c \
          asmif.c codeallg.c codepseudo.c \
	  code68k.c \
          code56k.c \
	  code601.c \
	  code68.c code6805.c code6809.c code6812.c code6816.c \
	  codeh8_3.c codeh8_5.c code7000.c \
	  code65.c code7700.c code4500.c codem16.c codem16c.c \
          code48.c code51.c code96.c code85.c code86.c \
	  codexa.c \
	  codeavr.c \
	  code29k.c \
	  code166.c \
	  codez80.c codez8.c \
          code96c141.c code90c141.c code87c800.c code47c00.c code97c241.c \
	  code16c5x.c code16c8x.c code17c4x.c \
	  codest6.c \
	  code6804.c \
          code3201x.c code3202x.c code3203x.c code3205x.c code370.c codemsp.c \
	  code78c10.c code75k0.c code78k0.c \
          codecop8.c \
          as.c

AS_OBJECTS = $(AS_SRCS:.c=.o)

PLIST_SRCS = endian.c bpemu.c hex.c nls.c stringutil.c decodecmd.c ioerrors.c \
             asmutils.c \
             plist.c

PLIST_OBJECTS = $(PLIST_SRCS:.c=.o)

BIND_SRCS = endian.c bpemu.c nls.c stringutil.c stdhandl.c decodecmd.c ioerrors.c \
	    asmutils.c \
	    bind.c

BIND_OBJECTS = $(BIND_SRCS:.c=.o)

P2HEX_SRCS = endian.c bpemu.c hex.c nls.c stringutil.c chunks.c decodecmd.c ioerrors.c \
             asmutils.c \
             p2hex.c

P2HEX_OBJECTS = $(P2HEX_SRCS:.c=.o)

P2BIN_SRCS = endian.c bpemu.c hex.c nls.c stringutil.c chunks.c decodecmd.c ioerrors.c \
             asmutils.c \
             p2bin.c

P2BIN_OBJECTS = $(P2BIN_SRCS:.c=.o)

ARCHFILES = *.c *.h header.tmpl \
            README Makefile Makefile.def.tmpl install.sh \
            lang_DE/*.rsc lang_EN/*.rsc \
            doc_DE doc_EN \
            *.asm include/*.inc \
            tests \
	    Makefile.def-samples

DISTARCHFILES = *.c *.h \
            README Makefile Makefile.def.tmpl install.sh \
            lang_DE/*.rsc lang_EN/*.rsc \
            doc_DE doc_EN \
            include/*.inc \
            tests \
            Makefile.def-samples

ALLOBJECTS = $(AS_OBJECTS) $(PLIST_OBJECTS) $(P2HEX_OBJECTS) $(P2BIN_OBJECTS)

ALLASMSRCS = $(ALLOBJECTS:.o=.s)

ALLTARGETS = asl plist bind p2hex p2bin


#---------------------------------------------------------------------------


all: $(ALLTARGETS)

test: $(ALLTARGETS)
	cd tests; ./testall

install: $(ALLTARGETS)
	./install.sh $(BINDIR) $(INCDIR)

clean:
	rm -f $(ALLTARGETS) *.o *.p *~ DEADJOE `find . -name "*.lst" -print` tests/testlog

asl: $(AS_OBJECTS)
	$(LD) -o asl $(AS_OBJECTS) -lm $(LDFLAGS)

plist: $(PLIST_OBJECTS)
	$(LD) -o plist $(PLIST_OBJECTS) -lm $(LDFLAGS)

bind: $(BIND_OBJECTS)
	$(LD) -o bind $(BIND_OBJECTS) -lm $(LDFLAGS)

p2hex: $(P2HEX_OBJECTS)
	$(LD) -o p2hex $(P2HEX_OBJECTS) -lm $(LDFLAGS)

p2bin: $(P2BIN_OBJECTS)
	$(LD) -o p2bin $(P2BIN_OBJECTS) -lm $(LDFLAGS)

#asmsrc: $(ALLASMSRCS)

#---------------------------------------------------------------------------


tape:
	tar cvf /dev/ntape $(ARCHFILES)

disk: archive
	mcopy -nvm asport.tar.gz a:ASPORT.TGZ

archive: asport.tar.gz

distrib:
	mkdir ../asl-$(VERSION)
	tar cf - $(DISTARCHFILES) | (cd ../asl-$(VERSION); tar xvf -)
	cd ..; tar cvf asport/asl-$(VERSION).tar asl-$(VERSION)
	rm -rf ../asl-$(VERSION)
	gzip -9 -f asl-$(VERSION).tar


asport.tar.gz: $(ARCHFILES)
	tar cvf asport.tar $(ARCHFILES)
	gzip -9 -f asport.tar

unjunk:
	rm -f config.h test.h loc.c gennop.c nops.asm `find . -name "testlog" -print` `find . -name "*~" -print`

#---------------------------------------------------------------------------


.SUFFIXES: .c
.c.o: 
	$(CC) $(CFLAGS) -D$(CHARSET) -I$(LANGRSC) -DSTDINCLUDES=\"${INCDIR}\" -c -o $*.o $*.c
#.c.s:
#	$(CC) $(CFLAGS) -D$(CHARSET) -I$(LANGRSC) -DSTDINCLUDES=\"${INCDIR}\" -S $*.c
#	mv u.out.s $*.s
