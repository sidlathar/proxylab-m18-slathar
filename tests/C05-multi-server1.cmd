# Make sure caches for different servers are not mixed
serve s1 s2
generate random-text1.txt 100K
generate random-text2.txt 100K
generate random-text3.txt 100K
fetch f1a random-text1.txt s1
fetch f2a random-text2.txt s1
fetch f3a random-text3.txt s1
delay 100
check f1a
check f2a
check f3a
# Make sure caching occurred
request r1a random-text1.txt s1
delay 100
check r1a
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
respond r3b r2b r1b
delay 100
check r1b
check r2b
check r3b
quit
