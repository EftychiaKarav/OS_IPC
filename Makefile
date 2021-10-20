# AM: 1115201800062
# KARAVANGELI EFTYCHIA
# Makefile arxeio

CC = gcc
CFLAGS = -g -Wall


all: p1 p2 chan enc1 enc2 

.PHONY: all clean

p1: p1.o semoper.o
	$(CC) $(CFLAGS) -o p1 p1.o semoper.o

p2: p2.o semoper.o
	$(CC) $(CFLAGS) -o p2 p2.o semoper.o

chan: channel.o semoper.o
	$(CC) $(CFLAGS) -o chan channel.o semoper.o

enc1: ENC1.o semoper.o
	$(CC) $(CFLAGS) -o enc1 ENC1.o semoper.o -lcrypto

enc2: ENC2.o semoper.o
	$(CC) $(CFLAGS) -o enc2 ENC2.o semoper.o -lcrypto



semoper.o: semoper.c semoper.h
	$(CC) $(CFLAGS) -c semoper.c

p1.o: p1.c semoper.h
	$(CC) $(CFLAGS) -c p1.c
	
p2.o: p2.c semoper.h
	$(CC) $(CFLAGS) -c p2.c

channel.o: channel.c semoper.h
	$(CC) $(CFLAGS) -c channel.c

ENC1.o: ENC1.c semoper.h
	$(CC) $(CFLAGS) -c ENC1.c -lcrypto

ENC2.o: ENC2.c semoper.h
	$(CC) $(CFLAGS) -c ENC2.c -lcrypto	


# .PHONY: clean

clean:
	rm -f p1 p2 chan enc1 enc2 p1.o p2.o ENC1.o ENC2.o channel.o semoper.o

