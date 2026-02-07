#include "types.h"
#include "user.h"

int main(int argc, char *argv[]) {
  struct pstat ps;

  if (getpinfo(&ps) < 0){
    printf(2, "getpinfo failed\n");
    exit();
  }

  for (int i = 0; i < NPROC; i++){
    if (ps.inuse[i]) {
      printf(1, "%d %d %d\n", ps.pid[i], ps.tickets[i], ps.ticks[i]);
    }
  }

  exit();
}
