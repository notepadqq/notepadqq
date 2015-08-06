#!/bin/sh

# If we're in a git repo and git is installed, update the submodules.
# If ".git" is not found, assume that the complete source
# code is already available.
if [ -d ".git" ] && [ -n "$(whereis -b git | awk '{print $2}')" ]; then
    git submodule init
    git submodule update
fi

cmdir=src/editor/libs/codemirror
cmerror="error: CodeMirror submodule not available at \`$cmdir'! Make sure you downloaded the source code using git as specified in README.md."
# check if directory exists
test -d "$cmdir" || (echo "$cmerror" && exit 1)
# check if directory is empty
test "$(ls -A $cmdir)" || (echo "$cmerror" && exit 1)

test -n "$(whereis -b autoreconf | awk '{print $2}')" || (echo "autoreconf not found!" && exit 1)

autoreconf -ivf "$@"
rm -rf autom4te.cache

