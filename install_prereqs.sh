#!/usr/bin/env bash

if [[ "${UID}" -ne 0 ]]
then
    echo "This command has to be run under the root user."
    exit 1
fi

IS_DNF=$(command -v dnf 2>/dev/null | grep -v "^no " | wc -l)
IS_YUM=$(command -v yum 2>/dev/null | grep -v "^no " | wc -l)
IS_APT=$(command -v apt-get 2>/dev/null | grep -v "^no " | wc -l)

if [[ "$IS_APT" -ne 0 ]];
then

    apt-get install -y gcc make libtool pkg-config lsb_release qt5-default qttools5-dev-tools \
      libqt5network5 libqt5webengine5 libqt5webenginewidgets5 libqt5webenginecore5 libqt5widgets5 \
      libqt5svg5-dev libqt5websockets5-dev libqt5webchannel5-dev qtwebengine5-dev libuchardet-dev 

else

    echo "Package manager is currently not supported. Please submit patches for this script."
    exit 1
fi

echo ""
echo "Prerequisites appear to be installed. Your next step is to configure with:"
echo "  ./configure            # release build"
echo "  ./configure  --debug   # debug build"
echo "  ./configure  --help    # view options"

# ============================================================
# ============================================================

# Use this to dump Debian
if false; then

packages=(qt5-default qttools5-dev-tools libqt5network5 libqt5webengine5 libqt5webenginewidgets5
          libqt5webenginecore5 libqt5widgets5 libqt5svg5-dev libqt5websockets5-dev
          libqt5webchannel5-dev qtwebengine5-dev libuchardet-dev)

echo "========================================"
lsb_release -a 2>&1 | grep -v '^No'
echo "========================================"
for package in "${packages[@]}";
do
    apt-cache show "$package" | head -n 3 | grep -v 'Source:'
    echo "========================================"
done

fi

exit 0

