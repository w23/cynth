#!/bin/bash

set -x

PID=

function trap_ctrlc ()
{
	[[ -z "${PID}" ]] || kill $PID
	exit 2
}

trap "trap_ctrlc" 2

CFLAGS=-Iatto/include
SOURCES="proto.c atto/src/app_linux.c atto/src/app_x11.c"

while [ true ]
do
	cc $CFLAGS -lGL -lX11 -lXfixes -lasound -pthread -lm $SOURCES -o proto
	if [ $? -eq 0 ]
	then
		./proto &
		PID=$!
		inotifywait -e modify -e delete -e move proto.c
		kill $PID
		PID=
	else
		sleep 1
	fi
done
