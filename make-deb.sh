#! /bin/bash

src_dir=$1

if [[ -z $src_dir ]]; then
    echo "Expected arguments: <source dir>"
    exit 1
fi

set -e
set -x

version=$(cat "$src_dir/VERSION")
arch=$(dpkg-architecture -q DEB_TARGET_ARCH)

# Build contents
mkdir -p install
cmake -D CMAKE_INSTALL_PREFIX="$(pwd)/install/usr" "$src_dir"
make install

# Fill in variables in the control file
cp -r --no-target-directory "$src_dir/debian" install/DEBIAN
sed --in-place "s/@VERSION@/$version/" install/DEBIAN/control
sed --in-place "s/@ARCHITECTURE@/$arch/" install/DEBIAN/control

# Make package
dpkg-deb --build install arrp_${version}_${arch}.deb
