#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


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

#ifndef PARSER_H
#define PARSER_H _
#include "parser.h"
#endif

#ifndef UTILS_H
#define UTILS_H _
#include "utils.h"
#endif

//Internal command list
#define CD "cd"
#define PWD "pwd"
#define SET "set"
#define PRINT "print"


int command_parser (struct command* my_command) {
  //Accepted commands
  if (!strcmp(my_command->name, CD)) {
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

int* parser (struct command* my_command, struct path my_path) {
  char* ptr1;
  char* ptr2;
  int int1;
  switch (command_parser(my_command)) {
    case 1:
      ptr1 = get_env_var_value((char*)"PWD", my_path);
      int1 = strlen(ptr1) + 1;
      ptr2 = (char*) malloc(int1);
      strcpy(ptr2, ptr1);
      ptr1 = run_cd(my_command->options[0], ptr2, my_path);
      if (ptr1 != NULL) {
        if (chdir(ptr1) == 0) {
          if (set_env_var((char*)"PWD", ptr1, my_path) == -1)
            printf("Could not set path variable\n");
        } else
            printf("Destination not found\n");
      }
      free(ptr1);
      break;
    case 2:
      run_pwd(get_env_var_value((char*)"PWD", my_path));
      break;
    case 3:
      to_uppercase(my_command->options[0]);
      int1 = set_env_var(my_command->options[0], my_command->options[1], my_path);
      if (int1 == -1)
        printf("Variable contains illegal characters\n");
      else if (int1 == -2)
        printf("Illegal path for pwd\n");
      else if (int1 == -3)
        printf("Unknown path variable\n");
      else
        printf("%s set to %s\n", to_uppercase(my_command->options[0]), my_command->options[1]);
      break;
    case 4:
      ptr1 = get_env_var_value(to_uppercase(my_command->options[0]), my_path);
      if (ptr1 == NULL)
        printf("Unknown path variable\n");
      else

        printf("%s\n", ptr1);
      break;
    default:
      return launch_command(my_command, my_path);
  }
  return NULL;
}
