#!/bin/sh

tmpdir=autoconf-archive-tmp

git clone --depth 1 "git://git.sv.gnu.org/autoconf-archive.git" $tmpdir
ls -1 $tmpdir/m4/*.m4 | sed "s@$tmpdir/m4/@@; s@.m4@@;" | tr '[:lower:]' '[:upper:]' > autoconf-archive.txt
rm -rf $tmpdir

