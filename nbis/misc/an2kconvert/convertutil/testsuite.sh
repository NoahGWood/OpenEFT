#!/bin/sh
reset
ID=$1
OD=$2

for f in `ls $ID`
do
	echo "./an2kconvert -co $ID/$f $OD/$f.xml"
	./an2kconvert -co $ID/$f "$OD/$f.xml"
	echo "\n"
done

