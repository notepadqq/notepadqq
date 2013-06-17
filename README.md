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
You can use Qt Creator to automatically build Notepadqq, and that's the recommended way.
Alternatively, you can build Notepadqq from command line:

    notepadqq/src$ qmake-qt4 notepadqq.pro -r -spec linux-g++
    notepadqq/src$ make
    notepadqq/src$ make clean

Compiled files will be available in notepadqq/build/
    

