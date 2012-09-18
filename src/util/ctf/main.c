/* counter to frequency */

#include <stdio.h>
#include <stdint.h>

static const uint16_t counters[] =
{
  0x23e3,
  0x23e2
};

int main(int ac, char** av)
{
  unsigned int i;

  for (i = 0; i < sizeof(counters) / sizeof(counters[0]); ++i)
  {
    /* const double f = (double)counters[i] / 0.262144; */
    /* const double f = (double)counters[i] / 0.001024; */
    /* const double f = (double)counters[i] * 0.953674; */
    const double f = (double)counters[i] * 30.517578;
    printf("%lf hz\n", f);
  }

  return 0;
}
