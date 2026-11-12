# Makefile to build the download application

CC := gcc
CFLAGS := -Wall

download: download.c
	$(CC) $(CFLAGS) -o download download.c

.PHONY: clean
clean:
	rm -f download
