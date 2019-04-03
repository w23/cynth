#!/bin/bash

set -x

CYNTH_DIR=$(dirname $0)
CYNTH=$1
if [[ -z "${CYNTH}" ]]
then
	echo "Usage: $0 source-to-monitor.c"
	exit 1
fi

PID=

function trap_ctrlc ()
{
	echo "Exit requested"
	[[ -z "${PID}" ]] || kill $PID
	exit 2
}

trap "trap_ctrlc" 2

CFLAGS="-ggdb3 -Wall -I$CYNTH_DIR/atto/include -I$CYNTH_DIR"
SOURCES="$CYNTH_DIR/protovis.c $CYNTH_DIR/atto/src/app_linux.c $CYNTH_DIR/atto/src/app_x11.c"

while [ true ]
do
	cc $CFLAGS -lGL -lX11 -lXfixes -lasound -pthread -lm "$CYNTH" $SOURCES -o "$CYNTH.exe"
	if [ $? -eq 0 ]
	then
		./"$CYNTH".exe &
		PID=$!
		inotifywait -e modify -e delete -e move "$CYNTH"
		kill $PID
		PID=
	else
		inotifywait -e modify -e delete -e move "$CYNTH"
	fi
done
