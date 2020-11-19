#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef PATH_H
#define PATH_H _
  #include "path.h"
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


void zombie_birth(int sig) {
  wait(NULL);
}

char* run_cd (char* destination, char* curr_location, struct path my_path) {
  char* next_place = (char*) malloc(strlen(destination)+1);
  strcpy(next_place, destination);
  next_place = strtok(next_place, "/");

  if (next_place == NULL) {
    free(next_place);
    return curr_location;
  } else if (!strcmp(next_place, "~")) {
    free(next_place);
    return get_env_var_value((char*)"HOME", my_path);
  } else if (!strcmp(next_place, ".")){
    free(next_place);
    return curr_location;
  } else if (!strcmp(next_place, "..")) {
    int decal = strlen(next_place) + 1;
    free(next_place);
    return run_cd(destination += decal, cut_string_from_tip(curr_location, '/'), my_path);
  } else {
    curr_location = (char*) realloc(curr_location, strlen(curr_location) + strlen(next_place) + 2);
    strcat(curr_location, "/");
    strcat(curr_location, next_place);
    int decal = strlen(next_place) + 1;
    free(next_place);
    return run_cd(destination += decal, curr_location, my_path);
  }
}

void run_pwd (char* env_var_pwd) {
  printf("%s\n", env_var_pwd);
}

void process_redirection(struct command* my_command, struct path my_path) {
  if (my_command->type == OVERWRITE || my_command->type == APPEND) {
    int input_fd;
    if (my_command->input == NULL) {
      input_fd = 0; //Keyboard
    } else {
      input_fd = open(my_command->input, O_RDONLY);
    }

    if (input_fd == -1) {
      printf("Error openning input file\n");
      exit(EXIT_SUCCESS);
    }

    int output_fd;
    if (my_command->output == NULL) {
      output_fd = 1; //Screen
    } else {
      if (my_command->type == OVERWRITE) {
        output_fd = open(my_command->output, O_WRONLY | O_TRUNC | O_CREAT);
      }
      else if (my_command->type == APPEND) {
        output_fd = open(my_command->output, O_WRONLY | O_APPEND | O_CREAT);
      }
    }

    if (output_fd == -1) {
      printf("Error openning output file\n");
      close(input_fd);
      exit(EXIT_SUCCESS);
    }

    dup2(input_fd, STDIN_FILENO);
    dup2(output_fd, STDOUT_FILENO);
  } else {
    printf("Error in command management : meaningless type\n");
    exit(EXIT_SUCCESS);
  }
}


void execution(struct command* my_command, struct path my_path) {
  chdir(get_env_var_value((char*)"PWD", my_path));

  //Is the code in /bin/... ???
  char* name_buff = (char*) malloc(strlen(my_command->name)+6);
  name_buff[0] = '\0';
  strcat(strcat(name_buff, "/bin/"), my_command->name);

  if (execve(name_buff, my_command->options, path_to_array(my_path)) == -1) {
    //NOP ! Maybe in /usr/bin
    name_buff = (char*) realloc(name_buff, strlen(my_command->name)+10);
    name_buff[0] = '\0';
    strcat(strcat(name_buff, "/usr/bin/"), my_command->name);

    if (execve(name_buff, my_command->options, path_to_array(my_path)) == -1) {
      printf("Could not find the command %s\n", my_command->name); //No !!
      exit(EXIT_SUCCESS);
    }
    //Yes !!
  }
}

int* launch_command(struct command* my_command, struct path my_path) {
  int* pids = (int*) malloc(sizeof(int)*2);
  pids[1] = -1;

  int fork_count = 0;

  int need_pipe = 0;
  int* pipefd = (int*) malloc(sizeof(int)*2);;
  int* pipefd2 = (int*) malloc(sizeof(int)*2);;

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

  if (pids[0] == -1)
    printf("Error during fork\n");
  else if (pids[0] == 0) {

    if (need_pipe) {
      close(pipefd[0]);
      dup2(pipefd[1], STDOUT_FILENO);
    }

    if (my_command->type != UNUSED)
      process_redirection(my_command, my_path);
    execution(my_command, my_path);

    exit(2);  //Just in case
  } else {

    while (my_command->pipe_entry != NULL) {
      my_command = my_command->pipe_entry;

      pids = (int*) realloc(pids, sizeof(int)*fork_count+2);
      pids[fork_count+1] = -1;

      int need_another_pipe = 0;
      if (my_command->pipe_entry != NULL)
        need_another_pipe = 1;

      if (need_another_pipe) {
        if (pipe(pipefd) == -1) {
          perror("pipe");
          return NULL;
        }
      }

      pids[fork_count] = fork();
      fork_count++;

      if (pids[fork_count-1] == -1)
        printf("Error during fork\n");
      else if (pids[fork_count-1] == 0) {

        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);

        if (need_another_pipe) {
          close(pipefd2[0]);
          dup2(pipefd2[1], STDOUT_FILENO);
        }

        if (my_command->type != UNUSED)
          process_redirection(my_command, my_path);
        execution(my_command, my_path);

        exit(2);
      } else {
        pipefd[0] = pipefd2[0];
        pipefd[1] = pipefd2[1];
      }
    }
  }

  close(pipefd[0]);
  close(pipefd[1]);
  close(pipefd2[0]);
  close(pipefd2[1]);

  if (my_command->bkgrd) {
    signal(SIGCHLD, zombie_birth);
    return pids;
  } else {
    wait(NULL);
    return NULL;
  }
}
