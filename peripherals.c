#include "peripherals.h"

#ifdef RPI4B
#define PBASE 0xfe000000
#else
#define PBASE 0x20000000
#endif

#ifdef RPI4B
#define TIMER_BASE = 0x7e00b000
#define ARM_TIMER_CTL   (TIMER_BASE+0x408)
#define ARM_TIMER_CNT   (TIMER_BASE+0x420)
#else
#define ARM_TIMER_CTL   (PBASE+0x0000B408)
#define ARM_TIMER_CNT   (PBASE+0x0000B420)
#endif

#define GPFSEL0         (PBASE+0x200000)
#define GPFSEL1         (PBASE+0x200004)
#define GPFSEL3         (PBASE+0x20000C)
#define GPFSEL4         (PBASE+0x200010)
#define GPSET0          (PBASE+0x20001C)
#define GPCLR0          (PBASE+0x200028)
#define GPSET1          (PBASE+0x200020)
#define GPCLR1          (PBASE+0x20002C)

#define AUX_ENABLES     (PBASE+0x215004)
#define AUX_MU_IO_REG   (PBASE+0x215040)
#define AUX_MU_IER_REG  (PBASE+0x215044)
#define AUX_MU_IIR_REG  (PBASE+0x215048)
#define AUX_MU_LCR_REG  (PBASE+0x21504C)
#define AUX_MU_MCR_REG  (PBASE+0x215050)
#define AUX_MU_LSR_REG  (PBASE+0x215054)
#define AUX_MU_MSR_REG  (PBASE+0x215058)
#define AUX_MU_SCRATCH  (PBASE+0x21505C)
#define AUX_MU_CNTL_REG (PBASE+0x215060)
#define AUX_MU_STAT_REG (PBASE+0x215064)
#define AUX_MU_BAUD_REG (PBASE+0x215068)

#define ACT_LED_BIT     (1 << (8))

// "burn" some CPU cycles for nothing - this just creates a delay
// void active_sleep(unsigned int ticks)
// {
//     volatile int ra;
//     for (ra = 0; ra < ticks; ra++)
//         ;
//         // dummy(ra);
// }

unsigned int uart_lcr(void)
{
    return GET32(AUX_MU_LSR_REG);
}

unsigned int uart_recv(void)
{
    while (!(GET32(AUX_MU_LSR_REG) & 0x01))
        ;

    return GET32(AUX_MU_IO_REG) & 0xFF;
}

unsigned int uart_check(void)
{
    if (GET32(AUX_MU_LSR_REG) & 0x01)
        return 1;
    return 0;
}

void uart_send(unsigned int c)
{
    while(!(GET32(AUX_MU_LSR_REG) & 0x20))
        ;

    PUT32(AUX_MU_IO_REG,c);
}

void uart_flush(void)
{
    active_sleep(0x80000);

    uart_send('F');
    uart_send('L');
    uart_send('S');
    uart_send('H');
    uart_send(0x0D);
    uart_send(0x0A);
    while(!(GET32(AUX_MU_LSR_REG)&0x40))
        ;
    uart_send('F');
    uart_send('L');
    uart_send('O');
    uart_send('K');
    uart_send(0x0D);
    uart_send(0x0A);
}

void failstring(unsigned int d)
{
    uart_send('F');
    uart_send('0' + (d / 100) % 10);
    uart_send('0' + (d / 10) % 10);
    uart_send('0' + (d % 10));
    uart_send(0x0D);
    uart_send(0x0A);
}

void okstring(void)
{
    uart_send('X');
    uart_send('F');
    uart_send('O');
    uart_send('K');
    uart_send(0x0D);
    uart_send(0x0A);
}

void uart_init(void)
{
    unsigned int ra;

    PUT32(AUX_ENABLES,1);
    PUT32(AUX_MU_IER_REG,0);
    PUT32(AUX_MU_CNTL_REG,0);
    PUT32(AUX_MU_LCR_REG,3);
    PUT32(AUX_MU_MCR_REG,0);
    PUT32(AUX_MU_IER_REG,0);
    PUT32(AUX_MU_IIR_REG,0xC6);
    #ifdef RPI4B
    PUT32(AUX_MU_BAUD_REG,541);
    #else
    PUT32(AUX_MU_BAUD_REG,270);
    #endif
    ra = GET32(GPFSEL1);
    ra &= ~(7<<12); // gpio14
    ra |= 2<<12;    // alt5
    ra &= ~(7<<15); // gpio15
    ra |= 2<<15;    // alt5
    PUT32(GPFSEL1,ra);
    PUT32(AUX_MU_CNTL_REG,3);
}

// aktivni "spanek" - spali nekolik taktu procesoru naprazdno
void active_sleep(unsigned int ticks)
{
    unsigned int ra;
    for (ra = 0; ra < ticks; ra++)
        dummy(ra);
}

void init_led(void)
{
    unsigned int ra;

    ra = GET32(GPFSEL0);
    ra &= ~(7<<24);
    ra |= 1<<24;
    PUT32(GPFSEL0,ra);

    PUT32(GPSET0, ACT_LED_BIT);
}

void blink(void)
{
    PUT32(GPCLR0, ACT_LED_BIT);
    active_sleep(0x80000);
    PUT32(GPSET0, ACT_LED_BIT);
    active_sleep(0x80000);
    PUT32(GPCLR0, ACT_LED_BIT);
    active_sleep(0x80000);
    PUT32(GPSET0, ACT_LED_BIT);
    active_sleep(0x300000);
}

int blinkstate = 0;

void short_blink(void)
{
    if (blinkstate == 0)
    {
        PUT32(GPCLR0, ACT_LED_BIT);
        blinkstate = 1;
    }
    else
    {
        PUT32(GPSET0, ACT_LED_BIT);
        blinkstate = 0;
    }
}

unsigned int ctonib(unsigned int c)
{
    if (c > '9')
        c -= 7;
    return (c & 0x0F);
}


// this gets called from start.s
void blink8()
{
    unsigned int reg;

    // read current content of function select register
    reg = GET32(GPFSEL0);
    reg &= ~(7 << 24);           // mask out current (3-bit, hence the 7) setting of pin 47 - see the BCM manual for how we came up with the "21"
    reg |= 1 << 24;              // set "1" to the setting, which means "output" (also see the manual)
    PUT32(GPFSEL0, reg);       // write back the value to the function select register

    // we will loop there indefinitely
    while(1)
    {
        // set the output on
        PUT32(GPSET0, 1 << (8));

        // wait for some time
        active_sleep(0x80000);

        // clear the output
        PUT32(GPCLR0, 1 << (8));

        // wait again
        active_sleep(0x80000);

        // .. and repeat
    }
}