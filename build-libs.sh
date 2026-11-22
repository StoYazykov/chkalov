#!/bin/sh

export CHKALOV=$(pwd)
mkdir -p lib/chkalov/core
echo "Building math library..."
./chkc lib-src/math.chk lib/chkalov/core/math.cvm
