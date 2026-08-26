#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(void){
   int p[2];
   pipe(p);
   int q[2];
   pipe(q);
  
  int pid = fork();
  //if pid = 0, that'll be the child flow
  if(pid == 0){
    close(p[1]);
    close(q[0]);

    char a;
    read(p[0], &a, 1);
    printf("%d:received ping\n", getpid());

    close(p[0]);
    char c = 'a';
    write(q[1], &c, 1);
    exit(0);
}else{
    char c = 'a';
    close(p[0]);
    write(p[1], &c, 1);
    close(p[1]);

    char b;
    read(q[0], &b, 1);
    close(q[0]);
    printf("%d:received pong\n", getpid());
    wait(0);
    exit(0);
}}
