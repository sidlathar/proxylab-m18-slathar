# Test use of cache
serve s1
generate random-text1.txt 10K
generate random-text2.txt 1K
request r1a random-text1.txt s1
request r2 random-text2.txt s1
delay 100
# Out of order response will cause sequential proxy to fail
respond r2 r1a
delay 200
check r1a
check r2
request r1b random-text1.txt s1
# No response needed, since can serve from cache
delay 200
check r1b
quit

