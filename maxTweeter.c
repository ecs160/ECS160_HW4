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

struct Result{
  struct Tweeter* arr;
  int num_tweeters;
};

int open_file(int argc, char *argv[]);
int find_name_column(int fd);
struct Result* find_tweeters(int fd, int num_commas, int author_comma);
int find_num_commas(int fd);
bool is_valid(char* line, int valid_commas);
char* get_author(char *line_buffer, int author_comma);
int comparator(const void* p1, const void* p2);

int main(int argc, char *argv[]){
  int fd = open_file(argc, argv);
  int num_commas = find_num_commas(fd);
  //printf("%d\n", num_commas);
  int name_column = find_name_column(fd);

  struct Result* res = find_tweeters(fd, num_commas, name_column);

  for(int i = 0; i < res->num_tweeters; i++){
    printf("%s, %d\n", res->arr[i].name, res->arr[i].number);
  }

  close(fd);
}

/*
 * Runs the algorithm.
 */
struct Result* find_tweeters(int fd, int num_commas, int author_comma){
  struct Tweeter tweeters[20000]; // declare an array of maximum number of tweeters
  FILE* file_stream = fdopen(fd, "r");
  int num_tweeters = 0;
  bool found_tweeter = false;

  if (!file_stream){
    exit(0);
  }

  char* line_buffer = (char*) malloc(1024);

  int i = 0;

  while(fgets(line_buffer, 1024, file_stream)){

    //printf("%s\n", line_buffer);
    if(!is_valid(line_buffer, num_commas)){
      printf("Invalid line\n");
      exit(0);
    }

    char* author = get_author(line_buffer, author_comma);

    for(int i = 0; i < num_tweeters; i++){

      // found tweeter
      if(strcmp(author, tweeters[i].name) == 0){
        tweeters[i].number++;
        found_tweeter = true;
        break;
      }
    }

    // tweeter not found, make a new entry.
    if(!found_tweeter){
      tweeters[num_tweeters].name = author;
      tweeters[num_tweeters].number = 1;
      num_tweeters++;
    }

    // reset
    found_tweeter = false;
  }

  // sort the array of tweeters with reference to the number of tweets
  qsort((void*)tweeters, num_tweeters, sizeof(struct Tweeter), comparator);

  int result_size;

  // check if there were more than 10 distinct tweeters
  if(num_tweeters < 10) // if not, only return the number of distinct tweeters.
    result_size = num_tweeters;
  else
    result_size = 10;

  // get Top 10 tweeters
  struct Tweeter *result = (struct Tweeter*) malloc(result_size * sizeof(struct Tweeter));

  for(int i = 0; i < result_size; i++)
    result[i] = tweeters[i];

  struct Result *res = (struct Result*) malloc(sizeof(struct Result));
  res->arr = result;
  res->num_tweeters = result_size;

  return res;
}

int comparator(const void* p1, const void* p2){
  int l = ((struct Tweeter* )p1)->number;
  int r = ((struct Tweeter* )p2)->number;

  return (r - l);
}

/*
 * Find the author of a Tweet, given a line and the comma number at which the
 * author begins.
 */
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

/*
 * Checks whether the line has a correct number of commas.
 */
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
    exit(0);
  }

  // allocate 1024 bytes for the buffer
  char *buffer = (char*) malloc(1024);
  int max_buffer = 1024;

  char* res = fgets(buffer, max_buffer, file_stream);

  if(!res){
    printf("Line read in longer than 1024\n");
    exit(0);
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

int find_name_column(int fd){
  FILE* file_stream = fdopen(fd, "r");

  // checks whether the file stream if valid
  if(file_stream == NULL){
    printf("Could not open file stream\n");
    exit(0);
  }

  // allocate 1024 bytes for the buffer
  char *buffer = (char*) malloc(1024);
  int max_buffer = 1024;

  char* res = fgets(buffer, max_buffer, file_stream);

  if(!res){
    printf("Line read in longer than 1024\n");
    exit(0);
  }

  char *token = strtok(buffer, ",");
  int position = 0;

  if(token == NULL){
    printf("Could not tokenize string to commas.");
    exit(0);
  }

  while(token != NULL){
    if(strcmp(token, "name") == 0 || strcmp(token, "\"name\"") == 0){
      break;
    }

    token = strtok(NULL, ",");
    position++;
  }

  // set the file offset back to 0
  lseek(fd, 0, SEEK_SET);

  return position;
}

/*
 * Given the command line arguments, checks if they are valid and returns a file
 * descriptor to the file.
 */
int open_file(int argc, char *argv[]){
  // check if number of arguments is correct. (=1)
  if(argc != 2){
    printf("Invalid arguments\n");
    exit(0);
  }

  // make sure no overflow attacks with a long file name
  size_t name_length = strnlen(argv[1], 1024);

  if(name_length > 1024){
    printf("file name too long\n");
    exit(0);
  }

  int fd = open(argv[1], O_RDONLY);

  // file could not be opened
  if(fd == -1){
    printf("File could not be opened.\n");
    exit(0);
  }

  return fd;
}

/*int find_name_column(int fd, int num_commas){
  FILE* file_stream = fdopen(fd, "r");

  // checks whether the file stream if valid
  if(file_stream == NULL){
    printf("Could not open file stream\n");
    exit(0);
  }

  // allocate 1024 bytes for the buffer
  char *buffer = (char*) malloc(1024);
  int max_buffer = 1024;

  char* res = fgets(buffer, max_buffer, file_stream);

  if(!res){
    printf("Line read in longer than 1024\n");
    exit(0);
  }

  printf("%s\n", buffer);

  int comma_indexes[num_commas + 1];
  comma_indexes[0] = 0;
  int header_length = strlen(buffer);
  int comma_number = 1;

  for(int i = 0; i < header_length; i++){

    if(buffer[i] == ','){
      comma_indexes[comma_number] = i;
      comma_number++;
    }
  }

  for(int i = 0; i < comma_number; i++){
    int length = comma_indexes[i+1] - comma_indexes[i] - 1;
    char* str = (char*) malloc(length);
    strncpy(str, buffer + comma_indexes[i] + 1, length);
    //str[length + 1] = '\0';
    printf("%s\n", str);
    free(str);
  }


  // set the file offset back to 0
  lseek(fd, 0, SEEK_SET);

  return 0;
} */
