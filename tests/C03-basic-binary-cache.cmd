# Test ability to retrieve binary file from cache
serve s1
generate random-binary1.bin 10K
generate random-binary2.bin 1K
request r1a random-binary1.bin s1
request r2 random-binary2.bin s1
delay 100
respond r2 r1a
delay 200
check r1a
check r2
request r1b random-binary1.bin s1
delay 200
check r1b
quit


