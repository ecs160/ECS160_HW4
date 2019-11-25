#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> // for close

int open_file(int argc, char *argv[]);
int find_name_column(int fd);

int main(int argc, char *argv[]){
  int fd = open_file(argc, argv);
  int name_column = find_name_column(fd);

  printf("%d\n", name_column);

  close(fd);
}

/*
 * This function parses the
 */
int find_name_column(int fd){
  FILE* file_stream = fdopen(fd, "r");

  if(file_stream == NULL){
    printf("Could not open file stream\n");
    exit(1);
  }

  // allocate 1024 bytes for the buffer
  char *buffer = (char*) malloc(1024);
  size_t max_buffer = 1024;

  ssize_t bytes_read = getline(&buffer, &max_buffer, file_stream);

  if(bytes_read > 1024){
    printf("Line read in longer than 1024\n");
    exit(1);
  }

  char *token = strtok(buffer, ",");
  int position = 0;

  if(token == NULL){
    printf("Could not tokenize string to commas.");
    exit(1);
  }

  while(token != NULL){
    if(strcmp(token, "name") == 0 || strcmp(token, "\"name\"") == 0){
      break;
    }

    token = strtok(NULL, ",");
    position++;
  }

  return position;
}

int open_file(int argc, char *argv[]){
  // check if number of arguments is correct. (=1)
  if(argc != 2){
    printf("Invalid arguments\n");
    exit(1);
  }

  // make sure no overflow attacks with a long file name
  size_t name_length = strnlen(argv[1], 1024);

  if(name_length > 1024){
    printf("file name too long\n");
    exit(1);
  }

  int fd = open(argv[1], O_RDONLY);

  // file could not be opened
  if(fd == -1){
    printf("File could not be opened, %d.\n", fd);
    exit(1);
  }

  return fd;
}
