#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[], char* envp[]){
  (void)envp;
  int numRuns;
  if(argc != 2){
    numRuns = 10;
  }
  else{
    numRuns = atoi(argv[1]);
  }

  const char* args1[] = {"./hostPP", "5555", "127.0.0.1", "6666", NULL};
  const char* args2[] = {"./hostPP", "6666", "127.0.0.1", "5555", NULL};
  pid_t child_pid1;
  pid_t child_pid2;
  int child_status;

  for(int i = 1; i < numRuns; i++){
    child_pid1 = fork();
    if(child_pid1 == 0) {
      execv((char*)args1[0], (char**)args1);
      fprintf(stderr, "Exec fail\n");
      exit(0);
    }
    child_pid2 = fork();
    if(child_pid2 == 0) {
      execv((char*)args2[0], (char**)args2);
      fprintf(stderr, "Exec fail\n");
      exit(0);
    }
    else {
      while((child_pid1 != -1) || (child_pid2 != -1)){
        pid_t tpid = wait(&child_status);
        if(tpid == child_pid1){
          child_pid1 = -1;
        }
        else if(tpid == child_pid2){
          child_pid2 = -1;
        }
      }
    }
  }

  return 0; 
}
