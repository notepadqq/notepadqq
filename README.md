Notepadqq
=========

Build dependencies
------------------
   * Qt 5.3.1
   * libqt5webkit5-dev
   * libqt5svg5-dev
   * libmagic-dev

How to build
------------
You can build Notepadqq from command line:

    notepadqq/src/ui$ qmake PREFIX=/opt/notepadqq notepadqq.pro -r -spec linux-g++
    notepadqq/src/ui$ make
    
Install
-------
You can run notepadqq from its build output folder. If however you want to install it, first build it
by following the above steps, then run

    notepadqq/src/ui$ sudo make install

Debian package
--------------
To build a Debian package, see here: https://github.com/notepadqq/notepadqq-packaging
