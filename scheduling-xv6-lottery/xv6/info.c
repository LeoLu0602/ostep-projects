
#include "types.h"
#include "user.h"

int main(void) {
    struct pstat p;

    getpinfo(&p);

    for (int i = 0; i < NPROC; ++i) {
        printf(1, "pid: %d, tickets: %d, ticks: %d\n", p.pid[i], p.tickets[i], p.ticks[i]);
    }

    exit();
}
