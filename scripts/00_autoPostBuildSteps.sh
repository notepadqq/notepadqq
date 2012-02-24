#!/bin/bash
for i in {01..99}
do
   if [ -e "$i"_* ]
   then
        echo
        echo === "$i"_* ===
        chmod +x "$i"_*
        ./"$i"_* <<autoinput
\n
autoinput
   fi
done
echo
echo
echo Done.
read -p "Press any key to exit... "
