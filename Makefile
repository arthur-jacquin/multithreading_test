SRC = 1_sequential.c 2_multithreaded.c 3_queue.c 4_cond.c
LIB = 4_pthread_queue.c
HEADERS = 4_pthread_queue.h termbox2.h
OBJ = ${SRC:.c=.o}
LIBOBJ = ${LIB:.c=.o} termbox2.o
EXE = ${SRC:.c=}

all: ${EXE}

.PHONY: all

clean:
	rm -f ${EXE} ${OBJ} ${LIBOBJ} multithreading_test.tar.gz

.PHONY: clean

dist: ${SRC} ${LIB} ${HEADERS} Makefile multithreading_test.html
	ls $^
	mkdir -p multithreading_test
	cp $^ multithreading_test
	tar -cf multithreading_test.tar multithreading_test
	gzip multithreading_test.tar
	rm -rf multithreading_test

.PHONY: dist

termbox2.o: termbox2.h
	cc -o $@ -c -DTB_IMPL $<

${SRC:.c=.o} ${LIB:.c=.o}: %.o: %.c
	cc -o $@ -c $<

${EXE}: %: %.o ${LIBOBJ}
	cc -o $@ $< ${LIBOBJ}
