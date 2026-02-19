#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

#define CHUNK_SIZE 1024 // 1 KB
#define TASK_QUEUE_SIZE (1024 * 1024 * 4) // compress up to 4 GB

typedef struct {
  void *(*fn)(void *);
  void *arg;  
} task;

typedef struct {
  char *data;
  int chunk_i;
} task_arg;

task task_queue[TASK_QUEUE_SIZE];
int queue_front = -1;
int queue_rear = -1;
int queue_cnt = 0;
pthread_mutex_t mutex_q = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_q_not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_q_not_full = PTHREAD_COND_INITIALIZER;

void submit_task(void *(*fn)(void *), void *arg) {
  pthread_mutex_lock(&mutex_q);
  
  while (queue_cnt == TASK_QUEUE_SIZE) {
    pthread_cond_wait(&cond_q_not_full, &mutex_q);
  }
  
  if (queue_rear == -1) {
    queue_front = queue_rear = 0;
  } else {
    queue_rear = (queue_rear + 1) % TASK_QUEUE_SIZE;
  }
   
  task_queue[queue_rear].fn = fn;
  task_queue[queue_rear].arg = arg;
  ++queue_cnt;
  pthread_cond_signal(&cond_q_not_empty);
  pthread_mutex_unlock(&mutex_q);
}

void *worker(void *arg) {
  while (1) {
    pthread_mutex_lock(&mutex_q);

    while (queue_cnt == 0) {
      pthread_mutex_unlock(&mutex_q);
      return NULL;
      
      // pthread_cond_wait(&cond_q_not_empty, &mutex_q);
    }
    
    task t = task_queue[queue_front];
  
    queue_front = (queue_front + 1) % TASK_QUEUE_SIZE;
    --queue_cnt;
    pthread_cond_signal(&cond_q_not_full);
    pthread_mutex_unlock(&mutex_q);
    t.fn(t.arg);
    free(t.arg);
  }

  return NULL;
}

void* compress(void *arg) {
  printf("compress %d\n", ((task_arg *)arg)->chunk_i);
  
  return NULL;
}

int main(int argc, char *argv[]) {
  //========== usage check ==========//
  if (argc < 2) {
    printf("pzip: file1 [file2 ...]\n");
    exit(1);
  }

  //========== get file stat ==========//
  int fd;

  if ((fd = open(argv[1], O_RDONLY)) == -1) {
    printf("pzip: open failed\n");
    exit(1);
  }
  
  struct stat statbuf;

  if (stat(argv[1], &statbuf) != 0) {
    printf("pzip: stat failed\n");
    exit(1);
  }
  //========== mmap ==========//
  char *data;

  if ((data = mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
    printf("pzip: mmap failed\n");
    close(fd);
    exit(1);
  }
  //========== create threads ==========//
  int n = get_nprocs();
  pthread_t threads[n];
  
  for (int i = 0; i < n; ++i) {
    if (pthread_create(&threads[i], NULL, &worker, NULL) != 0) {
      printf("pzip: pthread_create failed\n");
      exit(1);
    }
  }
  //========== fill task queue ==========//
  int chunk_num = ceil(((double)statbuf.st_size) / CHUNK_SIZE);
  task_arg *arg;
  
  for (int i = 0; i < chunk_num; ++i) {
    arg = (task_arg *)malloc(sizeof(task_arg));
    arg->data = data;
    arg->chunk_i = i;
    submit_task(&compress, (void *)arg); 
  }
  //========== cleanup ==========//
  for (int i = 0; i < n; ++i) {
    if (pthread_join(threads[i], NULL) != 0) {
      printf("pzip: pthread_join failed\n");
      exit(1);
    }
  }
  
  close(fd);
  munmap(data, statbuf.st_size);
  pthread_mutex_destroy(&mutex_q);
  pthread_cond_destroy(&cond_q_not_empty);
  pthread_cond_destroy(&cond_q_not_full);
  
  return 0;
}
