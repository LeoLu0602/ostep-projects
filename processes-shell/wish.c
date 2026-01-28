#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#define MAX_ARGS 10
#define PATH_MAX 100
#define MAX_CMD_CNT 100
#define MAX_CMD_LEN 200

int main(int argc, char *argv[]) {
  char error_message[30] = "An error has occurred\n";
  int pid;
  char *line = NULL;
  size_t len = 0;
  char *wish_argv[MAX_ARGS + 1];
  ssize_t n;
  char path[PATH_MAX];
  char *p;
  char *tok;
  int wish_argc = 0;
  int redir_pos = -1; // index of the last ">" in wish_argv, -1 if not present
  int redir_cnt = 0;
  FILE *fp = stdin;
  char cmds[MAX_CMD_CNT][MAX_CMD_LEN + 1];
  int cmd_cnt = 0;
  pid_t pids[MAX_CMD_CNT];
  int pid_cnt = 0;

  if (argc > 2) {
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(1);
  }

  // batch mode
  if (argc == 2) {
    fp = fopen(argv[1], "r");

    if (!fp) {
      write(STDERR_FILENO, error_message, strlen(error_message));
      exit(1);
    }
  }

  while (1) {
    if (argc == 1) {
      printf("wish> ");
    }

    n = getline(&line, &len, fp);

    if (n == -1) {
      // EOF
      if (feof(fp)) {
        if (argc == 1) {
          printf("\n");
        }

        exit(0);
      }

      write(STDERR_FILENO, error_message, strlen(error_message));
      continue;
    }

    // build cmds
    line[n - 1] = '\0'; // get rid of \n
    cmd_cnt = 0;
    p = line;

    while ((tok = strsep(&p, "&"))) {
      if (*tok == '\0') {
        continue;
      }

      snprintf(cmds[cmd_cnt++], MAX_CMD_LEN, "%s", tok);
    }

		pid_cnt = 0;

    for (int i = 0; i < cmd_cnt; ++i) {
      // build wish_argv
      p = cmds[i];
      wish_argc = 0;
      redir_pos = -1;
      redir_cnt = 0;

      while ((tok = strsep(&p, " "))) {
        if (*tok == '\0') {
          continue;
        }

        if (strcmp(tok, ">") == 0) {
          redir_pos = wish_argc;
          ++redir_cnt;
        }

        wish_argv[wish_argc++] = strdup(tok);
      }

      wish_argv[wish_argc] = NULL;

      // more than one > or more than one files followed >
      if (redir_cnt > 1 || (redir_cnt == 1 && redir_pos != wish_argc - 2)) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        continue;
      }

      if (strcmp(wish_argv[0], "exit") == 0) {
        if (wish_argc != 1) {
          write(STDERR_FILENO, error_message, strlen(error_message));
          continue;
        }

        exit(0);
      }

      if (strcmp(wish_argv[0], "cd") == 0) {
        if (wish_argc != 2) {
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
        write(STDERR_FILENO, error_message, strlen(error_message));
        continue;
      } else if (pid == 0) {
        // search /bin
        snprintf(path, sizeof(path), "/bin/%s", wish_argv[0]);

        // if (access(path, X_OK) != 0) {
        //   // search /usr/bin
        //   snprintf(path, sizeof(path), "/usr/bin/%s", wish_argv[0]);

        //   if (access(path, X_OK) != 0) {
        //     write(STDERR_FILENO, error_message, strlen(error_message));
        //     continue;
        //   }
        // }

        // handle redirection
        if (redir_cnt == 1) {
          int fd = open(wish_argv[wish_argc - 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);

          dup2(fd, STDOUT_FILENO);
          close(fd);
          wish_argv[redir_pos] = NULL;
        }

        execv(path, wish_argv);
        write(STDERR_FILENO, error_message, strlen(error_message));
      } else {
        pids[pid_cnt++] = pid;
      }
    }

    for (int i = 0; i < pid_cnt; ++i) {
      waitpid(pids[i], NULL, 0);
    }
  }

  if (argc == 2) {
    fclose(fp);
  }

  free(line);

  return 0;
}
