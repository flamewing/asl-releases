
include Makefile.def

CODE_SRCS = codeallg.c codepseudo.c codevars.c \
  	    code68k.c \
            code56k.c \
  	    code601.c \
  	    code68.c code6805.c code6809.c code6812.c code6816.c \
  	    codeh8_3.c codeh8_5.c code7000.c \
  	    code65.c code7700.c code4500.c codem16.c codem16c.c \
            code48.c code51.c code96.c code85.c code86.c \
  	    code8x30x.c codexa.c \
  	    codeavr.c \
  	    code29k.c \
  	    code166.c \
  	    codez80.c codez8.c \
            code96c141.c code90c141.c code87c800.c code47c00.c code97c241.c \
  	    code16c5x.c code16c8x.c code17c4x.c \
  	    codest6.c codest7.c codest9.c code6804.c \
            code3201x.c code3202x.c code3203x.c code3205x.c \
            code9900.c codetms7.c code370.c codemsp.c \
  	    code78c10.c code75k0.c code78k0.c \
            codescmp.c codecop8.c \
            codeflt1750.c

CODE_OBJECTS = $(CODE_SRCS:.c=.o)

ST_SRCS = stringutil.c stdhandl.c stringlists.c

ST_OBJECTS = $(ST_SRCS:.c=.o)

ASM_SRCS = asmdef.c asmsub.c asmpars.c asmmac.c asmcode.c asmdebug.c asmif.c \
           asmfnums.c asminclist.c asmitree.c \
           as.c as1750.c

ASM_OBJECTS = $(ASM_SRCS:.c=.o)

AS_SRCS = endian.c bpemu.c nls.c chunks.c decodecmd.c ioerrors.c
          
AS_OBJECTS = $(AS_SRCS:.c=.o)

PLIST_SRCS = endian.c bpemu.c hex.c nls.c stringutil.c decodecmd.c ioerrors.c \
             toolutils.c \
             plist.c

PLIST_OBJECTS = $(PLIST_SRCS:.c=.o)

PBIND_SRCS = endian.c bpemu.c nls.c stringutil.c stdhandl.c decodecmd.c ioerrors.c \
	     toolutils.c \
	     pbind.c

PBIND_OBJECTS = $(PBIND_SRCS:.c=.o)

P2HEX_SRCS = endian.c bpemu.c hex.c nls.c stringutil.c chunks.c decodecmd.c ioerrors.c \
             toolutils.c \
             p2hex.c

P2HEX_OBJECTS = $(P2HEX_SRCS:.c=.o)

P2BIN_SRCS = endian.c bpemu.c hex.c nls.c stringutil.c chunks.c decodecmd.c ioerrors.c \
             toolutils.c \
             p2bin.c

P2BIN_OBJECTS = $(P2BIN_SRCS:.c=.o)

ARCHFILES = *.c *.h header.tmpl *.lsm *.1 \
            README README.OS2 README.KR Makefile Makefile.dos Makefile.def.tmpl install.sh \
            lang_DE/*.rsc lang_EN/*.rsc \
            doc_DE doc_EN \
            *.asm include/*.inc \
            tests \
	    Makefile.def-samples

DISTARCHFILES = *.c *.h asl-$(VERSION).lsm *.1 \
            README README.OS2 Makefile Makefile.dos Makefile.def.tmpl install.sh \
            lang_DE/*.rsc lang_EN/*.rsc \
            doc_DE doc_EN \
            include/*.inc \
            tests \
            Makefile.def-samples

ALLOBJECTS = $(CODE_OBJECTS) $(AS_OBJECTS) $(PLIST_OBJECTS) $(P2HEX_OBJECTS) $(P2BIN_OBJECTS)

ALLASMSRCS = $(ALLOBJECTS:.o=.s)

# set EXEXTENSION im Makefile.def if you need a specific extension for 
# the executables (e.g. .exe for OS/2)

ASLTARGET = asl$(EXEXTENSION)
PLISTTARGET = plist$(EXEXTENSION)
PBINDTARGET = pbind$(EXEXTENSION)
P2HEXTARGET = p2hex$(EXEXTENSION)
P2BINTARGET = p2bin$(EXEXTENSION)

ALLTARGETS = $(ASLTARGET) $(PLISTTARGET) $(PBINDTARGET) $(P2HEXTARGET) $(P2BINTARGET)

#---------------------------------------------------------------------------


all: $(ALLTARGETS)

docs:
	cd doc_DE; $(MAKE)
	cd doc_EN; $(MAKE)

test: $(ALLTARGETS)
	cd tests; ./testall

install: $(ALLTARGETS)
	./install.sh $(BINDIR) $(INCDIR) $(MANDIR)

clean:
	rm -f $(ALLTARGETS) *.o *.p *~ DEADJOE `find . -name "*.lst" -print` tests/testlog

$(ASLTARGET): $(AS_OBJECTS) $(ASM_OBJECTS) $(ST_OBJECTS) $(CODE_OBJECTS)
	$(LD) -o $(ASLTARGET) $(ASM_OBJECTS) $(AS_OBJECTS) $(ST_OBJECTS) $(CODE_OBJECTS) -lm $(LDFLAGS)

$(PLISTTARGET): $(PLIST_OBJECTS)
	$(LD) -o $(PLISTTARGET) $(PLIST_OBJECTS) -lm $(LDFLAGS)

$(PBINDTARGET): $(PBIND_OBJECTS)
	$(LD) -o $(PBINDTARGET) $(PBIND_OBJECTS) -lm $(LDFLAGS)

$(P2HEXTARGET): $(P2HEX_OBJECTS)
	$(LD) -o $(P2HEXTARGET) $(P2HEX_OBJECTS) -lm $(LDFLAGS)

$(P2BINTARGET): $(P2BIN_OBJECTS)
	$(LD) -o $(P2BINTARGET) $(P2BIN_OBJECTS) -lm $(LDFLAGS)

#asmsrc: $(ALLASMSRCS)

#---------------------------------------------------------------------------


tape: unjunk
	tar cvf /dev/ntape $(ARCHFILES)

disk: archive
	mcopy -nvm asport.tar.gz a:ASPORT.TGZ

archive: asport.tar.gz

distrib: unjunk
	mkdir ../asl-$(VERSION)
	tar cf - $(DISTARCHFILES) | (cd ../asl-$(VERSION); tar xvf -)
	cd ..; tar cvf asport/asl-$(VERSION).tar asl-$(VERSION)
	rm -rf ../asl-$(VERSION)
	gzip -9 -f asl-$(VERSION).tar


asport.tar.gz: $(ARCHFILES) unjunk
	tar cvf asport.tar $(ARCHFILES)
	gzip -9 -f asport.tar

unjunk:
	rm -f n.c asmpars.cas.c include/fileform* config.h test.h loc.c gennop.c \
           nops.asm bind.* asmutils.* filenums.* includelist.* tests/warnlog_* \
           insttree.* flt1750.* \
	   `find . -name "testlog" -print` \
	   `find . -name "*~" -print` \
	   `find . -name "*.lst" -print`
	cd doc_DE; $(MAKE) clean
	cd doc_EN; $(MAKE) clean

#---------------------------------------------------------------------------


.SUFFIXES: .c
.c.o: 
	$(CC) $(CFLAGS) -D$(CHARSET) -I$(LANGRSC) -DSTDINCLUDES=\"${INCDIR}\" -c -o $*.o $*.c
#.c.s:
#	$(CC) $(CFLAGS) -D$(CHARSET) -I$(LANGRSC) -DSTDINCLUDES=\"${INCDIR}\" -S $*.c
#	mv u.out.s $*.s
