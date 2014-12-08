#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[], char* envp[]){
  (void)envp;
  int numRuns;
  const char port1[] = "6789";
  const char port2[] = "5678";
  if(argc < 3){
    printf("Not enough args: %s <runner1> <runner2> [num runs]\n", argv[0]);
    return 1;
  }
  if(argc > 3){
    numRuns = atoi(argv[3]);
  }
  else{
    numRuns = 10;
  }

  printf("Running %s with %s %d times on ports %s and %s \n", 
          argv[1], argv[2], numRuns, port1, port2);
  const char* args1[] = {argv[1], port1, "127.0.0.1", port2, NULL};
  const char* args2[] = {argv[2], port2, "127.0.0.1", port1, NULL};
  pid_t child_pid1;
  pid_t child_pid2;
  int child_status;

  for(int i = 1; i < numRuns; i++){
    child_pid1 = fork();
    if(child_pid1 == 0) {
      execv((char*)args1[0], (char**)args1);
      // execv((char*)args1[0], (char**)args1);
      fprintf(stderr, "Exec fail\n");
      exit(0);
    }
    child_pid2 = fork();
    if(child_pid2 == 0) {
      execv((char*)args2[0], (char**)args2);
      // execv((char*)args2[0], (char**)args1);
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
