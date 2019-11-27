#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> // for close
#include <stdbool.h>

struct Tweeter{
  int number;
  char* name;
};

int open_file(int argc, char *argv[]);
int find_name_column(int fd);
char** find_tweeters(int fd, int num_commas, int author_comma);
int find_num_commas(int fd);
bool is_valid(char* line, int valid_commas);
char* get_author(char *line_buffer, int author_comma);

int main(int argc, char *argv[]){
  int fd = open_file(argc, argv);
  int name_column = find_name_column(fd);
  int num_commas = find_num_commas(fd);

  char ** res = find_tweeters(fd, num_commas, name_column);

  printf("%d\n", name_column);

  close(fd);
}

char** find_tweeters(int fd, int num_commas, int author_comma){
  struct Tweeter tweeters[20000]; // declare an array of maximum number of tweeters
  FILE* file_stream = fdopen(fd, "r");
  int num_tweeters = 0;
  bool found_tweeter = false;

  if (!file_stream){
    exit(1);
  }

  char* line_buffer = (char*) malloc(1024);

  int i = 0;

  while(fgets(line_buffer, 1024, file_stream)){

    //printf("%s\n", line_buffer);
    if(!is_valid(line_buffer, num_commas)){
      printf("Invalid line\n");
      exit(1);
    }

    char* author = get_author(line_buffer, author_comma);

    //printf("%s\n", author);

    for(int i = 0; i < num_tweeters; i++){

      // found tweeter
      if(strcmp(author, tweeters[i].name) == 0){
        tweeters[i].number++;
        found_tweeter = true;
      }
    }

    if(!found_tweeter){
      tweeters[num_tweeters].name = author;
      tweeters[num_tweeters].number = 1;
      num_tweeters++;
    }

    found_tweeter = false;
  }

  printf("%d\n", num_tweeters);

  return NULL;
}

char* get_author(char *line_buffer, int author_comma){
  int start_pos = 0;
  int commas_found = 0;

  //printf("%s\n", line_buffer);
  for(start_pos = 0; commas_found < author_comma; start_pos++){
    if(line_buffer[start_pos] == ',')
      commas_found += 1;
  }

  int end_pos;

  //printf("%d\n", start_pos);

  for(end_pos = start_pos + 1; end_pos < 1024; end_pos++){
    if(line_buffer[end_pos] == ',')
      break;
  }

  //printf("%d\n", end_pos);

  char *res = (char*) malloc(end_pos - start_pos + 1);
  strncpy(res, line_buffer + start_pos, end_pos - start_pos);

  return res;
}


bool is_valid(char* line, int valid_commas){
  int line_len = strlen(line);
  int num_commas = 0;

  for(int i = 0; i < line_len; i++){
    if(line[i] == ',')
      num_commas++;
  }

  if(valid_commas == num_commas){
    return true;
  }

  printf("%d, %d, %s\n", valid_commas, num_commas, line);
  return false;
}

/*
 * Finds the number of commas in the header line. To be used to verify each
 * subsequent line is valid.
 */
int find_num_commas(int fd){
  FILE* file_stream = fdopen(fd, "r");

  if(file_stream == NULL){
    printf("Could not open file stream\n");
    exit(1);
  }

  // allocate 1024 bytes for the buffer
  char *buffer = (char*) malloc(1024);
  int max_buffer = 1024;

  char* res = fgets(buffer, max_buffer, file_stream);

  if(!res){
    printf("Line read in longer than 1024\n");
    exit(1);
  }

  int num_commas = 0;
  int num_chars = strlen(buffer);

  for(int i = 0; i < num_chars; i++){
    if(buffer[i] == ',')
      num_commas++;
  }

  lseek(fd, 0, SEEK_SET);

  return num_commas;
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
  int max_buffer = 1024;

  char* res = fgets(buffer, max_buffer, file_stream);

  if(!res){
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

  lseek(fd, 0, SEEK_SET);

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
