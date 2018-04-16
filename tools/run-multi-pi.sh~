#!/bin/bash

num="$1"

export CPU=x86
export OS=linux
export VARIANT=debug
export ROOTPATH=$HOME/Src
export ALLJOYN_ROOT=$ROOTPATH/alljoyn_src/core/alljoyn
export AJ_ROOT=$ALLJOYN_ROOT
export LD_LIBRARY_PATH=$AJ_ROOT/build/linux/$CPU/$VARIANT/dist/cpp/lib:$LD_LIBRARY_PATH

make -C ../cpp

if [ $num == "" ]
then
	echo You must enter the number of clients
else
	n=0; while (( n++ < $num ))
	do
		echo running new chat client
		../cpp/bin/Debug/DMS $num &>/dev/null &disown
		sleep 1
	done
fi
