test_make: test.c
	gcc -c -fpic mem.c -Wall -Werror
	gcc -shared -o libmem.so mem.o
	gcc -L./ -Wl,-rpath=./ -o test test.c -Wall -Werror -lmem
