LDFLAGS=-lGL -lX11 -lXfixes -lasound -pthread -lm
INCLUDES=-Iatto/include

all:
	make run
	make clean
	make -k all

run: proto
	./$? &
	echo $! > ./.pid
	inotifywait -e modify -e delete -e move proto.c
	kill -9 `cat ./.pid`
	rm ./.pid

proto.c.o atto/src/app_linux.c.o atto/src/app_x11.c.o: proto.c atto/src/app_linux.c atto/src/app_x11.c
	gcc -c ${INCLUDES} `echo $@ | sed 's/\.o//g'` -o $@

clean:
	rm proto.c.o proto

proto: proto.c.o atto/src/app_linux.c.o atto/src/app_x11.c.o
	gcc ${LDFLAGS} $? -o proto
