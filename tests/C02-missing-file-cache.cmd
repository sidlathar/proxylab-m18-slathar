# Test ability to handle missing file from cache
serve s1
request r1a random-text1.txt s1
request r2  random-text2.txt s1
delay 100
respond r2 r1a
delay 200
check r1a 404
check r2 404
request r1b random-text1.txt s1
delay 200
check r1b 404
quit


