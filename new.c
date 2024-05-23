#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define UPPER_SIZE 4294967295
#define ANSI_COLOR_GREEN "\e[0;32m"
#define ANSI_COLOR_RESET "\x1b[0m"

struct file{
	char *name;
	char *type;
};

int actual_number_files;
int debug = 0;

struct file *populate_struct(struct dirent *dir_struct,DIR *dir){ 
	int i = 0;
	struct file *new_ptr = NULL; //new ptr to store potential realloc return value
	struct file *file_properties;
	struct file *final_arr;
	int word_limit = 256; //the amount of words that we can store in the struct array
	size_t struct_buf = sizeof(struct file);
	file_properties = malloc(struct_buf * word_limit);
	struct_buf = sizeof(struct file) * word_limit;
	while((dir_struct = readdir(dir)) != NULL){
		char *file_name = dir_struct->d_name;
		if (i >= word_limit){
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
		file_properties[i].name = malloc(strlen(file_name)+1); //free this (recursively probably)
		if(file_properties[i].name == NULL){
			perror("malloc");
			exit(-1);
		}
		strcpy(file_properties[i].name,file_name);
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

void free_struct_mem(struct file *file_properties,int len){
	int i;
	for(i=0;i<len;i++){
		free(file_properties[i].name);
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

void display_result(int len,struct file *file_properties,int mode,char *current_directory){//mode(1) = row display, mode(2) = column display
	int i;
	if(mode != 1){
		printf("%s:\n",current_directory);
		printf("total: %d\n",actual_number_files);
	}
	for(i=0;i < actual_number_files;i++){
		switch(mode) {
			case 1:
				if(file_properties[i].name[0] == '.'){
					continue;
				}
				if(i == actual_number_files - 1){
					printf("%s\n",file_properties[i].name);
				}
				else{
					printf("%s ",file_properties[i].name);
				}
				break;
			case 2:
				printf("%s\n",file_properties[i].name);
				break;
		}
	}
}

void pop_struct_then_display(DIR *dir,int mode,char *current_directory){
	struct dirent *dir_struct;
	struct file *file_properties;
	file_properties = populate_struct(dir_struct, dir);
	display_result(actual_number_files, file_properties, mode,current_directory);
	free_struct_mem(file_properties, actual_number_files);
	free(file_properties);
}
int main(int argc, char*argv[]){
	if(argc < 2){
		printf("invalid number of arguments\n");
		exit(1);
	}
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
		}
		i++;
	}
	char directory[4095];
	int stat_result;
	struct stat stats;
	for(j=0;j < argc;j++){
		stat_result = stat(argv[j],&stats);
		if(stat_result == -1){
			perror("stat");
			exit(-1);
		}
		if(S_ISDIR(stats.st_mode)){
			strcpy(directory, argv[j]);
			DIR *dir = opendir(directory);
			if (!dir){
				perror("opendir");
				exit(1);
			}
			pop_struct_then_display(dir, display_mode,directory);
			closedir(dir);
		}
	}
	free(args);
	return 0;
}


