# Make sure caches for different servers are not mixed
serve s1 s2
generate random-text1.txt 100K
generate random-text2.txt 100K
generate random-text3.txt 100K
request r1a random-text1.txt s1
request r2a random-text2.txt s1
request r3a random-text3.txt s1
delay 100
respond r3a r2a r1a
delay 200
check r1a
check r2a
check r3a
delete random-text1.txt
delete random-text2.txt
delete random-text3.txt
generate random-text1.txt 99K
generate random-text2.txt 99K
generate random-text3.txt 99K
request r1b random-text1.txt s2
request r2b random-text2.txt s2
request r3b random-text3.txt s2
delay 100
respond r1b r2b r3b
delay 200
check r1b
check r2b
check r3b
# Check for caching
request r1c random-text1.txt s2
delay 200
check r1c
quit
