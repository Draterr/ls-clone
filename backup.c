#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#define UPPER_SIZE 4294967295

void free_inner(int len, char **arr){
	int i;
	for(i=0;i < len;i++){
		free(arr[i]);
	}
}

void print_2darray(int len,char **arr){
	int i;
	for(i=0;i<len;i++){
		printf("%s\n",arr[i]);
	}
}
char **filenames(struct dirent *dir_struct,DIR *dir){
	int i=0;
	int word_limit = 256; //the amount of words that we can store in the 2D array
	char **new_ptr;	//new ptr to store potential realloc return value
	size_t buffer_size = (sizeof(char *)*256);
	char **file_names = malloc(buffer_size);
	while((dir_struct= readdir(dir)) != NULL){
		char *file_name = dir_struct->d_name;
		if (i >= word_limit){
			if (buffer_size >= UPPER_SIZE){
				printf("Reached maxium amount of memory allowed\nexiting...");
				exit(-1);
			}
			printf("[-] reached max memory\n");
			printf("[+] allocating more memory\n");
			printf("[+] current buffer_size = %lu\n",buffer_size);
			printf("[+] current word_limit = %d\n",word_limit);
			word_limit = word_limit*2;
			new_ptr = realloc(file_names,buffer_size*2);
			buffer_size = buffer_size * 2;
			if (!new_ptr){
				perror("realloc");
				exit(-1);
				free(file_names);
			}
			file_names = new_ptr;
		}
		file_names[i] = malloc(256);
		strcpy(file_names[i],file_name);
		i++;
	}
		print_2darray(i,file_names);
		free_inner(i,file_names);
		if(new_ptr){
			free(new_ptr);
		}
	return file_names;
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
	filenames(dir_struct, dir);
	// print_2darray(i, file_names);
	// free_inner(i, file_names);
	// free(new_ptr);
	closedir(dir);
	return 0;
}

