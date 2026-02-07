
#include "types.h"
#include "user.h"

int main(void) {
    struct pstat p;

    getpinfo(&p);
    exit();
}
