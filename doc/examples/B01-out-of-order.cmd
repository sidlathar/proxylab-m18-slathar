serve s1
generate random-text1.txt 2K     # File 1
generate random-text2.txt 4K     # File 2
request r1 random-text1.txt s1   # Request r1
request r2 random-text2.txt s1   # Request r2
delay 100
respond r2                       # Respond out of order
delay 100
check r2
