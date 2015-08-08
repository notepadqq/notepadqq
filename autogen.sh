#!/bin/sh

# If we're in a git repo and git is installed, update the submodules.
# If ".git" is not found, assume that the complete source code is already available.
if [ -d ".git" ] && [ $(which git) ]; then
    git submodule init
    git submodule update
fi

cmdir=src/editor/libs/codemirror
cmerror="error: CodeMirror submodule not available at \`$cmdir'! Make sure you downloaded the source code using git as specified in README.md."
# check if directory exists
test -d "$cmdir" || (echo "$cmerror" && exit 1)
# check if directory is empty
test "$(ls -A $cmdir)" || (echo "$cmerror" && exit 1)

test -n "$(which autoreconf)" || (echo "autoreconf not found!" && exit 1)
test -n "$(which pkg-config)" || (echo "pkg-config not found!" && exit 1)

autoreconf --install --force "$@"
rm -rf autom4te.cache

