#!/bin/sh
#
# A hook script to run Mike Vanier's style guide on all C files.

# Ignore code that isn't part of the prospective commit
git stash -q --keep-index
# TODO: Add test script
# ./run_tests.sh

FILES_PATTERN=$(find -regex '.*/.*\.\(c\|h\)$')
RESULT=$(c_style_check $FILES_PATTERN)
echo $RESULT

git stash pop -q
if [ -n "$RESULT" ]; then
  echo "failed"
  exit 1
else
  exit 0
fi
