#!/bin/bash

compile()
{
    brew install qt
    export PATH="/usr/local/opt/qt/bin:$PATH"
    
    ./configure
    make
}

deploy()
{
    compile

    cd out/release
    macdeployqt notepadqq.app -dmg
    mv notepadqq.dmg notepadqq-${NQQ_VERSION}.dmg
}

if [ "$NQQ_BUILD_TYPE" == "DEPLOY" ]; then
    deploy
else
    compile
fi
