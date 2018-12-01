# <img src="https://user-images.githubusercontent.com/4319621/36906314-e3f99680-1e35-11e8-90fd-f959c9641f36.png" alt="Notepadqq" width="32" height="32" /> Notepadqq [![Build Status](https://travis-ci.com/notepadqq/notepadqq.svg?branch=master)](https://travis-ci.com/notepadqq/notepadqq) [![Snap Status](https://build.snapcraft.io/badge/notepadqq/notepadqq.svg)](https://build.snapcraft.io/user/notepadqq/notepadqq)

### Links

* [What is it?](#what-is-it)
* [Build it yourself](#build-it-yourself)
* [Download it](#distribution-packages)

#### What is it?

Notepadqq is a text editor designed by developers, for developers. 

![screenshot_20180302_163505](https://notepadqq.com/s/images/snapshot1.png)

Please visit our [Wiki](https://github.com/notepadqq/notepadqq/wiki) for more screenshots and details.

Build it yourself
-----

| Build dependencies    | Dependencies      |
|-----------------------|-------------------|
| Qt 5.6 or higher      | Qt 5.6 or higher  |
| qtwebengine5-dev      | qtwebengine5      |
| libqt5websockets5-dev | libqt5websockets5 |
| libqt5svg5-dev        | libqt5svg5        |
| qttools5-dev-tools    | coreutils         |
| libuchardet-dev       | libuchardet       |

#### Get the source

    $ git clone --recursive https://github.com/notepadqq/notepadqq.git
    $ cd notepadqq

#### Build

    notepadqq$ ./configure --prefix /usr
    notepadqq$ make
    
If you encounter errors make sure to have the necessary libraries installed. For Ubuntu you can do that using apt-get:

    notepadqq$ sudo apt-get install qt5-default qttools5-dev-tools qtwebengine5-dev libqt5websockets5-dev libqt5svg5 libqt5svg5-dev libuchardet-dev

For CentOS:

    notepadqq$ sudo yum install -y qt5-qtbase-devel qt5-qttools-devel qt5-qtwebengine-devel qt5-qtwebsockets-devel qt5-qtsvg-devel uchardet qt5-qtwebchannel-devel
    
#### Install

You can run notepadqq from its build output folder. If however you want to install it, first build it
by following the above steps, then run

    notepadqq$ sudo make install

#### Qt

If the newest version of Qt isn't available on your distribution, you can use the [online installer](http://www.qt.io/download-open-source) to get the latest libraries and install them into your home directory (`$HOME/Qt`). Notepadqq will automatically use them.

Distribution Packages
---------------------

#### Most distributions: Snap (recommended)

To install the latest stable version:

    sudo snap install --classic notepadqq

You don't have the `snap` command? Follow the instructions at https://docs.snapcraft.io/core/install and then install Notepadqq as shown above.

You can follow the unstable development releases from the "edge" channel.

#### Launchpad PPA
You should prefer using Snap packages, which are natively supported on Ubuntu (see above). Anyway, Notepadqq is also available from an [official PPA](https://launchpad.net/~notepadqq-team/+archive/ubuntu/notepadqq):

    sudo add-apt-repository ppa:notepadqq-team/notepadqq
    sudo apt-get update
    sudo apt-get install notepadqq

#### Debian
Download a deb package from the Ubuntu PPA: [download](https://launchpad.net/~notepadqq-team/+archive/ubuntu/notepadqq/+packages)

#### Arch Linux (community-maintained)
Notepadqq is available from Arch's [community repositories](https://www.archlinux.org/packages/community/x86_64/notepadqq/). To install using pacman:

    sudo pacman -S notepadqq

Alternatively it can be found in the AUR:

 * Stable (pre-built Debian package): [notepadqq-bin](https://aur.archlinux.org/packages/notepadqq-bin/)
 * Development (git version): [notepadqq-git](https://aur.archlinux.org/packages/notepadqq-git/)

#### OpenSUSE (community-maintained)
Notepadqq is avilable in OpenSUSE's main repository:

     sudo zypper in notepadqq
     
#### Solus (community-maintained)
Notepadqq is available in the `shannon` (stable) repository:

     sudo eopkg it notepadqq

#### Others
Use a package for a compatible distribution, or build from [source](https://github.com/notepadqq/notepadqq.git).
If you want to submit a package: https://github.com/notepadqq/notepadqq-packaging

#### Compiling on macOS
While we do not (yet) officially support it, compiling and running Notepadqq on macOS is not very difficult. Instructions can be found [here](https://github.com/notepadqq/notepadqq/wiki/Compiling-Notepadqq-on-macOS).
