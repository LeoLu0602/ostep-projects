#include "types.h"
#include "user.h"

int main(void) {
    int pid;

    // child A: 30 tickets
    pid = fork();

    if (pid == 0) {
        settickets(30);
        
        for (int i = 0; i < 100; ++i) {
            sleep(1);
        }
    }

    // child B: 20 tickets
    pid = fork();

    if (pid == 0) {
        settickets(20);
        
        for (int i = 0; i < 100; ++i) {
            sleep(1);
        }
    }

    // child C: 10 tickets
    pid = fork();

    if (pid == 0) {
        settickets(10);
        
        for (int i = 0; i < 100; ++i) {
            sleep(1);
        }
    }

    // parent just waits
    while (wait() > 0)
        ;

    exit();
}
