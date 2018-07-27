# Test ability to retrieve 1MB binary file
serve s1
generate big-binary.bin 1M
request r1 big-binary.bin s1
delay 100
respond r1
delay 200
check r1
delete big-binary.bin
quit


