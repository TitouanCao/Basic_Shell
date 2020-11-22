#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include "ansi_colors.h"

#include "readline.c"

#ifndef environment_H
#define environment_H _
  #include "environment.h"
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


#define MAX_PROC 5  //Personnal choice to limit the amount of simultaneously running (group of) processes to 5

 /*Contains pids of processes running in background,
 A "row" is given to a command running in background
 in this row there can have multiples processes
 example: cat < toto | cat > titi &
 The two cat are independant processes both running in background
 */
int** pids_matrix;
struct environment* my_environment; //Variable containing environment variables

void clear_pids_matrix(int** pids_matrix);


/*
Calls the functions to clean all the processes,
free the currently used allocated slots,
and exit the main program
*/
void shut_down() {
  printf("Clearing opened processes...\n");
  clear_pids_matrix(pids_matrix);
  printf("...and shut down.\nThank you for using Basic Shell!\n");
  free(pids_matrix);
  free_environment(my_environment);
  exit(0);
}

/*
Signal handler for interruption
Called when receiving interruption signal,
mostly used to close properly and to react to Ctrl+c
*/
void sig_handler(int sig) {
  signal(SIGINT, sig_handler);
  printf(BRED "\nInterruption ! All processes stopped !\n" reset);
  printf("If you wish to quit Basic Shell you can type \"quit\" anytime.\n\n");
}

/*
Display the pwd before every command
*/
void display_start_line() {
  //Get the machine name
  char hostname[50];  //Maximum 49 characters
  hostname[49] = '\0';
  int check = gethostname(hostname, 49);
  if (check != -1)
    printf(BGRN "%s@%s", get_env_var_value((char*)"USER", my_environment), hostname);
  else
    printf(BGRN "%s", get_env_var_value((char*)"USER", my_environment));

  printf(BWHT ":");
  printf(BBLU "%s", get_env_var_value((char*)"PWD", my_environment));
  printf(BWHT "$ " reset);
}

/*
Browse through pids of background processes and
kill those whic are not dead yet
*/
void clear_pids_matrix(int** pids_matrix) {
  int i = 0;
  int j = 0;
  while(i < MAX_PROC) {
    if (pids_matrix[i] != NULL) {
      j = 0;
      while(pids_matrix[i][j] != -1) {
        if (pids_matrix[i][j] != 0) {
          if (-1==kill(pids_matrix[i][j], SIGKILL)) {
            printf("Could not kill process %i\n", pids_matrix[i][j]);
          } else printf("Killed process %i\n", pids_matrix[i][j]);
        }
        j++;
      }
    }
    i++;
  }
}

/*
Keep the pids matrix updated.
Check if the registered processes are still running
thank to a call to kill with sig = 0 (do nothing)
A value of 0 in this table means there is no process
or the process is dead and cleared (thanks to waipid)

NOT WORKING CORRECTLY WOTH WAITPID WNOHANG at the moment
KILL(..., 0) always returns 0, pids are not updated
*/
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
        printf("%i is %i\n", pids_matrix[i][j], alive);
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

/*
Unused functions that shows what's inside
pids matrix, can be called for debug
*/
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
  signal(SIGINT, sig_handler);  //Set the signal handle

  pids_matrix = (int**) malloc(sizeof(int*)*MAX_PROC+1);  //Pids matrix allocation
  for (int i = 0; i < MAX_PROC; i++) {
    pids_matrix[i] = NULL;       //Setting rows to NULL
  }
  int pids_list_counter = 0;    //Number of used rows

  int* pids_from_exec = (int*) malloc(sizeof(int)); //pids returned after the execution of a command
  pids_from_exec[0] = -1; //Set to empty: -1 = end of the row

  int print_it = 0; //Iterator for background processes count, only for display

  my_environment = create_environment(envp);  //Create environment struct from envp

  if (my_environment == NULL) { //In case we receive no envp
    printf("No environment variables found opening this program, quiting\n");
    exit(EXIT_FAILURE);
  }

  //Block used to clear the terminal window thanks to a call to clear
  char** init_args = (char**) malloc(sizeof(char*)*2);
  init_args[0] = (char*) malloc(strlen("clear")+1);
  init_args[1] = NULL;
  strcpy(init_args[0], "clear");
  struct command* init_command = parse_line(init_args, 0, my_environment);
  if(init_command->valid)
    parser(init_command, my_environment);
  free(init_command);

  for (;;) {
    //Shows the user and the current location before every command (exept background ones)
    display_start_line();

    fflush(stdout); //Refresh output
    char* line = readline();  //Get inputed text
    char** words = split_in_words(line); //Splits it into char**

    struct command* my_command = parse_line(words, 0, my_environment);  //Create a comand struct from the char**

    if (nbr_of_alive_pids_in_matrix(pids_matrix, MAX_PROC) >= MAX_PROC) { //Check if we reached the maximum amount of processes in background
      printf("Cannot load more processes (%i), please wait or kill this processus\n", MAX_PROC);
    } else {
      if (my_command->valid) {  //Check the command is valid

        //Parse the comand and execute it if correct, return background processes's pids
        int* pids_from_exec = parser(my_command, my_environment);

        //If the command generated background processes
        if (pids_from_exec != NULL) {

          //Special return: -2, not a pid, but a side use to express "quit"
          if (pids_from_exec[0] == -2) {  //Quit, empty allocated memory and die
            free(pids_from_exec);
            free_command(my_command);
            free(words);
            free(line);
            shut_down();
          }

          //Add returned pids to the matrix
          int i = 0;
          while (pids_matrix[i] != NULL)
            i++;
          pids_matrix[i] = pids_from_exec;
          pids_list_counter++;  //Iterate the counter

          //Displays that a new background process got created
          printf("[%i] : %i\n", print_it, pids_from_exec[0]);
          print_it++; //Iterate the corresponding counter
        }
      }
    }

    show_pids();

    free_command(my_command); //Liberate the memory allocated to the command structure

    pids_list_counter = update_pids_matrix(pids_list_counter);  //Updates the matrix pids and the counter

    free(words);  //Liberate memory allocated to the char* and char**
    free(line);
  }
  return 0;
}
