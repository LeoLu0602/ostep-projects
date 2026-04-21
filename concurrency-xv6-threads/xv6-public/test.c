#include "types.h"
#include "user.h"

void fcn(void *a, void *b) {

}

int main(int argc, char *argv[]) {
  printf(1, "clone: %d\n", clone(&fcn));
  exit();
}
