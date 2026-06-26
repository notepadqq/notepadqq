FROM ubuntu:26.04
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

WORKDIR /app/
CMD bash
