.PHONY: demo, test, clean, clean_all

demo: main.c 
	gcc main.c -c -o demo.o
	gcc demo.o -o demo -lopencv_highgui -lopencv_core -lopencv_imgproc


test: test.c
	gcc test.c -o test -lopencv_highgui -lopencv_core -lopencv_imgproc

clean: 
	rm -f *.o

clean_all:
	rm -f *.o
	rm -f test
	rm -f demo
