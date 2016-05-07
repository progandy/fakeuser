CC ?= gcc

CFLAGS ?= -Wall

CFLAGS += -D_GNU_SOURCE -D_XOPEN_SOURCE=700 -std=c99

all: libfakeuser.so fakeadd

libfakeuser.so: fakeuser.c util.c
	$(CC)  $(CFLAGS) -shared -fPIC -o libfakeuser.so fakeuser.c util.c -ldl $(LDFLAGS)

fakeadd: fakeadd.c util.c
	$(CC) $(CFLAGS) -o fakeadd fakeadd.c util.c -ldl $(LDFLAGS)
