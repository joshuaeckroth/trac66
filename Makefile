all: trac test_trac

trac: trac.o traclib.o
	gcc -Wall -Werror -g -o trac trac.o traclib.o

test_trac: test_trac.o traclib.o
	gcc -Wall -Werror -g -o test_trac test_trac.o traclib.o

traclib.o: traclib.c trac.h
	gcc -Wall -Werror -g -c traclib.c

trac.o: trac.c trac.h
	gcc -Wall -Werror -g -c trac.c

test_trac.o: test_trac.c trac.h
	gcc -Wall -Werror -g -c test_trac.c

.phony: clean
clean:
	rm trac *.o

