# Test ability to handle out-of-order requests
serve s1
generate random-text1.txt 2K 
generate random-text2.txt 4K 
request r1 random-text1.txt s1
request r2 random-text2.txt s1
delay 100
respond r2
respond r1
delay 200
check r1
check r2
quit

