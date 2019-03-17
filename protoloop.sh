#!/bin/bash

while [ true ]
do
	sleep 1
	cc -m32 -Os -ffast-math -mfpmath=387 -march=i486 -lasound -pthread proto.c -lm -o proto
	if [ $? -eq 0 ]
	then
		#./proto & # - | playsound --rate 20000 --format U8 --channels 1 --stdin RAW &
		./proto - | playsound --rate 20000 --format U8 --channels 1 --stdin RAW &
		PID=$!
		inotifywait -e modify -e delete -e move proto.c
		kill $PID
	fi
done
