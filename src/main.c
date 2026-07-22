#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <fcntl.h>


char *builtins[] = {"echo", "exit", "type", "pwd", "cd", NULL};

// Helper function to check if a command is a builtin
int ifBuiltin(char *cmd) {
    for (int i = 0; builtins[i] != NULL; i++) {
        if (strcmp(builtins[i], cmd) == 0) {
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


    // Input parsing begins here
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

    // Input parsing ends here


    char *outfile = NULL;
    int append_mode = 0;
    for (int i = 0; i < argCount; i++) {
      if (!strcmp(argList[i], ">")) {
        outfile = argList[i + 1];
        append_mode = 0;
        argList[i] = NULL;
        argCount = i;
        break;
      } else if (!strcmp(argList[i], ">>") == 0) {
        outfile = argList[i + 1];
        append_mode = 1;
        argList[i] = NULL;
        argCount = i;
        break;
      }
    }

    int saved_stdout = -1;
    if (outfile != NULL) {
      saved_stdout = dup(STDOUT_FILENO);
      int flags = O_WRONLY | O_CREAT | (append_mode ? O_APPEND : O_TRUNC);
      int fd = open(outfile, flags, 0644);
      if (fd >= 0) {
        dup2(fd, STDOUT_FILENO);
        close(fd);
      }
    }

    char *command = argList[0];
    // If the command received is "exit", break out of the loop
    if (!strcmp(command, "exit")) {
      break;
    }

    // If the command is "echo", print everything after the first arg
    else if (!strcmp(command, "echo")) {
      for (int i = 1; i < argCount; i++) {
        printf("%s%s", argList[i], (i == argCount - 1) ? "" : " ");
      }
      printf("\n");
    }
    // Handle type
    else if (!strcmp(command, "type")) {

      if (ifBuiltin(argList[1])) {
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
    if (saved_stdout != -1) {
      dup2(saved_stdout, STDOUT_FILENO);
      close(saved_stdout);
    }
  }

  return 0;
}
 