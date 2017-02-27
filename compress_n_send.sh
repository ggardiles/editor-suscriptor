#!/bin/sh

rm -f ./EDSU.2016.tar.gz
tar -cvzf EDSU.2016.tar.gz autores editor/ intermediario/ subscriptor/ memoria.txt
rsync -avz EDSU.2016.tar.gz decompress.sh y16a042@triqui4.fi.upm.es:/homefi/alumnos/y/y16a042/DATSI/SD/EDSU.2016
