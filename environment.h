struct row {
  char* header;
  char* content;
};

struct environment {
  struct row* rows;
  int size;
};

struct environment create_environment(char** env_var);

char** environment_to_array(struct environment my_environment);

void show_environment(struct environment my_environment);

int is_valid_row(struct row);

struct row get_env_var(char* name, struct environment);

char* get_env_var_value(char* name, struct environment);

int set_env_var(char* name, char* value, struct environment);

void free_environment(struct environment my_environment);
