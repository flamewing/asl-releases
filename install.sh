#!/bin/sh
set -v
mkdirhier $1
chmod 755 $1
for i in asl plist p2hex p2bin; do
 strip $i
 cp $i $1
 chmod 755 $1/$i
done

mkdirhier $2
chmod 755 $2
cd include
for i in *.inc; do
 cp $i $2
 chmod 644 $2/$i
done
cd ..

mkdirhier $3/man1
chmod 755 $3
for i in *.1; do
 cp $i $3/man1
 chmod 644 $3/man1/$i 
done
