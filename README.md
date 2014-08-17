Notepadqq
=========

_Keep in mind that this is an experimental branch._

Dependencies
------------
   * Qt 5.3.1
   * libmagic-dev

Bug Tracking
------------
To report any bug, use Launchpad: https://bugs.launchpad.net/notepadqq

How to build
------------
You can build and install Notepadqq from command line:

    notepadqq/src/ui$ qmake PREFIX=/usr/local notepadqq.pro -r -spec linux-g++
    notepadqq/src/ui$ make
    
`make install` is not currently supported.
