BLD=$(HOME)
#CUNIT=$(BLD)/CUnit-2.1-3
ABT=$(BLD)/argobots/src/.libs/

CC = armclang


#CCFLAGS=-std=c99
CFLAGS=-I$(BLD)/OpenCL-Headers/include -I$(BLD)/socl/include -std=c99 -O2 -DCL_TARGET_OPENCL_VERSION=120
LDFLAGS=-lcunit -L$(BLD)/socl/lib -lsocl -lnuma -L$(ABT) -labt -lstdc++ -ldl -lpthread -lm -lncurses -lz `llvm-config --ldflags` `llvm-config --libs`


LIBS = -lsocl



# Check our platform and make sure we define the APPLE variable
# and set up the right compiler flags and libraries
PLATFORM = $(shell uname -s)
ifeq ($(PLATFORM), Darwin)
	LIBS = -framework OpenCL
endif

DeviceInfo: DeviceInfo.c
	$(CC) $^ $(CFLAGS) $(LDFLAGS)  -o $@


clean:
	rm -f DeviceInfo
