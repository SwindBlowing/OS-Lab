#include "ulib.h"

int main(int argc, char *argv[]) {
  assert(argc > 0);
  //printf("%p\n", argv);
  assert(strcmp(argv[0], "echo") == 0);
  assert(argv[argc] == NULL);
  for (int i = 1; i < argc; ++i) {
	//printf("here\n");
    printf("%s%c", argv[i], i == argc - 1 ? '\n' : ' ');
  }
  char *sh1_argv[] = {"sh1", NULL};
  exec("sh1", sh1_argv);
  assert(0);
}
