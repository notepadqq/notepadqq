QT_BASE_DIR=$SNAPCRAFT_STAGE/opt/qt59
export QTDIR=$QT_BASE_DIR
export PATH=$QT_BASE_DIR/bin:$PATH

if [[ $(uname -m) == "x86_64" ]]; then
  export LD_LIBRARY_PATH=$QT_BASE_DIR/lib/x86_64-linux-gnu:$QT_BASE_DIR/lib:$LD_LIBRARY_PATH
else
  export LD_LIBRARY_PATH=$QT_BASE_DIR/lib/i386-linux-gnu:$QT_BASE_DIR/lib:$LD_LIBRARY_PATH
fi

export PKG_CONFIG_PATH=$QT_BASE_DIR/lib/pkgconfig:$PKG_CONFIG_PATH

TEST=`echo $0 | grep wrapper`
if [ "$TEST" != "" ]; then
   exec `echo $0 | sed s/-wrapper//` $*
fi


#echo $SNAP
#echo "-----"
#ll $SNAPCRAFT_PART_INSTALL
#ll $SNAPCRAFT_PART_BUILD
#ll $SNAP
#ll $SNAPCRAFT_STAGE
#source $SNAPCRAFT_STAGE/opt/qt59/bin/qt59-env.sh

export QMAKE=$(which qmake)
export LRELEASE=$(which lrelease)

export CXX=$(which g++-7)

./configure --prefix $SNAPCRAFT_PART_INSTALL/usr/local
make BUILD_SNAP=1
make install
