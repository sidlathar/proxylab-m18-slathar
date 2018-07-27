#!/usr/bin/env bash
dir=$(pwd)
MAX_BASIC=12
MAX_CONCURRENCY=10
MAX_CACHE=10
BASIC_WEIGHT=1
CONCURRENCY_WEIGHT=1
CACHE_WEIGHT=1

if [ -d "temp" ]; then rm -r temp; fi
mkdir temp
cp proxy temp
if [ $# -eq 0 ]; then
  src/pxyregress.py -p temp/proxy -l results.log
  #rm -r response_files
  #rm -r source_files
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
else
  src/pxydrive.py -p temp/proxy -f $1
  #rm -r response_files
  #rm -r source_files
fi
rm -r temp
exit
