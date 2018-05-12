MAINTAINER Notepadqq

RUN apt-get -qq update && apt-get -y install \
    build-essential \
    clang-format-6.0 \
    coreutils \
    gcc \
    git \
    libqt5svg5-dev \
    libqt5websockets5-dev \
    pkg-config \
    qtbase5-dev \
    qttools5-dev-tools \
    qtwebengine5-dev

WORKDIR /build/
CMD bash
