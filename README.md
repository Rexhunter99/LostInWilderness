# Lost In Wilderness
An open world, voxel-based sandbox game about surviving in the wilderness, beginning with only your wits and the will to survive.

===================================  
## How to build the binaries

### Windows
The project can be compiled on Windows via MSVC or MinGW GCC compilers,
MSVC 14.0 is the current compiler from Microsoft and is completely free as part of their Visual Studio Community release.
MinGW GCC is an open-source third-party project to build Windows binaries with the GCC compiler.

To build on either compiler you will need to acquire the appropriate libraries for them (.lib for MSVC and .a for GCC)
A suitable debugger is also recommended, CDB comes with the Windows Kit 8.0 (also free), where as GDB typically comes
with MinGW GCC.

C++11 standards are required for this project as it uses move constructors and various other newer technologies introduced
by the C++11 standard.  MSVC 14.0 is the first compiler to fully implement the standard for Microsoft, GCC has implemented
the standard since early builds of GCC 4.x

Once your preferred development environment is installed and configured you will need to acquire the appropriate libraries:
* libpng12 (or higher)
* libglfw3
* libglew
_For legal reasons we cannot provide a 'pack' with these libraries_

Additionally you will need to acquire the GL Math library which is a C++ header include.

... _To be completed_ ...

**-Rexhunter99**

### Mac OS X 
To be added
  
-----------------------  
### Ubuntu-based Linux distributions
**Note:** Ubuntu information may also be applied to Debian distributions as Ubuntu itself is based on Debian at its core.
Please beware however and make sure you change any steps as appropriate for your particular Linux environment.

Use your preferred package manager (most likely apt-get or aptitude) and install the following dependencies:
* libpng12-dev
* libglfw3-dev
* libglew-dev  
* libglm-dev  
* libx11-dev  
* libxi-dev  
* libxrandr-dev  
* libxxf86vm-dev  
* libxinerama-dev  
* libxcursor-dev  
* gcc

If you wish to debug:
* gdb  
* libcc1  
   
Then either install:
* codeblocks
* codeblocks-contrib _Optional_
* codeblocks-dbg _Optional_
Or:
* make  
  
With Code::Blocks, just open the included lost_in_wilderness.cbp project/workspace file and build either the "Debug" or "Release" version (selectable from a drop-down menu at the top). The build icon should be a gear in the toolbar near the top.  
  
If using Make, open a terminal and use cd to go into the source directory. Alternately, open a terminal window in the source directory.  
Then type:  
make  
  
The project will begin building and will output information to the terminal. When done, find the built binaries in LostInWilderness/bin/ as gaiacraft and gaiacraft-dbg.  
You can also type:
`make -j N`
(where N is the number of cores)  
to speed the build process.      
    
-----------------------  
To use the debug binary, have GDB installed and run it like this:
gdb gaiacraft-dbg

While GDB is running, enter:  
run

If GDB catches any signal, such as SIGABORT or SIGSEGV, type these commands for useful debugging information:  
* where  
* list

'where' will print the call stack from when the signal occurred.  
'list' will print a few lines before and after the line it stopped on, in addition to that line.

To quit GDB, type 'quit' and then 'y' when it asks whether to terminate the running process.

**-TambourineReindeer**
