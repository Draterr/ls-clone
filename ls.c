#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <grp.h>
#include "file.h"

#define UPPER_SIZE 4294967295
#define ANSI_COLOR_GREEN "\e[0;32m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_BLUE "\e[0;34m"
#define ANSI_COLOR_YELLOW "\e[0;33m"
#define ANSI_COLOR_CYAN "\e[0;36m"
#define ANSI_COLOR_PURPLE "\e[0;35m"
#define ANSI_COLOR_RED "\e[0;31m"
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_BACKGROUND_RED "\e[41m"
#define ANSI_BACKGROUND_YELLOW "\e[43m"


int actual_number_files; //get the number of files for the current struct
int debug = 0; // set debug information
int col_size; // the number of columns for the current terminal
size_t longest_filename = 0; //the longest filename which we have to assign the column width to
size_t longest_filesize = 0; //the longest file_size which we have to assign the column width to
size_t longest_username = 0; //the longest username which we have to assign the column width to
size_t longest_group = 0; //the longest group which we have to assign the column width to
size_t longest_month = 0; //the longest month which we have to assign the column width to
size_t longest_hardlink = 0; //the longest hardlink which we have to assign the column width to
unsigned int blocks; //the total block size in 1024 bytes

int getcurrent_year(){	//function to get the current year function usage: (if last modification time is not in the current year display the year)
	time_t year = time(&year);
	struct tm *current_time = localtime(&year);
	int current_year = current_time->tm_year + 1900;
	return current_year;

}

char *get_file_type(mode_t st_mode){ 
	if(st_mode & S_ISUID && S_ISREG(st_mode)){
		return ANSI_BACKGROUND_RED;
	}
	else if(st_mode & S_ISGID && S_ISREG(st_mode)){
		return ANSI_BACKGROUND_YELLOW;
	}
	else if(S_ISREG(st_mode) && st_mode & S_IXUSR && st_mode & S_IXGRP && st_mode & S_IXOTH){
		return ANSI_COLOR_GREEN;
	}
	else if(S_ISDIR(st_mode)){
		return ANSI_COLOR_BLUE;
	}
	else if(S_ISCHR(st_mode)){
		return ANSI_COLOR_YELLOW;
	}
	else if(S_ISBLK(st_mode)){
		return ANSI_COLOR_PURPLE;
	}
	else if(S_ISFIFO(st_mode)){
		return ANSI_COLOR_RED;
	}
	else if(S_ISLNK(st_mode)){
		return ANSI_COLOR_CYAN;
	}
	else if(S_ISSOCK(st_mode)){
		return ANSI_COLOR_MAGENTA;
	}
	else if(S_ISREG(st_mode)){
		return ANSI_COLOR_RESET;
	}
	else{
		return "none";
	}
}

char *populate_permission(mode_t st_mode,char *file_type){
	char *permission = malloc(sizeof(char) * 11);
	memset(permission, 0, 11);
	//first bit of permission
	strcmp(file_type,ANSI_COLOR_GREEN) == 0 ? permission[0] = '-': 0;
	strcmp(file_type,ANSI_COLOR_RESET) == 0 ? permission[0] = '-': 0;
	strcmp(file_type,ANSI_BACKGROUND_RED) == 0 ? permission[0] = '-': 0;
	strcmp(file_type,ANSI_BACKGROUND_YELLOW) == 0 ? permission[0] = '-': 0;
	strcmp(file_type,ANSI_COLOR_BLUE) == 0 ? permission[0] = 'd': 0;
	strcmp(file_type,ANSI_COLOR_YELLOW) == 0 ? permission[0] = 'c': 0;
	strcmp(file_type,ANSI_COLOR_PURPLE) == 0 ? permission[0] = 'b': 0;
	strcmp(file_type,ANSI_COLOR_RED) == 0 ? permission[0] = 'f': 0;
	strcmp(file_type,ANSI_COLOR_CYAN) == 0 ? permission[0] = 'l': 0;
	strcmp(file_type,ANSI_COLOR_MAGENTA) == 0 ? permission[0] = 's': 0;

	//rwx-rwx-rwx
	permission[1] = (st_mode & S_IRUSR) ? 'r': '-';
	permission[2] = (st_mode & S_IWUSR) ? 'w': '-';
	permission[3] = (st_mode & S_IXUSR) ? 'x': '-';
	permission[4] = (st_mode & S_IRGRP) ? 'r': '-';
	permission[5] = (st_mode & S_IWGRP) ? 'w': '-';
	permission[6] = (st_mode & S_IXGRP) ? 'x': '-';
	permission[7] = (st_mode & S_IROTH) ? 'r': '-';
	permission[8] = (st_mode & S_IWOTH) ? 'w': '-';
	permission[9] = (st_mode & S_IXOTH) ? 'x': '-';

	//suid/sgid 
	permission[3] = (st_mode & S_ISUID) ? 's': 'x';
	permission[6] = (st_mode & S_ISGID) ? 's': 'x';
	
	return permission;
}

struct file *populate_struct(struct dirent *dir_struct,DIR *dir,char *directory){  //populate the struct for each file
	int i = 0;
	struct file *new_ptr = NULL; //new ptr to store potential realloc return value
	struct file *file_properties;
	struct file *final_arr;
	struct stat file_stats;
	int word_limit = 256; //the amount of words that we can store in the struct array
	size_t struct_buf = sizeof(struct file); //memory size for the struct array
	file_properties = malloc(struct_buf * word_limit);
	struct_buf = sizeof(struct file) * word_limit;
	blocks = 0;
	while((dir_struct = readdir(dir)) != NULL){
		char *file_name = dir_struct->d_name;
		char file_dir[(sizeof(char *) * strlen(directory)) + 3 + (sizeof(char *) * strlen(file_name))];
		strncpy(file_dir, directory,strlen(directory)+1); //copy the directory name to the file_dir var
		strcat(file_dir,"/"); //add the "/" to the end of the directory "/var" -> "/var/"
		strcat(file_dir,file_name); //add the current file_name to the directory "/var/" -> "/var/current_file"
		if(lstat(file_dir, &file_stats) == -1){ //error handling for lstat
			perror("lstat");
		}
		if(strlen(file_name) > longest_filename){ //find the longest filename
			longest_filename = strlen(file_name);
		}
		if (i >= word_limit){ //dynamically allocate memory for the struct array
			if (struct_buf >= UPPER_SIZE){
				printf("Reached maxium amount of memory allowed\nexiting...");
				exit(-1);
			}
			word_limit = word_limit * 2;
			struct_buf = sizeof(struct file) * word_limit;
			new_ptr = realloc(file_properties,struct_buf);
			if (new_ptr == NULL){
				perror("realloc");
				exit(-1);
				free(file_properties);
			}
			if(debug){
				printf("[-] reached max memory\n");
				printf("[+] allocating more memory\n");
				printf("[+] current buffer_size = %lu\n",struct_buf);
				printf("[+] current word_limit = %d\n",word_limit);
			}
			file_properties = new_ptr;
		}
		file_properties[i].name = malloc(strlen(file_name)+1); //free this (recursively)
		if(file_properties[i].name == NULL){
			perror("malloc");
			exit(-1);
		}

		//populate modification time
		int current_year = getcurrent_year(); 
		struct tm *time = localtime(&file_stats.st_mtime); 
		char month[10];
		if(time->tm_year + 1900 == current_year){ //calculate if the year the file was not modified is equal to the current year(as per ls)
			strftime(file_properties[i].last_modified,80,"%B %d %H:%M",time);
		}
		else{
			strftime(file_properties[i].last_modified,80,"%B %d %Y",time);
		}
		strncpy(file_properties[i].name,file_name,strlen(file_name)+1);
		strftime(month, 10, "%B", time);
		if(strlen(month) > longest_month){
			longest_month = strlen(month);
		}
		//populate permission / color code
		char *color = get_file_type(file_stats.st_mode);
		char *permission = populate_permission(file_stats.st_mode,color);
		file_properties[i].perm = malloc(sizeof(char) * 11 + 1);
		strcpy(file_properties[i].perm,permission);
		free(permission);
		file_properties[i].color = malloc(strlen(color)+1);
		strncpy(file_properties[i].color,color,strlen(color)+1);

		//populate file_size
		file_properties[i].size = file_stats.st_size;
		if(file_stats.st_size > longest_filesize){
			longest_filesize = file_stats.st_size;
		}
		//populate uid/gid
		struct passwd *user_id = getpwuid(file_stats.st_uid);
		if(user_id == 0){
			perror("getpwuid");
			exit(-1);
		}
		file_properties[i].user_id = malloc(sizeof(char) * strlen(user_id->pw_name)+1);
		strcpy(file_properties[i].user_id,user_id->pw_name);
		struct group *group_id = getgrgid(file_stats.st_gid);
		file_properties[i].group_id = malloc(sizeof(char) * strlen(group_id->gr_name)+1);
		strcpy(file_properties[i].group_id, group_id->gr_name);

		if(strlen(file_properties[i].user_id) > longest_username){
			longest_username = strlen(file_properties[i].user_id);
		}
		if(strlen(file_properties[i].group_id) > longest_group){
			longest_group = strlen(file_properties[i].group_id);
		}

		//populate hardlink
		file_properties[i].hard_links = file_stats.st_nlink;
		if(file_stats.st_nlink > longest_hardlink){
			longest_hardlink = file_stats.st_nlink;
		}

		//find number of blocks taken
		blocks += file_stats.st_blocks;
		i++;
	}
	if(!new_ptr){
		final_arr = realloc(file_properties,i * sizeof(struct file));
	}
	else{
		final_arr = realloc(new_ptr,i * sizeof(struct file));
	}
	if(final_arr == NULL){
		perror("realloc");
		exit(-1);
	}
	actual_number_files = i;
	return final_arr;
}

void free_struct_mem(struct file *file_properties,int len){ //recursively free memory allocated for each struct array's child
	int i;
	for(i=0;i<len;i++){
		free(file_properties[i].name);
		free(file_properties[i].color);
		free(file_properties[i].perm);
		free(file_properties[i].user_id);
		free(file_properties[i].group_id);
	}
}
char *parse_args(int argc,char *argv[]){
	int character = 0;
	int i=0;
	char *options = "dlatr"; // "l" long listing format, "a" list all attributes, "t" list sort by time, "r" reverse order 
	char *args = NULL;
	args = malloc(2 * sizeof(char));
	memset(args, 0, 2*sizeof(char));
	do{
		character = getopt(argc, argv, options);
		if(character == 'd'){
			debug = 1;
		}
		args[i] = character;
		i++;
	}
	while(character != -1);
	return args;
}

struct file* sort_name(struct file *file_properties){ //sort the name alphabetically (.) and (..) are first
	int i;
	int j;
	for(j=0;j<actual_number_files;j++){ //bubble sort
		struct file tmp;
		for(i=0;i<actual_number_files - j - 1;i++){
			if(strcmp(file_properties[i].name,file_properties[i+1].name) > 0){
				tmp = file_properties[i];
				file_properties[i] = file_properties[i+1];
				file_properties[i+1]= tmp;
			}
		}
	}
	return file_properties;
}

void display_result(int len,struct file *file_properties,int mode,char *current_directory,int col_size){//mode(1) = row display, mode(2) = column display
	int i;
	size_t current_len;
	int tmp_col;
	tmp_col = col_size;
	int col_len = 0;
	for(i=0;i<actual_number_files;i++){
		current_len = strlen(file_properties[i].name);
		char str_longest_file_size[11];
		char str_current_file_size[11];
		char str_longest_hard_link[20];
		sprintf(str_longest_file_size,"%d",(int)longest_filesize);
		sprintf(str_current_file_size, "%d", (int)file_properties[i].size);
		sprintf(str_longest_hard_link, "%d", (int)longest_hardlink);
		switch(mode) {
			case 1:
				if(file_properties[i].name[0] == '.'){
					continue;
				}
					if(longest_filename < tmp_col){
						int padding = longest_filename - strlen(file_properties[i].name);
						printf("%s%-s%s%*s" , file_properties[i].color ,file_properties[i].name, ANSI_COLOR_RESET,padding, " ");
						if(i == actual_number_files - 1){
							printf("\n");
						}
						tmp_col = tmp_col - longest_filename;

					}
					else if(longest_filename > tmp_col){
						int padding = longest_filename - strlen(file_properties[i].name);
						tmp_col = col_size;
						printf("\n");
						printf("%s%-s%s%*s",file_properties[i].color,file_properties[i].name, ANSI_COLOR_RESET,padding," ");
						tmp_col = tmp_col - longest_filename;
					}
				if(i == actual_number_files - 1){
					printf("\n");
				}
				break;	
			case 2:
				if(file_properties[i].name[0] == '.'){
					continue;
				}
				printf("%*s ",10,file_properties[i].perm);
				printf("%*lu ",(int)strlen(str_longest_hard_link),file_properties[i].hard_links);
				printf("%*s ",(int) longest_username, file_properties[i].user_id);
				printf("%*s ",(int) longest_group,file_properties[i].group_id);
				printf("%*lu ",(int)strlen(str_longest_file_size),file_properties[i].size);
				printf("%*s ",((int)longest_month + 9) ,file_properties[i].last_modified);
				printf("%s%s%s\n",file_properties[i].color, file_properties[i].name, ANSI_COLOR_RESET);
				break;	
			case 3:
				if(i == 0){
					printf("total %u\n",blocks/2);
				}
				printf("%*s ",10,file_properties[i].perm);
				printf("%*lu ",(int)strlen(str_longest_hard_link),file_properties[i].hard_links);
				printf("%*s ",(int) longest_username, file_properties[i].user_id);
				printf("%*s ",(int) longest_group,file_properties[i].group_id);
				printf("%*lu ",(int)strlen(str_longest_file_size),file_properties[i].size);
				printf("%*s ",((int)longest_month + 9) ,file_properties[i].last_modified);
				printf("%s%s%s\n",file_properties[i].color, file_properties[i].name, ANSI_COLOR_RESET);
				break;	
			case 4:
				if(i == 0){
					printf("%s:\n",current_directory);
					printf("total %u\n",blocks/2);
				}
				printf("%*s ",10,file_properties[i].perm);
				printf("%*lu ",(int)strlen(str_longest_hard_link),file_properties[i].hard_links);
				printf("%*s ",(int) longest_username, file_properties[i].user_id);
				printf("%*s ",(int) longest_group,file_properties[i].group_id);
				printf("%*lu ",(int)strlen(str_longest_file_size),file_properties[i].size);
				printf("%*s ",((int)longest_month + 9) ,file_properties[i].last_modified);
				printf("%s%s%s\n",file_properties[i].color, file_properties[i].name, ANSI_COLOR_RESET);
				if(i == actual_number_files - 1){
					printf("\n");
				}
				break;	
		}
				
	}
}

void pop_struct_then_display(DIR *dir,int mode,char *current_directory){
	struct dirent *dir_struct;
	struct file *file_properties;
	file_properties = populate_struct(dir_struct, dir,current_directory);
	file_properties = sort_name(file_properties);
	display_result(actual_number_files, file_properties, mode,current_directory,col_size);
	free_struct_mem(file_properties, actual_number_files);
	free(file_properties);
}
int main(int argc, char*argv[]){
	struct dirent *dir_struct;
	char *args;
	int display_mode = 1;
	args = parse_args(argc,argv);
	int i = 0;
	int j;
	if(args[0] == -1){			
		display_mode = 1;
	}
	while(args[i] != -1){
		switch (args[i]) {
			case 'l':
				display_mode = 2;
				break;
			case 'a':
				display_mode = 3;
				break;
		}
		i++;
	}
	if(isatty(1)){
		struct winsize sizes;
		int status = ioctl(1, TIOCGWINSZ, &sizes);//we assume that the program is ran inside a terminal
		if(status == 0){
			col_size = sizes.ws_col;
	}
	char directory[4095];
	int stat_result;
	int has_dir_arg = 0;
	int more_than_1_dir = 0;
	struct stat stats;
	for(j=0;j < argc;j++){
		stat_result = stat(argv[j],&stats);
		if(S_ISDIR(stats.st_mode)){
			more_than_1_dir++;
			}
		}
	if(more_than_1_dir > 1){
		display_mode = 4;
		}
	for(j=0;j < argc;j++){
		stat_result = stat(argv[j],&stats);
		if(S_ISDIR(stats.st_mode)){
			has_dir_arg = 1;
			strncpy(directory, argv[j],4095);
			DIR *dir = opendir(directory);
			if (!dir){
				perror("opendir");
				exit(1);
			}
			pop_struct_then_display(dir, display_mode,directory);
			closedir(dir);
		}
	}
	if(has_dir_arg == 0){
		strncpy(directory, ".",4095);
		DIR *dir = opendir(directory);
		if (!dir){
			perror("opendir");
			exit(1);
		}
		pop_struct_then_display(dir, display_mode,directory);
		closedir(dir);
		}
	free(args);
	return 0;
}
}
