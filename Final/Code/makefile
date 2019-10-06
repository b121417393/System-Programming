all: myshell pi

myshell: myshell.c
	gcc -g myshell.c -o myshell

pi: pi.c
	gcc -O3 -pthread pi.c -o pi

clean:
	rm myshell pi
