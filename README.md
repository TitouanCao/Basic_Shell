# Basic_Shell
A very basic implementation of Shell for Linux, with simple functionalities.
___
## Start
After opening a terminal in the directory the Makefile is, symply typing `make` should build all the executables.<br/>
Once the executables are built you can execute the main with `./shell`.
___
## Features
*Commands's name are not case-sensitive, but values are.<br/>
There are no Shell interpretation of wildcards.*
### Internal commands
`pwd` displays the current pwd from the path variables. <br/>
`cd <location>` moves the pwd to the given location if it exists in the filesystem. Special entries accepted: 
- `..` returns to the previous directory.
- `~` returns to the home directory.
- `/` returns to the root. <a/>

`print <variable>` displays the given variable from the path if it exists. If no variable is given displays all the variables. <br/>
`set <variable> <value>` set the given variable to the given value. Update the pwd if pwd is modified and check it is still correct.<br/>
`quit` exit the program. <br/>
Handles `Ctrl+c` signal.

### External commands
Can handle any command which executable file is located somewhere in the computer which filepath is located in path variable.

### Pipes
With the character `|` you can combine commands. The command on the left side will send its result to the command on the right side. There are no limit to the amount of commands you can combine. In case a left command has already an output redirection, the redirection has the priority and the next command wont be executed.

### Redirections
With the character `>` you can set the output for the given command. The writing will overwrite the previous content. <br/>   
With the characters `>>` you can set the output for the given command. The writing will append at the end of the previous content. <br/>
With the caracter `<` you can set the input for the given command.

### Background commands
Commands can be launched in background with the character `&` at the end of the line. <br/>
A maximum of 5 processes can run in background simultaneously, once this limit reached you cannot launch anymore background commands and must wait a process to terminate. <br/>
When killing the Shell all background processes will also be killed.

___
## Files
`Readline` is provided by professor Olivier Gruber. <br/>
`Shell` contains the main and manages processes to limit them and kill them when necessary.<br/>
`Environment` implements a structure to handle environment variables and functions to access them more easily.<br/>
`Command`implements a structure to handle commands to handle them in a more practical way. It checks if the command's syntax is correct but does not check the existence of inputed values. Thus it has a role of pre-parser.<br/>
`Parser`receive a `struct command*` and forward it to the right treatment with some adjustments if necessary. It is very light considering `Command` already check most of error cases and values relevence if checked in later functions.<br/>
`Command_management` implements internal functions and calls external commands. It handles pipe and redirection processing.<br/>
`Utils` proposes come useful functions, although most of them are not used in current implementation.<br/>
