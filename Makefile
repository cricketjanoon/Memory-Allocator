test_make: main.c
	gcc -c -fpic mem.c -Wall -Werror
	gcc -shared -o libmem.so mem.o
	gcc -L./ -Wl,-rpath=./ -o main main.c -Wall -Werror -lmem