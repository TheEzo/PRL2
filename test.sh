#!/bin/bash

testing=0

if [ $# -ne 1 ]
then
  echo "Vyzadovan jeden parametr: retezec cisel oddelenych carkou"
  exit 1
fi

numbers=$1
count=$(($(echo $numbers |tr "," "\n" |wc -l) - 1))
echo $numbers > numbers

mpic++ --prefix /usr/local/share/OpenMPI -o vid vid.cpp

if [ $testing -ne 1 ]
then
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

  mpirun --prefix /usr/local/share/OpenMPI -np $count vid
else
  rm -f test.out numbers
  printf "10" >> numbers
  iterations=2
  test_data=(1 2 3 2 10 9 10 15 20 25 30 35 50 55 56 60 65 70 90 95 97 98 99 110 112 115 116 119 140 150 153 155 160 170)
  for i in "${test_data[@]}"
  do
    count=$((iterations-1))
    printf $iterations >> test.out
    printf ","$i >> numbers
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
    for j in {1..30}
    do
      echo $((iterations - 1))
      mpirun --prefix /usr/local/share/OpenMPI -np $count vid
    done
    printf "\n" >> test.out
    iterations=$((iterations+1))
  done
fi

rm -f numbers vid
