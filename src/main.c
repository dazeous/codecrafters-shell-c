#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// Checks if the provided argument exists in the system path and is executable, returns path if it is, None if not
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
    char *argList[10];
    int argCount = 0;

    // Create an args array
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

    // If the command is "echo", print everything after the first arg
    else if (!strcmp(command, "echo")) {
      for (int i = 1; i < argCount; i++) {
        printf("%s ", argList[i]);
      }
      printf("\n");
    }
    // Handle type
    else if (!strcmp(command, "type")) {

      //TODO: implement a better check here
      if (strstr("echo exit type", argList[1]) != NULL) {
        printf("%s is a shell builtin\n", argList[1]);
      }
      else {
        char *fullpath = ifBinaryExists(argList[1]);
        if (fullpath) {
          printf("%s is %s\n", argList[1], fullpath);
          free(fullpath);
        }
        else {
          printf("%s: not found\n", argList[1]);
        }
      }
    }
    else if (!strcmp(command, "pwd")) {
      char *cwd = getcwd(NULL, 0);
      if (cwd) {
        printf("%s\n", cwd);
        free(cwd);
      }
    }
    else {
      // Check if binary exists
      char *binPath = ifBinaryExists(command);
      // If it does, fork the current process
      if (binPath) {
        pid_t pid = fork();
        // If the process is the child, call execv
        if (pid == 0) {
          execv(binPath, argList);
        }
        // If the process is the parent, wait for the child to terminate
        else {
          wait(NULL);
        }
        free(binPath);
      }
      // If the binary doesn't exist, print command not found
      else {
        printf("%s: command not found\n", command);
      }
    }  
  }
  return 0;
}
