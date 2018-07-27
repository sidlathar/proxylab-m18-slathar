# Make sure don't cache large objects
serve s1
generate random-text1.txt 200K
generate random-text2.txt 20K
request r1a random-text1.txt s1
request r2a random-text2.txt s1
delay 100
respond r2a r1a
delay 100
check r1a
check r2a
delete random-text1.txt
delay 200
# Should not serve from cache
request r1b random-text1.txt s1
# Should serve from cache
request r2b random-text2.txt s1
delay 200
respond r1b
delay 100
check r1b 404
check r2b
quit

