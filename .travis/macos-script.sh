#!/bin/bash

compile()
{
    brew install qt
    export PATH="/usr/local/opt/qt/bin:$PATH"
    
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
