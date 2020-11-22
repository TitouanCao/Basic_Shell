#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

#ifndef PARSER_H
#define PARSER_H _
#include "parser.h"
#endif

#ifndef UTILS_H
#define UTILS_H _
#include "utils.h"
#endif

//Internal command list
#define QUIT "quit"
#define CD "cd"
#define PWD "pwd"
#define SET "set"
#define PRINT "print"

//Transforms name into int categories
int command_parser (struct command* my_command) {
  //Accepted commands
  if (!strcmp(my_command->name, QUIT))  {
    return -1;
  } else if (!strcmp(my_command->name, CD)) {
    return 1;
  } else if (!strcmp(my_command->name, PWD)) {
    return 2;
  } else if (!strcmp(my_command->name, SET)) {
    return 3;
  } else if (!strcmp(my_command->name, PRINT)) {
    return 4;
  } else {
    return 5; //External commands
  }
}

int* parser (struct command* my_command, struct environment* my_environment) {
  //Used variables
  char* ptr1;
  char* ptr2;
  int int1;
  int* int2;
  switch (command_parser(my_command)) {
    case -1:  //QUIT
      int2 = (int*) malloc(sizeof(int));
      *int2 = -2;  //Not a good use of pids array, but indicates to quit
      return int2;
      break;
    case 1:
      ptr1 = get_env_var_value((char*)"PWD", my_environment); //Get the current location
      ptr2 = (char*) malloc(strlen(ptr1) + 1);  //Allocate the same size
      strcpy(ptr2, ptr1); //Copy it in an other pointer

      ptr1 = run_cd(my_command->options[0], ptr2, my_environment);  //Send to processing

      if (ptr1 != NULL) { //Just checking
        if (chdir(ptr1) == 0) { //Verify this dirname exists
          if (set_env_var((char*)"PWD", ptr1, my_environment) == -1)  //Set the pwd accordingly
            printf(BRED "Could not set environment variable\n" reset);
        } else
            printf(BRED "Destination not found\n" reset);
      }

      free(ptr1); //Free the previously allocated ptr2, retrieved by ptr1
      break;
    case 2:
      //Displays the pwd path variable
      printf("%s\n", get_env_var_value((char*)"PWD", my_environment));
      break;
    case 3:
      to_uppercase(my_command->options[0]); //Make variable name non case sensitive

      //Try to set the given variable
      int1 = set_env_var(my_command->options[0], my_command->options[1], my_environment);
      //Displays the right error or shows the process succeeded
      if (int1 == -1)
        printf(BRED "Variable contains illegal characters\n" reset);
      else if (int1 == -2)
        printf(BRED "Illegal environment for pwd\n" reset);
      else if (int1 == -3)
        printf(BRED "Unknown environment variable\n" reset);
      else
        printf("%s set to %s\n", to_uppercase(my_command->options[0]), my_command->options[1]);

      break;
    case 4:
      //Case we did not give an argument -> print all
      if (my_command->options == NULL) {
        show_environment(my_environment); //Displays the whole environment
        break;
      }
      //Get the right variable
      ptr1 = get_env_var_value(to_uppercase(my_command->options[0]), my_environment);

      if (ptr1 == NULL) //Check the variable was found
        printf(BRED "Unknown environment variable\n" reset);
      else  //Displays
        printf("%s\n", ptr1);
      break;
    default:
      //Generic external command management
      return launch_command(my_command, my_environment);  //Return pids
  }
  return NULL;  //Returns nothing in case of internal commands (no child)
}
