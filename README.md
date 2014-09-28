Notepadqq
=========

Build dependencies
------------------
   * Qt 5.3
   * libqt5webkit5-dev
   * libqt5svg5-dev

How to build
------------
You can build Notepadqq from command line:

    notepadqq$ ./configure --prefix /usr
    notepadqq$ make
    
Install
-------
You can run notepadqq from its build output folder. If however you want to install it, first build it
by following the above steps, then run

    notepadqq$ sudo make install

Debian package
--------------
To build a Debian package, see here: https://github.com/notepadqq/notepadqq-packaging

Arch Linux Package
--------------
You can install the package from AUR into different versions:
   Stable (pre-build Debian package): https://aur.archlinux.org/packages/notepadqq-bin/
   Dev (git version): https://aur.archlinux.org/packages/notepadqq-git/
