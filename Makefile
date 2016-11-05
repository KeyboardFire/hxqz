.PHONY: all

all: hxqz

a.out: hxqz.c
	gcc hxqz.c -Wall -Wpedantic -o hxqz
