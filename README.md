Notepadqq
=========

Dependencies
------------
   * QScintilla (libqscintilla2-8)
   * libmagic-dev

Bug Tracking
------------
To report any bug, use Launchpad: https://bugs.launchpad.net/notepadqq

How to build
------------
Yu can build and install Notepadqq from command line:

    notepadqq/src$ qmake-qt4 PREFIX=/usr notepadqq.pro -r -spec linux-g++
    notepadqq/src$ make
    notepadqq/src$ make install
