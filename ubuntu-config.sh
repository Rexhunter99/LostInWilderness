#!/bin/bash
# Acquire all development libraries that are needed to build the project

## Install cURL and Wget (if not already installed)
apt-get install curl wget

##
LIW_CWD=$(pwd) # LIW working directory
LIBGLFW3_VER=$(curl --silent https://api.github.com/repos/glfw/glfw/releases | grep tag_name | head --lines=1 | cut --delimiter='"' -f 4)
LIBGLFW3_FILENAME="glfw-$LIBGLFW3_VER"
LIBGLFW3_SRC=$(curl --silent https://api.github.com/repos/glfw/glfw/releases | grep tarball_url | head --lines=1 | cut --delimiter='"' -f 4)


LIBPNG_SRC="ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng16/"
# This will output something like "libpng-1.6.21.tar.xz"
LIBPNG_FILENAME=$(curl --silent "$LIBPNG_SRC" | grep 'tar.xz$' | awk '{print $9}')
LIBPNG_SRC+="$LIBPNG_FILENAME"
# Remove extension to use as folder name
LIBPNG_NAME="${LIBPNG_FILENAME%%'.tar.xz'}"

## Install Build Utilities
apt-get install cmake

## Install X Org/X11 libraries
apt-get install xorg-dev libx11-dev libxi-dev libxrandr-dev libxxf86vm-dev libxinerama-dev libxcursor-dev

## Install OpenGL libraries and utility libraries
apt-get install libgl1-mesa-dev libglu1-mesa-dev libglew-dev libglm-dev

## Install Third Party libraries
# Note: this script will download and configure the libpng16 library
#apt-get install libpng12-dev

## Install Archive Utilities
apt-get install xz-utils

## We have to get the GLFW3 library
echo Getting the GLFW3 source-code
wget --quiet --directory-prefix=/tmp --output-document=$LIBGLFW3_FILENAME.tar.gz $LIBGLFW3_SRC
echo Extracting source-code from archive $LIBGLFW3_FILENAME.tar.gz
tar -xzf /tmp /tmp/$LIBGLFW3_FILENAME.tar.gz
cd /tmp/$LIBGLFW3_FILENAME
### Make the project files for Unix specifically
cmake -G "Unix Makefiles"
make
make install

cd $LIW_CWD

## We also have to get the libPNG-1.6 library manually
echo Getting the libPNG source-code
wget --quiet --directory-prefix=/tmp $LIBPNG_SRC
echo Extracting source-code from archive $LIBPNG_FILENAME
tar -xJf /tmp/$LIBPNG_FILENAME
cd /tmp/$LIBPNG_NAME
### Configure, make and install the library
./configure
make check
make install
cp /usr/local/lib/libpng16.so.16 /usr/lib/libpng16.so.16
cd $LIW_CWD

echo Cleaning up...
rm -r /tmp/$LIBGLFW3_FILENAME
rm -r /tmp/$LIBPNG_NAME
rm /tmp/$LIBGLFW3_FILENAME.tar.gz*
# Note: if using xz (or bzip2), does this automatically on successful decompress unless -k or --keep is specified
rm /tmp/$LIBPNG_FILENAME*
