#!/bin/sh
set -v

# assure we don't copy to absolute paths if $PREFIX is not set:

if [ "${PREFIX}" != "" ]; then
  PREFIX=${PREFIX}/
fi
BINPATH=${PREFIX}$1
INCPATH=${PREFIX}$2
MANPATH=${PREFIX}$3
LIBPATH=${PREFIX}$4
DOCPATH=${PREFIX}$5

# this is not a perfect solution, but I don't know a better one at the moment:

if [ -f /usr/X11R6/bin/mkdirhier ] ; then
  MKDIRHIER="/usr/X11R6/bin/mkdirhier"
else
  if [ -f /usr/bin/X11/mkdirhier ] ; then
    MKDIRHIER="/usr/bin/X11/mkdirhier"
  else
    MKDIRHIER="mkdir -p"
  fi
fi

echo "Installing files:"

if [ "${BINPATH}" != "" ]; then
 ${MKDIRHIER} ${BINPATH}
 chmod 755 ${BINPATH}
 for i in asl plist alink pbind p2hex p2bin; do
  echo ${BINPATH}/$i
  strip $i
  cp $i ${BINPATH}
  chmod 755 ${BINPATH}/$i
 done
fi

if [ "${INCPATH}" != "" ]; then
 ${MKDIRHIER} ${INCPATH}
 chmod 755 ${INCPATH}
 for path in . avr s12z s12z/vh s12z/vc s12z/vca coldfire st7 stm8 stm8/stm8s stm8/stm8l stm8/stm8af stm8/stm8al stm8/stm8t; do
  mkdir ${INCPATH}/${path}
  chmod 755 ${INCPATH}/${path}
  for file in include/${path}/*.inc; do
   base=`basename ${file}`
   echo ${INCPATH}/${path}/${base}
   cp ${file} ${INCPATH}/${path}
   chmod 644 ${INCPATH}/${path}/$base
  done
 done
fi

if [ "${MANPATH}" != "" ]; then
 ${MKDIRHIER} ${MANPATH}/man1
 chmod 755 ${MANPATH} ${MANPATH}/man1
 for i in man/*.1; do
  echo ${MANPATH}/man1/`basename $i`
  cp $i ${MANPATH}/man1
  chmod 644 ${MANPATH}/man1/`basename $i`
 done
fi

if [ "${LIBPATH}" != "" ]; then
 ${MKDIRHIER} ${LIBPATH}
 chmod 755 ${LIBPATH}
 for i in *.msg; do
  echo ${LIBPATH}/$i
  cp $i ${LIBPATH}
  chmod 644 ${LIBPATH}/$i
 done
fi

if [ "${DOCPATH}" != "" ]; then
 ${MKDIRHIER} ${DOCPATH}
 chmod 755 ${DOCPATH}
 for i in DE EN; do
  if [ -f doc_$i/as.html ]; then
    echo ${DOCPATH}/as-$i.doc
    cp doc_$i/as.doc ${DOCPATH}/as-$i.doc
  fi
  echo ${DOCPATH}/as-$i.tex
  cp doc_$i/as.tex ${DOCPATH}/as-$i.tex
  if [ -f doc_$i/as.html ]; then
   echo ${DOCPATH}/as-$i.html
   cp doc_$i/as.html ${DOCPATH}/as-$i.html
  fi
  if [ -f doc_$i/as.dvi ]; then
   echo ${DOCPATH}/as-$i.dvi
   cp doc_$i/as.dvi ${DOCPATH}/as-$i.dvi
  fi
  if [ -f doc_$i/as.ps ]; then
   echo ${DOCPATH}/as-$i.ps
   cp doc_$i/as.ps ${DOCPATH}/as-$i.ps
  fi
  if [ -f doc_$i/as.pdf ]; then
   echo ${DOCPATH}/as-$i.pdf
   cp doc_$i/as.pdf ${DOCPATH}/as-$i.pdf
  fi
  chmod 644 ${DOCPATH}/as-$i.*
 done
 cp doc_DE/taborg*.tex ${DOCPATH}
 chmod 644 ${DOCPATH}/taborg*.tex
 cp doc_DE/ps*.tex ${DOCPATH}
 chmod 644 ${DOCPATH}/ps*.tex
 cp COPYING ${DOCPATH}
 chmod 644 ${DOCPATH}/COPYING
fi
