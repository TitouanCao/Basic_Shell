#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#ifndef environment_H
#define environment_H _
  #include "environment.h"
#endif

#ifndef UTILS_H
#define UTILS_H _
#include "utils.h"
#endif

struct environment* create_environment (char** env_var) {
  //Allocation and default assignation
  struct environment* my_environment = (struct environment*) malloc(sizeof(struct environment));
  my_environment->size = 0;
  my_environment->rows = NULL;

  if (env_var == NULL)  //Check we have an environment
    return my_environment;

  int default_size = 50;  //Minimal default size for variable array

  my_environment->rows = (struct row*) malloc(default_size*sizeof(struct row)); //Default allocation of pointers to rows

  int i = 0;

  //Allocation and assignation of all rows
  //Buffers allocation
  char* curr_row = (char*) malloc(strlen(env_var[i]) + 1);
  char* name_buff = (char*) malloc(strlen(env_var[i]) + 1);

  while(env_var[i] != NULL) {
    //Buffers reallocation, adapting to the current variable
    curr_row = (char*) realloc(curr_row, strlen(env_var[i]) + 1);
    name_buff = (char*) realloc(name_buff, strlen(env_var[i]) + 1);

    strcpy(curr_row, env_var[i]); //Copy the content into the content buffer
    strcpy(name_buff, curr_row);  //Copy the content buffer into the name buffer
    strtok(name_buff, "=");   //Get the string before the = in the content buffer (= is an illegal character to be in the content, that is why strtok will return the correct value for curr_row)
    curr_row += strlen(name_buff) + 1;  //Moving the buffer content so it does not contain the name

    //Allocating and copying name and content in the corresponding fields
    my_environment->rows[i].header = (char*) malloc(strlen(name_buff) + 1);
    my_environment->rows[i].content = (char*) malloc(strlen(curr_row) + 1);
    strcpy(my_environment->rows[i].header, name_buff);
    strcpy(my_environment->rows[i].content, curr_row);

    curr_row -= strlen(name_buff) + 1; //Moving back the pointer to its original position so it can be freed/realloced

    my_environment->size++; //Iterate environment size
    if (my_environment->size >= default_size) { //If we reach the default size we realloc the rows
      default_size = 2*my_environment->size;  //We double it so we don't have to do it frequently
      my_environment->rows = (struct row*) realloc(my_environment->rows, default_size*sizeof(struct row));
    }
    i++;
  }
  //Liberate buffers
  free(curr_row);
  free(name_buff);

  //Realloc to the just fitting size
  my_environment->rows = (struct row*) realloc(my_environment->rows, my_environment->size*sizeof(struct row));

  //Returns the struct
  return my_environment;
}

char** environment_to_array(struct environment* my_environment) {
  char** env_var = (char**) malloc(my_environment->size*sizeof(char*)); //Allocating a new char**
  for (int i = 0; i < my_environment->size; i++) {  //Allocating and copying each variable
    env_var[i] = (char*) malloc(strlen(my_environment->rows[i].header) + strlen(my_environment->rows[i].content) + 1);
    env_var[i][0] = '\0';
    strcat(strcat(strcat(env_var[i], my_environment->rows[i].header), "="), my_environment->rows[i].content);
  }
  return env_var;
}

void show_environment(struct environment* my_environment) {
  for(int i = 0; i < my_environment->size; i++) {
    printf("[%i]%s=%s\n", i, my_environment->rows[i].header, my_environment->rows[i].content);
  }
}


int is_valid_row (struct row my_row) {
  if (my_row.header == NULL || my_row.content == NULL)
    return 0;
  return 1;
}


struct row get_env_var (char* name, struct environment* my_environment) {
  //Set default row
  struct row def_res;
  def_res.header = NULL;
  def_res.content = NULL;
  if (my_environment->rows == NULL) return def_res;

  int i = 0;
  if (!is_valid_row(my_environment->rows[i]))  //Check if the row is valid
    return def_res;
  while(strcmp(my_environment->rows[i].header, name)) { //While the name does not match
    i++;
    if (i >= my_environment->size)  //If we didn't find
      return def_res;
    if (!is_valid_row(my_environment->rows[i])) //Check again
      return def_res;
  }
  return my_environment->rows[i]; //Return the matching row
}

char* get_env_var_value (char* name, struct environment* my_environment) {
  struct row targ = get_env_var(name, my_environment);  //Get the row
  if (is_valid_row(targ)) //Check again but the previous function already checked
    return targ.content;  //Return the content
  return NULL;
}

//Same but with index, -1 if not found
int get_env_var_index (char* name, struct environment* my_environment) {
  if (my_environment->rows == NULL) return -1;

  int i = 0;
  if (!is_valid_row(my_environment->rows[i]))
    return -1;
  while(strcmp(my_environment->rows[i].header, name)) {
    i++;
    if (!is_valid_row(my_environment->rows[i]))
      return -1;
  }
  return i;
}

int set_env_var(char* name, char* value, struct environment* my_environment) {
  if (!is_valid_environment_var(value)) //Check for illegal characters
    return -1;

  struct row targ = get_env_var(name, my_environment);  //get the row
  if (is_valid_row(targ)) { //Check for illegal row
    if (!strcmp(name, "PWD")) //Case we change pwd
      if(!is_valid_pwd(value))  //Check for existing directory
        return -2;
    if (strlen(value) > strlen(targ.content)) { //If new value is longer than the previous one
      int index = get_env_var_index(name, my_environment);  //Get corresponding index
      if (index == -1)  //Check error
        return index; //Return error
      my_environment->rows[index].content = (char*) realloc(my_environment->rows[index].content, strlen(value)+1);  //Realloc with the right size
      targ = get_env_var(name, my_environment); //Get again
    }
    strcpy(targ.content, value);  //Update content
    return 0;
  } else {
    return -3;
  }
}

void free_environment(struct environment* my_environment) {
  if (my_environment == NULL)
    return;
  if (my_environment->rows != NULL) {
    for (int i = 0; i < my_environment->size; i++) {  //Free each row
      free(my_environment->rows[i].header);
      free(my_environment->rows[i].content);
    }
    free(my_environment->rows); //Free the array of rows
  }
}
