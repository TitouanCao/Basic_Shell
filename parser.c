#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


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
    return 5;
  }
}

int* parser (struct command* my_command, struct environment my_environment) {
  char* ptr1;
  char* ptr2;
  int int1;
  int* int2;
  switch (command_parser(my_command)) {
    case -1:
    int2 = (int*) malloc(sizeof(int));
      *int2 = -2;
      return int2;
      break;
    case 1:
      ptr1 = get_env_var_value((char*)"PWD", my_environment);
      int1 = strlen(ptr1) + 1;
      ptr2 = (char*) malloc(int1);
      strcpy(ptr2, ptr1);
      ptr1 = run_cd(my_command->options[0], ptr2, my_environment);
      if (ptr1 != NULL) {
        if (chdir(ptr1) == 0) {
          if (set_env_var((char*)"PWD", ptr1, my_environment) == -1)
            printf("Could not set environment variable\n");
        } else
            printf("Destination not found\n");
      }
      free(ptr1);
      break;
    case 2:
      run_pwd(get_env_var_value((char*)"PWD", my_environment));
      break;
    case 3:
      to_uppercase(my_command->options[0]);
      int1 = set_env_var(my_command->options[0], my_command->options[1], my_environment);
      if (int1 == -1)
        printf("Variable contains illegal characters\n");
      else if (int1 == -2)
        printf("Illegal environment for pwd\n");
      else if (int1 == -3)
        printf("Unknown environment variable\n");
      else
        printf("%s set to %s\n", to_uppercase(my_command->options[0]), my_command->options[1]);
      break;
    case 4:
      if (my_command->options == NULL) {
        show_environment(my_environment);
        break;
      }
      ptr1 = get_env_var_value(to_uppercase(my_command->options[0]), my_environment);
      if (ptr1 == NULL)
        printf("Unknown environment variable\n");
      else
        printf("%s\n", ptr1);
      break;
    default:
      return launch_command(my_command, my_environment);
  }
  return NULL;
}
