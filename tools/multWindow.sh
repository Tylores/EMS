#!/bin/bash

num="$1"

if [ "$num" = "" ];
then
	echo You must enter the number of clients
else
	n=0
	while [ $n -lt $num ];
	do
		tmux new-window -d ./run.sh
		sleep 1
		let n+=1
	done
fi
