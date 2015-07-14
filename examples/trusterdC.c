#include <stdio.h>
#include <stdlib.h>

typedef int (*FUNCPTR)(char *script);
int boot(char*, FUNCPTR);

int myCallback(char *script) {
  printf("%s\n",script);
  return 0;
}

int main(int argc, char **argv) {
  boot("puts 1+2",myCallback);
  return 0;
}
