#include <stdio.h>

/* tabulate -x³ + 2x² - x + 4 */
/* int a0 = 0, a1 = 0, a2 = -6, a3 = -2, a4 = 0, a5 = 4; */

/* tabulate a more boring polynomial */
int a0 = 1, a1 = -1, a2 = 1, a3 = -1, a4 = 1, a5 = -1;

main() {
  for (;;) {
    printf("%d\n", a5);
    a5 += a4;
    a4 += a3;
    a3 += a2;
    a2 += a1;
    a1 += a0;
  }
}
