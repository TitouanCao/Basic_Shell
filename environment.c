#include <stdlib.h> //pour la definition de NULL
#include <string.h>
#include <stdio.h>
#include "environment.h"

#ifndef UTILS_H
#define UTILS_H _
#include "utils.h"
#endif

struct environment create_environment (char** env_var) {
  struct environment my_environment;
  my_environment.size = 0;
  my_environment.rows = NULL;

  if (env_var == NULL)
    return my_environment;

  int default_size = 50;

  my_environment.rows = (struct row*) malloc(default_size*sizeof(struct row)); //Taille par défaut de base

  int i = 0;

  char* curr_row = (char*) malloc(strlen(env_var[i]) + 1);
  char* name_buff = (char*) malloc(strlen(env_var[i]) + 1);

  while(env_var[i] != NULL) {
    curr_row = (char*) realloc(curr_row, strlen(env_var[i]) + 1);
    name_buff = (char*) realloc(name_buff, strlen(env_var[i]) + 1);

    strcpy(curr_row, env_var[i]);
    strcpy(name_buff, curr_row);
    strtok(name_buff, "=");
    curr_row += strlen(name_buff) + 1;

    my_environment.rows[i].header = (char*) malloc(strlen(name_buff) + 1);
    my_environment.rows[i].content = (char*) malloc(strlen(curr_row) + 1);
    strcpy(my_environment.rows[i].header, name_buff);
    strcpy(my_environment.rows[i].content, curr_row);

    curr_row -= strlen(name_buff) + 1; //On ramène le pointeur à son emplacement initial

    my_environment.size++;
    if (my_environment.size >= default_size) {
      default_size = 2*my_environment.size;
      my_environment.rows = (struct row*) realloc(my_environment.rows, default_size*sizeof(struct row)); //Pour ne pas réallouer souvent
    }
    i++;
  }
  my_environment.rows = (struct row*) realloc(my_environment.rows, my_environment.size*sizeof(struct row));

  return my_environment;
}

char** environment_to_array(struct environment my_environment) {
  char** env_var = (char**) malloc(my_environment.size*sizeof(char*));
  for (int i = 0; i < my_environment.size; i++) {
    env_var[i] = (char*) malloc(strlen(my_environment.rows[i].header) + strlen(my_environment.rows[i].content) + 1);
    env_var[i][0] = '\0';
    strcat(strcat(strcat(env_var[i], my_environment.rows[i].header), "="), my_environment.rows[i].content);
  }
  return env_var;
}

void show_environment(struct environment my_environment) {
  for(int i = 0; i < my_environment.size; i++) {
    printf("[%i]%s=%s\n", i, my_environment.rows[i].header, my_environment.rows[i].content);
  }
}


int is_valid_row (struct row my_row) {
  if (my_row.header == NULL || my_row.content == NULL)
    return 0;
  return 1;
}


struct row get_env_var (char* name, struct environment my_environment) {
  struct row def_res;
  def_res.header = NULL;
  def_res.content = NULL;
  if (my_environment.rows == NULL) return def_res;

  int i = 0;
  if (my_environment.rows[i].header == NULL || my_environment.rows[i].content == NULL)
    return def_res;
  while(strcmp(my_environment.rows[i].header, name)) {
    i++;
    if (my_environment.rows[i].header == NULL || my_environment.rows[i].content == NULL)
      return def_res;
  }
  return my_environment.rows[i];
}

char* get_env_var_value (char* name, struct environment my_environment) {
  struct row targ = get_env_var(name, my_environment);
  if (is_valid_row(targ))
    return targ.content;
  return NULL;
}

int get_env_var_index (char* name, struct environment my_environment) {
  if (my_environment.rows == NULL) return -1;

  int i = 0;
  if (my_environment.rows[i].header == NULL || my_environment.rows[i].content == NULL)
    return -1;
  while(strcmp(my_environment.rows[i].header, name)) {
    i++;
    if (my_environment.rows[i].header == NULL || my_environment.rows[i].content == NULL)
      return -1;
  }
  return i;
}

int set_env_var(char* name, char* value, struct environment my_environment) {
  if (!is_valid_environment_var(value))
    return -1;

  struct row targ = get_env_var(name, my_environment);
  if (is_valid_row(targ)) {
    if (!strcmp(name, "PWD"))
      if(!is_valid_pwd(value))
        return -2;
    if (strlen(value) > strlen(targ.content)) {
      int index = get_env_var_index(name, my_environment);
      if (index == -1)
        return index;
      my_environment.rows[index].content = (char*) realloc(my_environment.rows[index].content, strlen(value)+1);
      targ = get_env_var(name, my_environment);
    }
    strcpy(targ.content, value);
    return 0;
  } else {
    return -3;
  }
}

void free_environment(struct environment my_environment) {
  if (my_environment.rows != NULL) {
    for (int i = 0; i < my_environment.size; i++) {
      free(my_environment.rows[i].header);
      free(my_environment.rows[i].content);
    }
    free(my_environment.rows);
  }
}
