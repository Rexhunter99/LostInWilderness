#############################################################################
# Makefile for Lost in Wilderness
# Original template by whyglinux <whyglinux@gmail.com>
# Modified by TambourineReindeer
#
# The Makefile provides the following targets to make:
#   $ make           compile and link
#   $ make NODEP=yes compile and link without generating dependencies
#   $ make objs      compile only (no linking)
#   $ make tags      create tags for Emacs editor
#   $ make ctags     create ctags for VI editor
#   $ make clean     clean objects and the executable file
#   $ make distclean clean objects, the executable and dependencies
#   $ make cleandeps clean dependecy files only
#   $ make help      get the usage of the makefile
#===========================================================================

# The pre-processor and compiler options.
CFLAGS = -Wall -DGLM_FORCE_RADIANS

# The linker options. Note: lglfw is glfw3 but the filename is the same as
# previous versions so -lglfw3 won't find it unless a symbolic link is made.
# May make a permanent solution to this in the future.
LIBS   = -lpng -lglfw -lm

# The pre-processor options used by the cpp (man cpp for more).
CPPFLAGS  = $(CFLAGS) -std=c++11 -ffast-math
CXXFLAGS  = $(CPPFLAGS)

# The options used in linking as well as in any direct use of ld.
LDFLAGS   = $(LIBS)

# The directories in which source files reside.
# If not specified, only the current directory will be serached.
SRCDIRS   =

# The target name and architecture/OS checks.
# If the name is not specified, current directory name or `a.out' will be used.

PROGRAMD   = bin/gaiacraft-dbg
PROGRAMR   = bin/gaiacraft
PROGRAM = $(PROGRAMD) $(PROGRAMR)

ifeq ($(OS),Windows_NT)
    CFLAGS     += -D_WIN32 -DGLEW_STATIC -D_WINDOWS
    LIBS       += -lglew32s -lopengl32 -lgdi32 -luser32 -lkernel32 -lws2_32
    ifeq ($(or $(PROCESSOR_ARCHITECTURE),$(PROCESSOR_ARCHITEW6432)),AMD64)
        CFLAGS += -D_AMD64
    endif
    ifeq ($(PROCESSOR_ARCHITECTURE),x86)
        CFLAGS += -D_IA32
    endif
else
    UNAME_S := $(shell uname -s)
    LIBS       += -lGL -lGLEW -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -lpthread
    ifeq ($(UNAME_S), Linux)
        CFLAGS += -D_LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
        CFLAGS += -D_OSX
    endif
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
        CFLAGS += -D_AMD64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        CFLAGS += -D_IA32
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        CFLAGS += -D_ARM
    endif
endif

$(PROGRAMD): CPPFLAGS += -g -pg -O0 -D_DEBUG -Q -v
$(PROGRAMD): LIBS += -pg
$(PROGRAMR): CPPFLAGS += -O3 -O6 -DNDEBUG
$(PROGRAMR): LIBS += -s

# Compiler
CC     = gcc
CXX    = g++

# The source file types (headers excluded).
SRCEXTS = .cpp .CPP

# The header file types.
HDREXTS = .h .H

ETAGS = etags
ETAGSFLAGS =

CTAGS = ctags
CTAGSFLAGS =

## Stable Section: usually no need to be changed. But you can add more.
##==========================================================================
SHELL   = /bin/sh
EMPTY   =
SPACE   = $(EMPTY) $(EMPTY)
ifeq ($(PROGRAM),)
  CUR_PATH_NAMES = $(subst /,$(SPACE),$(subst $(SPACE),_,$(CURDIR)))
  PROGRAM = $(word $(words $(CUR_PATH_NAMES)),$(CUR_PATH_NAMES))
  ifeq ($(PROGRAM),)
    PROGRAM = a.out
  endif
endif
ifeq ($(SRCDIRS),)
  SRCDIRS = .
endif
SOURCES = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))
HEADERS = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(HDREXTS))))
SRC_CXX = $(filter-out %.c,$(SOURCES))
OBJS    = $(addsuffix .o, $(basename $(SOURCES)))
DEPS    = $(OBJS:.o=.d)

## Define some useful variables.
DEP_OPT = $(shell if `$(CC) --version | grep "GCC" >/dev/null`; then \
                  echo "-MM -MP"; else echo "-M"; fi )
DEPEND      = $(CC)  $(DEP_OPT) $(CFLAGS) $(CPPFLAGS)
DEPEND.d    = $(subst -g ,,$(DEPEND))
COMPILE.c   = $(CC)  $(MY_CFLAGS) $(CFLAGS)   $(CPPFLAGS) -c
COMPILE.cxx = $(CXX) $(MY_CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c
LINK.c      = $(CC)  $(MY_CFLAGS) $(CFLAGS)   $(CPPFLAGS) $(LDFLAGS)
LINK.cxx    = $(CXX) $(MY_CFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS)

.PHONY: all objs tags ctags clean distclean cleandeps help show
.INTERMEDIATE: $(OBJS)

# Delete the default suffixes
.SUFFIXES:

all: $(PROGRAM)

# Rules for creating dependency files (.d).
#------------------------------------------

%.d:%.cpp
	@echo -n $(dir $<) > $@
	@$(DEPEND.d) $< >> $@

%.d:%.CPP
	@echo -n $(dir $<) > $@
	@$(DEPEND.d) $< >> $@

# Rules for generating object files (.o).
#----------------------------------------
objs:$(OBJS)

%.o:%.cpp
	$(COMPILE.cxx) $< -o $@

%.o:%.CPP
	$(COMPILE.cxx) $< -o $@

# Rules for generating the tags.
#-------------------------------------
tags: $(HEADERS) $(SOURCES)
	$(ETAGS) $(ETAGSFLAGS) $(HEADERS) $(SOURCES)

ctags: $(HEADERS) $(SOURCES)
	$(CTAGS) $(CTAGSFLAGS) $(HEADERS) $(SOURCES)

# Rules for generating the executable.
#-------------------------------------
$(PROGRAM):$(OBJS)
ifeq ($(SRC_CXX),)              # C program
	$(LINK.c)   $(OBJS) $(LIBS) -o $@
	@echo Type ./$@ to execute the program.
else                            # C++ program
	$(LINK.cxx) $(OBJS) $(LIBS) -o $@
	@echo Type ./$@ to execute the program.
endif

ifndef NODEP
ifneq ($(DEPS),)
  sinclude $(DEPS)
endif
endif

clean:
	$(RM) $(OBJS) $(PROGRAM)

distclean: clean
	$(RM) $(DEPS) TAGS

cleandeps:
	$(RM) $(DEPS)

# Show help.
help:
	@echo 'Usage: make [TARGET]'
	@echo 'TARGETS:'
	@echo '  all       (=make) compile and link.'
	@echo '  NODEP=yes make without generating dependencies.'
	@echo '  objs      compile only (no linking).'
	@echo '  tags      create tags for Emacs editor.'
	@echo '  ctags     create ctags for VI editor.'
	@echo '  clean     clean objects and the executable file.'
	@echo '  cleandeps clean dependecy files only'
	@echo '  distclean clean objects, the executable and dependencies.'
	@echo '  show      show variables (for debug use only).'
	@echo '  help      print this message.'

# Show variables (for debug use only.)
show:
	@echo 'PROGRAM     :' $(PROGRAM)
	@echo 'SRCDIRS     :' $(SRCDIRS)
	@echo 'HEADERS     :' $(HEADERS)
	@echo 'SOURCES     :' $(SOURCES)
	@echo 'SRC_CXX     :' $(SRC_CXX)
	@echo 'OBJS        :' $(OBJS)
	@echo 'DEPS        :' $(DEPS)
	@echo 'DEPEND      :' $(DEPEND)
	@echo 'COMPILE.c   :' $(COMPILE.c)
	@echo 'COMPILE.cxx :' $(COMPILE.cxx)
	@echo 'link.c      :' $(LINK.c)
	@echo 'link.cxx    :' $(LINK.cxx)

## End of the Makefile ##  Suggestions are welcome  ## All rights reserved ##
#############################################################################
