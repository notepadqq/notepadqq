#!/bin/bash

compile()
{
    brew install qt
    brew install uchardet

    export PATH="/usr/local/opt/qt/bin:$PATH"
    export PKG_CONFIG_PATH="/usr/local/opt/qt/lib/pkgconfig"

    ./configure
    make || return 1
}

deploy()
{
    compile || return 1

    cd out/release
    macdeployqt notepadqq.app -dmg
    mv notepadqq.dmg notepadqq-${NQQ_VERSION}.dmg || return 1
}

if [ "$NQQ_BUILD_TYPE" == "DEPLOY" ]; then
    deploy
else
    compile
fi
