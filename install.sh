#!/bin/sh
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
 