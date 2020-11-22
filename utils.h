/*
Useful random functions
*/

/*
Cuts the last element of a string according to the given delimiter
Free the given string and allocate a new one
*/
char* cut_string_from_tip (char* string, char delim);

/*
Make every alpha character of the string uppercase
Returns the same pointer as parameter
*/
char* to_uppercase(char* string);

/*
Make every alpha character of the string lowercase
Returns the same pointer as parameter
*/
char* to_lowercase(char* string);

/*
Checks if the name if correct for a filename
Does not check if the file exists in the filesystem
1 -< yes, 0 -> no
*/
int is_valid_file_name(char* fd);

/*
Checks if the string contains illegal characters
Illegal characters are: '', ' ', '\t', '<', '>', '|', '='
*/
int is_valid_environment_var(char* var);

/*
Check if the given string corresponds to an
existing directory in the filesystem
*/
int is_valid_pwd(char* filename);

/*
Counts how many pids are in an array of pids
*/
int pids_tab_len(int* pids);

/*
Counts how many pids are in a matrix of pids
*/
int pids_matrix_len(int** pids);

/*
Counts how many alive pids (!=0) are in an array of pids
*/
int nbr_of_alive_pids_in_tab(int* pids);

/*
Counts how many alive pids (!=0) are in a matrix of pids
*/
int nbr_of_alive_pids_in_matrix(int** pids, int max_i);

/*
Set a pid to 0 in a pid matrix
If a row does not contain any alive pid, it is set to NULL
*/
int erase_pid_from_matrix(int** pids, int pid);

/*
Add a pid into an array of pids
Takes the forst avaible (0) spot
*/
void add_pid_to_pids_table(int* pids, int pid);
