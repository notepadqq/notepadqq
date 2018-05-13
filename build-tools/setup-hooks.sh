#!/bin/sh
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo -n "Setting up git hooks..."
# All of our hooks for convenience go here
ln -sf "${DIR}/githooks/pre-commit" "${DIR}/../.git/hooks/pre-commit"
echo "Done."
