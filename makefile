CC ?= gcc
PREFIX=/usr
LIBDIR=$(PREFIX)/lib
BINDIR=$(PREFIX)/bin

CFLAGS ?= -Wall
CFLAGS += -D_GNU_SOURCE -D_XOPEN_SOURCE=700 -std=c99

all: libfakeuser.so fakeadd

libfakeuser.so: fakeuser.c util.c
	$(CC)  $(CFLAGS) -shared -fPIC -o libfakeuser.so fakeuser.c util.c -ldl $(LDFLAGS)

fakeadd: fakeadd.c util.c
	$(CC) $(CFLAGS) -o fakeadd fakeadd.c util.c -ldl $(LDFLAGS)

install:
	install -m 0755 -d "$(DESTDIR)/$(LIBDIR)"
	install -m 0644 libfakeuser.so "$(DESTDIR)/$(LIBDIR)"
	install -m 0755 -d "$(DESTDIR)/$(LIBDIR)"
	install -m 0644 fakeuser "$(DESTDIR)/$(BINDIR)"
	install -m 0644 fakeadd "$(DESTDIR)/$(BINDIR)"

