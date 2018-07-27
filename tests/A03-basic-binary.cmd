# Test ability to retrieve binary file
serve s1
generate random-binary.bin 10K
request r1 random-binary.bin s1
delay 100
respond r1
delay 100
check r1
quit


