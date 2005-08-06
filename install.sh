#!/bin/sh
# set -v

echo "Installing files:"

if [ "$1" != "" ]; then
 mkdirhier $1
 chmod 755 $1
 for i in asl plist alink pbind p2hex p2bin; do
  echo $1/$i
  strip $i
  cp $i $1
  chmod 755 $1/$i
 done
fi

if [ "$2" != "" ]; then
 mkdirhier $2
 chmod 755 $2
 for i in include/*.inc; do
  base=`basename $i`
  echo $2/$base
  cp $i $2
  chmod 644 $2/$base
 done
fi

if [ "$3" != "" ]; then
 mkdirhier $3/man1
 chmod 755 $3 $3/man1
 for i in man/*.1; do
  echo $3/man1/`basename $i`
  cp $i $3/man1
  chmod 644 $3/man1/$i
 done
fi

if [ "$4" != "" ]; then
 mkdirhier $4
 chmod 755 $4
 for i in *.msg; do
  echo $4/$i
  cp $i $4
  chmod 644 $4/$i
 done
fi

if [ "$5" != "" ]; then
 mkdirhier $5
 chmod 755 $5
 for i in DE EN; do
  echo $5/as-$i.doc
  cp doc_$i/as.doc $5/as-$i.doc
  echo $5/as-$i.tex
  cp doc_$i/as.tex $5/as-$i.tex
  if [ -f doc_$i/as.dvi ]; then
   echo $5/as-$i.dvi
   cp doc_$i/as.dvi $5/as-$i.dvi
  fi
  if [ -f doc_$i/as.ps ]; then
   echo $5/as-$i.ps
   cp doc_$i/as.ps $5/as-$i.ps
  fi
  if [ -f doc_$i/as.pdf ]; then
   echo $5/as-$i.pdf
   cp doc_$i/as.pdf $5/as-$i.pdf
  fi
  chmod 644 $5/as-$i.*
 done
 cp doc_DE/taborg*.tex $5
 chmod 644 $5/taborg*.tex
 cp doc_DE/ps*.tex $5
 chmod 644 $5/ps*.tex
 cp COPYING $5
 chmod 644 $5/COPYING
fi
