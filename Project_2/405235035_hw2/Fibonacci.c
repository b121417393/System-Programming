#include <stdio.h>

double Fab(double i) {  //函式Fab(i)求費式數列第i+1個的值

	if (i == 0)  //設定f(0)=0
		return 0;
	else if (i == 1)  //設定f(1)=1
		return 1;
	else if (i >= 2)  //設定f(i)=f(i-1)+f(i-2)
		return Fab(i - 1) + Fab(i - 2);   //進入recursive直到i=0或i=1
	else
		return 0;
}

int main() {
	int i =0;
	double result = 0;

	for(i=0;i<1000;i++){
		result = Fab(i);
		printf("Fab(%d) = %.14g\n", i , result);
	}

	return 0;
}
