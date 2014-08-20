Notepadqq
=========

_Keep in mind that this is an experimental branch._

Dependencies
------------
   * Qt 5.3.1
   * libmagic-dev

How to build
------------
You can build Notepadqq from command line:

    notepadqq/src/ui$ qmake PREFIX=/usr/local notepadqq.pro -r -spec linux-g++
    notepadqq/src/ui$ make
    
Then, if you want to install it, run

    notepadqq/src/ui$ sudo make install
