# Lost In Wilderness
An open world, voxel-based sandbox game about surviving in the wilderness, beginning with only your wits and the will to survive.

===================================  
**Building the binaries:**

Windows:    
To be added
  
-----------------------  
Mac OS X:  
To be added
  
--------------------  
Ubuntu-based Linux distros:  
Use your preferred package manager (most likely apt-get or aptitude) and install the following dependencies:  
libpng12-dev  
libglfw3-dev  
libglew-dev  
libglm-dev  
libx11-dev  
libxi-dev  
libxrandr-dev  
libxxf86vm-dev  
libxinerama-dev  
libxcursor-dev  
gcc

If you wish to debug:  
gdb  
libcc1  
   
Then either install:  
codeblocks  
*(Optional)*  
codeblocks-contrib  
codeblocks-dbg  

Or:  
make  
  
With Code::Blocks, just open the included lost_in_wilderness.cbp project file and build either the "Debug" or "Release" version (selectable from a drop-down menu at the top). The build icon should be a gear in the toolbar near the top.  
  
If using Make, open a terminal and use cd to go into the source directory. Alternately, open a terminal window in the source directory.  
Then type:  
make  
  
The project will begin building and will output information to the terminal. When done, find the built binaries in LostInWilderness/bin/ as gaiacraft and gaiacraft-dbg.  
You can also type:  
make -j N  
(where N is the number of cores)  
to speed the build process.      
    
-----------------------  
To use the debug binary, have GDB installed and run it like this:
gdb gaiacraft-dbg

While GDB is running, enter:  
run

If GDB catches any signal, such as SIGABORT or SIGSEGV, type these commands for useful debugging information:  
where  
list

'where' will print the call stack from when the signal occurred.  
'list' will print a few lines before and after the line it stopped on, in addition to that line.

To quit GDB, type 'quit' and then 'y' when it asks whether to terminate the running process.

*-TambourineReindeer*
