CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -Iinclude
LIBS=-lsqlite3 -lws2_32
SRC=src/main.c src/database/database.c src/server/server.c src/auth/auth.c
TARGET=library_server.exe

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)
