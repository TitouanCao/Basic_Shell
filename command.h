/*
A structure for commands allowing to know:
Its name: the executable name or the name of the internal command
Its arguments (options), always starting with the name
If it must be launched in background (1) or not (0)
If it is valid, usable
If it contains redirection and the kind of opening (append or truncate)
Its input file
Its ouput file
The command coming after the pipe
WARNING: A command can only contain ONE redirection of each type
Otherwise the last one will take priority
Ex: ls > toto > titi : writes the output of ls in titi
*/

struct command {
  char* name;
  char** options; //Starts with the name

  int bkgrd;  // 0 no, 1 yes; is the same in the whole chain of commands
  int valid;  // 0 no, 1 yes
  int type;   // 0 -> unused, 1 -> overwrite/read, 2 -> append

  char* input; //NULL if unused or for keyboard
  char* output; //NULL if unused or for screen

  struct command* pipe_entry; //NULL if no pipe
};

/*
Create a struct command* from a char** strarting to read at the char* of the given index
Does not free the argument
*/
struct command* parse_line(char** command_line, int index, struct environment* my_environment);

/*
Liberate all the allocated memory of this command recursively
*/
void free_command(struct command* my_command);
