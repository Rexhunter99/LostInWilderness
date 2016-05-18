#!/bin/bash
# Acquire all development libraries that are needed to build the project

## Install cURL and Wget (if not already installed)
apt-get install curl wget

##
LIW_CWD=$(pwd) # Current working directory
LIBGLFW3_VER=$(curl --silent https://api.github.com/repos/glfw/glfw/releases | grep tag_name | head --lines=1 | cut --delimiter='"' -f 4)
LIBGLFW3_FILENAME="glfw-$LIBGLFW3_VER" #.tar.gz
LIBGLFW3_SRC=$(curl --silent https://api.github.com/repos/glfw/glfw/releases | grep tarball_url | head --lines=1 | cut --delimiter='"' -f 4)

LIBPNG_VER=${$(curl --silent ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng16/ | awk '/sortable_data/ && /*.tar.xz$/' | head --lines=1 | cut --delimiter='"' -f 2):1}
LIBPNG_FILENAME="libpng-$LIBPNG_VER" #.tar.xz
LIBPNG_SRC=$(curl --silent ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng16/ | awk '/sortable_data/ && /*.tar.xz$/' | head --lines=1 | cut --delimiter='"' -f 8)

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
wget --quiet --directory-prefix=/tmp --output-document=$LIBPNG_FILENAME.tar.xz $LIBPNG_SRC
echo Extracting source-code from archive $LIBPNG_FILENAME.tar.xz
tar -xJf /tmp/$LIBPNG_FILENAME.tar.xz
cd /tmp/$LIBPNG_FILENAME
### Configure, make and install the library
./configure
make check
make install
cp /usr/local/lib/libpng16.so.16 /usr/lib/libpng16.so.16
cd $LIW_CWD

echo Cleaning up...
rm -r /tmp/$LIBGLFW3_FILENAME
rm -r /tmp/$LIBPNG_FILENAME
rm /tmp/$LIBGLFW3_FILENAME.tar.gz*
# Note: if using xz (or bzip2), does this automatically on successful decompress unless -k or --keep is specified
rm /tmp/$LIBPNG_FILENAME.tar.xz*
