          === Notepadqq ===


DEPENDENCIES
   QScintilla: libqscintilla2-5

_________________________________________________________

HOW TO BUILD
   You can use Qt Creator to automatically build Notepadqq, and that's the recommended way.
   Alternatively, you can build Notepadqq from command line:

   | notepadqq/trunk/src$ qmake-qt4 notepadqq.pro -r -spec linux-g++
   | notepadqq/trunk/src$ make
   | notepadqq/trunk/src$ make clean

   Compiled files will be in notepadqq/trunk/build/
    

