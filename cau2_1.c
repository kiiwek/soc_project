// cập nhật thời gian chính xác hơn

#include <io.h>
#include <stdio.h>
#include <sys/alt_irq.h>

#include "altera_avalon_pio_regs.h"
#include "altera_avalon_timer_regs.h"
#include "system.h"

#define LED_BASE 0x11020
#define SWITCH_BASE 0x11030

unsigned int counter_sec = 0;
unsigned int counter_min = 0;
unsigned int counter_hour = 0;
static unsigned char segcode[] = {
    0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8,
    0x80, 0x90, 0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E};
int *sec0;
int *sec1;
int *min0;
int *min1;
int *hour0;
int *hour1;
int *sw;

int switch_state = 0;  // 0: đồng hồ tắt, 1: đồng hồ chạy

void display_7_segment() {
    *sec0 = segcode[counter_sec % 10];
    *sec1 = segcode[counter_sec / 10];
    *min0 = segcode[counter_min % 10];
    *min1 = segcode[counter_min / 10];
    *hour0 = segcode[counter_hour % 10];
    *hour1 = segcode[counter_hour / 10];
}

void timer_Init() {
    unsigned int period = 0;
    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_0_BASE,
                                     ALTERA_AVALON_TIMER_CONTROL_STOP_MSK);
    period = (50000000 - 1) / 5;
    IOWR_ALTERA_AVALON_TIMER_PERIODL(TIMER_0_BASE, period);
    IOWR_ALTERA_AVALON_TIMER_PERIODH(TIMER_0_BASE, (period >> 16));
    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_0_BASE,
                                     ALTERA_AVALON_TIMER_CONTROL_CONT_MSK |
                                         ALTERA_AVALON_TIMER_CONTROL_ITO_MSK |
                                         ALTERA_AVALON_TIMER_CONTROL_START_MSK);
}

void Timer_IRQ_Handler(void *isr_context) {
    if (switch_state == 1) {
        counter_sec++;
        if (counter_sec == 60) {
            counter_min++;
            counter_sec = 0;
            if (counter_min == 60) {
                counter_hour++;
                counter_min = 0;
                if (counter_hour == 24) {
                    counter_hour = 0;
                }
            }
        }
        display_7_segment();
        printf("System time: %d : %d : %d \n", counter_hour, counter_min, counter_sec);
    }

    // Clear Timer interrupt bit
    IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_0_BASE, ALTERA_AVALON_TIMER_STATUS_TO_MSK);
}

int main() {
    sec0 = (int *)HEX0_BASE;
    sec1 = (int *)HEX1_BASE;
    min0 = (int *)HEX2_BASE;
    min1 = (int *)HEX3_BASE;
    hour0 = (int *)HEX4_BASE;
    hour1 = (int *)HEX5_BASE;
    sw = (int *)SWITCH_BASE;

    timer_Init();
    alt_ic_isr_register(TIMER_0_IRQ_INTERRUPT_CONTROLLER_ID, TIMER_0_IRQ, Timer_IRQ_Handler, NULL, NULL);

    while (1) {
        switch_state = IORD_ALTERA_AVALON_PIO_DATA(SWITCH_BASE);
        if (switch_state == 1) {
            alt_ic_irq_enable(TIMER_0_IRQ_INTERRUPT_CONTROLLER_ID, TIMER_0_IRQ);
        } else {
            alt_ic_irq_disable(TIMER_0_IRQ_INTERRUPT_CONTROLLER_ID, TIMER_0_IRQ);
        }
    }

    return 0;
}
