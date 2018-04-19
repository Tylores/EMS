#!/bin/bash

export CPU=x86_64
export OS=linux
export VARIANT=debug
export ROOTPATH=/home/tylor/src
export ALLJOYN_ROOT=$ROOTPATH/alljoyn
export AJ_ROOT=$ALLJOYN_ROOT
export LD_LIBRARY_PATH=$AJ_ROOT/build/linux/$CPU/$VARIANT/dist/cpp/lib:$LD_LIBRARY_PATH

#start stand-alone AllJoyn Router
tmux new-window -d ./$AJ_ROOT/build/linux/$CPU/$VARIANT/dist/cpp/bin/alljoyn-daemon --config-file=./rn-config.xml

#compile code and run
make -C ../cpp
./../cpp/bin/Debug/EMS

