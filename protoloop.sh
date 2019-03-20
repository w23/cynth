#!/usr/bin/env sh

set -x

function notcup() {
	
	if [ -f ./.pid ]
	then if [ -z `cat ./.pid` ]
		then
			rm ./.pid
		else
			kill -9 `cat ./.pid`
			rm ./.pid			
		fi
	fi

	make clean

}

trap notcup 2

while [ true ]
do
	make -k all
	if [ "$?" -eq "0" ]
	then
		./proto &
		echo $! > ./.pid
		inotifywait -e modify -e delete -e move proto.c
		notcup
	else
		exit 0
	fi
done
