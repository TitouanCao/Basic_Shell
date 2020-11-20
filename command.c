#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "command.h"

#ifndef UTILS_H
#define UTILS_H _
#include "utils.h"
#endif

//Types of command
#define UNUSED 0
#define OVERWRITE 1
#define APPEND 2

int manage_redirection(struct command* my_command, char** command_line, int index, int j) {
  if (command_line[index] != NULL) {
    if (!strcmp(command_line[index], ">") || !strcmp(command_line[index], "<")) {
      return 0;
    }
    if (command_line[index] != NULL && command_line[index+1] != NULL) {
      //Case there are no redirection
      if (strcmp(command_line[index+1], ">") && strcmp(command_line[index+1], "<")) {
        return 0;
      }

      int ftype = 0;
      if (command_line[index+2] == NULL) {
        printf("Incorrect command format - missing file\n");
        return -1;
      }

      if (!strcmp(command_line[index+1], ">") && ! strcmp(command_line[index+2], ">")) {
        if (command_line[index+3] == NULL) {
          printf("Incorrect command format - missing file\n");
          return -1;
        } else if (!is_valid_file_name(command_line[index+3])) {
          printf("Incorrect command format - incorrect file\n");
          return -1;
        }
        ftype = 2;
      }

      if (!strcmp(command_line[index+1], ">") && ftype == 0)
        ftype = 1;
      else if (!strcmp(command_line[index+1], "<"))
        ftype = 3;


      if (command_line[index+2] != NULL) {
        //index -> the string before the redirection symbol

        int type = 1; //There is an input
        if (command_line[index][0] == '-')
          type = 0; //Case of options, not an input
        if (!strcmp(my_command->name, "grep") && j == 1)
          type = 0; //Case of grep selector, not an input
        if (j == 0)
          type = 0; //Case name of the command, not an input

        switch (ftype) {
        case 1:
          my_command->type = OVERWRITE;

          my_command->output = (char*) malloc(strlen(command_line[index+2]) + 1);
          strcpy(my_command->output, command_line[index+2]);

          if (type) {
            my_command->input = (char*) malloc(strlen(command_line[index]) + 1);
            strcpy(my_command->input, command_line[index]);
          } else {
            my_command->input = NULL;
          }
          break;
        case 2:
          my_command->type = APPEND;

          my_command->output = (char*) malloc(strlen(command_line[index+3]) + 1);
          strcpy(my_command->output, command_line[index+3]);

          if (type) {
            my_command->input = (char*) malloc(strlen(command_line[index]) + 1);
            strcpy(my_command->input, command_line[index]);
          } else {
            my_command->input = NULL;
          }
          break;
        case 3:
          my_command->type = OVERWRITE;

          if (type) {
            my_command->output = (char*) malloc(strlen(command_line[index]) + 1);
            strcpy(my_command->output, command_line[index]);
          } else {
            my_command->output = NULL;
          }

          my_command->input = (char*) malloc(strlen(command_line[index+2]) + 1);
          strcpy(my_command->input, command_line[index+2]);
        }
        return 1;
      } else {
        printf("Incorrect command format - missing file\n");
        return -1;
      }
    }
    return 0;
  }
  return 0;
}

int manage_pipe(struct command* my_command, char** command_line, int index) {
  if (command_line[index] != NULL && !strcmp(command_line[index], "|")) {
    my_command->pipe_entry = (struct command*) malloc(sizeof(struct command*));
    my_command->pipe_entry = parse_line(command_line, index+1);
    return 1;
  } else {
    return 0;
  }
}

struct command* parse_line(char** command_line, int index) {
  struct command* my_command = (struct command*) malloc(sizeof(struct command));
  my_command->name = NULL;
  my_command->options = NULL;
  my_command->bkgrd = 0;
  my_command->valid = 0;
  my_command->type = UNUSED;
  my_command->input = NULL;
  my_command->output = NULL;
  my_command->pipe_entry = NULL;

  if(command_line == NULL) {
    //printf("No command found\n");
    return my_command;
  } else if (command_line[index] == NULL) {
    //printf("No command found\n");
    return my_command;
  } else {
    //Traitement de tous les cas de fonctions
    to_uppercase(command_line[index]);

    if (!strcmp(command_line[index], "QUIT")) {    //Cas sortie
      my_command->name = (char*) malloc(strlen("quit")+1);
      strcpy(my_command->name, "quit");
    }

    if (!strcmp(command_line[index], "PWD")) {    //Cas PWD
      my_command->name = (char*) malloc(strlen("pwd")+1);
      strcpy(my_command->name, "pwd");
    }

    else if (!strcmp(command_line[index], "PRINT")) {    //Cas PRINT
      my_command->name = (char*) malloc(strlen("print")+1);
      strcpy(my_command->name, "print");

      if (command_line[1] != NULL) {
        my_command->options = (char**) malloc(sizeof(char*));
        my_command->options[0] = (char*) malloc(strlen(command_line[1])+1);
        my_command->options[1] =  NULL;

        strcpy(my_command->options[0], command_line[1]);
      }
    }

    else if (!strcmp(command_line[index], "SET")) {    //Cas SET
      my_command->name = (char*) malloc(strlen("set")+1);
      strcpy(my_command->name, "set");

      if (command_line[1] == NULL || command_line[2] == NULL) {
        printf("Missing arguments\n");
        return my_command;
      }
      else {
        my_command->options = (char**) malloc(sizeof(char*)*2);
        my_command->options[0] = (char*) malloc(strlen(command_line[1])+1);
        my_command->options[1] = (char*) malloc(strlen(command_line[2])+1);
        my_command->options[2] = NULL;

        strcpy(my_command->options[0], command_line[1]);
        strcpy(my_command->options[1], command_line[2]);
      }
    }

    else if (!strcmp(command_line[index], "CD")) {  //Cas CD
      if (command_line[1] == NULL) {
        printf("Invalid destination - Nothing found\n");
        return my_command;
      }
      my_command->name = (char*) malloc(strlen("cd")+1);
      strcpy(my_command->name, "cd");

      my_command->options = (char**) malloc(sizeof(char*));

      int add = 1;
      if (command_line[1][strlen(command_line[1])-1] != '/')
        add++;
      my_command->options[0] = (char*) malloc(strlen(command_line[1])+add);
      my_command->options[1] = NULL;

      strcpy(my_command->options[0], command_line[1]);
      if (add == 2)
        strcat(my_command->options[0], "/");
    }

    else {
      //External commands
      my_command->name = (char*) malloc(strlen(command_line[index])+1);
      strcpy(my_command->name, to_lowercase(command_line[index]));

      //traitement générique
      int i = index;
      int j = 0;
      int len_tot = 0;

      while(command_line[i] != NULL) {
        len_tot++;
        i++;
      }

      i = index;
      while(command_line[i] != NULL) {
        if (!strcmp(command_line[i], "|")) {
          j++;
          i++;
          break;
        }
        i++;
        j++;
      }


      int len = j;
      if (!strcmp(command_line[len_tot-1], "&")) {  //Vérification background
        my_command->bkgrd = 1;
        len--;
      }
      i = index;
      j = 0;

      my_command->options = (char**) malloc(sizeof(char*)*len);

      int r = 0;
      int k = 0;  //There is a redirection or not
      while(j < len) {

        if (r == 0) {
          r = manage_redirection(my_command, command_line, i, j);
          if (r == 1)
            k = j+1;
        }

        if (r==-1) break;
        if (manage_pipe(my_command, command_line, i))
          break;

        if (r==0||r==1) {
          my_command->options[j] = (char*) malloc(strlen(command_line[i])+1);
          strcpy(my_command->options[j], command_line[i]);
          if (r==1)
            r=2;  //Stop filling options
        }
        i++;
        j++;
      }
      if (k != 0)
        my_command->options[k] = NULL;
      else
        my_command->options[j] = NULL;
    }
  }
  my_command->valid = 1;
  return my_command;
}

void free_command(struct command* my_command) {
  if(my_command != NULL) {
    free(my_command->name);

    int i = 0;
    if (my_command->options != NULL) {
      while(my_command->options[i] != NULL) {
        free(my_command->options[i]);
        i++;
      }
    }

    free(my_command->options);

    free(my_command->input);
    free(my_command->output);

    free_command(my_command->pipe_entry);
    free(my_command->pipe_entry);
  }
}
