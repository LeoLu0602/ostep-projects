#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_ARGS 10

int main(int argc, char *argv[]) {
  if (argc > 2) {
    printf("wish: too many args\n");
    exit(1);
  }
  
  // batch mode
  if (argc == 2) {
    exit(0);
  }
  
  // interactive mode
  int pid;
  char *line = NULL;
  size_t len = 0;
  char *wish_argv[MAX_ARGS + 1];

  while (1) {
    printf("wish> ");

    if (getline(&line, &len, stdin) == -1) {
      if (feof(stdin)) {
	// EOF
	exit(0);
      }

      exit(1);
    }

    pid = fork();

    if (pid < 0) {
      // error
      printf("wish: fork failed\n");
      exit(1);
    } else if (pid == 0) {
      // child
      wish_argv[1] = NULL; // tmp
      execv(wish_argv[0], wish_argv);
      printf("wish: execv failed\n");
      exit(1);
    } else {
      // parent
      wait(NULL);
    }
  }

  free(line);

  return 0;
}
