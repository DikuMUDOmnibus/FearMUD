CC=gcc2
CFLAGS=
OFLAGS=
LIBS=



map2bmp: map2bmp.o
	${CC} ${OFLAGS} -o map2bmp map2bmp.o ${LIBS}

map2bmp.o: map2bmp.c
	${CC} ${CFLAGS} -c map2bmp.c

clean:
	rm *.o
