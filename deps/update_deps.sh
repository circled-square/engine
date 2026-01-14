#!/bin/bash

SCRIPT_DIR=$(realpath $(dirname "$0"))
DEPS_FILE=$SCRIPT_DIR/deps.txt
DEPS_HASH_CACHE_FILE=$SCRIPT_DIR/.deps_hash_cache

echo script dir           = $SCRIPT_DIR
echo deps file            = $DEPS_FILE
echo deps hash cache file = $DEPS_HASH_CACHE_FILE

# TODO: option to silence git-clone output
# TODO: git-clone output should be indented

if [ ! -f $DEPS_FILE ]; then
    echo "deps file not found! exiting"
    exit
fi

touch $DEPS_HASH_CACHE_FILE # the rest of the script assumes the file exists

echo

while read DEPENDENCY; do
  cd $SCRIPT_DIR

  # unpacks two semicolon-separated strings and saves them to \1 and \2
  UNPACK_REGEX='^\s*([^ ]*)\s*;\s*(.*)\s*$'

  DEP_NAME=$(echo $DEPENDENCY | sed -E "s/$UNPACK_REGEX/\1/")
  CLONE_DIR=$DEP_NAME
  CLONE_SRC=$(echo $DEPENDENCY | sed -E "s/$UNPACK_REGEX/\2/")
  echo "$DEP_NAME (from $CLONE_SRC)"

  CLONE_CURR_HASH=$(grep $DEP_NAME $DEPS_HASH_CACHE_FILE | head -n1 |  sed -E "s/$UNPACK_REGEX/\2/")
  CLONE_REMOTE_HASH=$(git ls-remote $CLONE_SRC | { read first rest ; echo $first ; })

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
  git clone --depth 1 $CLONE_SRC $CLONE_DIR | sed -E 's/(.*)/        git: \1/'

  echo "    removing .git directory \"$CLONE_DIR/.git/\"..."
  rm -fr -- $CLONE_DIR/.git/

  echo "    updating hash cache..."
  if grep $DEP_NAME $DEPS_HASH_CACHE_FILE; then
    sed -i -E "s/^$DEP_NAME;.*$/$DEP_NAME;$CLONE_REMOTE_HASH/" $DEPS_HASH_CACHE_FILE
  else
    echo -e "$DEP_NAME;$CLONE_REMOTE_HASH" >> $DEPS_HASH_CACHE_FILE
  fi
done < $DEPS_FILE

echo
echo Cleaning up...

$SCRIPT_DIR/cleanup.sh

echo
echo All dependencies updated!

