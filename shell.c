#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include "readline.c"

#ifndef PATH_H
#define PATH_H _
  #include "path.h"
#endif

#ifndef COMMAND_H
#define COMMAND_H _
#include "command.h"
#endif

#ifndef PARSER_H
#define PARSER_H _
#include "parser.h"
#endif

#ifndef UTILS_H
#define UTILS_H _
#include "utils.h"
#endif

#define MAX_PROC 5

int** pids_matrix;
struct path my_path;

void clear_pids_matrix(int** pids_matrix);

void shut_down() {
  printf("Clearing opened processes...\n");
  clear_pids_matrix(pids_matrix);
  printf("...and shut down.\n");
  free(pids_matrix);
  free_path(my_path);
  exit(0);
}

void sig_handler(int sig) {
  signal(SIGINT, sig_handler);
  printf("\nWant to quit ? [ENTER/y]");
  fflush(stdout);
  char* ans = readline();

  if (ans[0] == 'y' || ans[0] == '\0') {
    shut_down();
  }

  printf("Poursuing execution...\n");
  printf("%s:%s$ ", get_env_var_value((char*)"USER", my_path), get_env_var_value((char*)"PWD", my_path));
  fflush(stdout);
}

void clear_pids_matrix(int** pids_matrix) {
  int i = 0;
  int j = 0;
  while(i < MAX_PROC) {
    if (pids_matrix[i] != NULL) {
      j = 0;
      while(pids_matrix[i][j] != -1) {
        if (pids_matrix[i][j] != 0) {
          kill(pids_matrix[i][j], SIGKILL);
          printf("Killed process %i\n", pids_matrix[i][j]);
        }
        j++;
      }
    }
    i++;
  }
}

int update_pids_matrix(int list_len) {
  int i = 0;
  int j = 0;
  int alive = 1;
  while(i < MAX_PROC) {
    if (pids_matrix[i] != NULL) {
      j = 0;
      while(pids_matrix[i][j] != -1) {
        if (-1 == kill(pids_matrix[i][j], 0))
          alive = 0;
        else
          alive = 1;
        if (pids_matrix[i][j] != 0 && !alive) {
          pids_matrix[i][j] = 0;
          if (nbr_of_alive_pids_in_tab(pids_matrix[i]) == 0) {
            pids_matrix[i] = NULL;
            list_len--;
            break;
          }
        }
        j++;
      }
    }
    i++;
  }
  return list_len;
}

void show_pids() {
  int i = 0;
  int j = 0;
  while(i < MAX_PROC) {
    if (pids_matrix[i] != NULL) {
      j = 0;
      while(pids_matrix[i][j] != -1) {
        printf("pids_matrix[%i][%i] = %i\n", i, j, pids_matrix[i][j]);
        j++;
      }
    }
    i++;
  }
}

int main(int argc, char** argv, char**envp) {
  signal(SIGINT, sig_handler);

  pids_matrix = (int**) malloc(sizeof(int*)*MAX_PROC+1);
  for (int i = 0; i < MAX_PROC; i++) {
    pids_matrix[i] = NULL;
  }
  int pids_list_counter = 0;

  int* pids_from_exec = (int*) malloc(sizeof(int));
  pids_from_exec[0] = -1;

  my_path = create_path(envp);
  show_path(my_path);

  int print_it = 0;

  for (;;) {
    printf("%s:%s$ ", get_env_var_value((char*)"USER", my_path), get_env_var_value((char*)"PWD", my_path));
    fflush(stdout);
    char* line = readline();

    char** words = split_in_words(line);
    struct command* my_command = parse_line(words, 0);

    if (nbr_of_alive_pids_in_matrix(pids_matrix, MAX_PROC) >= MAX_PROC) {
      printf("Cannot load more processes (%i), please wait or kill this processus\n", MAX_PROC);
    } else {
      if (my_command->valid) {

        int* pids_from_exec = parser(my_command, my_path);

        if (pids_from_exec != NULL) {

          if (pids_from_exec[0] == -2) {  //Quit
            free_command(my_command);
            free(words);
            free(line);
            shut_down();
          }

          int i = 0;
          while(pids_matrix[i] != NULL)
            i++;
          pids_matrix[i] = pids_from_exec;
          pids_list_counter++;

          printf("[%i] : %i\n", print_it, pids_from_exec[0]);
          print_it++;
        }

        free_command(my_command);
      }
    }

    pids_list_counter = update_pids_matrix(pids_list_counter);

    //show_pids();

    free(words);
    free(line);
  }
  return 0;
}
