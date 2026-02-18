#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct {
  char *data;
  int start;
  int end;
} thread_arg;

void *worker(void *arg) {
  thread_arg *p = (thread_arg *)arg;

  printf("%d, %d\n", p->start, p->end);
  free(p);

  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("pzip: file1 [file2 ...]\n");
    exit(1);
  }

  int fd;

  if ((fd = open(argv[1], O_RDONLY)) == -1) {
    printf("pzip: open failed\n");
    exit(1);
  }
  
  int n = 4; // number of chunks
  struct stat statbuf;

  if (stat(argv[1], &statbuf) != 0) {
    printf("pzip: stat failed\n");
    exit(1);
  }
  
  char *data;

  if ((data = mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
    printf("pzip: mmap failed\n");
    close(fd);
    exit(1);
  }
  
  int chunk_size = statbuf.st_size / n;
  pthread_t threads[n];

  for (int i = 0; i < n; ++i) {
    thread_arg *p = (thread_arg *)malloc(sizeof(thread_arg));

    p->data = data;
    p->start = i * chunk_size;
    p->end = (i + 1) * chunk_size - 1;

    if (pthread_create(&threads[i], NULL, worker, p) != 0) {
      printf("pzip: pthread_create failed\n");
      exit(1);
    }
  }
  
  close(fd);
  munmap(data, statbuf.st_size);
  
  for (int i = 0; i < n; ++i) {
    if (pthread_join(threads[i], NULL) != 0) {
      printf("pzip: pthread_join failed\n");
      exit(1);
    }
  }

  return 0;
}
