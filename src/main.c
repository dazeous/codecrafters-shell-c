#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


char *PATH = getenv("PATH");

char *checkIfBinaryExists(char *args) {
  char *folder = strtok(PATH, ";");
  while (folder != NULL) {
    strcat(folder, args);
    if (access(folder, X_OK) == 0) {
      return folder;
    } 
    folder = strtok(NULL, ";");
  }
  return "NOT_FOUND";
}

int main(int argc, char *argv[]) {

  while (1) {
    // Flush after every printf
    setbuf(stdout, NULL);

    printf("$ ");
    // Buffer to store user input
    char userInput[1024];

    // Receive user input and store it in buffer
    fgets(userInput, sizeof(userInput), stdin);

    // Remove the newline character from the end
    userInput[strcspn(userInput, "\n")] = '\0';
    char *fspace = strchr(userInput, ' ');
    char *args;
    if (fspace != NULL) {
      *fspace = '\0';
      args = fspace + 1;
    }
    char *command = userInput;
    // If the command received is "exit", break out of the loop
    if (!strcmp(command, "exit")) {
      break;
    }
    else if (!strcmp(command, "echo")) {
      printf("%s\n", args);
    }
    else if (!strcmp(command, "type")) {
      if (strstr("echo exit type", args) != NULL) {
        printf("%s is a shell builtin\n", args);
      }
      else {
        char *fullPath = checkIfBinaryExists(args);
        if (!strcmp(fullPath, "NOT_FOUND")) {
          printf("%s: not found\n", args);
        }
        else {
          printf("%s is %s\n", args, fullPath);
        }
      }
    }
    else {
      printf("%s: command not found\n", command);
    }  
  }
  return 0;
}
