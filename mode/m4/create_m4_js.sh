#!/bin/sh

tmp=macros~

cat macros.txt autoconf-archive.txt | tr '\n' ' ' | fold -s -w 71 | sed "s/^/    '/; s/$/' +/" > $tmp
sed -e "/@MACROS@/r $tmp" -e "/@MACROS@/d" m4.js.in > m4.js
rm $tmp

