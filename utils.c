#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

char* cut_string_from_tip (char* string, char delim) {
  int i = strlen(string)-1;
  char* my_string = (char*) malloc(strlen(string));
  strcpy(my_string, string);
  while(my_string[i] != '/') {
    i--;
    if (i < 0) return NULL;
  }
  my_string[i] = '\0';
  return my_string;
}

char* to_uppercase(char* string) {
  for (int i = 0; i < (int)strlen(string); i++)
    string[i] = toupper(string[i]);
  return string;
}

char* to_lowercase(char* string) {
  for (int i = 0; i < (int)strlen(string); i++)
    string[i] = tolower(string[i]);
  return string;
}

int is_valid_file_name(char* fd) {
  int check = open(fd, O_CREAT);
  close(check);
  if (check == -1)
    return 0;
  else
    return 1;
}

int is_valid_environment_var(char* var) {
  int len = strlen(var)+1;
  for (int i = 0; i < len; i++) {
    if (var[i] == ' ' || var[i] == '\t' || var[i] == '<' || var[i] == '>' || var[i] == '|' || var[i] == '=')
      return 0;
  }
  return 1;
}

int is_valid_pwd(char* filename) {
  DIR* ptr = opendir(filename);
  if (ptr != NULL) {
    closedir(ptr);
    return 1;
  }
  else
    return 0;
}

int pids_tab_len(int* pids) {
  int i = 0;
  while (pids[i] != -1) {
    i++;
  }
  return i;
}

int pids_matrix_len(int** pids) {
  int i = 0;
  int sum = 0;
  while(pids[i] != NULL) {
    sum = sum + pids_tab_len(pids[i]);
    i++;
  }
  return sum;
}

int nbr_of_alive_pids_in_tab(int* pids) {
  if (pids == NULL)
    return 0;
  int i = 0;
  int sum = 0;
  while (pids[i] != -1) {
    if (pids[i] != 0)
      sum++;
    i++;
  }
  return sum;
}

int nbr_of_alive_pids_in_matrix(int** pids, int max_i) {
  int i = 0;
  int sum = 0;
  while(i < max_i) {
    if (pids[i] != NULL)
      sum = sum + nbr_of_alive_pids_in_tab(pids[i]);
    i++;
  }
  return sum;
}

int erase_pid_from_matrix(int** pids, int pid) {
  int i = 0;
  int j = 0;
  int sup = 0;
  while(pids[i] != NULL) {
    while(pids[i][j] != -1) {
      if (pids[i][j] == 0) {
        pids[i][j] = 0;
      }
      j++;
    }
    if (nbr_of_alive_pids_in_tab(pids[i]) == 0) {
      pids[i] = NULL;
      sup++;
    }
    i++;
  }
  return sup;
}

void add_pid_to_pids_table(int* pids, int pid) {
  int i = 0;
  while (pids[i] != -1) {
    if (pids[i] == 0) {
      pids[i] = pid;
      printf("I set pids[%i] = %i\n", i, pids[i]);
      return;
    }
    printf("Loop, not -1 : pids[%i] = %i\n", i, pids[i]);
    i++;
  }
  printf("Loop, -1 : pids[%i] = %i\n", i, pids[i]);
  if (i == 0) {
    printf("First pid\n");
    pids = (int*) realloc(pids, sizeof(int)*2);
    pids[0] = pid;
    pids[1] = -1;
  } else {
    pids = (int*) realloc(pids, sizeof(int)*i+2);
    pids[i+1] = -1;
    pids[i] = pid;
    printf("II set pids[%i] = %i\n", i, pids[i]);
    printf("II set pids[%i] = %i\n", i+1, pids[i+1]);
  }
}
