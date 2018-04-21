#!/bin/bash

export CPU=x86_64
export OS=linux
export VARIANT=debug
export ROOTPATH=/home/tylor/src
export ALLJOYN_ROOT=$ROOTPATH/alljoyn
export AJ_ROOT=$ALLJOYN_ROOT
export LD_LIBRARY_PATH=$AJ_ROOT/build/linux/$CPU/$VARIANT/dist/cpp/lib:$LD_LIBRARY_PATH

#run alljoyn router
killall alljoyn-daemon
sleep 1
$AJ_ROOT/build/linux/$CPU/$VARIANT/dist/cpp/bin/alljoyn-daemon --config-file=./rn-config.xml &

#run app
make -C ../cpp
./../cpp/bin/Debug/EMS

