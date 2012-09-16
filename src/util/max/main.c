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
  static const double fpwm = 40413;

  /* which is 16 bits */
#define CONFIG_TIMER 1

#if (CONFIG_TIMER == 1)
  static const double tprescals[] =
  {
    1,
    8,
    64,
    256,
    1024
  };

  static const unsigned int tbits = 16;
  static const unsigned int cbits = 8;
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

  static const unsigned int tbits = 8;
  static const unsigned int cbits = 16;
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
