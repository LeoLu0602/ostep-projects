#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#define CHUNK_SIZE 256
#define TASK_QUEUE_SIZE 1024
#define MAX_CHUNK_NUM (1024 * 1024 * 4) // compress up to 1 GB
#define MAX_FILES 5

typedef struct {
  int nblock;
  int cnt_arr[64];
  char c_arr[64];
} compressed_chunk;

typedef struct {
  void *(*fn)(void *);
  void *arg;  
} task;

typedef struct {
  int chunk_i;
  char *start;
  char *end;
} task_arg;

task task_queue[TASK_QUEUE_SIZE];
int queue_front = -1;
int queue_rear = -1;
int queue_cnt = 0;
int stop = 0;

compressed_chunk res[MAX_CHUNK_NUM];
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

    while (queue_cnt == 0 && !stop) {
      pthread_cond_wait(&cond_q_not_empty, &mutex_q);
    }

    if (queue_cnt == 0 && stop) {
      pthread_mutex_unlock(&mutex_q);
      break;
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
  int chunk_i = ((task_arg *)arg)->chunk_i;
  char *start = ((task_arg *)arg)->start;
  char *end = ((task_arg *)arg)->end;
  char *cur = start;
  char last = '\0';
  int first_block = 1;
  int cnt = 0;
  int nblock = 0;

  while (cur <= end) {
    if (first_block) {
      first_block = 0;
      last = *cur;
      ++cur;
      cnt = 1;
      continue;
    }

    if (*cur == last) {
      ++cnt;
    } else {
      res[chunk_i].cnt_arr[nblock] = cnt;
      res[chunk_i].c_arr[nblock] = last;
      ++nblock;
      last = *cur;
      cnt = 1;
    }

    ++cur;
  }
  
  res[chunk_i].cnt_arr[nblock] = cnt;
  res[chunk_i].c_arr[nblock] = last;
  ++nblock;
  res[chunk_i].nblock = nblock;

  return NULL;
}

void merge(int chunk_num) {
  for (int i = 0; i < chunk_num; ++i) {
    for (int j = 0; j < res[i].nblock; ++j) {
      if (
	  i != chunk_num - 1 && 
	  j == res[i].nblock - 1 &&
	  res[i].c_arr[j] == res[i + 1].c_arr[0]
      ) {
	res[i + 1].cnt_arr[0] += res[i].cnt_arr[j];
	continue;
      }

      fwrite(&(res[i].cnt_arr[j]), sizeof(int), 1, stdout);
      fwrite(&(res[i].c_arr[j]), 1, 1, stdout);
    }
  }
}

int main(int argc, char *argv[]) {
  //========== usage check ==========//
  if (argc < 2) {
    printf("pzip: file1 [file2 ...]\n");
    exit(1);
  }

  if (argc > MAX_FILES + 1) {
    printf("pzip: too many files (max: %d)\n", MAX_FILES);
    exit(1);
  }
  //========== get file stat & mmap ==========//
  int file_num = argc - 1;
  struct stat file_stats[MAX_FILES];
  char *file_data[MAX_FILES];

  for (int i = 0; i < file_num; ++i) {
    //========== get file stat ==========//
    int fd;

    if ((fd = open(argv[i + 1], O_RDONLY)) == -1) {
      printf("pzip: open failed\n");
      exit(1);
    }
    
    if (stat(argv[i + 1], &file_stats[i]) != 0) {
      printf("pzip: stat failed\n");
      exit(1);
    }
    //========== mmap ==========//
    if ((file_data[i] = mmap(NULL, file_stats[i].st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
      printf("pzip: mmap failed\n");
      close(fd);
      exit(1);
    }

    close(fd);
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
  int chunk_cnt = 0;

  for (int i = 0; i < file_num; ++i) {
    int chunk_num = ceil(((double)file_stats[i].st_size) / CHUNK_SIZE);
    task_arg *arg;
    
    for (int j = 0; j < chunk_num; ++j) {
      arg = (task_arg *)malloc(sizeof(task_arg));
      arg->chunk_i = chunk_cnt;
      ++chunk_cnt;
      arg->start = file_data[i] + j * CHUNK_SIZE;
      arg->end = (j == chunk_num - 1) ? file_data[i] + file_stats[i].st_size - 1 : file_data[i] + (j + 1) * CHUNK_SIZE - 1;
      submit_task(&compress, (void *)arg); 
    }
  }
  //========== shutdown ==========//
  pthread_mutex_lock(&mutex_q);
  stop = 1;
  pthread_cond_broadcast(&cond_q_not_empty);
  pthread_mutex_unlock(&mutex_q);
  //========== cleanup ==========//
  for (int i = 0; i < n; ++i) {
    if (pthread_join(threads[i], NULL) != 0) {
      printf("pzip: pthread_join failed\n");
      exit(1);
    }
  }
  //========== merge ==========//
  merge(chunk_cnt);
  //========== cleanup ==========//
  for (int i = 0; i < file_num; ++i) {
    munmap(file_data[i], file_stats[i].st_size);
  }

  pthread_mutex_destroy(&mutex_q);
  pthread_cond_destroy(&cond_q_not_empty);
  pthread_cond_destroy(&cond_q_not_full);
  
  return 0;
}
