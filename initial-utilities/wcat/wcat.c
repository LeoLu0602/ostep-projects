#include <stdlib.h>
#include <stdio.h>

#define BUFFER_SIZE 512

int main(int argc, char *argv[]) {
  FILE *fp;
  char buffer[BUFFER_SIZE];

  for (int i = 1; i < argc; ++i) {
    if (!(fp = fopen(argv[i], "r"))) {
      printf("wcat: cannot open file\n");
      exit(1);
    }

    while (fgets(buffer, BUFFER_SIZE, fp)) {
      printf("%s", buffer);
    }

    if (fclose(fp)) {
      printf("wcat: error closing %s\n", argv[i]);
      exit(1);
    }
  }

  return 0;
}
