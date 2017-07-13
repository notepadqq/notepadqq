#!/bin/bash

# build-deps: fuse build-essential wget git
# libxcb-* libxi-dev libxrender-dev libsm-dev libice-dev libx11-dev libxcomposite-dev
# libpng12-dev libsqlite3-dev libfontconfig1-dev libfreetype6-dev libjpeg-dev
# zlib1g-dev libicu-dev libglib2.0-dev libgl1-mesa-dev

set -e
set -x

APP=notepadqq
LOWERAPP=${APP,,}
ARCH=x86_64
MULTIARCH=x86_64-linux-gnu

JOBS=4
BUILDROOT="$PWD"
VERSION=$(git describe | sed 's|^v||')

qt_top="/var/tmp"
qt="$qt_top/qt-static-trusty"

cd "$qt_top"
tarball="qt571-qtwebkit571-x86_64-static-linux-trusty.tar.xz"
wget "https://github.com/darealshinji/qt5-with-qtwebkit/releases/download/qt5.7.1%2Bqtqwebkit5.7.1/$tarball"
tar xf $tarball

cd "$BUILDROOT"

# a bit hardening, strip unused functions, don't add unneeded dependencies
export CFLAGS="-O2 -fstack-protector --param=ssp-buffer-size=4 -fno-strict-aliasing -ffunction-sections -fdata-sections -D_FORTIFY_SOURCE=2"
export CXXFLAGS="$CFLAGS"
export LDFLAGS="-Wl,-z,relro -Wl,--gc-sections -Wl,--as-needed"

# build notepadqq
export PATH="$qt/bin:$PATH"
qmake PREFIX=/usr QMAKE_CXXFLAGS="$CXXFLAGS" QMAKE_LFLAGS="$LDFLAGS -static-libgcc -static-libstdc++" LRELEASE=lrelease
make -j$JOBS

# link ICU libs statically to not depend on the libstdc++ shared library; that means more compatibility
sed -i -e 's|-licui18n|-Wl,-Bstatic -licui18n -Wl,-Bdynamic|g; s|-licudata|-Wl,-Bstatic -licudata -Wl,-Bdynamic|g; s|-licuuc|-Wl,-Bstatic -licuuc -Wl,-Bdynamic|g' ./src/ui/Makefile
rm -f ./out/release/lib/notepadqq-bin
make -j$JOBS

mkdir -p build/$APP.AppDir
make install DESTDIR="$PWD/build/$APP.AppDir" INSTALL_ROOT="$PWD/build/$APP.AppDir"

# source AppImage functions
test -e functions.sh || wget -c "https://raw.githubusercontent.com/probonopd/AppImages/master/functions.sh" -O functions.sh
. functions.sh

cd build/$APP.AppDir

strip ./usr/lib/notepadqq/notepadqq-bin
# replace launch script in /usr/bin
mv -f ./usr/lib/notepadqq/notepadqq-bin ./usr/bin/notepadqq
# move appdata where nqq can find it
mv ./usr/share/notepadqq ./usr/appdata

copy_deps

mv -f ./usr/lib/$MULTIARCH/* ./usr/lib/
mv -f ./lib/$MULTIARCH/* ./usr/lib/
move_lib

delete_blacklisted

get_desktop
get_icon
get_desktopintegration $APP
get_apprun

cd ..

generate_appimage

