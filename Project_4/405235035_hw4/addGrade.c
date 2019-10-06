#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE 32

int main(){
	int fd, ret;
	ssize_t numIn, numOut;
	char grade[BUF_SIZE];

	//When there is a grade to enter.
	while ((numIn = read(0, grade, BUF_SIZE)) > 0){
		
		//Open file.
		fd = open("./grade.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);

		//Lock file.
		printf("grade.txt is writting now\n");
		ret = flock(fd, LOCK_EX);

		//Write file.
		numOut = write(fd, grade, (ssize_t)numIn);
		sleep(5);

		//Unlock file.
		printf("writting complete\n");
		ret = flock(fd, LOCK_UN);

		//Close file.
		close(fd);
	}

	return 0;
}
