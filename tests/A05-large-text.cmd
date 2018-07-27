# Test ability to retrieve 1MB text file
serve s1
generate long-text.txt 1M
request r1 long-text.txt s1
delay 100
respond r1
delay 200
check r1
delete long-text.txt
quit

