#!/bin/sh
macros="$(cat macros.txt | tr '\n' ' ')"
sed "s/@MACROS@/$macros/" m4.js.in > m4.js
