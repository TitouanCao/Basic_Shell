struct command {
  char* name;
  char** options;

  int bkgrd;
  int valid;
  int type; // 0 -> doesn't apply, 1 -> overwrite, 2 -> append

  char* input; //NULL if unused or for keyboard
  char* output; //NULL if unused or for screen

  struct command* pipe_entry;
};

struct command* parse_line(char** command_line, int index);

void free_command(struct command* my_command);
