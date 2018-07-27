# Test ability to handle missing file
serve s1
request r1 random-text.txt s1
delay 100
respond r1
delay 100
check r1 404
quit


