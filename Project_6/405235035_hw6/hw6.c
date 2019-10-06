#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include <sys/inotify.h>
#include <dirent.h>
#include <unistd.h>

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

int level = 0;
char wd[1024][10000];
int fd, inotify_ret = 0;

//Print notification message.
void printInotifyEvent(struct inotify_event *event){
	char buf[4096] = "";
	sprintf(buf, "[%s] ", wd[event->wd]);
	strncat(buf, "{", 4096);
	if (event->mask & IN_ACCESS)
		strncat(buf, "ACCESS, ", 4096);
	if (event->mask & IN_ATTRIB)
		strncat(buf, "ATTRIB, ", 4096);
	if (event->mask & IN_CLOSE_WRITE)
		strncat(buf, "CLOSE_WRITE, ", 4096);
	if (event->mask & IN_CLOSE_NOWRITE)
		strncat(buf, "CLOSE_NOWRITE, ", 4096);
	if (event->mask & IN_CREATE)
		strncat(buf, "CREATE, ", 4096);
	if (event->mask & IN_DELETE)
		strncat(buf, "DELETE, ", 4096);
	if (event->mask & IN_DELETE_SELF)
		strncat(buf, "DELETE_SELF, ", 4096);
	if (event->mask & IN_MODIFY)
		strncat(buf, "MODIFY, ", 4096);
	if (event->mask & IN_MOVE_SELF)
		strncat(buf, "MOVE_SELF, ", 4096);
	if (event->mask & IN_MOVED_FROM)
		strncat(buf, "MOVED_FROM, ", 4096);
	if (event->mask & IN_MOVED_TO)
		strncat(buf, "MOVED_TO, ", 4096);
	if (event->mask & IN_OPEN)
		strncat(buf, "OPEN, ", 4096);
	if (event->mask & IN_IGNORED)
		strncat(buf, "IGNORED, ", 4096);
	if (event->mask & IN_ISDIR)
		strncat(buf, "ISDIR, ", 4096);
	if (event->mask & IN_Q_OVERFLOW)
		strncat(buf, "Q_OVERFLOW, ", 4096);
	buf[strlen(buf) - 2] = '\0';
	strncat(buf, "}", 4096);
	sprintf(buf, "%s cookie=%d", buf, event->cookie);
	if (event->len > 0)
		sprintf(buf, "%s name = %s\n", buf, event->name);
	else
		sprintf(buf, "%s name = null\n", buf);
	printf("%s", buf);
}

//Print file type and name.
void printName(char *type, char *name){
	printf("%s", type);
	for (int i = 0; i < level; i++)
		printf("  ");
	if (strcmp("d", type) == 0)
		printf("+");
	else
		printf("|");
	printf("%s\n", name);
}

//Use recursive retrieval of all files
void listDir(char *pathName){
	level++;

	DIR *curDir = opendir(pathName);

	char *newPathName = (char *)malloc(PATH_MAX);
	char filename[1024][64], tempname[64];
	char filetype[1024][2], temptype[2];

	struct dirent entry;
	struct dirent *result;

	int ret, i = 0, j = 0, count = 0;
	ret = readdir_r(curDir, &entry, &result);

	while (result != NULL){

		if (strcmp(entry.d_name, ".") == 0 || strcmp(entry.d_name, "..") == 0){
			ret = readdir_r(curDir, &entry, &result);
			continue;  //Skip when file is . or ..
		}

		if (entry.d_type == DT_LNK){
			sprintf(filename[i], "%s", entry.d_name);
			sprintf(filetype[i], "l");
			i++;
			count++;
		}

		else if (entry.d_type == DT_REG){
			sprintf(filename[i], "%s", entry.d_name);
			sprintf(filetype[i], "f");
			i++;
			count++;
		}

		else if (entry.d_type == DT_DIR){
			printName("d", entry.d_name);
			sprintf(newPathName, "%s/%s", pathName, entry.d_name);
			listDir(newPathName);  //Recursively when the file is a folder
		}

		ret = readdir_r(curDir, &entry, &result);
	}

	//Sort the file using bubble sort.
	for (i = count - 1; i > 0; i--)
		for (j = 0; j < i; j++)
			if (strcmp(filename[j], filename[j + 1]) > 0){
				strcpy(tempname, filename[j + 1]), strcpy(temptype, filetype[j + 1]);
				strcpy(filename[j + 1], filename[j]), strcpy(filetype[j + 1], filetype[j]);
				strcpy(filename[j], tempname), strcpy(filetype[j], temptype);
			}

	//Print file name.
	for (i = 0; i < count; i++)
		printName(filetype[i], filename[i]);

	closedir(curDir);
	level--;
}

//Use recursion to include all files in monitoring.
void inotify_Dir(char *pathName){

	DIR *curDir = opendir(pathName);
	char *newPathName = (char *)malloc(PATH_MAX);

	struct dirent entry;
	struct dirent *result;

	int ret;
	ret = readdir_r(curDir, &entry, &result);

	while (result != NULL){

		char pathname[PATH_MAX];

		if (strcmp(entry.d_name, ".") == 0 || strcmp(entry.d_name, "..") == 0){
			ret = readdir_r(curDir, &entry, &result);
			continue;  //Skip when file is . or ..
		}

		else if (entry.d_type == DT_LNK){
			sprintf(pathname, "%s/%s", pathName, entry.d_name);
			inotify_ret = inotify_add_watch(fd, pathname, IN_ALL_EVENTS);
			strcpy(wd[inotify_ret], entry.d_name);
		}

		else if (entry.d_type == DT_REG){
			sprintf(pathname, "%s/%s", pathName, entry.d_name);
			inotify_ret = inotify_add_watch(fd, pathname, IN_ALL_EVENTS);
			strcpy(wd[inotify_ret], entry.d_name);
		}

		else if (entry.d_type == DT_DIR){
			sprintf(newPathName, "%s/%s", pathName, entry.d_name);
			inotify_Dir(newPathName);  //Recursively when the file is a folder.
			inotify_ret = inotify_add_watch(fd, newPathName, IN_ALL_EVENTS);
			strcpy(wd[inotify_ret], entry.d_name);
		}

		ret = readdir_r(curDir, &entry, &result);
	}

	closedir(curDir);
}

//Main function.
int main(int argc, char **argv){

	//Print all files by recursive
	listDir(argv[1]);

	int num;
	char *p;
	char inotify_entity[BUF_LEN];

	fd = inotify_init();
	//Monitor all files by recursive
	inotify_Dir(argv[1]);

	//Print out monitoring messages
	while (1){
		num = read(fd, inotify_entity, BUF_LEN);
		for (p = inotify_entity; p < inotify_entity + num;){
			printInotifyEvent((struct inotify_event *)p);
			p += sizeof(struct inotify_event) + ((struct inotify_event *)p)->len;
		}
	}

	return 0;
}
