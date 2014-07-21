#!/bin/bash
# benchmark all the programs
# q01 - q05
for t in {1..16}
do
    mkdir -p ${t}
    echo ${t}
    for  n in 1000000 10000000 100000000 1000000000
    do
        (time ./q01 ${t} ${n}) > ${t}/q01_${n}.out 2> ${t}/q01_${n}.time
        (time ./q02 ${t} ${n}) > ${t}/q02_${n}.out 2> ${t}/q02_${n}.time
        (time ./q03 ${t} ${n}) > ${t}/q03_${n}.out 2> ${t}/q03_${n}.time
        (time ./q04 ${t} ${n}) > ${t}/q04_${n}.out 2> ${t}/q04_${n}.time
        (time ./q05 ${t} ${n}) > ${t}/q05_${n}.out 2> ${t}/q05_${n}.time
    done 

done 
