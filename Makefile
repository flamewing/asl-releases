include Makefile.def

CURRDIR=./
TAPE=/dev/ntape
DATE=`date +"%d%m%Y"`

include makedefs.src

include objdefs.unix

include makedefs.files

ALLFLAGS = $(CFLAGS) -D$(CHARSET) -DSTDINCLUDES=\"$(INCDIR)\" -DLIBDIR=\"$(LIBDIR)\"

#---------------------------------------------------------------------------
# primary targets

all: $(ALLTARGETS)

docs: $(TEX2DOCTARGET) $(TEX2HTMLTARGET)
	cd doc_DE; $(MAKE) TEX2DOC=../$(TEX2DOCTARGET) TEX2HTML=../$(TEX2HTMLTARGET) RM="rm -f"
	cd doc_EN; $(MAKE) TEX2DOC=../$(TEX2DOCTARGET) TEX2HTML=../$(TEX2HTMLTARGET) RM="rm -f"

$(ASLTARGET): $(AS_OBJECTS) $(ASM_OBJECTS) $(ST_OBJECTS) $(CODE_OBJECTS) $(NLS_OBJECTS)
	$(LD) -o $(ASLTARGET) $(ASM_OBJECTS) $(AS_OBJECTS) $(ST_OBJECTS) $(CODE_OBJECTS) $(NLS_OBJECTS) -lm $(LDFLAGS)

$(PLISTTARGET): $(PLIST_OBJECTS) $(NLS_OBJECTS)
	$(LD) -o $(PLISTTARGET) $(PLIST_OBJECTS) $(NLS_OBJECTS) -lm $(LDFLAGS)

$(ALINKTARGET): $(ALINK_OBJECTS) $(NLS_OBJECTS)
	$(LD) -o $(ALINKTARGET) $(ALINK_OBJECTS) $(NLS_OBJECTS) -lm $(LDFLAGS)

$(PBINDTARGET): $(PBIND_OBJECTS) $(NLS_OBJECTS)
	$(LD) -o $(PBINDTARGET) $(PBIND_OBJECTS) $(NLS_OBJECTS) -lm $(LDFLAGS)

$(P2HEXTARGET): $(P2HEX_OBJECTS) $(NLS_OBJECTS)
	$(LD) -o $(P2HEXTARGET) $(P2HEX_OBJECTS) $(NLS_OBJECTS) -lm $(LDFLAGS)

$(P2BINTARGET): $(P2BIN_OBJECTS) $(NLS_OBJECTS)
	$(LD) -o $(P2BINTARGET) $(P2BIN_OBJECTS) $(NLS_OBJECTS) -lm $(LDFLAGS)

$(RESCOMPTARGET): $(RESCOMP_OBJECTS)
	$(LD) -o $(RESCOMPTARGET) $(RESCOMP_OBJECTS) $(LDFLAGS)

$(TEX2DOCTARGET): $(TEX2DOC_OBJECTS)
	$(LD) -o $(TEX2DOCTARGET) $(TEX2DOC_OBJECTS) $(LDFLAGS) -lm

$(TEX2HTMLTARGET): $(TEX2HTML_OBJECTS)
	$(LD) -o $(TEX2HTMLTARGET) $(TEX2HTML_OBJECTS) $(LDFLAGS) -lm

$(UNUMLAUTTARGET): $(UNUMLAUT_OBJECTS)
	$(LD) -o $(UNUMLAUTTARGET) $(UNUMLAUT_OBJECTS) $(LDFLAGS)

#---------------------------------------------------------------------------
# special rules for objects dependant on string resource files

include makedefs.str

#---------------------------------------------------------------------------
# supplementary targets

test: $(ALLTARGETS)
	cd tests; ./testall

install: $(ALLTARGETS)
	./install.sh $(BINDIR) $(INCDIR) $(MANDIR) $(LIBDIR) $(DOCDIR)

clean:
	rm -f $(ALLTARGETS) $(RESCOMPTARGET) $(TEX2DOCTARGET) $(TEX2HTMLTARGET) *.$(OBJEXTENSION) *.p *.rsc tests/testlog
	cd doc_DE; $(MAKE) RM="rm -f" clean
	cd doc_EN; $(MAKE) RM="rm -f" clean

#---------------------------------------------------------------------------
# create distributions

distrib: unjunk
	mkdir ../asl-$(VERSION)
	tar cf - $(DISTARCHFILES) | (cd ../asl-$(VERSION); tar xvf -)
	cd ..; tar cvf asl-$(VERSION).tar asl-$(VERSION)
	mv ../asl-$(VERSION).tar ./
	rm -rf ../asl-$(VERSION)
	gzip -9 -f asl-$(VERSION).tar

bindist:
	mkdir asl-$(VERSION)
	chmod 755 asl-$(VERSION)
	./install.sh asl-$(VERSION)/bin asl-$(VERSION)/include asl-$(VERSION)/man asl-$(VERSION)/lib asl-$(VERSION)/doc
	tar cvf asl-$(VERSION)-bin.tar asl-$(VERSION)
	rm -rf asl-$(VERSION)
	gzip -9 -f asl-$(VERSION)-bin.tar 

#---------------------------------------------------------------------------
# for my own use only...

tape: unjunk
	tar cvf $(TAPE) $(ARCHFILES)

disk: unjunk archive
	mcopy -nvm asport.tar.gz a:ASPORT.TGZ

disks: unjunk archives
	echo Insert disk 1...
	read tmp
	mcopy -nvm asport1.tar.gz a:ASPORT1.TGZ
	echo Insert disk 2...
	read tmp
	mcopy -nvm asport2.tar.gz a:ASPORT2.TGZ

archive: unjunk asport.tar.gz

barchive: unjunk asport.tar.bz2

archives: unjunk asport1.tar.gz asport2.tar.gz

asport.tar.gz: $(ARCHFILES)
	tar cvf asport.tar $(ARCHFILES)
	gzip -9 -f asport.tar

asport.tar.bz2: $(ARCHFILES)
	tar cvf asport.tar $(ARCHFILES)
	bzip2 asport.tar

asport1.tar.gz: $(ARCH1FILES)
	tar cvf asport1.tar $(ARCH1FILES)
	gzip -9 -f asport1.tar

asport2.tar.gz: $(ARCH2FILES)
	tar cvf asport2.tar $(ARCH2FILES)
	gzip -9 -f asport2.tar

snap: unjunk
	-mount /mo
	-mkdir -p /mo/public/asport/snap_$(DATE)
	cp -av $(ARCHFILES) /mo/public/asport/snap_$(DATE)
	umount /mo

unjunk:
	rm -f tmp.* n.c include/stddef56.inc asmpars.cas.c include/fileform* config.h test.h loc.c gennop.c \
           nops.asm bind.* asmutils.* asmmessages.* filenums.* includelist.* tests/warnlog_* \
           insttree.* flt1750.* t_65.* test87c8.* testst9.* testst7.* testtms7.* test3203.* \
           ioerrors.new.c codeallg.* ASM*.c *_msg*.h p2BIN.* \
           decodecmd.* ioerrors.* stringutil.* *split.c marks.c \
	   `find . -name "testlog" -print` \
	   `find . -name "*~" -print` \
	   `find . -name "core" -print` \
           `find . -name "*.core" -print` \
	   `find . -name "*.lst" -print` \
	   `find . -name "lst" -print` \
           `find . -name "*.noi" -print`
	cd doc_DE; $(MAKE) clean RM="rm -f"
	cd doc_EN; $(MAKE) clean RM="rm -f"

depend:
	$(CC) $(ALLFLAGS) -MM *.c >depfile

#---------------------------------------------------------------------------

.SUFFIXES: .c
.c.$(OBJEXTENSION):
	$(CC) $(ALLFLAGS) -c $*.c
