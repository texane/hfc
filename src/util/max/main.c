#include <stdio.h>
#include <math.h>

static inline double compute_tmax
(double fcpu, double tprescal, unsigned int tbits)
{
  return ((double)(1 << tbits) * tprescal) / fcpu;
}

int main(int ac, char** av)
{
  static const double fcpu = 16000000;
  static const double fpwm = 500000;

  /* which is 16 bits */
#define CONFIG_TIMER 2

#if (CONFIG_TIMER == 1)
  static const double tprescals[] =
  {
    1,
    8,
    64,
    256,
    1024
  };

  static const unsigned int tbits = 16; /* 16 hard */
  static const unsigned int cbits = 16; /* 8 hard + 8 soft */
#elif (CONFIG_TIMER == 2)
  static const double tprescals[] =
  {
    1,
    8,
    16,
    32,
    64,
    128,
    256,
    1024
  };

  static const unsigned int tbits = 16; /* 8 hard + 8 soft */
  static const unsigned int cbits = 18; /* 16 hard + 2 soft */
#endif /* CONFIG_TIMER == x */

  unsigned int i;

  for (i = 0; i < sizeof(tprescals) / sizeof(tprescals[0]); ++i)
  {
    const double tmax = compute_tmax(fcpu, tprescals[i], tbits);
    const double fmax = (double)(1 << cbits) / tmax;
    const double fres = (1.0 / tmax);
    const double nreg = 1 + ceil(fmax / fpwm);
    printf("%lf %lf %lf %lf %lf\n", tprescals[i], fmax, floor(log2(fmax)), fres, nreg);
  }

  return 0;
}
