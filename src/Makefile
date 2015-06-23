.PHONY: clean  clean_all  jk
CFLAGS=-ggdb -lopencv_highgui -lopencv_core -lopencv_imgproc 


jk: jkColorCluster.o joke.o
	g++ jkColorCluster.o joke.o -o jk $(CFLAGS)

jkColorCluster.o: jkColorCluster.c joke.h
	g++ jkColorCluster.c -c -o jkColorCluster.o $(CFLAGS)

joke.o: joke.c joke.h
	g++ joke.c -c -o joke.o $(CFLAGS)

white: white.c
	g++ white.c -o white $(CFLAGS)



clean: 
	rm -f *.o

clean_all:
	rm -f *.o
	rm -f jk white
