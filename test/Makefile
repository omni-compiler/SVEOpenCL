BLD=$(HOME)
#CUNIT=$(BLD)/CUnit-2.1-3
ABT=$(BLD)/argobots/src/.libs/


CFLAGS=-I$(BLD)/OpenCL-Headers/include -I$(BLD)/socl/include -std=c99 -O2 -DCL_TARGET_OPENCL_VERSION=120
LDFLAGS=-lcunit -L$(BLD)/socl/lib -lsocl -L$(ABT) -labt -lnuma -lstdc++ -ldl -lpthread -lm -lncurses -lz `llvm-config --ldflags` `llvm-config --libs` 

TEST_SRCS=main.c platform_layer.c command_queue.c buffer_object.c program.c kernel.c
TEST_OBJS=$(patsubst %.c, %.o, $(TEST_SRCS))

all: $(TEST_OBJS)
	armclang -O2 $(TEST_OBJS) -o test $(LDFLAGS)

$(TEST_OBJS): $(TEST_SRCS)
	armclang -O2 -c $(TEST_SRCS) $(CFLAGS)

clean:
	rm -f test *.o _SOCL_*.cl _SOCL_*.ll
