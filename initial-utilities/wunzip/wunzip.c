#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("wunzip: file1 [file2 ...]\n");
    exit(1);
  }

  FILE *fp;
  int cnt;
  char c;

  for (int i = 1; i < argc; ++i) {
    if (!(fp = fopen(argv[i], "r"))) {
      printf("wunzip: cannot open file\n");
      exit(1);
    }

    while (fread(&cnt, 4, 1, fp)) {
      fread(&c, 1, 1, fp);

      for (int j = 0; j < cnt; ++j) {
	fwrite(&c, 1, 1, stdout);
      }
    }

    if (fclose(fp)) {
      printf("wunzip: cannot close file\n");
      exit(1);
    }
  }

  return 0;
}
