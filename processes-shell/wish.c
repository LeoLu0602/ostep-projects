#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#define MAX_ARGS 10
#define PATH_MAX 100

int main(int argc, char *argv[]) {
  char error_message[30] = "An error has occurred\n";
  
  if (argc > 2) {
    write(STDERR_FILENO, error_message, strlen(error_message));
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
  ssize_t n;
  char path[PATH_MAX];
  char *p;
  char *tok;
  int i = 0;
  int redir_pos = -1; // index of the last ">" in wish_argv, -1 if not present
  int redir_cnt = 0; 

  while (1) {
    printf("wish> ");

    if ((n = getline(&line, &len, stdin)) == -1) {
      if (feof(stdin)) {
	// EOF
	exit(0);
      }

      write(STDERR_FILENO, error_message, strlen(error_message));
      continue;
    }
    
    line[n - 1] = '\0'; // get rid of \n
    
    // build wish_argv
    p = line;
    i = 0;
    redir_pos = -1;
    redir_cnt = 0;

    while ((tok = strsep(&p, " "))) {
      if (*tok == '\0') {
	continue;
      }
      
      if (strcmp(tok, ">") == 0) {
	redir_pos = i;
	++redir_cnt;
      }

      wish_argv[i++] = strdup(tok);
    }
    
    wish_argv[i] = NULL;

    // more thatn one > or more than one files followed >
    if (redir_cnt > 1 || (redir_cnt == 1 && redir_pos != i - 2)) {
	write(STDERR_FILENO, error_message, strlen(error_message));
	continue;
    }
    
    if (strcmp(wish_argv[0], "exit") == 0) {
      exit(0);
    }

    if (strcmp(wish_argv[0], "cd") == 0) {
      if (i != 2) {
	write(STDERR_FILENO, error_message, strlen(error_message));
	continue;
      }

      if (chdir(wish_argv[1])) {
	write(STDERR_FILENO, error_message, strlen(error_message));
      }

      continue;
    }

    if (strcmp(wish_argv[0], "path")) {

    }

    pid = fork();

    if (pid < 0) {
      // error
      write(STDERR_FILENO, error_message, strlen(error_message));
      continue;
    } else if (pid == 0) {
      // child
      // search /bin
      snprintf(path, strlen("/bin/") + strlen(line) + 1, "/bin/%s", line);
      
      if (access(path, X_OK) != 0) {
	// search /usr/bin
	snprintf(path, strlen("/usr/bin/") + strlen(line) + 1, "/usr/bin/%s", line);

	if (access(path, X_OK) != 0) {
	  write(STDERR_FILENO, error_message, strlen(error_message));
	  continue;
	}
      }

      wish_argv[0] = strdup(path);
      
      // handle redirection
      if (redir_cnt == 1) {
	int fd = open(wish_argv[i - 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);

	dup2(fd, STDOUT_FILENO);
	close(fd);
	wish_argv[redir_pos] = NULL;
      }

      execv(wish_argv[0], wish_argv);
      write(STDERR_FILENO, error_message, strlen(error_message));
      continue;
    } else {
      // parent
      wait(NULL);
    }
  }

  free(line);

  return 0;
}

