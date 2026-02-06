/*
 * @file timer.h
 * @version 0.0.1
 * PIT (Programmable Interval Timer) for cursor blinking
 */

#ifndef TIMER_H
#define TIMER_H

typedef unsigned char uint8_t;

void timer_init(void);
void timer_handler(void);

extern void irq0_handler(void);

#endif // TIMER_H
