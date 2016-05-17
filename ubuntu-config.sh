#!/bin/bash
# Acquire all development libraries that are needed to build the project

## Install cURL and Wget (if not already installed)
apt-get install curl wget

##
LIW_CWD=$(pwd) # Current working directory
LIBGLFW3_VER=$(curl -s https://api.github.com/repos/glfw/glfw/releases | grep tag_name | head --lines=1 | cut --delimiter='"' --fields=4)
LIBGLFW3_FILENAME=glfw-$LIBGLFW3_VER #.tar.gz
LIBGLFW3_SRC=$(curl -s https://api.github.com/repos/glfw/glfw/releases | grep tarball_url | head --lines=1 | cut --delimiter='"' --fields=4)
LIBPNG_DIR="libpng-1.6.21"
LIBPNG_SRC="ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng16/libpng-1.6.21.tar.gz"

## Install Build Utilities
apt-get install cmake

## Install X Org/X11 libraries
apt-get install xorg-dev libx11-dev libxi-dev libxrandr-dev libxxf86vm-dev libxinerama-dev libxcursor-dev

## Install OpenGL libraries and utility libraries
apt-get install libgl1-mesa-dev libglu1-mesa-dev libglew-dev libglm-dev

## Install Third Party libraries
# Note: this script will download and configure the libpng16 library
#apt-get install libpng12-dev

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
wget --quiet --directory-prefix /tmp $LIBPNG_SRC
echo Extracting source-code from tarball
tar -xzf /tmp/libpng-1.6.21.tar.gz -C /tmp
cd /tmp/libpng-1.6.21
### Configure, make and install the library
./configure
make check
make install
cp /usr/local/lib/libpng16.so.16 /usr/lib/libpng16.so.16
cd $LIW_CWD

echo Cleaning up...
rm -r /tmp/$LIBGLFW3_FILENAME
rm -r /tmp/libpng-1.6.21
rm /tmp/$LIBGLFW3_FILENAME.tar.gz*
rm /tmp/libpng-1.6.21.tar.gz*
