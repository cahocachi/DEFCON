#!/bin/bash

cd "`dirname "$(readlink -f "$0")"`"

LD_LIBRARY_PATH=/usr/local/lib:/usr/lib:./lib ./defcon.bin "$@"

