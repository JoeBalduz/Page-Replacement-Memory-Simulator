make: memorysim.o
	gcc -std=c99 memorysim.o -o memorysim

memorysim.o: memorysim.c
	gcc -c -std=c99 memorysim.c

clean:
	rm *.o memorysim 
