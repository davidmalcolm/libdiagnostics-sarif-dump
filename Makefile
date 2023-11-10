all: sarif-dump test

clean:
	rm -f sarif-dump

# Hacks while I bring libdiagnostics up
DEV_INCLUDE_PATH=/home/david/coding-3/gcc-newgit-canvas-2023/src/gcc/
DEV_BUILD_PATH=/home/david/coding-3/gcc-newgit-canvas-2023/build/gcc/


CXXFLAGS= -g
DEPS = sarif-dump.cc \
	$(DEV_INCLUDE_PATH)/libdiagnostics.h \
	$(DEV_INCLUDE_PATH)/libdiagnostics++.h \
	$(DEV_BUILD_PATH)/libdiagnostics.so \
	Makefile

sarif-dump: $(DEPS)
	g++ sarif-dump.cc \
	  $(CXXFLAGS) \
	  -o sarif-dump \
	  $(shell pkg-config --libs jsoncpp) \
	  -I $(DEV_INCLUDE_PATH) \
	  -L $(DEV_BUILD_PATH) \
	  -ldiagnostics

test: sarif-dump
	LD_LIBRARY_PATH=$(DEV_BUILD_PATH) ./run-all-tests.sh
