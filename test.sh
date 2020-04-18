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

if [ $count -le 20 ]
then
  mpirun --prefix /usr/local/share/OpenMPI -np $((count-1)) vid
else
  mpirun --prefix /usr/local/share/OpenMPI -np 20 vid
fi
