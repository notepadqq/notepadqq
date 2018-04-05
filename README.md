# <img src="https://user-images.githubusercontent.com/4319621/36906314-e3f99680-1e35-11e8-90fd-f959c9641f36.png" alt="Notepadqq" width="32" height="32" /> Notepadqq [![Build Status](https://travis-ci.org/notepadqq/notepadqq.svg?branch=master)](https://travis-ci.org/notepadqq/notepadqq) [![Snap Status](https://build.snapcraft.io/badge/notepadqq/notepadqq.svg)](https://build.snapcraft.io/user/notepadqq/notepadqq)

### Links

* [What is it?](#what-is-it)
* [Build it yourself](#build-it-yourself)
* [Download it](#distribution-packages)

#### What is it?

Notepadqq is a text editor designed from developers, for developers. 

![screenshot_20180302_163505](https://notepadqq.com/s/images/snapshot1.png)

Please visit our [Wiki](https://github.com/notepadqq/notepadqq/wiki) for more screenshots and details.

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

For Fedora(if >17 replace yum by dnf), CentOS RedHat(if >6 replace yum by dnf):

    notepadqq$ sudo yum install -y qt5 qt5-devel qt5-qtbase-devel qt5-qttools-devel qt5-qtwebkit-devel qt5-qtsvg-devel

For build rpm

for Centos Readhat require epel-release

    notepadqq$ sudo yum install -y epel-release
   
Builder dépendencie for build rpm
    
    notepadqq$ sudo yum install -y rpmdevtools yum-utils wget
    notepadqq$ sudo yum -y groupinstall "Fedora Packager"
    
Download Source and build rpm
    
    notepadqq$ cd ~/rpmbuild/SOURCES
    notepadqq$ wget https://github.com/notepadqq/notepadqq/archive/v1.3.1.tar.gz - O notepadqq-1.3.1.tar.gz
    notepadqq$ wget https://github.com/notepadqq/notepadqq/raw/master/notepadqq.spec
    notepadqq$ rpmbuild -ba notepadqq.spec
    
#### Install

You can run notepadqq from its build output folder. If however you want to install it, first build it
by following the above steps, then run

    notepadqq$ sudo make install

#### Qt

Notepadqq might work on Qt 5.2, but it is recommended to use Qt 5.3 or later. If the newest version isn't available on your distribution, you can use the [online installer](http://www.qt.io/download-open-source) to get the latest libraries and install them into your home directory (`$HOME/Qt`). Notepadqq will automatically use them.

Distribution Packages
---------------------

#### Most distributions: Snap (recommended)

To install the latest stable version:

    notepadqq$ sudo snap install notepadqq

If, instead, you want to follow the (UNSTABLE) development releases:

    notepadqq$ sudo snap install --edge notepadqq

You don't have the `snap` command? Follow the instructions at https://docs.snapcraft.io/core/install and then install Notepadqq as shown above.

#### Launchpad PPA
You should prefer using Snap packages, which are natively supported on Ubuntu (see above). Anyway, Notepadqq is also available from an [official PPA](https://launchpad.net/~notepadqq-team/+archive/ubuntu/notepadqq):

    notepadqq$ sudo add-apt-repository -y ppa:notepadqq-team/notepadqq
    notepadqq$ sudo apt-get update
    notepadqq$ sudo apt-get -y install notepadqq

#### Debian
Download a deb package from the Ubuntu PPA: [download](https://launchpad.net/~notepadqq-team/+archive/ubuntu/notepadqq/+packages)

#### Fedora (Only Stable Version actual 03/2018 version 26 and 27)

    notepadqq$ sudo dnf -y install https://rpms.remirepo.net/fedora/remi-release-$(cat /etc/os-release | grep VERSION_ID= | sed 's|VERSION_ID=||').rpm http://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-$(cat /etc/os-release | grep VERSION_ID= | sed 's|VERSION_ID=||').noarch.rpm http://download1.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-$(cat /etc/os-release | grep VERSION_ID= | sed 's|VERSION_ID=||').noarch.rpm
    notepadqq$ sudo dnf -y copr enable andykimpe/Fedora-Stable
    notepadqq$ sudo dnf -y install notepadqq
    
copr fedora stable project page

https://copr.fedorainfracloud.org/coprs/andykimpe/Fedora-Stable/

#### Arch Linux (community-maintained)
Notepadqq is available from Arch's [community repositories](https://www.archlinux.org/packages/community/x86_64/notepadqq/). To install using pacman:

    notepadqq$ sudo pacman -S notepadqq

Alternatively it can be found in the AUR:

 * Stable (pre-built Debian package): [notepadqq-bin](https://aur.archlinux.org/packages/notepadqq-bin/)
 * Development (git version): [notepadqq-git](https://aur.archlinux.org/packages/notepadqq-git/)

#### OpenSUSE (community-maintained)
Notepadqq is avilable in OpenSUSE's main repository:

     notepadqq$ sudo zypper in notepadqq
     
#### Solus (community-maintained)
Notepadqq is available in the `shannon` (stable) repository:

     notepadqq$ sudo eopkg it notepadqq

#### Others
Use a package for a compatible distribution, or build from [source](https://github.com/notepadqq/notepadqq.git).
If you want to submit a package: https://github.com/notepadqq/notepadqq-packaging

#### Compiling on macOS
While we do not (yet) officially support it, compiling and running Notepadqq on macOS is not very difficult. Instructions can be found [here](https://github.com/notepadqq/notepadqq/wiki/Compiling-Notepadqq-on-macOS).
