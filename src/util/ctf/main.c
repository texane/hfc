/* counter to frequency */

#include <stdio.h>
#include <stdint.h>

static const uint32_t counters[] =
{
  0x00023dfd,
  0x00023dfc,
  0x00023dfb
};

int main(int ac, char** av)
{
  unsigned int i;

  for (i = 0; i < sizeof(counters) / sizeof(counters[0]); ++i)
  {
    /* const double f = (double)counters[i] / 0.262144; */
    /* const double f = (double)counters[i] / 0.001024; */
    /* const double f = (double)counters[i] * 0.953674; */
    /* const double f = (double)counters[i] * 30.517578; */
    const double f = (double)counters[i] * 1.907349;
    printf("%lf hz\n", f);
  }

  return 0;
}
