#!/bin/bash
# Suponemos que existen file1.txt file2.txt file3.dat y newfile.txt
# y que la utilidad mytar esta en el directorio actual
./mytar -cf filetar.mtar file1.txt file2.txt file3.dat
./mytar -tf filetar.mtar
./mytar -af filetar.mtar newfile.txt
./mytar -tf filetar.new.mtar
mkdir out
cp filetar.mtar out
cd out
../mytar -xf filetar.mtar

