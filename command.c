#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ansi_colors.h"

#ifndef environment_H
#define environment_H _
  #include "environment.h"
#endif

#ifndef COMMAND_H
#define COMMAND_H _
  #include "command.h"
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
Sets the correct input/output to the command and returns int
to inform if we should write the word in option or not
Returns -1 in case of error
Returns 0 if next is redirection : treatment
Returns 1 if current is redirection
Returns 2 if previous is redirection
Returns 3 if no redirection
=> When 1 or 2 we must not write the word in the options (arguments for the command)
=> When -1 we stop everything
We check if the NEXT (so not the current one) word IS a redirection character
*/
int manage_redirection(struct command* my_command, char** command_line, int index) {
  if (command_line[index] != NULL) {  //Check word is non null

    //Check where is the redirection character
    int current_is_redirection = !strcmp(command_line[index], "<") || !strcmp(command_line[index], ">");
    int next_is_redirection = command_line[index+1] != NULL && (!strcmp(command_line[index+1], "<") || !strcmp(command_line[index+1], ">"));
    int previous_is_redirection = index > 0 && (!strcmp(command_line[index-1], "<") || !strcmp(command_line[index-1], ">"));

    //If current word is a redirection character we should not do anything
    if (current_is_redirection)
      return 1;

    //If we are between two redirection symbol we don't want to write the word in the arguments
    if (!previous_is_redirection && !next_is_redirection)
      return 3;

    //Check we have another word
    if (command_line[index+1] != NULL) {

      //Are we RIGHT after a redirection ? If so we don't want to write the argument
      if (previous_is_redirection && !next_is_redirection) {
        if (current_is_redirection) //Useless in practice but make things clearer
          return 1;
        else
          return 2;
      }

      int ftype = 0;  //The type of opening we want (read/overwrite/append)
      if (command_line[index+2] == NULL) {  //No file found
        printf(BRED "Incorrect command format - missing file\n" reset);
        return -1;
      }

      if (!strcmp(command_line[index+1], ">") && !strcmp(command_line[index+2], ">")) { //We have an append written in two words (thanks readline è_é)
        if (command_line[index+3] == NULL) {  //Check we have a file
          printf(BRED "Incorrect command format - missing file\n" reset);
          return -1;
        } else if (!is_valid_file_name(command_line[index+3])) {  //Check the name corresponds to a real filename
          printf(BRED "Incorrect command format - incorrect file\n" reset);
          return -1;
        }
        ftype = 2;  //Necessarly an "append" type
      } else {
        if (!is_valid_file_name(command_line[index+2])) { //Also check the name corresponds to a real filename otherwise
          printf(BRED "Incorrect command format - incorrect file\n" reset);
          return -1;
        }
      }

      if (!strcmp(command_line[index+1], ">") && ftype == 0)  // Has a > but not an append type -> overwrite type
        ftype = 1;
      else if (!strcmp(command_line[index+1], "<")) //Otherwise it's a read
        ftype = 3;

      switch (ftype) {  //Switch among the three cases
      case 1:
        my_command->type = OVERWRITE;

        my_command->output = (char*) malloc(strlen(command_line[index+2]) + 1); //Allocate and copy, 2nd word
        strcpy(my_command->output, command_line[index+2]);

        break;
      case 2:
        my_command->type = APPEND;

        my_command->output = (char*) malloc(strlen(command_line[index+3]) + 1); //Allocate and copy, 3rd word
        strcpy(my_command->output, command_line[index+3]);

        break;
      case 3:
        if (my_command->type != APPEND) //Case we write < after >>
          my_command->type = OVERWRITE;

        my_command->input = (char*) malloc(strlen(command_line[index+2]) + 1);  //Allocate and copy, 2nd word
        strcpy(my_command->input, command_line[index+2]);
      }

      if (previous_is_redirection)  //Case were we have two redirections like this : ls < toto > titi; we don't want to write toto even if we processed the second redirection
        return 2;
      else
        return 0;
    }
    if (index > 1 && previous_is_redirection)
      return 2;
    else
      return 3; //Case we have a simple command
  }
  return -1;
}

/*
Create a new function to represent the function after the pipe, in pipe_entry
This process goes recursive as we add pipes and commands are chained like a list
*/
int manage_pipe(struct command* my_command, char** command_line, int index, struct environment* my_environment) {
  if (command_line[index] != NULL && !strcmp(command_line[index], "|")) { //Check it is a pipe
    my_command->pipe_entry = (struct command*) malloc(sizeof(struct command*)); //Allocation for a new structure of command
    my_command->pipe_entry = parse_line(command_line, index+1, my_environment); //Goes recursive
    return 1; //We met a pipe
  } else {
    return 0; //We did not meet a pipe
  }
}

struct command* parse_line(char** command_line, int index, struct environment* my_environment) {
  //Default command
  struct command* my_command = (struct command*) malloc(sizeof(struct command));
  my_command->name = NULL;
  my_command->options = NULL;
  my_command->bkgrd = 0;
  my_command->valid = 0;
  my_command->type = UNUSED;
  my_command->input = NULL;
  my_command->output = NULL;
  my_command->pipe_entry = NULL;

  if(command_line == NULL) {  //Unexpected case where readline failed ?
    return my_command;
  } else if (command_line[index] == NULL) { //Nothing inputed
    return my_command;
  } else {
    //Set all alpha characters to uppercase to we can write non case sensitive commands in the terminal
    to_uppercase(command_line[index]);

    //---------------INTERNAL COMMANDS-----------------//

    //---------------QUIT-----------------//

    if (!strcmp(command_line[index], "QUIT")) {
      my_command->name = (char*) malloc(strlen("quit")+1);  //Set name only
      strcpy(my_command->name, "quit");
    }

    //---------------PWD-----------------//

    if (!strcmp(command_line[index], "PWD")) {
      my_command->name = (char*) malloc(strlen("pwd")+1); //Set name only
      strcpy(my_command->name, "pwd");
    }

    //---------------PRINT-----------------//

    else if (!strcmp(command_line[index], "PRINT")) {
      my_command->name = (char*) malloc(strlen("print")+1); //Set name
      strcpy(my_command->name, "print");

      if (command_line[1] != NULL) {  //Check the argument
        my_command->options = (char**) malloc(sizeof(char*)); //Set the argument

        my_command->options[0] = (char*) malloc(strlen(command_line[1])+1);
        my_command->options[1] =  NULL;

        strcpy(my_command->options[0], command_line[1]);
      }
      //case no argument -> print all, checked in the parser
    }

    //---------------SET-----------------//

    else if (!strcmp(command_line[index], "SET")) {
      my_command->name = (char*) malloc(strlen("set")+1); //Set name
      strcpy(my_command->name, "set");

      //Check we have the two arguments
      if (command_line[1] == NULL || command_line[2] == NULL) {
        printf(BRED "Missing arguments\n" reset);
        return my_command;
      } else {
        if (command_line[3] != NULL) {  //Check we have exactly two arguments
          printf(BRED "Too many arguments, expected 2\n" reset);
          return my_command;
        } else {  //Set the two arguments
          my_command->options = (char**) malloc(sizeof(char*)*2);

          my_command->options[0] = (char*) malloc(strlen(command_line[1])+1);
          my_command->options[1] = (char*) malloc(strlen(command_line[2])+1);
          my_command->options[2] = NULL;

          strcpy(my_command->options[0], command_line[1]);
          strcpy(my_command->options[1], command_line[2]);
        }
      }
    }

    //---------------CD-----------------//

    else if (!strcmp(command_line[index], "CD")) {
      my_command->name = (char*) malloc(strlen("cd")+1);  //Set name
      strcpy(my_command->name, "cd");

      if (command_line[1] == NULL) {  //Check argument
        printf(BRED "Invalid destination - Nothing found\n" reset);
        return my_command;
      }

      my_command->options = (char**) malloc(sizeof(char*));

      int add = 1;  //Need to add a / at the end ? (Yes if there is not, just to have a standard of / ending dirname)
      if (command_line[1][strlen(command_line[1])-1] != '/')  //There is a slash at the end ?
        add++;  //Yes

      my_command->options[0] = (char*) malloc(strlen(command_line[1])+add); //Set the argument (dirname)
      my_command->options[1] = NULL;

      strcpy(my_command->options[0], command_line[1]);
      if (add == 2)
        strcat(my_command->options[0], "/");  //Add a slash if there was not
    }

    else {

      //---------------EXTERNAL COMMANDS-----------------//

      my_command->name = (char*) malloc(strlen(command_line[index])+1); //Set name
      strcpy(my_command->name, to_lowercase(command_line[index]));


      /*
      example: ls -l | grep shell
      ls -l | grep shell : is the command
      ls -l : is a sub command
      grep shell : is the other sub command
      */
      int i = index;  //Index in current command
      int j = 0;  //Index in the current sub command
      int len_tot = 0;  //Length of the whole command

      //Get total length
      while(command_line[i] != NULL) {
        len_tot++;
        i++;
      }

      //Get sub command length
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

      //Check if the command is a background command
      int len = j;
      if (!strcmp(command_line[len_tot-1], "&")) {
        my_command->bkgrd = 1;
        len--;
      }
      i = index;
      j = 0;

      my_command->options = (char**) malloc(sizeof(char*)*len);

      int r = 0;
      while(i-index < len) {  //Process each word of the sub command

        //Set redirection
        r = manage_redirection(my_command, command_line, i);
        if (r==-1)  //Error case
          return my_command;

        //Check pipe
        if (manage_pipe(my_command, command_line, i, my_environment))
          break;  //The pipe indicates the end of the sub command

        //Cases where we should write the word in the command arguments
        //That is to say when it's not a redirection character or destination file
        if (r!=1 && r!=2) {
          if (!strcmp(command_line[i], "~")) {  //Interprets the ~ character as the dirname of the session
            char* home = get_env_var_value((char*)"HOME", my_environment);
            my_command->options[j] = (char*) malloc(strlen(home)+1);  //Writes in the argument
            strcpy(my_command->options[j], home);
          } else {
            my_command->options[j] = (char*) malloc(strlen(command_line[i])+1);
            strcpy(my_command->options[j], command_line[i]);
          }
          j++;
        }
        i++;
      }
      my_command->options[j] = NULL;  //Set the end of the argument list
    }
  }
  my_command->valid = 1;  //The command went throughout the process, it is valid
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
