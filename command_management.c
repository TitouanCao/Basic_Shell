#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef environment_H
#define environment_H _
  #include "environment.h"
#endif

#ifndef COMMAND_H
#define COMMAND_H _
#include "command.h"
#endif

#ifndef COMMANDT_H
#define COMMANDT_H _
#include "command_management.h"
#endif

#ifndef UTILS_H
#define UTILS_H _
#include "utils.h"
#endif

//Types of command
#define UNUSED 0
#define OVERWRITE 1
#define APPEND 2

/*
void zombie_birth(int sig) {
  printf("SIGNAL\n");
  wait(NULL);
}
*/
char* run_cd (char* destination, char* curr_location, struct environment my_environment) {
  char* next_place = (char*) malloc(strlen(destination)+1);
  strcpy(next_place, destination);

  if (next_place[0] == '/') {
    strtok(next_place, "/");
    curr_location = (char*) realloc(curr_location, strlen(next_place) + 1);
    strcpy(curr_location, next_place);
    free(next_place);
    return curr_location;
  } else {
    next_place = strtok(next_place, "/");
  }

  if (next_place == NULL) {
    free(next_place);
    return curr_location;
  } else if (!strcmp(next_place, "~")) {
    free(next_place);
    return get_env_var_value((char*)"HOME", my_environment);
  } else if (!strcmp(next_place, ".")){
    free(next_place);
    return curr_location;
  } else if (!strcmp(next_place, "..")) {
    int decal = strlen(next_place) + 1;
    free(next_place);
    return run_cd(destination += decal, cut_string_from_tip(curr_location, '/'), my_environment);
  } else {
    curr_location = (char*) realloc(curr_location, strlen(curr_location) + strlen(next_place) + 2);
    strcat(curr_location, "/");
    strcat(curr_location, next_place);
    int decal = strlen(next_place) + 1;
    //free(next_place);
    return run_cd(destination += decal, curr_location, my_environment);
  }
}

void run_pwd (char* env_var_pwd) {
  printf("%s\n", env_var_pwd);
}

void process_redirection(struct command* my_command, int pipe_in, int pipe_out, struct environment my_environment) {
  int input_fd;
  if (my_command->input == NULL) {
    if (pipe_in != -1) {
      input_fd = pipe_in;
    }
    else input_fd = 0; //Keyboard
  } else {
    input_fd = open(my_command->input, O_RDONLY);
  }

  if (input_fd == -1) {
    printf("Error opening input file\n");
    exit(EXIT_FAILURE);
  }

  int output_fd;
  if (my_command->output == NULL) {
    if (pipe_out != -1)
      output_fd = pipe_out;
    else output_fd = 1; //Screen
  } else {
    if (my_command->type == OVERWRITE) {
      output_fd = open(my_command->output, O_WRONLY | O_TRUNC | O_CREAT);
    }
    else if (my_command->type == APPEND) {
      output_fd = open(my_command->output, O_WRONLY | O_APPEND | O_CREAT);
    }
  }

  if (output_fd == -1) {
    printf("Error opening output file\n");
    close(input_fd);
    exit(EXIT_FAILURE);
  }

  dup2(input_fd, STDIN_FILENO);
  dup2(output_fd, STDOUT_FILENO);
}


void execution(struct command* my_command, struct environment my_environment) {
  chdir(get_env_var_value((char*)"PWD", my_environment));

  char* environment = get_env_var_value((char*)"PATH", my_environment);

  char* environment_copy = (char*) malloc(strlen(environment)+1);
  strcpy(environment_copy, environment);

  char** path = (char**) malloc(sizeof(char*)*strlen(environment_copy));

  int i = 0;
  int j = 0;
  char* buff = strtok(environment_copy, ":");
  while (buff != NULL) {
    path[i] = (char*) malloc(strlen(buff)+1);
    strcpy(path[i], buff);
    strcat(path[i], "/");
    i++;
    buff = strtok(NULL, ":");
  }

  free(environment_copy);
  j = i-1;
  i = 0;

  char* name_buff = (char*) malloc(strlen(my_command->name)+strlen(path[0])+1);
  name_buff[0] = '\0';
  strcat(strcat(name_buff, path[0]), my_command->name);

  while (execve(name_buff, my_command->options, environment_to_array(my_environment)) == -1) {
    if (i >= j) {
      printf("Could not find the command %s in this computer\n", my_command->name);
      exit(EXIT_FAILURE);
    }

    i++;
    name_buff = (char*) realloc(name_buff, strlen(my_command->name)+strlen(path[i])+1);
    name_buff[0] = '\0';
    strcat(strcat(name_buff, path[i]), my_command->name);

  }
}

int* launch_command(struct command* my_command, struct environment my_environment) {
  int* pids = (int*) malloc(sizeof(int)*2);
  pids[1] = -1;

  int fork_count = 0;
  int break_pipe = 0;

  int need_pipe = 0;
  int need_another_pipe = 0;
  int* pipefd = (int*) malloc(sizeof(int)*2);;
  int* pipefd2 = (int*) malloc(sizeof(int)*2);;

  int pipe_in = -1;
  int pipe_out = -1;

  if (my_command->pipe_entry != NULL)
    need_pipe = 1;

  if (need_pipe) {
    if (pipe(pipefd) == -1) {
      perror("pipe");
      return NULL;
    }
  }

  pids[0] = fork();
  fork_count++;

  break_pipe = (my_command->output != NULL) && need_pipe;

  if (pids[0] == -1)
    printf("Error during fork\n");
  else if (pids[0] == 0) {

    if (need_pipe && !break_pipe) {
      close(pipefd[0]);
      pipe_out = pipefd[1];
    } else pipe_out = -1;

    process_redirection(my_command, pipe_in, pipe_out, my_environment);

    execution(my_command, my_environment);

    exit(2);  //Just in case
  } else {

    while (my_command->pipe_entry != NULL) {
      if (break_pipe)
        break;

      my_command = my_command->pipe_entry;

      pids = (int*) realloc(pids, sizeof(int)*fork_count+2);
      pids[fork_count+1] = -1;

      need_another_pipe = 0;
      if (my_command->pipe_entry != NULL)
        need_another_pipe = 1;

      if (need_another_pipe) {
        if (pipe(pipefd2) == -1) {
          perror("pipe");
          return NULL;
        }
      }

      pids[fork_count] = fork();
      fork_count++;

      if (pids[fork_count-1] == -1)
        printf("Error during fork\n");
      else if (pids[fork_count-1] == 0) {

        if (!break_pipe) {
          close(pipefd[1]);
          pipe_in = pipefd[0];
        } else pipe_in = -1;

        if (need_another_pipe) {
          close(pipefd2[0]);
          pipe_out = pipefd2[1];
        } else pipe_out = -1;

        process_redirection(my_command, pipe_in, pipe_out, my_environment);

        execution(my_command, my_environment);

        exit(2);
      } else {
        if (need_pipe) {
          close(pipefd[0]);
          close(pipefd[1]);
        }
        pipefd[0] = pipefd2[0];
        pipefd[1] = pipefd2[1];
      }
    }
  }

  if (need_another_pipe) {
    close(pipefd2[0]);
    close(pipefd2[1]);
  }

  if (my_command->bkgrd) {
    int i = 0;
    while (pids[i] != -1) {
      waitpid(pids[i], NULL, WNOHANG);
      i++;
    }
    return pids;
  } else {
    int i = 0;
    while (pids[i] != -1) {
      waitpid(pids[i], NULL, 0);
      i++;
    }
    return NULL;
  }
}
