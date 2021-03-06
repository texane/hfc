#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>


#define CONFIG_UART 1
#if CONFIG_UART /* uart */

static inline void set_baud_rate(long baud)
{
  uint16_t UBRR0_value = ((F_CPU / 16 + baud / 2) / baud - 1);
  UBRR0H = UBRR0_value >> 8;
  UBRR0L = UBRR0_value;
}

static void uart_setup(void)
{
  /* #define CONFIG_FOSC (F_CPU * 2) */
  /* const uint16_t x = CONFIG_FOSC / (16 * BAUDS) - 1; */
#if 0 /* (bauds == 9600) */
  const uint16_t x = 206;
#elif 0 /* (bauds == 115200) */
  const uint16_t x = 16;
#elif 0 /* (bauds == 500000) */
  const uint16_t x = 3;
#elif 0 /* (bauds == 1000000) */
  const uint16_t x = 1;
#endif

  set_baud_rate(9600);

  /* baud doubler off  - Only needed on Uno XXX */
  UCSR0A &= ~(1 << U2X0);

  UCSR0B = 1 << TXEN0;

  /* default to 8n1 framing */
  UCSR0C = (3 << 1);
}

static void uart_write(uint8_t* s, uint8_t n)
{
  for (; n; --n, ++s)
  {
    /* wait for transmit buffer to be empty */
    while (!(UCSR0A & (1 << UDRE0))) ;
    UDR0 = *s;
  }

  /* wait for last byte to be sent */
  while ((UCSR0A & (1 << 6)) == 0) ;
}

#endif /* CONFIG_UART */


static inline uint8_t nibble(uint32_t x, uint8_t i)
{
  return (x >> (i * 4)) & 0xf;
}

static inline uint8_t hex(uint8_t x)
{
  return (x >= 0xa) ? 'a' + x - 0xa : '0' + x;
}

static uint8_t* uint32_to_string(uint32_t x)
{
  static uint8_t buf[8];

  buf[7] = hex(nibble(x, 0));
  buf[6] = hex(nibble(x, 1));
  buf[5] = hex(nibble(x, 2));
  buf[4] = hex(nibble(x, 3));
  buf[3] = hex(nibble(x, 4));
  buf[2] = hex(nibble(x, 5));
  buf[1] = hex(nibble(x, 6));
  buf[0] = hex(nibble(x, 7));

  return buf;
}


/* high resolution frequency counter implementation
 */

/* timer2 interrupt handler. timer1 is an extended
   32 bits register (16 bits hard + 16 softs)
   incremented once per:
   1 / (fcpu / prescal) <=> prescal / fcpu
   thus, it will overflow at:
   2^16 * prescal / fcpu
   on tim2 overflow, the interrupt handler is called
   and stores the tim1 current value in tim1_cur_counter.
   thus, the tim1 value integrated over the whole
   tim1 period is:
   (tim1_ovf_counter * 2^16) + tim1_cur_counter.
   tim2_is_ovf is set to notify the application.
 */

static volatile uint8_t tim2_ovf_counter;
static volatile uint8_t tim2_is_ovf;
static volatile uint16_t tim1_cur_counter;

ISR(TIMER2_OVF_vect)
{
  if ((tim2_ovf_counter--) == 0)
  {
    /* disable tim1 before reading */
    TCCR1B = 0;
    tim1_cur_counter = TCNT1;

    /* disable tim2 */
    TCCR2B = 0;

    tim2_is_ovf = 1;
  }
}

/* timer2 interrupt handler. timer2 is a 8 bits counter
   incremented by the input signal rising edges. since
   8 bits are not enough to integrate, an auxiliary
   register (tim2_ovf_counter) is updated on overflow.
   tim2_ovf_counter is an 8 bits register, and will
   overflow without any notice past 0xff.
 */

static volatile uint8_t tim1_ovf_counter;

ISR(TIMER1_OVF_vect)
{
  ++tim1_ovf_counter;
}

static void hfc_start(void)
{
  /* resolution: 1.907349 hz per tick */
  /* fmax: 500 khz */
  /* acquisition time: 0.524288 seconds */

  /* disable interrupts */
  TIMSK1 = 0;
  TIMSK2 = 0;

  /* reset stuff */
  tim1_ovf_counter = 0;
  tim1_cur_counter = 0;
  tim2_is_ovf = 0;

  /* 0x100 overflows make 16 bits */
  tim2_ovf_counter = 0xff;

  /* configure tim2
     normal operation
     prescaler 128
     enable interrupt on overflow
   */
  TCNT2 = 0;
  TIMSK2 = 1 << 0;
  TCCR2A = 0;
  TCCR2B = 0;

  /* configure tim1
     t1 pin (pd5) rising edge as external clock
   */
  DDRD &= ~(1 << 5);
  TCNT1 = 0;
  TIMSK1 = 1 << 0;
  TCCR1A = 0;
  TCCR1B = 0;

  /* start tim1, tim2 */
  TCCR1B = 7 << 0;
  TCCR2B = 5 << 0;
}

static uint8_t hfc_poll(void)
{
  return tim2_is_ovf;
}

static uint32_t hfc_wait(void)
{
  /* busy wait for tim1 to overflow. returns the resulting
     16 bits counter, to be multiplied by the frequency
     resolution (refer to hfc_start) to get the actual
     frequency.
   */

  /* force inline, do not use hfc_poll */
  while (tim2_is_ovf == 0) ;

  return ((uint32_t)tim1_ovf_counter << 16) | (uint32_t)tim1_cur_counter;
}

static inline uint32_t hfc_start_wait(void)
{
  hfc_start();
  return hfc_wait();
}


/* test application. configure timer2 for pwm
   generation and use its output (pb3) as the
   hfc input (pd4).
 */

int main(void)
{
#if 0 /* TIM2_PWM */

  /* tim2 fast pwm mode */

  DDRB |= 1 << 3;
  TIMSK2 = 0;
  TCNT2 = 0;

  /* fpwm = (16 * 10^6) / (OCR2A * 2 * prescal) */
#if 0 /* prescal = 1, fpwm = 41025.64102 */
  OCR2A = 195;
  TCCR2B = (1 << 3) | (1 << 0);
  TCCR2A = (1 << 6) | (3 << 0);
#else /* hires pwm, prescal = 1024 */
  OCR2A = 255; /* 30.63725490196078431372 */
  OCR2A = 240; /* 32.55208333333333333333 */
  OCR2A = 230; /* 33.96739130434782608695 */
  OCR2A = 220; /* 35.51136363636363636363 */
  OCR2A = 210; /* 37.20238095238095238095 */
  OCR2A = 200; /* 39.06250000000000000000 */
  OCR2A = 20; /* 390.6250000000000000000 */
  OCR2A = 10; /* 781.2500000000000000000 */
  OCR2A = 2; /* 3906.25000000000000000000 */
  OCR2A = 1; /* 7812.50000000000000000000 */

  TCCR2B = (1 << 3) | (7 << 0);
  TCCR2A = (1 << 6) | (3 << 0);
#endif

#endif /* TIM2_PWM */

#if CONFIG_UART
  uart_setup();
#endif

  sei();

  while (1)
  {
    uart_write(uint32_to_string(hfc_start_wait()), 8);
    uart_write((uint8_t*)"\r\n", 2);
  }

  return 0;
}
