#!/bin/sh

# Read package version
PKG_VERSION=$(dpkg-parsechangelog -l../debian/changelog | sed -n 's/^Version: //p' | cut -d "-" -f 1)
echo Detected version $PKG_VERSION

TMP_DIR=/tmp/pkg-notepadqq-$PKG_VERSION

# Clean the build directory
rm -rf "$TMP_DIR"
mkdir "$TMP_DIR"

# Copy the source code
cp -r .. "$TMP_DIR"/notepadqq-$PKG_VERSION

# Remove built files from source code dir
rm -rf "$TMP_DIR"/notepadqq-$PKG_VERSION/out

# Create source "orig" tar file
cd "$TMP_DIR"/notepadqq-$PKG_VERSION
tar --gzip --exclude-vcs -cf "$TMP_DIR"/notepadqq_$PKG_VERSION.orig.tar.gz *

# Create source package
debuild -S

# Remove source directory
rm -rf "$TMP_DIR"/notepadqq-$PKG_VERSION

echo
echo "Package created in: $TMP_DIR"

echo
read -p "Do you want to upload the package on Launchpad (ppa:notepadqq-team/notepadqq)? (y/n) " wantupload
if [ "$wantupload" = "y" ]
then
	cd "$TMP_DIR"
	dput ppa:notepadqq-team/notepadqq 'notepadqq_'$PKG_VERSION*'_source.changes'
fi
