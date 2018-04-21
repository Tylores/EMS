#!/bin/bash

file="/home/tylor/dev/data/PClog.txt"
echo "MEM, CPU" > $file

while(true); do
	MEM=$(free -m | awk 'NR==2{printf "%.2f%%, ", $3*100/$2}')
	CPU=$(top -bn1 | grep load | awk '{printf "%.2f%%", $(NF-2)}')
	echo "$MEM$CPU" >> $file
	sleep 1
done
