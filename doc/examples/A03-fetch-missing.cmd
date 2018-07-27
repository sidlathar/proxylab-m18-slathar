serve s1                      # Set up server
fetch f1 nonexistent.file s1  # Fetch file
delay 100                     # Wait for request and response
check f1 404                  # Check for not-found status 
