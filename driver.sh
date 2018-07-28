#!/usr/bin/env bash

#simple program to accompany Pxydrive. Used by autograder to grade submissions

dir=$(pwd)
MAX_BASIC=60
MAX_CONCURRENCY=30
MAX_CACHE=30
BASIC_WEIGHT=5
CONCURRENCY_WEIGHT=3
CACHE_WEIGHT=3

if [ -d "temp" ]; then rm -r temp; fi
mkdir temp
cp proxy temp
if [ $# -eq 0 ]; then
  src/pxyregress.py -p temp/proxy -t 40 -l results.log
  cd logs
  numPassed=0
  basicScore=0
  concurrencyScore=0
  cacheScore=0
  for file in A*.log; do
    result=$(tac $file |egrep -m 1 .)
    if [ "$result" = "ALL TESTS PASSED" ]; then
      numPassed=`expr ${numPassed} + 1`
    fi
  done
  basicScore=$((numPassed*BASIC_WEIGHT))
  numPassed=0
  for file in B*.log; do
    result=$(tac $file |egrep -m 1 .)
    if [ "$result" = "ALL TESTS PASSED" ]; then
      numPassed=`expr ${numPassed} + 1`
    fi
  done
  concurrencyScore=$((numPassed*CONCURRENCY_WEIGHT))
  numPassed=0
  for file in C*.log; do
    result=$(tac $file |egrep -m 1 .)
    if [ "$result" = "ALL TESTS PASSED" ]; then
      numPassed=`expr ${numPassed} + 1`
    fi
  done
  cacheScore=$((numPassed*CACHE_WEIGHT))
  totalScore=$((basicScore+concurrencyScore+cacheScore))
  cd ..
  echo "{ \"scores\": {\"Basic\":${basicScore}, \"Concurrency\":${concurrencyScore}, \"Caching\":${cacheScore}}, \"scoreboard\": [${totalScore}, ${basicScore}, ${concurrencyScore}, ${cacheScore}]}"
elif [ $1 = "A" ]; then
  src/pxyregress.py -p temp/proxy -s A -l results.log
elif [ $1 = "B" ]; then
  src/pxyregress.py -p temp/proxy -s B -l results.log
elif [ $1 = "C" ]; then
  src/pxyregress.py -p temp/proxy -s C -l results.log
elif [ $1 = "h" ]; then
  echo "Run without arguments to run full Pxyregress test suite"
  echo "Run with argument:"
  echo "'A' to test basic proxy functionality"
  echo "'B' to test concurrency"
  echo "'C' to test cache functionality"
  echo "<path to tracefile> to test a specific trace"
elif [ $1 = "H" ]; then
  echo "Run without arguments to run full Pxyregress test suite"
  echo "Run with argument:"
  echo "'A' to test basic proxy functionality"
  echo "'B' to test concurrency"
  echo "'C' to test cache functionality"
  echo "<path to tracefile> to test a specific trace"
else
  src/pxydrive.py -p temp/proxy -f $1
fi
rm -r temp
exit
