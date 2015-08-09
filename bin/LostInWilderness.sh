#!/bin/bash

MACHINE_ARCH=$(uname -p)

export LD_LIBRARY_PATH="/usr/local/lib"

if [ $MACHINE_ARCH = "x86_64" ]
then
	# Run the 64-Bit ELF executable
	echo "64-Bit Architecture" 
	./x64/liw
elif [ $MACHINE_ARCH = "x86" ]
then
	# Run the 32-Bit ELF executable
	echo "32-Bit Architecture"
	./x86/liw
else
	echo "Unknown system architecture!"
fi

