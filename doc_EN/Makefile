INCFILES = ../doc_DE/taborg*.tex ../doc_DE/tabids*.tex ../doc_DE/pscpu.tex ../doc_DE/pscomm.tex

all: as.doc as.html as.ps as.pdf

as.ps: as.dvi
	dvips as -o

as.pdf: as.tex $(INCFILES)
	pdflatex as
	pdflatex as
	makeindex as
	pdflatex as 

as.dvi: as.tex $(INCFILES)
	latex as
	latex as
	makeindex as
	latex as

as.doc: as.tex $(INCFILES) $(TEX2DOC)
	$(TEX2DOC) as.tex as.doc
	$(TEX2DOC) as.tex as.doc

as.html: as.tex $(INCFILES) $(TEX2HTML)
	$(TEX2HTML) as.tex as.html
	$(TEX2HTML) as.tex as.html

clean:
	$(RM) as.dvi
	$(RM) as.ps
	$(RM) as.pdf
	$(RM) as.i*
	$(RM) *log
	$(RM) as.aux
	$(RM) as.out
	$(RM) as.toc
	$(RM) *~
	$(RM) DEADJOE
	$(RM) as.doc
	$(RM) as.dtoc
	$(RM) as.daux
	$(RM) as.html
	$(RM) as.htoc
	$(RM) as.haux
