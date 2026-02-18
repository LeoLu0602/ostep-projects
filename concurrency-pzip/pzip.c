#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("pzip: file1 [file2 ...]\n");
    exit(1);
  }
  
  int n = 4; // number of chunks
  struct stat statbuf;

  if (stat(argv[1], &statbuf) != 0) {
    printf("pzip: stat failed\n");
    exit(1);
  }

  int chunk_size = statbuf.st_size / n;
  
  printf("chunk_size: %d\n", chunk_size);
  /*
  FILE *fp;
  char c;
  char last = EOF;
  int cnt = 0;

  for (int i = 1; i < argc; ++i) {
    if (!(fp = fopen(argv[i], "r"))) {
      printf("pzip: cannot open file\n");
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
      printf("pzip: cannot close file\n");
      exit(1);
    }
  }

  fwrite(&cnt, sizeof(cnt), 1, stdout);
  fwrite(&last, sizeof(last), 1, stdout);
  */

  return 0;
}
