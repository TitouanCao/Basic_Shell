/*
Features processings for all internal commands
and external commands transfer to execve
*/

int* launch_command (struct command* my_command, struct environment* my_environment);

/*
Change directory
Accepts : ".", "..", "~", "/" (last one goes from root)
Returns (potentially reallocated) curr_location pointer, nothing is freed
*/
char* run_cd (char* destination, char* curr_location, struct environment* my_environment);
