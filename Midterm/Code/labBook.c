#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#define BUF_SIZE 4096

void catchname(char *name){
	
	char buffer[100];
	//catch the pwd
	getcwd(buffer,100);
	//catch the name
	strcpy(name,buffer+6);
	
	return;
}

void gettime(char *timename){

	time_t t=time(NULL);
	struct tm *nowtime = localtime(&t);
	
	//catch time
	int yyyy = nowtime->tm_year + 1900;
	int mm = nowtime->tm_mon + 1;
	int dd = nowtime->tm_mday;
	int hh = nowtime->tm_hour;
	int min = nowtime->tm_min;
	int ss = nowtime->tm_sec;

	//print the time to the timename.
	sprintf(timename,"%d-%d-%d-%d:%d:%d",yyyy,mm,dd,hh,min,ss);
	
	return;
}


void mycp(char *source , char *time , char *name ,char *target){
	int inputFd, outputFd;
    ssize_t numIn, numOut;
    char buffer[BUF_SIZE];
    off_t begin=0, end=0;
    int fileSize, blockSize, pos=0;
    struct passwd *user;
    struct passwd *professor;
    
    user= getpwnam(name);
    professor= getpwnam("ron");
 
	//open the file
    inputFd = open (source, O_RDONLY); 

	//enter the target dir
	setuid(0);
	chdir("/home/labBook");
	
	//creat the Dir and set the owner.
	mkdir(name,0500);
	chown(name,user->pw_uid,user->pw_gid);
	chdir(name);

    //creat the Dir and set the owner.
	mkdir(time,0500);
	chown(time,user->pw_uid,user->pw_gid);
	chdir(time);

	//creat the target file.
    outputFd = open(target, O_WRONLY | O_CREAT, 0600);
    ftruncate(outputFd, 0);
    
    //check the file size.
    fileSize = lseek(inputFd, 0, SEEK_END);
    lseek(inputFd, 0, SEEK_SET);

	//copy the file.
	while (1) {
		pos = lseek(inputFd, pos, SEEK_DATA);
		begin = pos;
		pos = lseek(inputFd, pos, SEEK_HOLE);
		end = pos;
		blockSize=end-begin;
		lseek(inputFd, begin, SEEK_SET);
		lseek(outputFd, begin, SEEK_SET);
		while((numIn = read (inputFd, buffer, BUF_SIZE)) > 0) {
			numOut = write (outputFd, buffer, (ssize_t) numIn);
			if (numIn != numOut) perror("numIn != numOut");
			blockSize-=numIn;
			if (blockSize == 0) break;
		}
		if (lseek(outputFd, 0, SEEK_CUR) == fileSize) break;
    }
    
    //clode the file
    close (inputFd);
    close (outputFd);
    //set the owner to ron (user cannot to modify contain file)
	chown(target,professor->pw_uid,professor->pw_gid);
 
    return;
	}


int main(int argc , char *argv[]){

	char timename[100];
	char name[100];
	
	//get now time
	gettime(timename);
	//ger user name
	catchname(name);
	
	mycp(argv[1],timename,name,argv[1]);
	
	return 0;
}
