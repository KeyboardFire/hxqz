PREFIX ?= /usr/local

.PHONY: install

hxqz: hxqz.c
	gcc hxqz.c -Wall -Wpedantic -o hxqz

install: hxqz
	install -D hxqz $(DESTDIR)$(PREFIX)/bin/hxqz
	install -Dm644 hxqz.1 $(DESTDIR)$(PREFIX)/share/man/man1/hxqz.1
