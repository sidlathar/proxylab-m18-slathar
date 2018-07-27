serve s1                      # Set up server
generate random-text.txt 10K  # Generate text file
request r1 random-text.txt s1 # Initiate request r1
delay 100                     # Wait for request to propagate
respond r1                    # Allow server to respond
delay 100                     # Wait for response to propagate
check r1                      # Compare retrieved file to original
