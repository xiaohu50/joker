.PHONY: demo  test  clean  clean_all seg jk
CFLAGS=-ggdb -lopencv_highgui -lopencv_core -lopencv_imgproc -DDEBUG
demo: main.c 
	g++ main.c -c -o demo.o
	g++ demo.o -o demo -lopencv_highgui -lopencv_core -lopencv_imgproc

seg: seg.c
	g++ seg.c -o seg -lopencv_highgui -lopencv_core -lopencv_imgproc 

jk: jkColorCluster.o joke.o
	g++ jkColorCluster.o joke.o -o jk $(CFLAGS)

jkColorCluster.o: jkColorCluster.c joke.h
	g++ jkColorCluster.c -c -o jkColorCluster.o $(CFLAGS)

joke.o: joke.c joke.h
	g++ joke.c -c -o joke.o $(CFLAGS)



test: test1 test2 test3 test4

test1: test1.c
	g++ test1.c -o test1 -lopencv_highgui -lopencv_core -lopencv_imgproc

test2: test2.c
	g++ test2.c -o test2 -lopencv_highgui -lopencv_core -lopencv_imgproc

test3: test3.c
	g++ test3.c -o test3 -lopencv_highgui -lopencv_core -lopencv_imgproc

test4: test4.c
	g++ test4.c -o test4 -lopencv_highgui -lopencv_core -lopencv_imgproc -DDEBUG

clean: 
	rm -f *.o

clean_all:
	rm -f *.o
	rm -f test1 test2 
	rm -f demo
