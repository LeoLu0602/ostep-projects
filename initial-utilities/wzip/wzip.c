#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("wzip: file1 [file2 ...]\n");
    exit(1);
  }

  FILE *fp;
  char c;
  char last = EOF;
  int cnt = 0;

  for (int i = 1; i < argc; ++i) {
    if (!(fp = fopen(argv[i], "r"))) {
      printf("wzip: cannot open file\n");
      exit(1);
    }

    while ((c = fgetc(fp)) != EOF) {
      if (c == last) {
	++cnt;
	continue;
      }
      
      if (last != EOF) {
	fwrite(&cnt, sizeof(cnt), 1, stdout);
	fwrite(&last, sizeof(last), 1, stdout);
      }
     
      last = c;
      cnt = 1;
    }

    if (fclose(fp)) {
      printf("wzip: cannot close file\n");
      exit(1);
    }
  }

  fwrite(&cnt, sizeof(cnt), 1, stdout);
  fwrite(&last, sizeof(last), 1, stdout);

  return 0;
}
