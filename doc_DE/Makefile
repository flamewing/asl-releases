as.ps: as.dvi
	dvips as -o

as.dvi: as.tex
	latex as
	latex as
	makeindex as
	latex as

clean:
	rm -f as.dvi as.ps as.i* *log as.aux as.toc *~
