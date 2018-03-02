# <img src="https://user-images.githubusercontent.com/4319621/36906314-e3f99680-1e35-11e8-90fd-f959c9641f36.png" alt="Notepadqq" width="36" height="36" /> Notepadqq [![Build Status](https://travis-ci.org/notepadqq/notepadqq.svg?branch=master)](https://travis-ci.org/notepadqq/notepadqq) [![Snap Status](https://build.snapcraft.io/badge/notepadqq/notepadqq.svg)](https://build.snapcraft.io/user/notepadqq/notepadqq)

### Index

* [What is it?](#what-is-it)
* [Build it yourself](#build-it-yourself)
* [Download it](#distribution-packages)


#### What is it?

Notepadqq is a text editor designed from developers, for developers. With its more than 100 supported languages, it is the ideal editor for your daily tasks. 

![screenshot_20180302_163505](https://notepadqq.com/s/images/snapshot1.png)


#### How do you like it?

Whether you are a dark theme guy or a light one, you'll find your favourite color scheme.

Your code should be the protagonist: that's why we designed Notepadqq to put focus on the content, without unnecessary fancyness. 

![img](https://notepadqq.com/s/images/colorschemes.png)

#### Speed is not a compromise

As developers we know how it's important to be quick and agile. These are what we believe to be the most important things for any tool that will be used multiple times a day.

Notepadqq gives you a hand with multiple selection, regular expression searches, and real-time highlighting. 

[![multiselect](https://user-images.githubusercontent.com/4319621/36907445-f89b0a4e-1e38-11e8-8e17-b85a23eb4a04.gif)](https://notepadqq.com/s/videos/multiselect.webm)

(Click to view in better quality.)


Build it yourself
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

#### Build

    notepadqq$ ./configure --prefix /usr
    notepadqq$ make
    
If you encounter errors make sure to have the necessary libraries installed. For Ubuntu you can do that using apt-get:

    notepadqq$ sudo apt-get install qt5-default qttools5-dev-tools libqt5webkit5 libqt5webkit5-dev libqt5webkit5-qmlwebkitplugin libqt5svg5 libqt5svg5-dev

For CentOS:

    notepadqq$ sudo yum install -y qt5-qtbase-devel qt5-qttools-devel qt5-qtwebkit-devel qt5-qtsvg-devel
    
#### Install

You can run notepadqq from its build output folder. If however you want to install it, first build it
by following the above steps, then run

    notepadqq$ sudo make install

#### Qt

Notepadqq might work on Qt 5.2, but it is recommended to use Qt 5.3 or later. If the newest version isn't available on your distribution, you can use the [online installer](http://www.qt.io/download-open-source) to get the latest libraries and install them into your home directory (`$HOME/Qt`). Notepadqq will automatically use them.

Distribution Packages
---------------------

#### Most distributions: Snap (recommended) ★

To install the latest stable version:

    sudo snap install notepadqq

If, instead, you want to follow the (UNSTABLE) development releases:

    sudo snap install --edge notepadqq

You don't have the `snap` command? Follow the instructions at https://docs.snapcraft.io/core/install and then install Notepadqq as shown above.

#### AppImage ★
The QWebKit version used in later versions of Ubuntu causes Notepadqq to be unstable. AppImage solves this issue by packaging its own QWebKit version. Continous builds of Notepadqq's AppImage are available [here](https://github.com/notepadqq/notepadqq/releases/tag/continuous).

#### Ubuntu (official packages) ★
You should prefer using Snap packages, which are natively supported on Ubuntu (see above). Anyway, Notepadqq is also available from an [official PPA](https://launchpad.net/~notepadqq-team/+archive/ubuntu/notepadqq):

    sudo add-apt-repository ppa:notepadqq-team/notepadqq
    sudo apt-get update
    sudo apt-get install notepadqq

#### Debian (official packages) ★
Download a deb package from the Ubuntu PPA: [download](https://launchpad.net/~notepadqq-team/+archive/ubuntu/notepadqq/+packages)

#### Arch Linux ★
Notepadqq is available from Arch's [community repositories](https://www.archlinux.org/packages/community/x86_64/notepadqq/). To install using pacman:

    sudo pacman -S notepadqq

Alternatively it can be found in the AUR:

 * Stable (pre-built Debian package): [notepadqq-bin](https://aur.archlinux.org/packages/notepadqq-bin/)
 * Development (git version): [notepadqq-git](https://aur.archlinux.org/packages/notepadqq-git/)

#### OpenSUSE  ★
Notepadqq is avilable in OpenSUSE's main repository:

     sudo zypper in notepadqq

#### Others ★
Use a package for a compatible distribution, or build from [source](https://github.com/notepadqq/notepadqq.git).
If you want to submit a package: https://github.com/notepadqq/notepadqq-packaging

#### Compilation on Mac OSX ★
While we do not (yet) officially support it, compiling and running Notepadqq on OSX is not very difficult. Instructions can be found [here](https://github.com/notepadqq/notepadqq/wiki/Compiling-Notepadqq-on-Mac-OSX).
