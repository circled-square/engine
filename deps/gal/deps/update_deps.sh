#!/bin/bash

SCRIPT_DIR=$(realpath $(dirname "$0"))
DEPS_FILE=$SCRIPT_DIR/deps.txt
DEPS_HASH_CACHE_FILE=$SCRIPT_DIR/.deps_hash_cache

echo script dir           = $SCRIPT_DIR
echo deps file            = $DEPS_FILE
echo deps hash cache file = $DEPS_HASH_CACHE_FILE

if [ ! -f $DEPS_FILE ]; then
    echo "deps file not found! exiting"
    exit
fi

touch $DEPS_HASH_CACHE_FILE # the rest of the script assumes the file exists

echo

while read DEPENDENCY; do
  cd $SCRIPT_DIR

  DEP_NAME=$(echo $DEPENDENCY | sed 's/^\([a-zA-Z0-9_-]*\)[ \t]*;.*$/\1/')
  CLONE_DIR=$DEP_NAME
  CLONE_SRC=$(echo $DEPENDENCY | sed 's/.*;\(.*\)$/\1/')
  echo "$DEP_NAME (from $CLONE_SRC)"

  CLONE_CURR_HASH=$(grep $DEP_NAME $DEPS_HASH_CACHE_FILE | head -n1 | sed 's/.*;\(.*\)/\1/')
  CLONE_REMOTE_HASH=$(git ls-remote $CLONE_SRC | head -n1 | { read first rest ; echo $first ; })

  if [ -d $CLONE_DIR ]; then
    if [ -n "$CLONE_CURR_HASH" ] && [ "$CLONE_CURR_HASH" = "$CLONE_REMOTE_HASH" ]; then
      echo "    hashes match! skipping dependency $DEP_NAME"
      continue
    else
      echo "    hashes do not match! curr=$CLONE_CURR_HASH != remote=$CLONE_REMOTE_HASH"
      echo "    removing old files in $CLONE_DIR..."
      rm -fr -- $CLONE_DIR
    fi
  else
    echo "    dependency $DEP_NAME not found in local tree"
  fi

  echo "    cloning into dir $CLONE_DIR from \"$CLONE_SRC\"..."
  git clone --depth 1 $CLONE_SRC $CLONE_DIR 2>&1 | sed -z 's/^/        git: /'

  echo "    removing .git directory \"$CLONE_DIR/.git/\"..."
  rm -fr -- $CLONE_DIR/.git/

  echo "    updating hash cache..."
  sed -i "/^$DEP_NAME;.*$/d" $DEPS_HASH_CACHE_FILE
  echo -e "$DEP_NAME;$CLONE_REMOTE_HASH" >> $DEPS_HASH_CACHE_FILE
done < $DEPS_FILE

echo
echo Cleaning up...

$SCRIPT_DIR/cleanup.sh

echo
echo All dependencies updated!

