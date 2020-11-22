/*
Send the command to the right processing function
*/

/*
Switch among the different processings
Returns the pids of background processes if there are, NULL otherwise
*/
int* parser (struct command* my_command, struct environment* my_environment);
