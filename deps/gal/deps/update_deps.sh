#!/bin/bash

SCRIPT_DIR=$(realpath $(dirname "$0"))
DEPS_FILE=$SCRIPT_DIR/deps.txt

echo script dir = $SCRIPT_DIR
echo deps file  = $DEPS_FILE

while read DEPENDENCY; do
  cd $SCRIPT_DIR

  CLONE_DIR=$(echo $DEPENDENCY | sed 's/^\([a-zA-Z0-9_-]*\)[ \t]*;.*$/\1/')
  CLONE_SRC=$(echo $DEPENDENCY | sed 's/.*;\(.*\)$/\1/')

  echo removing old files in $CLONE_DIR
  rm -fr -- $CLONE_DIR

  echo cloning into dir $CLONE_DIR from $CLONE_SRC
  git clone --depth 1 $CLONE_SRC $CLONE_DIR

  echo removing .git directory $CLONE_DIR/.git/
  rm -fr -- $CLONE_DIR/.git/
done < $DEPS_FILE

echo
echo Cleaning up...

$SCRIPT_DIR/cleanup.sh

echo && echo
echo All dependencies updated!

