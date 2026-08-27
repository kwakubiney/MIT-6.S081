#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void sieve(int input_fd);

int
main(void)
{ 

  /*
   * Here, we first created a pipe to assist with moving numbers through the generator
   * to the first sieve process.
   */
  int p[2];
  pipe(p);
  int pid = fork();

  if (pid == 0) {
    close(p[1]);
    sieve(p[0]);
    exit(0);
  } else {
    close(p[0]);
    /*
     * Generator logic for the numbers, we write to the write endpoint of the pipe.
     */
    for (int number = 2; number <= 35; number++) {
      write(p[1], &number, sizeof(number));
    }
    close(p[1]);
    wait(0);
    exit(0);
  }
}

void
sieve(int input_fd)
{
  /*
   * Each sieve stage prints the prime number the first time (first number that comes
   * through) and has to create a new pipe to feed the next phase of numbers to the
   * next stage.
   *
   * So we create a next_pipe and essentially route the next phase of numbers through
   * its write endpoint. But of course, the next stage is also a forked process, so
   * after forking, we can just call sieve again with the input being next_pipe's read
   * endpoint for the next call to use for the printing and next phase writing (to the
   * next next_pipe), and it goes on and on till each parent's write endpoint closes
   * (with each close) and read() can return zero for each stage.
   */

  int prime;
  int bytes_read = read(input_fd, &prime, sizeof(prime));

  if (bytes_read != sizeof(prime)) {
    close(input_fd);
    return;
  }

  printf("prime %d\n", prime);

  int next_pipe[2];
  pipe(next_pipe);

  int pid = fork();
  if (pid == 0) {
    close(next_pipe[1]);
    close(input_fd);
    sieve(next_pipe[0]);
    exit(0);
  } else {
    close(next_pipe[0]);
    int number;
    while ((bytes_read = read(input_fd, &number, sizeof(number))) == sizeof(number)) {
      if (number % prime != 0) {
        write(next_pipe[1], &number, sizeof(number));
      }
    }
    close(input_fd);
    close(next_pipe[1]);
    wait(0);
  }
}
