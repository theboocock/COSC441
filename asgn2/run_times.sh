#!/bin/bash

touch times.txt

./ctime ./q01 2>> times.txt
./ctime ./q02 2>> times.txt
./ctime ./q03 2>> times.txt
./ctime ./q01_100 2>> times.txt
./ctime ./q02_100 2>> times.txt
./ctime ./q03_100 2>> times.txt
