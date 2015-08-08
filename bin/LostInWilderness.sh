#!/bin/bash

if(bash uname -p == x86_64)
{
# Run the 64-Bit ELF executable
	echo "64-Bit Architecture" 
	./x64/liw
}
else
{
# Run the 32-Bit ELF executable
	echo "32-Bit Architecture"
	./x86/liw
}
