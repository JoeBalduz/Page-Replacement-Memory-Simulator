make: memorysim.o
	gcc memorysim.o -o memorysim

memorysim.o: memorysim.c
	gcc -c memorysim.c

clean:
	rm *.o memorysim 
