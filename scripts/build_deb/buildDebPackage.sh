#!/bin/bash
cd ../../
read -p "Package version (e.g. 0.13.9): " pkg_version

# Create notepadqq-{vers}.tar.gz file
rm --force --interactive=never --recursive notepadqq-$pkg_version
cp --recursive src notepadqq-$pkg_version
tar --gzip --exclude-vcs -cf notepadqq-$pkg_version.tar.gz notepadqq-$pkg_version
rm --force --interactive=never --recursive notepadqq-$pkg_version

# Create "tar.gz" packages
mv notepadqq-$pkg_version.tar.gz packages/debian/
cd packages/debian/
cp notepadqq-$pkg_version.tar.gz notepadqq_$pkg_version.orig.tar.gz
tar -xf notepadqq-$pkg_version.tar.gz
cd notepadqq-$pkg_version
dh_make -s -c gpl3 -e danieleds0@gmail.com --templates "../../overlay"
cd debian
rm *.ex *.EX README.Debian
cd ..
debuild -S
echo
echo
cd ..
read -p "Do you want to compile the package? (y/n) " wantcompile
if [ "$wantcompile" == "y" ]
then
    sudo pbuilder build *.dsc
fi
echo
echo
read -p "Do you want to upload the package on Launchpad (ppa:notepadqq-team/notepadqq)? (y/n) " wantupload
if [ "$wantupload" == "y" ]
then
    dput ppa:notepadqq-team/notepadqq 'notepadqq_'$pkg_version*'_source.changes'
fi
echo
echo
read -p "Press enter to exit... "
