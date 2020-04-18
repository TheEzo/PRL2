#!/bin/bash

if [ $# -ne 1 ]
then
  echo "Vyzadovan jeden parametr: retezec cisel oddelenych carkou"
  exit 1
fi

numbers=$1
count=$(echo $numbers |tr "," "\n" |wc -l)

#echo $count
echo $numbers > numbers

mpic++ --prefix /usr/local/share/OpenMPI -o vid vid.cpp
# TODO spocita cpu count
if [ $count -le 20 ]
then
  mpirun --prefix /usr/local/share/OpenMPI -np 4 vid #$((count-1)) vid
  mpirun --prefix /usr/local/share/OpenMPI -np 4 lin_vid #$((count-1)) lin_vid
else
  mpirun --prefix /usr/local/share/OpenMPI -np 20 vid
  mpirun --prefix /usr/local/share/OpenMPI -np 20 lin_vid
fi

#rm -f numbers vid