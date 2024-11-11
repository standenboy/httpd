CC=cc
CFLAGS=-ggdb
LIBS=-lm

SRC = httpd.c
OBJ = ${SRC:.c=.o}

all: httpd 

.c.o:
	${CC} -c ${CFLAGS} $<
httpd: ${OBJ}
	${CC} -o $@ ${OBJ} ${LIBS}
install: all 
	cp lisp /usr/local/bin/httpd
clean:
	rm -rf httpd *.o
uninstall: 
	rm /usr/local/bin/httpd

.PHONY: all clean install uninstall
