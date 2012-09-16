/* counter to frequency */

#include <stdio.h>
#include <stdint.h>

static const uint16_t counters[] =
{
  0x20,
  0x22,
  0x23,
  0x25,
  0x27,
  0x29,
  0x186,
  0x2e8,
  0xaaa,
  0x1000
};

int main(int ac, char** av)
{
  unsigned int i;

  for (i = 0; i < sizeof(counters) / sizeof(counters[0]); ++i)
  {
    /* const double f = (double)counters[i] / 0.262144; */
    /* const double f = (double)counters[i] / 0.001024; */
    const double f = (double)counters[i] * 0.953674;
    printf("%lf hz\n", f);
  }

  return 0;
}
