#!/usr/bin/env make run

# Makefile information:
#
# Dependencies:
#	Libraries: GL, X11, Xfixes, asound, m, pthread
#	Executables: env, echo, rm, echo, sed, gcc
#	Subprojects: git:github.com/w23/atto.git
#
# Produces:
#	Executables: proto
#	Pre-compiled objects: $(BUILDDIR)/proto.c.o
#	Subproject pre-compiled objects: $(BUILDDIR)/atto/src/app_linux.c.o $(BUILDDIR)/atto/src/app_x11.c.o

# Makefile predefined variables:

LDFLAGS :=-lGL -lX11 -lXfixes -lasound -pthread -lm
INCLUDES:=-Iatto/include
BUILDDIR?=build
.SILENT : clean distclean atto/src/app_linux.c atto/src/app_x11.c
.DEFAULT: run

# run (or default) target calls proto target
run: proto

# this targets detects list of compilable files with includings and makes objects
$(BUILDDIR)/proto.c.o $(BUILDDIR)/atto/src/app_linux.c.o $(BUILDDIR)/atto/src/app_x11.c.o: proto.c atto/src/app_linux.c atto/src/app_x11.c
	gcc -c ${INCLUDES} `echo $@ | sed 's/\.o//g'` -o $(BUILDDIR)/$@

# this targets calls git for update submodule atto 
# atto/src/app_linux.c atto/src/app_x11.c:
#	git submodule update --recursive
# Commented for now, causing trouble with changed files in atto/src.

# clean target removes proto.c.o and proto after detection of this files [./protoloop.sh _notcup]
clean: $(BUILDDIR)/proto.c.o proto
	rm $?

# distclean target removes all compiled files and program [called only by self]
distclean: $(BUILDDIR)/proto.c.o $(BUILDDIR)/atto/src/app_linux.c.o $(BUILDDIR)/atto/src/app_x11.c.o proto
	rm $?

# proto target collects and link all object files together in proto program
proto: $(BUILDDIR)/proto.c.o $(BUILDDIR)/atto/src/app_linux.c.o $(BUILDDIR)/atto/src/app_x11.c.o
	gcc ${LDFLAGS} $? -o $@

# TODO:
# 1. Detect why this file not work properly with any what conditions.
# 2. More documentate targets and prettify other.
# 3. Destroy all external scripts for callination of looping if its possible
