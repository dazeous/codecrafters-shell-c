#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    char *fSpace = strchr(userInput, ' ');
    char *arguments;
    if (fspace != NULL) {
      *fspace = '\0';
      arguments = fspace + 1;
    }
    char *command = userInput;
    // If the command received is "exit", break out of the loop
    if (!strcmp(command, "exit")) {
      break;
    }
    else if (!strcmp(command, "echo")) {
      printf("%s\n", arguments);
    }
    else {
      printf("%s: command not found\n", command);
    }  
  }
  return 0;
}
