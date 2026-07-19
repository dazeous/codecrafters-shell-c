#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>


// Checks if the provided argument exists in the system path and is executable, returns path if it is, None if not
char *ifBinaryExists(char *arg) {
    char *path = getenv("PATH");
    char *path_copy = strdup(path);

    char *folder = strtok(path_copy, ":");

    while (folder != NULL) {
        char *fullpath = malloc(strlen(folder) + strlen(arg) + 2);

        sprintf(fullpath, "%s/%s", folder, arg);

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

    // Receive user input and store it in buffer, if EOF, exit
    if (!fgets(userInput, sizeof(userInput), stdin)) break;


    // Remove the newline character from the end
    userInput[strcspn(userInput, "\n")] = '\0';
    char *argList[10];
    int argCount = 0;

    // // Populate the args array
    // char *token = strtok(userInput, " ");
    // while (token != NULL) {
    //   argList[argCount++] = token;
    //   token = strtok(NULL, " ");
    // }

    
    // char *command = argList[0];
    int in_quotes = 0;
    // int i = 0;
    // char temp[1024] = "";
    // int temp_len = 0;
    // while (userInput[i] != '\0') { 
    //   if (userInput[i] == '\'') {
    //     in_quotes = !in_quotes;
    //   }
    //   else if (userInput[i] == ' ' && !in_quotes) {
    //     while (userInput[i + 1] == ' ') i++;
    //     argList[argCount++] = strdup(temp);
    //     temp[0] = '\0';
    //     temp_len = 0;
    //   }
    //   else {
    //     temp[temp_len++] = userInput[i];
    //     temp[temp_len] = '\0';
    //   }
    //   i++;
    // }
    // if (temp[0] != '\0') {
    //   argList[argCount++] = strdup(temp);
    // }
    // if (argCount == 0) continue;
    // argList[argCount] = NULL;

    char *read = userInput;
    char *write = userInput;
    argList[argCount++] = write;
    while (*read != '\0') {
      if (*read == '\'') {
        in_quotes = !in_quotes;
      }
      else if (*read == ' ' && !in_quotes) {
        while (*(read + 1) == ' ') read++;
        *write = '\0';
        write++;
        argList[argCount++] = write;
      }
      else {
        *write = *read;
        write++;
      }
      read++;
    }
    *write = '\0';


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
      if (strstr("echo exit type pwd cd", argList[1]) != NULL) {
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
    else if (!strcmp(command, "cd")) {
      if (!strcmp(argList[1], "~")) {
        chdir(getenv("HOME"));
      }
      else if (chdir(argList[1]) != 0) {
        printf("cd: %s: No such file or directory\n", argList[1]);
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
