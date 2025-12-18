#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("wgrep: searchterm [file ...]\n");
    exit(1);
  }

  FILE *fp;
  char *line = NULL;
  size_t len = 0;

  if (argc == 2) {
    while (getline(&line, &len, stdin) != -1) {
      if (strstr(line, argv[1])) {
	printf("%s", line);
      }
    }

    exit(0);
  }

  for (int i = 2; i < argc; ++i) {
    if (!(fp = fopen(argv[i], "r"))) {
      printf("wgrep: cannot open file\n");
      exit(1);
    }

    while (getline(&line, &len, fp) != -1) {
	if (strstr(line, argv[1])) {
	  printf("%s", line);
	}
    }

    free(line);
    
    if (fclose(fp)) {
      printf("wgrep: cannot close file\n");
      exit(1);
    }
  }

  return 0;
}
