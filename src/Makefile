.PHONY: demo, test, clean, clean_all

demo: main.c 
	g++ main.c -c -o demo.o
	g++ demo.o -o demo -lopencv_highgui -lopencv_core -lopencv_imgproc


test: test.c
	g++ test.c -o test -lopencv_highgui -lopencv_core -lopencv_imgproc

clean: 
	rm -f *.o

clean_all:
	rm -f *.o
	rm -f test
	rm -f demo
