/*
Structure representing environment variables rather than a simple char**
It features the possibility to access any variable easily and modify it,
and transform the structure back to char**
*/
struct row {
  char* header;
  char* content;
};

struct environment {
  struct row* rows;
  int size;
};

/*
Creates a struct environment* given a char**
The argument is not freed
Returns an incomplete struct if env_var is missing
*/
struct environment* create_environment(char** env_var);


/*
Creates a char** given a struct environment*
The struct is NOT freed in the process
*/
char** environment_to_array(struct environment* my_environment);

/*
Displays the content of a struct environment*
*/
void show_environment(struct environment* my_environment);


/*
Checks if a row (variable) is valid that is to
say, does not include null fields
*/
int is_valid_row(struct row);

/*
Returns the row of the environment corresponding to the given
variable name in the given environment if it exists.
If not found returns a row with null fields (not valid row)
*/
struct row get_env_var(char* name, struct environment* my_environment);

/*
Returns the value of the variable which name is given in the given environment
if it exists, otherwise returns NULL
*/
char* get_env_var_value(char* name, struct environment* my_environment);

/*
Modify the variable corresponding to the given name to the given value
in the given environment if the variable is found.
If it succeds returns 0
If the value contains illegal characters returns -1
If the variable is pwd and the given value incorrect returns -2
If the row corresponding to the variable is invalid returns -3
*/
int set_env_var(char* name, char* value, struct environment* my_environment);

/*
Liberate all the memory allocated for the struct environment*
*/
void free_environment(struct environment* my_environment);
