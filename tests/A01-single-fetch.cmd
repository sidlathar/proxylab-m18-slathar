# Test ability to fetch very small text file
serve s1
generate random-text.txt 50
fetch f1 random-text.txt s1
delay 100
check f1
trace f1
quit

