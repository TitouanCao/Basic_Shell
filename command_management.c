#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ansi_colors.h"

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

#define MAX_DIRNAME_LEN 30

// The / (slash) is the separator for elements
char* run_cd (char* destination, char* curr_location, struct environment* my_environment) {
  if (destination == NULL || curr_location == NULL) //Check non nullity
    return NULL;

  if (strlen(destination) == 0) //Nowhere to go
    return curr_location;

  //Copy the destination to split its first element, this pointer is freed anyway
  char* next_place = (char*) malloc(strlen(destination)+1);
  strcpy(next_place, destination);

  //Case we want to start from root
  if (next_place[0] == '/') {
    //Change current location
    curr_location = (char*) realloc(curr_location, 2);
    strcpy(curr_location, "/");

    free(next_place);
    //Goes recursive, iterate destination
    return run_cd(destination += 1 , curr_location, my_environment);
  } else {
    //Otherwise get the first element (directory) from the destination
    next_place = strtok(next_place, "/");
  }

  //Switch among different special characters
  if (!strcmp(next_place, "~")) { //Back to home
    free(next_place);
    return get_env_var_value((char*)"HOME", my_environment);  //Despising anything else, goes to home
  } else if (!strcmp(next_place, ".")) { //Nothing
    free(next_place);
    return curr_location;
  } else if (!strcmp(next_place, "..")) { //Goes one directory backwards
    int decal = strlen(next_place) + 1;
    free(next_place);
    //Call to a function removing the last element (/ being the separator)
    return run_cd(destination += decal, cut_string_from_tip(curr_location, '/'), my_environment);
  } else {
    //Not a special character
    curr_location = (char*) realloc(curr_location, strlen(curr_location) + strlen(next_place) + 2);

    //Check we are not in root (case there are two slashes or more like this //)
    if (strcmp(curr_location, "/") != 0 )
      strcat(curr_location, "/");

    //Concat the currlocation
    strcat(curr_location, next_place);
    int decal = strlen(next_place) + 1;

    free(next_place);
    //Goes recursive
    return run_cd(destination += decal, curr_location, my_environment);
  }
}

/*
Manages input and output file descriptors
/!\ Pipes are overwitten by redirections /!\
*/
void process_redirection(struct command* my_command, int pipe_in, int pipe_out, struct environment* my_environment) {

  //--------------INPUT-------------------//

  int input_fd;
  if (my_command->input == NULL) {  //Check if the command has an input (from redirection)
    if (pipe_in != -1) {  //If not check if we have an input from the previous command (pipe)
      input_fd = pipe_in;
    }
    else input_fd = 0; //Otherwise the standard input is still the keyboard
  } else {
    //Open the file given in argument
    input_fd = open(my_command->input, O_RDONLY);
  }

  //This should never happen as we check the validity of file during command creation
  if (input_fd == -1) {
    printf(BRED "Error opening input file\n" reset);
    exit(EXIT_FAILURE);
  }

  //--------------OUTPUT-------------------//

  int output_fd;
  if (my_command->output == NULL) { //Check if the command has an output (from redirection)
    if (pipe_out != -1) //If not check if we have an output to the next command (pipe)
      output_fd = pipe_out;
    else output_fd = 1; //Otherwise the standard input is still the screen
  } else {
    //Case we want to overwrite the file
    if (my_command->type == OVERWRITE) {
      output_fd = open(my_command->output, O_WRONLY | O_TRUNC | O_CREAT);
    }
    //Case we only append content at the end of the file
    else if (my_command->type == APPEND) {
      output_fd = open(my_command->output, O_WRONLY | O_APPEND | O_CREAT);
    }
  }

  //This should never happen as we check the validity of file during command creation
  if (output_fd == -1) {
    printf(BRED "Error opening output file\n" reset);
    close(input_fd);
    exit(EXIT_FAILURE);
  }

  //--------------DUPLICATE FD-------------------//

  //Duplicate obtained file descriptors in stdin and stdout
  dup2(input_fd, STDIN_FILENO);
  dup2(output_fd, STDOUT_FILENO);
}

/*
Final adjustments before the invoking exec
Impossible to return from this function
*/
void execution(struct command* my_command, struct environment* my_environment) {
  //Get Path variable (locations of executables)
  char* environment = get_env_var_value((char*)"PATH", my_environment);

  //Copy it to process it
  char* environment_copy = (char*) malloc(strlen(environment)+1);
  strcpy(environment_copy, environment);

  //Variable to get split elements (dirnames)
  char** path = (char**) malloc(sizeof(char*)*strlen(environment_copy));

  //use of strtok to split path into usable locations
  // : is the separator
  int i = 0;
  int j = 0;
  //Buffer
  char* str_token = strtok(environment_copy, ":");
  while (str_token != NULL) {
    path[i] = (char*) malloc(strlen(str_token)+1);  //Copy each element if the array
    strcpy(path[i], str_token);
    strcat(path[i], "/"); //And concat / to make it usable
    i++;
    str_token = strtok(NULL, ":");  //Moves to the next element
  }

  //We can now liberate this copied environment, we have what we need in our split array
  free(environment_copy);
  j = i-1;
  i = 0;

  //Creates the full filename
  char* name_buff = (char*) malloc(strlen(my_command->name)+strlen(path[0])+1);
  name_buff[0] = '\0';
  strcat(strcat(name_buff, path[0]), my_command->name);

  //While we cannot find the executable
  while (access(name_buff, X_OK) == -1) {
    //Case we went through all the possible locations
    if (i >= j) { //Could not find the command
      printf(BRED "Could not find the command %s in this computer\n" reset, my_command->name);
      exit(EXIT_FAILURE);
    }

    //Recreates the name with the next path
    i++;
    name_buff = (char*) realloc(name_buff, strlen(my_command->name)+strlen(path[i])+1);
    name_buff[0] = '\0';
    strcat(strcat(name_buff, path[i]), my_command->name);
  }

  //Invokes exec, bye bye !
  execve(name_buff, my_command->options, environment_to_array(my_environment));
  perror("execve"); //Something went wrong
  exit(EXIT_FAILURE); //Never return from this command !
}

int* launch_command(struct command* my_command, struct environment* my_environment) {
  int* pids = (int*) malloc(sizeof(int)*2); //Array to store background processes pids
  pids[1] = -1; //Set the end

  int fork_count = 0; //Size of the array
  int break_pipe = 0; //Need to break the pipe ? (in case of multiple outputs: ls > toto | grep shell : writes ls in toto and grep reads from the keyboard)

  int need_pipe = 0;  //Need a pipe between the first sub command and the the second one; goes iterative
  int need_another_pipe = 0;  //Need a pipe between the second sub command and the third one, goes iterative
  int* pipefd = (int*) malloc(sizeof(int)*2);  //First pipe
  int* pipefd2 = (int*) malloc(sizeof(int)*2); //Second pipe

  //File descriptors for the pipe
  int pipe_in = -1;
  int pipe_out = -1;

  //Is there a pipe ?
  if (my_command->pipe_entry != NULL)
    need_pipe = 1;

  //Create the pipe if we need
  if (need_pipe) {
    if (pipe(pipefd) == -1) { //Check error
      perror("pipe");
      return NULL;
    }
  }

  pids[0] = fork(); //Start first child
  fork_count++; //Iterate counter

  //Double output ?
  break_pipe = (my_command->output != NULL) && need_pipe;

  if (pids[0] == -1)  //Check fork error
    printf(BRED "Error during fork\n" reset);
  else if (pids[0] == 0) {  //Child's code

    //Close unused pipe side and get used pipe side
    if (need_pipe && !break_pipe) {
      close(pipefd[0]);
      pipe_out = pipefd[1];
    } else pipe_out = -1;

    //Set the FINAL input and output from redirections and pipe
    process_redirection(my_command, pipe_in, pipe_out, my_environment);

    //Send the command for execution, this function is terminal (cannot exit from)
    execution(my_command, my_environment);

  } else {  //Parent's code

    //While we have a command to handle
    while (my_command->pipe_entry != NULL) {
      //Move to the next sub command
      my_command = my_command->pipe_entry;

      //Extend pids array
      pids = (int*) realloc(pids, sizeof(int)*fork_count+2);
      pids[fork_count+1] = -1;

      //There is again an other sub command ?
      need_another_pipe = 0;
      if (my_command->pipe_entry != NULL)
        need_another_pipe = 1;

      //Creates a new pipe
      if (need_another_pipe) {
        if (pipe(pipefd2) == -1) {
          perror("pipe");
          return NULL;
        }
      }

      //Fork once again, we need a new child to handle another sub command
      pids[fork_count] = fork();
      fork_count++;

      if (pids[fork_count-1] == -1) //Fork error check
        printf(BRED "Error during fork\n" reset);
      else if (pids[fork_count-1] == 0) { //Child code

        //Set the input from pipe if not broken
        if (!break_pipe) {
          close(pipefd[1]);
          pipe_in = pipefd[0];
        } else pipe_in = -1;

        //Set the output from pipe if needed
        if (need_another_pipe) {
          close(pipefd2[0]);
          pipe_out = pipefd2[1];
        } else pipe_out = -1;

        //Set the FINAL input and output from redirections and pipe
        process_redirection(my_command, pipe_in, pipe_out, my_environment);

        //Send the command for execution, this function is terminal (cannot exit from)
        execution(my_command, my_environment);

      } else {  //Parent's code

        //If we used a pipe we close it in the parent
        if (need_pipe) {
          close(pipefd[0]);
          close(pipefd[1]);
        }
        //The first pipe becomes the second one
        pipefd[0] = pipefd2[0];
        pipefd[1] = pipefd2[1];
      }
    }
  }

  //Close the last opened pipe if there was one
  if (need_another_pipe) {
    close(pipefd2[0]);
    close(pipefd2[1]);
  }

  //Prevent from zombies
  int i = 0;
  if (my_command->bkgrd) {  //Case we launch the command in background
    while (pids[i] != -1) { //For each child
      waitpid(pids[i], NULL, WNOHANG);  //Wait non blocking, will reap the child once exited but continues execution while waiting
      i++;
    }
    return pids;  //Returns their pipds
  } else {  //Case we launch the command in foreground
    while (pids[i] != -1) { //For each child
      waitpid(pids[i], NULL, 0);  //Wait for each child to exit, blocking
      i++;
    }
    return NULL;  //Does not need to return anything
  }
}
