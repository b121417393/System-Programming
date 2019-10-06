#include <stdio.h>

double Fib(int n) { 

	double a = 0, b = 1, c = 1;
	int i=0;

	if(n==0)
		return 0;

	else if(n==1)
		return 1;
		
	else
		for (i = 2; i <= n; i++) {
			c = a + b;
			a = b;
			b = c;
		}
		return c;
}

void printFib(int i , double result){
	printf("Fab(%d) = %.14g\n", i , result);  //Output
}

int main(){
	int i=0 ;
	double result = 0 ; 

	for (i=0;i<1000;i++){
		result=Fib(i);
		printFib(i,result);
	}

	return 0;
}
