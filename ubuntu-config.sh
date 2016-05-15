#!/bin/bash
# Acquire all development libraries that are needed to build the project

##
LIW_CWD=$(pwd)
LIBLGFW3_DIR="glfw-3.1.2"
LIBGLFW3_SRC="https://github.com/glfw/glfw/releases/download/3.1.2/glfw-3.1.2.zip"
LIBPNG_DIR="libpng-1.6.21"
LIBPNG_SRC="ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng16/libpng-1.6.21.tar.gz"

## Install Utilities
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
wget --quiet --directory-prefix /tmp $LIBGLFW3_SRC
echo Extracting source-code from zip
unzip -q -d /tmp /tmp/glfw-3.1.2.zip
cd /tmp/glfw-3.1.2
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
rm -r /tmp/glfw-3.1.2
rm -r /tmp/libpng-1.6.21
rm /tmp/glfw-3.1.2.zip*
rm /tmp/libpng-1.6.21.tar.gz*
