all:
	clang -ggdb main.c -o mmd

run: all
	./mmd test.md test.html
