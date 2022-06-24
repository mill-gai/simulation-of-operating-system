mysh: shell.c interpreter.c shellmemory.c kernel.c
	gcc -D FRAMESIZE=$(framesize) -D VARMEMSIZE=$(varmemsize) -c shell.c interpreter.c shellmemory.c kernel.c
	gcc -o mysh shell.o interpreter.o shellmemory.o kernel.o

clean:
	rm mysh; rm *.o