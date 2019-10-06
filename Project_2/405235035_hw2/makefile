all: fib.o fib2.o

fib.o: Fibonacci.c
	gcc --std=c11 -g -pg Fibonacci.c -o fib.o

fib2.o: Fibonacci2.c
	gcc --std=c11 -g -pg Fibonacci2.c -o fib2.o

clean:
	rm *.o
