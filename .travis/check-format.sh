#!/bin/bash
gcf_cmd="$(which git-clang-format-6.0) --binary $(which clang-format-6.0)"
if [ "$TRAVIS_PULL_REQUEST" == "false" ]; then
    base_commit="HEAD^"
    echo "Running clang-format against parent commit $(git rev-parse $base_commit)"
else
    base_commit="$TRAVIS_BRANCH"
    echo "Running clang-format against branch $base_commit, with hash $(git rev-parse $base_commit)"
fi
output="$($gcf_cmd --commit $base_commit --diff --extensions 'cpp,h')"
if [ "$output" == "no modified files to format" ] || [ "$output" == "clang-format did not modify any files" ]; then
    echo "clang-format passed."
    exit 0
else
    echo "clang-format failed:"
    echo "$output"
    exit 1
fi
