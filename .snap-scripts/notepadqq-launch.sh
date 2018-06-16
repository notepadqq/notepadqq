#!/bin/bash

QT_BASE_DIR=$SNAP/opt/qt59
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

#source $SNAP/opt/qt59/bin/qt59-env.sh

export LD_LIBRARY_PATH=$SNAP/usr/lib/gcc/x86_64-linux-gnu/7/:$SNAP/usr/lib/gcc/x86_64-linux-gnu/:$LD_LIBRARY_PATH


$SNAP/usr/local/lib/notepadqq/notepadqq-bin
