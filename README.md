Notepadqq
=========

_Keep in mind that this is an experimental branch._

Build dependencies
------------------
   * Qt 5.3.1
   * libqt5webkit5-dev
   * libmagic-dev

How to build
------------
You can build Notepadqq from command line:

    notepadqq/src/ui$ qmake PREFIX=/opt/notepadqq DEPLOY=false notepadqq.pro -r -spec linux-g++
    notepadqq/src/ui$ make
    
Make sure you're using qmake from Qt 5.3.1 (`qmake -v`). If not, you might need to specify its full path.

`DEPLOY=false` will hard-link notepadqq to your local Qt libraries. If you want to deploy notepadqq to
different machines (e.g. if you're building a Linux package), you need to run qmake with `DEPLOY=true` and
then copy the needed Qt libraries within the same folder as the notepadqq binary file (find them with `ldd notepadqq|grep libQt`.

Install
-------
You can run notepadqq from its build output folder. If however you want to install it, first build it
by following the above steps, then run

    notepadqq/src/ui$ sudo make install

Debian package
--------------
To build a Debian package:

    notepadqq$ dpkg-buildpackage -b -us -uc
