all: hook trampoline

hook: hook.c
	gcc hook.c -Wall -std=c99 -o hook.o

trampoline: trampoline.S
	nasm -f bin trampoline.S -o trampoline.bin

clean:
	rm hook.o trampoline.bin
