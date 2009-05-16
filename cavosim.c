#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>

#define BIT(n) (1 << (n))
enum { nbits = 12, immediate_mask = BIT(nbits-1) };
#define ONES(number_of_ones) (BIT(number_of_ones) - 1)
#define SIGN_EXTEND(x) ((((x) & BIT(nbits-2)) << 1) | (x & ONES(nbits-1)))
#define INSTRUCTION_MASK (ONES(nbits-1) & ~ONES(nbits-4))
enum instructions { jump = 0, subtract, nand, fetch, store, nop };

typedef int cavo_word;       /* only use the bottom 12 bits */
enum { memory_size = BIT(nbits-1) };
cavo_word p, a, x, i, tmp, mem[memory_size]; /* CPU registers and memory */

void panic() {
  perror("panicking");
  abort();
}

void do_store(cavo_word addr, cavo_word value) {
  printf("mem[%d] ← %d\n", (int)addr, (int)value);
  mem[addr] = value;
}


void go() {
  for (;;) {

    /* fetch */
    printf("[%d] ", p);
    fflush(stdout);
    i = mem[p++];
    p %= memory_size;

    /* execute */

    if (i & immediate_mask) { /* $ */
      x = a;
      a = SIGN_EXTEND(i);

    } else {
      switch ((i & INSTRUCTION_MASK) >> (nbits-4)) {

      case jump:                /* . */
        if (BIT(nbits-1) & x) a = x;
        else {
          tmp = a;
          a = p;
          p = tmp % memory_size;
        }
        break;

      case subtract:            /* - */
        a = (x - a) & ONES(nbits);
        break;

      case nand:                /* | */
        a = ~(a & x);
        break;

      case fetch:               /* @ */
        a = mem[a & ONES(nbits-1)];
        break;

      case store:               /* ! */
        do_store(a & ONES(nbits-1), x);
        break;

      case nop:
        break;

      default:
        panic();
      }
    }
  }
}

int main(int argc, char **argv) {
  FILE *f = fopen(argv[1], "rb");
  int i;
  if (!f) panic();
  for (i = 0; i < memory_size; i++) {
    if (EOF == fscanf(f, "%d\n", &mem[i])) break;
  }
  p = 0;
  go();
  return 0;
}
