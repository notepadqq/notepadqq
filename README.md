Notepadqq
=========

Dependencies
------------
   * QScintilla (libqscintilla2-8)

Bug Tracking
------------
To report any bug, use Launchpad: https://bugs.launchpad.net/notepadqq

You want to implement a new feature?
------------------------------------
The current "object_rewrite" branch will soon become the main one: try making your changes in object_rewrite!

How to build
------------
You can use Qt Creator to automatically build Notepadqq, and that's the recommended way.
Alternatively, you can build Notepadqq from command line:

    notepadqq/src$ qmake-qt4 notepadqq.pro -r -spec linux-g++
    notepadqq/src$ make
    notepadqq/src$ make clean

Compiled files will be available in notepadqq/build/
    

