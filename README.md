Notepadqq
=========

[![Build Status](https://travis-ci.org/notepadqq/notepadqq.svg?branch=master)](https://travis-ci.org/notepadqq/notepadqq)

Build
-----

| Build dependencies | Dependencies  |
|--------------------|---------------|
| Qt 5.3             | Qt 5.3        |
| libqt5webkit5-dev  | libqt5webkit5 |
| libqt5svg5-dev     | libqt5svg5    |
| qttools5-dev-tools | coreutils     |

#### Get the source

    $ git clone --recursive https://github.com/notepadqq/notepadqq.git
    $ cd notepadqq

#### Build it

    notepadqq$ sudo apt-get install qt5-default qttools5-dev-tools libqt5webkit5 libqt5webkit5-dev libqt5webkit5-qmlwebkitplugin libqt5svg5 libqt5svg5-dev
    notepadqq$ ./configure --prefix /usr
    notepadqq$ make
    
Install
-------
You can run notepadqq from its build output folder. If however you want to install it, first build it
by following the above steps, then run

    notepadqq$ sudo make install

Qt
--
Notepadqq might work on Qt 5.2, but it is recommended to use Qt 5.3 or later. If the newest version isn't available on your distribution, you can use the [online installer](http://www.qt.io/download-open-source) to get the latest libraries and install them into your home directory (`$HOME/Qt`). Notepadqq will automatically use them.

Distribution Packages
---------------------

#### AppImage (recommended for Ubuntu users) ★
The QWebKit version used in later versions of Ubuntu causes Notepadqq to be unstable. AppImage solves this issue by packaging its own QWebKit version. Continous builds of Notepadqq's AppImage are available [here](https://github.com/notepadqq/notepadqq/releases/tag/continuous).

#### Ubuntu (official packages) ★
Notepadqq is available from an [official PPA](https://launchpad.net/~notepadqq-team/+archive/ubuntu/notepadqq):

    sudo add-apt-repository ppa:notepadqq-team/notepadqq
    sudo apt-get update
    sudo apt-get install notepadqq

#### Debian (official packages) ★
Download a deb package from the Ubuntu PPA: [download](https://launchpad.net/~notepadqq-team/+archive/ubuntu/notepadqq/+packages)

#### Arch Linux
Notepadqq is available from Arch's [community repositories](https://www.archlinux.org/packages/community/x86_64/notepadqq/). To install using pacman:

    sudo pacman -S notepadqq

Alternatively it can be found in the AUR:

 * Stable (pre-built Debian package): [notepadqq-bin](https://aur.archlinux.org/packages/notepadqq-bin/)
 * Development (git version): [notepadqq-git](https://aur.archlinux.org/packages/notepadqq-git/)

#### OpenSUSE 
Notepadqq is avilable in OpenSUSE's main repository:

     sudo zypper in notepadqq

#### Others
Use a package for a compatible distribution, or build from [source](https://github.com/notepadqq/notepadqq.git).
If you want to submit a package: https://github.com/notepadqq/notepadqq-packaging
