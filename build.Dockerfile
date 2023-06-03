# This Docker image builds Notepadqq on Ubuntu. Use this to test whether the
# current code builds fine.

# If you are looking for an image to use as development environment, use the
# main "Dockerfile", which doesn't include a build step (you can enter the shell
# in the container and build it yourself)

FROM ubuntu:22.04
LABEL maintainer="Notepadqq"

RUN apt-get -qq update && apt-get --no-install-recommends -y install \
    build-essential \
    clang-format \
    coreutils \
    gcc \
    git \
    libgl1-mesa-dev \
    libqt6svg6-dev \
    libqt6websockets6-dev \
    libqt6core5compat6-dev \
    libqt6opengl6-dev \
    pkg-config \
    qt6-base-dev \
    qt6-tools-dev-tools \
    qt6-webengine-dev \
    qt6-l10n-tools \
    libuchardet-dev

COPY . /app/
WORKDIR /app/

RUN make clean; exit 0
RUN ./configure --qmake /usr/bin/qmake6 --lrelease /usr/lib/qt6/bin/lrelease && make
RUN src/ui-tests/ui-tests

CMD bash
