
#include <altera_avalon_timer_regs.h>
#include <io.h>
#include <stdio.h>
#include <system.h>

#include "sys/alt_irq.h"

#define LED_RED_RIGHT 0x03

volatile int* direction = (int*)SWITCH_BASE;
int switch0;
unsigned int tmp;
unsigned int start = 0x03;

void timer_Init() {
    unsigned int period = 0;
    // Stop Timer
    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_0_BASE,
                                     ALTERA_AVALON_TIMER_CONTROL_STOP_MSK);
    // Configure period
    period = 50000000 - 1;
    IOWR_ALTERA_AVALON_TIMER_PERIODL(TIMER_0_BASE, period);
    IOWR_ALTERA_AVALON_TIMER_PERIODH(TIMER_0_BASE, (period >> 16));
    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_0_BASE,
                                     ALTERA_AVALON_TIMER_CONTROL_CONT_MSK |       // Continue counting mode
                                         ALTERA_AVALON_TIMER_CONTROL_ITO_MSK |    // Interrupt enable
                                         ALTERA_AVALON_TIMER_CONTROL_START_MSK);  // Start Timer
}

void timer_IRQ_Handler(void* isr_context) {
    switch0 = *direction & 0x01;
    if (switch0 == 0) {
        tmp = start & 0x200;
        tmp = tmp >> 9;
        start = start << 1;
        start = start | tmp;
        IOWR(LED_BASE, 0, start);
        usleep(500000 / 2);
    } else if (switch0 == 1) {
        tmp = start & 0x001;
        tmp = tmp << 9;
        start = start >> 1;
        start = start | tmp;
        IOWR(LED_BASE, 0, start);
        usleep(500000 / 2);
    }

    // Clear Timer interrupt bit
    IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_0_BASE, ALTERA_AVALON_TIMER_STATUS_TO_MSK);
}

int main() {
    timer_Init();
    alt_ic_isr_register(0, TIMER_0_IRQ, timer_IRQ_Handler, (void*)0, (void*)0);

    return 0;
}
