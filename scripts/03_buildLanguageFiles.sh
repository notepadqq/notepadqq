#!/bin/bash
lrelease ../src/notepadqq.pro
mkdir -p ../notepadqq-build-desktop/L10n
rm --force --interactive=never ../notepadqq-build-desktop/L10n/*
mv ../src/L10n/*.qm ../notepadqq-build-desktop/L10n
echo
echo
echo Done.
read -p "Press any key to exit... "
