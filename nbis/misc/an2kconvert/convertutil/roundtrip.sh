#!/bin/sh
reset
ID=$1
OD=$2

for f in `ls $ID`
do
	echo "$ ./an2kconvert -co $ID/$f $OD/$f.xml"
	./an2kconvert -co "$ID/$f" "$OD/$f.xml"
	echo "\n"
	echo "$ xmllint --noout --schema schemas/ITL-2008-Package-Annex-B.xsd $OD/$f.xml"
	xmllint --noout --schema schemas/ITL-2008-Package-Annex-B.xsd "$OD/$f.xml"
	echo "\n"
	echo "$ ./an2kconvert -co $OD/$f.xml $OD/$f.xml.an2"
	./an2kconvert -co "$OD/$f.xml" "$OD/$f.xml.an2"
	echo "\n"
done

