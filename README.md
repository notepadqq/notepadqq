Notepadqq
=========

Build
-----

| Build dependencies | Dependencies  |
|--------------------|---------------|
| Qt 5.3             | Qt 5.3        |
| libqt5webkit5-dev  | libqt5webkit5 |
| libqt5svg5-dev     | libqt5svg5    |
| qttools5-dev-tools | coreutils     |

#### Get the source

    $ git clone https://github.com/notepadqq/notepadqq.git
    $ cd notepadqq

#### Build it

    notepadqq$ ./configure --prefix /usr
    notepadqq$ make
    
Install
-------
You can run notepadqq from its build output folder. If however you want to install it, first build it
by following the above steps, then run

    notepadqq$ sudo make install

Qt
--
Notepadqq might work on Qt 5.2, but it's recommended to use Qt 5.3 or later. If the newest version isn't available on your distribution, you can use the [online installer](http://www.qt.io/download-open-source) to get the latest libraries and install them into your home directory (`$HOME/Qt`). Notepadqq will automatically use them.

Distribution Packages
---------------------

#### Ubuntu (official packages) ★
Notepadqq is available from an [official PPA](https://launchpad.net/~notepadqq-team/+archive/ubuntu/notepadqq):

    sudo add-apt-repository ppa:notepadqq-team/notepadqq
    sudo apt-get update
    sudo apt-get install notepadqq

#### Debian (official packages) ★
Download a deb package from the Ubuntu PPA: [download](https://launchpad.net/~notepadqq-team/+archive/ubuntu/notepadqq/+packages)

#### Arch Linux
You can install different versions of the package from AUR:

 * Stable (pre-built Debian package): [notepadqq-bin](https://aur.archlinux.org/packages/notepadqq-bin/)
 * Development (git version): [notepadqq-git](https://aur.archlinux.org/packages/notepadqq-git/)

#### Others
Use a package for a compatible distribution, or build from [source](https://github.com/notepadqq/notepadqq.git).
If you want to submit a package: https://github.com/notepadqq/notepadqq-packaging
