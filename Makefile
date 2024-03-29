#
# Makefile for Proxy Lab
#
# You may modify is file any way you like (except for the handin
# rule). Autolab will execute the command "make" on your specific
# Makefile to build your proxy from sources.
#
CC = gcc
CFLAGS = -g -Wall -Wextra -Werror -std=c99
LDFLAGS = -lpthread

all: proxy tiny-code

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

proxy.o: proxy.c csapp.h
	$(CC) $(CFLAGS) -c proxy.c

proxy: proxy.o csapp.o

tiny-code:
	(cd tiny; make)

# Creates a tarball in ../proxylab-handin.tar that you should then
# hand in to Autolab. DO NOT MODIFY THIS!
handin:
	(make clean; cd ..; tar cvf proxylab-handin.tar --exclude tiny --exclude nop-server.py --exclude proxy --exclude driver.sh --exclude port-for-user.pl --exclude free-port.sh --exclude ".*" --exclude src --exclude tests --exclude doc --exclude logs --exclude response_files --exclude source_files proxylab-m18*)

clean:
	rm -f *~ *.o proxy core *.tar *.zip *.gzip *.bzip *.gz
	(cd tiny; make clean)

