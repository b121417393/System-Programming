#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	FILE *file_one , *file_two;
	char buffer[5] , tempname[] = "./temp_XXXXXX";

	//Get the unique file name.
	mktemp(tempname);

	//Open the original file.
	file_one = fopen(argv[1], "r+");
	//Create a temporary file.
	file_two = fopen(tempname,"w+");

	//Copy the contents of the file until EOF.
	while(1)
	{
		fread(buffer,1,1,file_one);
		if(feof(file_one))
			break;
		fwrite(buffer,1,1,file_two);
	}

	printf("Tempfile has benn created\n");
	printf("Please press any key to continue!");
	getchar();

	//Move the temporary file and rename it
	rename(tempname,argv[2]);

	fclose(file_one);
	fclose(file_two);
	
	return 0;
}
