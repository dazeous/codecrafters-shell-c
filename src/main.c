#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

char *builtins[] = {"echo", "exit", "type", "pwd", "cd", NULL};

// Helper function to check if a command is a builtin
int does_exist_in_array(char *array[], char *cmd) {
    for (int i = 0; array[i] != NULL; i++) {
        if (strcmp(array[i], cmd) == 0) {
            return 1;
        }
    }
    return 0;
}

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

    char *read = userInput;
    char *write = userInput;

    int in_single_quotes = 0;
    int in_double_quotes = 0;
    int in_word = 0;
    int escape_sequenced = 0;

    while (*read != '\0') {
        if (escape_sequenced) {
          // Current character is escaped; write it as literal text
          if (!in_word) {
              argList[argCount++] = write;
              in_word = 1;
          }
          *write++ = *read;
          escape_sequenced = 0;
        } 
        else if (*read == '\\' && ((!in_single_quotes && !in_double_quotes) || (in_double_quotes && strchr("\"$`\n\\", *(read + 1))))) {
            // Backslash outside quotes triggers escape mode
          escape_sequenced = 1;
        }
        else if (*read == '\'' && !in_double_quotes) {
            in_single_quotes = !in_single_quotes;
        } 
        else if (*read == '"' && !in_single_quotes) {
            in_double_quotes = !in_double_quotes;
        } 
        else if (*read == ' ' && !in_single_quotes && !in_double_quotes) {
            if (in_word) {
                *write++ = '\0';
                in_word = 0;
            }
        } 
        else {
            if (!in_word) {
                argList[argCount++] = write;
                in_word = 1;
            }
            *write++ = *read;
        }
        read++;
    }
        if (in_word) {
        *write = '\0';
    }

    argList[argCount] = NULL;

    char *command = argList[0];
    // If the command received is "exit", break out of the loop
    if (!strcmp(command, "exit")) {
      break;
    }

    // If the command is "echo", print everything after the first arg
    else if (!strcmp(command, "echo")) {
      if (does_exist_in_array(argList, ">")) {
      FILE *file = fopen(argList[argCount - 1], "w");
        for (int i = 1; !strcmp(argList[i], ">"); i++) {
          fprintf(file, argList[i], "\n");
        }
        fclose(file);
      }
      
      else {
        for (int i = 1; i < argCount; i++) {
          printf("%s%s", argList[i], (i == argCount - 1) ? "" : " ");
        }
        printf("\n");

      }
    }
    // Handle type
    else if (!strcmp(command, "type")) {

      if (does_exist_in_array(builtins, argList[1])) {
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
 