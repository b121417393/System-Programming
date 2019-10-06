#include <stdio.h>

double Fab(double i) {  //�禡Fab(i)�D�O���ƦC��i+1�Ӫ���

	if (i == 0)  //�]�wf(0)=0
		return 0;
	else if (i == 1)  //�]�wf(1)=1
		return 1;
	else if (i >= 2)  //�]�wf(i)=f(i-1)+f(i-2)
		return Fab(i - 1) + Fab(i - 2);   //�i�Jrecursive����i=0��i=1
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
