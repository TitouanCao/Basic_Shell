struct row {
  char* header;
  char* content;
};

struct path {
  struct row* rows;
  int size;
};

struct path create_path(char** env_var);

char** path_to_array(struct path my_path);

void show_path(struct path my_path);

int is_valid_row(struct row);

struct row get_env_var(char* name, struct path);

char* get_env_var_value(char* name, struct path);

int set_env_var(char* name, char* value, struct path);

void free_path(struct path my_path);
