
TARGET          = dicts
SOURCES         = dicts.c md5.c
HEADERS         = dicts.h md5.h
OBJECTS         = $(SOURCES:%.c=%.o)

CC              = gcc
#CFLAGS          = -g -O0 -Wall -W -Wshadow -Wno-unused-parameter -Wno-unused-result
CFLAGS          = -O2 -Wall -W -Wshadow -Wno-unused-parameter -Wno-unused-result
LDFLAGS         = -Wl,-s
LIBS            = 

%.o: %.c $(HEADERS)
	$(CC) -c $*.c $(CFLAGS)

all: $(OBJECTS)
	$(CC) -o $(TARGET) $(LDFLAGS) $(OBJECTS) $(LIBS)

clean:
	rm -f *.o $(TARGET)

.PHONY: all clean
