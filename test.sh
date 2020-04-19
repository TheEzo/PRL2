#!/bin/bash

if [ $# -ne 1 ]
then
  echo "Vyzadovan jeden parametr: retezec cisel oddelenych carkou"
  exit 1
fi

numbers=$1
count=$(($(echo $numbers |tr "," "\n" |wc -l) - 1))
echo $numbers > numbers

if [ $count -ge 20 ]
then
  if [ $(($count / 2)) -ge 20 ]
  then
    count=16
  else
    count=8
  fi
else
  if [ $count -eq 16 ]
  then
    count=16
  else
    if [ $count -eq 8 ]
    then
     count=8
    else
      if [ $count -eq 4 ]
      then
        count=4
      else
        if [ $count -eq 2 ]
        then
          count=2
        else
          if [ $count -ge 12 ]
        then
          count=8
        else
          count=4
        fi
        fi
      fi
    fi
  fi
fi

mpic++ --prefix /usr/local/share/OpenMPI -o vid vid.cpp

mpirun --prefix /usr/local/share/OpenMPI -np $count vid
mpirun --prefix /usr/local/share/OpenMPI -np $count lin_vid

rm -f numbers vid
