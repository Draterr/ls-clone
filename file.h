#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>

struct file{
	char *name;
	char *perm;
	nlink_t hard_links;
	char *user_id;
	char *group_id;
	off_t size;
	char last_modified[80];
	char *color;
};
