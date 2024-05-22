#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#define UPPER_SIZE 4294967295

struct file{
	char *name;
};

int actual_number_files;

struct file *populate_struct(struct dirent *dir_struct,DIR *dir){
	int i = 0;
	struct file *new_ptr = NULL; //new ptr to store potential realloc return value
	struct file *file_properties;
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
			printf("[-] reached max memory\n");
			printf("[+] allocating more memory\n");
			printf("[+] current buffer_size = %lu\n",struct_buf);
			printf("[+] current word_limit = %d\n",word_limit);
			word_limit = word_limit * 2;
			struct_buf = sizeof(struct file) * word_limit;
			new_ptr = realloc(file_properties,struct_buf);
			if (new_ptr == NULL){
				perror("realloc");
				exit(-1);
				free(file_properties);
			}
			file_properties = new_ptr;
		}
		file_properties[i].name = malloc(strlen(file_name)+1); //free this (recursively probably)
		strcpy(file_properties[i].name,file_name);
		i++;
	}
	struct file *final_arr = realloc(new_ptr, i * sizeof(struct file));
	printf("%d\n",i);
	actual_number_files = i;
	return final_arr;
}

void free_struct_mem(struct file *file_properties,int len){
	int i;
	for(i=0;i<len;i++){
		free(file_properties[i].name);
	}
}

int main(int argc, char*argv[]){
	if(argc != 2){
		printf("invalid number of arguments\n");
		exit(1);
	}
	struct dirent *dir_struct;
	char *directory = argv[1];
	DIR *dir = opendir(directory);
	char *dir_name[256];
	if (!dir){
		perror("opendir");
		exit(1);
	}
	struct file *file_properties = populate_struct(dir_struct,dir);
	int i;
	printf("the amount of files : %d\n",actual_number_files);
	for(i=0;i < actual_number_files;i++){
		printf("%s\n",file_properties[i].name); //todo: this causes segfaults ;-;
	}
	free_struct_mem(file_properties, actual_number_files);
	free(file_properties);
	closedir(dir);
	return 0;
}


