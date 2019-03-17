#!/bin/bash

PID=

function trap_ctrlc ()
{
	[[ -z "${PID}" ]] || kill $PID
	exit 2
}

trap "trap_ctrlc" 2

while [ true ]
do
	cc -m32 -Os -ffast-math -mfpmath=387 -march=i486 -lasound -pthread proto.c -lm -o proto
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
