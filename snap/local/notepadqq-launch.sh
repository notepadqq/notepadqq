#!/bin/bash
#
# Runtime launcher for notepadqq.
#
# This runs as a command-chain entry before the app's actual `command`.
# Its job: figure out the Debian multiarch triplet for the architecture
# we're actually running on, and prepend the snap's own Qt6/ICU lib
# directories to LD_LIBRARY_PATH so they take precedence over anything
# the host system provides (classic confinement does not isolate the
# dynamic linker's search path from the host).
#
# $SNAP is guaranteed to be set by snapd before any command-chain link
# runs, so this is safe to rely on here even though it can't be
# resolved at YAML/build time.

set -eu

case "$(uname -m)" in
    x86_64)
        ARCH_TRIPLET="x86_64-linux-gnu"
        ;;
    aarch64)
        ARCH_TRIPLET="aarch64-linux-gnu"
        ;;
    armv7l)
        ARCH_TRIPLET="arm-linux-gnueabihf"
        ;;
    *)
        echo "notepadqq-launch.sh: unsupported architecture '$(uname -m)'" >&2
        exit 1
        ;;
esac

export LD_LIBRARY_PATH="$SNAP/usr/lib/$ARCH_TRIPLET:$SNAP/usr/lib/$ARCH_TRIPLET/qt6:$SNAP/usr/lib/notepadqq:${LD_LIBRARY_PATH:-}"

exec "$@"