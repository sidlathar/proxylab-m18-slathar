serve s1                      # Set up server
generate random-data.bin 10K  # Generate binary file
fetch f1 random-data.bin s1   # Fetch file
delay 100                     # Wait for request and response
check f1                      # Compare retrieved file to original
