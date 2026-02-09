#include "types.h"
#include "user.h"

int main(int argc, char *argv[]) {
  int pid;
  int pid1, pid2, pid3;
  struct pstat ps;

  settickets(69);

	// child 1
  pid = fork();

  if (pid == 0){
    
    while (1)
        ;
  }

  pid1 = pid;

	// child 2
  pid = fork();

  if (pid == 0){

    settickets(20);
    
		while (1)
			;
  }
  
	pid2 = pid;

	// child 3
  pid = fork();

  if (pid == 0){
    settickets(10);
    
		while (1)
			;
  }
  
	pid3 = pid;

	// let scheduler run
  sleep(500);

	// take a snapshot
  if (getpinfo(&ps) >= 0){
		for (int i = 0; i < NPROC; i++) {
			if (ps.inuse[i] && (ps.pid[i] == pid1 || ps.pid[i] == pid2 || ps.pid[i] == pid3)) {
				printf(1, "pid: %d, tickets: %d, ticks: %d\n", ps.pid[i], ps.tickets[i], ps.ticks[i]);
			}
		}
  } else {
		printf(1, "getpinfo failed\n");
	}

	kill(pid1);
	kill(pid2);
	kill(pid3);

  while (wait() > 0)
  	;

  exit();
}
