#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


char *ifBinaryExists(char *argList) {
    char *path = getenv("PATH");
    char *path_copy = strdup(path);     

    char *folder = strtok(path_copy, ":");

    while (folder != NULL) {
        char *fullpath = malloc(strlen(folder) + strlen(argList) + 2);

        sprintf(fullpath, "%s/%s", folder, argList);

        if (access(fullpath, X_OK) == 0) {
            free(path_copy);
            return fullpath;             
        }

        free(fullpath);
        folder = strtok(NULL, ":");
    }

    free(path_copy);
    return NULL;
}

int main(int argc, char *argList[]) {

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
    // char *fspace = strchr(userInput, ' ');
    // char *args;
    // if (fspace != NULL) {
    //   *fspace = '\0';
    //   args = fspace + 1;
    // }

    char *argList[10];
    int argCount = 0;

    char *token = strtok(userInput, " ");
    while (token != NULL) {
      argList[argCount++] = token;
      token = strtok(NULL, " ");
    }
    argList[argCount] = NULL;
    
    char *command = argList[0];
    // If the command received is "exit", break out of the loop
    if (!strcmp(command, "exit")) {
      break;
    }
    else if (!strcmp(command, "echo")) {
      for (int i = 1; i < argCount; i++) {
        printf("%s ", argList[i]);
      }
      printf("\n");
    }
    else if (!strcmp(command, "type")) {
      if (strstr("echo exit type", argList[0]) != NULL) {
        printf("%s is a shell builtin\n", argList[0]);
      }
      else {
        char *fullpath = ifBinaryExists(argList[0]);
        if (fullpath) {
          printf("%s is %s\n", argList[0], fullpath);
          free(fullpath);
        }
        else {
          printf("%s: not found\n", argList[0]);
        }
      }
    }
    else {
      char *binPath = ifBinaryExists(command);
      if (binPath) {
        execv(binPath, argList);
      }
      else {
        printf("%s: command not found\n", command);
      }
    }  
  }
  return 0;
}
