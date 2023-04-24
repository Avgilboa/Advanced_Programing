.PHONY: all
all: task _lib

task:	codec.h basic_main.c
	gcc thread_pool.c -o thread_pool -lpthread libCodec.so
_lib: task
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.

.PHONY: clean
clean:
	-rm encoder tester libCodec.so 2>/dev/null
