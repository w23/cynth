LDFLAGS=-lGL -lX11 -lXfixes -lasound -pthread -lm
INCLUDES=-Iatto/include

all: run clean
	make -k all

run: proto
	./$? &
	inotifywait -e modify -e delete -e move proto.c
	killall -9 $?

proto.c.o atto/src/app_linux.c.o atto/src/app_x11.c.o: proto.c atto/src/app_linux.c atto/src/app_x11.c
	gcc -c ${INCLUDES} `echo $@ | sed 's/\.o//g'` -o $@

clean: proto.c.o proto
	rm $?

distclean: proto.c.o atto/src/app_linux.c.o atto/src/app_x11.c.o proto
	rm $?

proto: proto.c.o atto/src/app_linux.c.o atto/src/app_x11.c.o
	gcc ${LDFLAGS} $? -o proto
