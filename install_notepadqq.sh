#!/usr/bin/env bash

if ! ./configure --prefix=/usr/local; then
    echo "Configure failed"
    exit 1
fi

if ! make -j4; then
    echo "Make failed"
    exit 1
fi

if ! sudo make install; then
    echo "Make install failed"
    exit 1
fi

sudo make distclean

exit 0
