#! /bin/bash

src_dir=$1
platform=$2

if [[ -z $src_dir || -z $platform ]]; then
    echo "Expected arguments: <source dir> <platform>"
    exit 1
fi

set -e
set -x

version=$(cat "$src_dir/VERSION")
pakage_name=arrp_${version}_${platform}

# Build contents
mkdir -p $pakage_name
cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX="$(pwd)/$pakage_name" "$src_dir"
make install

# Make ZIP
zip -r $pakage_name.zip $pakage_name/
