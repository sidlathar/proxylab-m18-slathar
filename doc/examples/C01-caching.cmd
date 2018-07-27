serve s1
generate random-text.txt 10K
fetch f1 random-text.txt s1    # Handled by server
delay 100
check f1
request r1 random-text.txt s1  # Handled by proxy
delay 100
check r1                       # No action required by server
