#include "types.h"
#include "user.h"

void fcn(void *arg1, void *arg2) {
  printf(1, "arg1 = %d, arg2 = %d\n", (int)arg1, (int)arg2);
  exit();
}

int main(int argc, char *argv[]) {
  void *stack = malloc(4096);
  int pid = clone(&fcn, (void *)69, (void *)247, stack);

  printf(1, "pid: %d\n", pid);
  exit();
}
