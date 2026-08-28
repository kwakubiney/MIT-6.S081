#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

void
run(char *line, int length, int argc, char *argv[])
{
  char *args[MAXARG];
  int count = 0;
  int i = 0;

  for(int j = 1; j < argc; j++){
    args[count++] = argv[j];
  }

  while(i < length){
    while(i < length && line[i] == ' '){
      line[i] = '\0';
      i++;
    }

    if(i < length){
      args[count++] = &line[i];
    }

    while(i < length && line[i] != ' '){
      i++;
    }
  }

  args[count] = 0;

  if(fork() == 0){
    exec(args[0], args);
    exit(1);
  }

  wait(0);
}
// The shell creates the pipe and connects it to standard input.
// For each input line, xargs forks and runs the command with the new arguments.
// AI helped with the C implementation. I provided the general design.
int
main(int argc, char *argv[])
{
  char line[512];
  char character;
  int length = 0;

  while(read(0, &character, 1) > 0){
    if(character == '\n'){
      line[length] = '\0';
      run(line, length, argc, argv);
      length = 0;
    } else {
      line[length++] = character;
    }
  }

  if(length > 0){
    line[length] = '\0';
    run(line, length, argc, argv);
  }

  exit(0);
}
