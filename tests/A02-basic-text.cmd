# Test ability to retrieve text file
serve s1
generate random-text.txt 10K
request r1 random-text.txt s1
delay 100
respond r1
delay 100
check r1
quit

