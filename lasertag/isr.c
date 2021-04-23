/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

#include "isr.h"
#include "buttons.h"
#include "filter.h"
#include "hitLedTimer.h"
#include "include/interrupts.h"
#include "include/mio.h"
#include "lockoutTimer.h"
#include "switches.h"
#include "transmitter.h"
#include "trigger.h"
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define RESET 0
#define DECREMENT 1
#define INCREMENT 1
#define MAX_COUNT 1000

typedef uint32_t
    isr_AdcValue_t; // Used to represent ADC values in the ADC buffer.

struct adcQueue *adc;

///////// adc queue code ///////////////
struct adcQueue {
  uint32_t first, last, size, capacity;
  isr_AdcValue_t *array;
};

// This returns the number of values in the ADC buffer.
uint32_t isr_adcBufferElementCount() { return adc->size; }

// function to create a queue
// of given capacity.
// It initializes size of queue as 0
void createQueue(uint32_t capacity) {
  adc = (struct adcQueue *)malloc(sizeof(struct adcQueue));
  adc->capacity = capacity;
  adc->first = adc->size = RESET;

  // This is important, see the enqueue
  adc->last = capacity - DECREMENT;
  adc->array = (isr_AdcValue_t *)malloc(adc->capacity * sizeof(int));
}

// Queue is full when size becomes
// equal to the capacity
bool adcFull() { return (adc->size == adc->capacity); }

// Queue is empty when size is 0
bool adcEmpty() { return (adc->size == RESET); }

// This adds data to the ADC queue. Data are removed from this queue and used by
// the detector.
void isr_addDataToAdcBuffer(isr_AdcValue_t item) {
  if (adcFull(adc))
    return;
  adc->last = (adc->last + INCREMENT) % adc->capacity;
  adc->array[adc->last] = item;
  adc->size = adc->size + INCREMENT;
}

// This removes a value from the ADC buffer.
isr_AdcValue_t isr_removeDataFromAdcBuffer() {
  if (adcEmpty(adc))
    return INT_MIN;
  isr_AdcValue_t item = adc->array[adc->first];
  adc->first = (adc->first + INCREMENT) % adc->capacity;
  adc->size = adc->size - DECREMENT;
  return item;
}

// init the adc buffer
void adcBuffer_init() { createQueue(MAX_COUNT); }

////////// isr code ////////

// Performs inits for anything in isr.c
void isr_init() {
  transmitter_init();
  hitLedTimer_init();
  trigger_init();
  lockoutTimer_init();
  switches_init();
  buttons_init();
  mio_init(false);
  adcBuffer_init();
}

// This function is invoked by the timer interrupt at 100 kHz.
void isr_function() {
  // SM ticks
  transmitter_tick();
  hitLedTimer_tick();
  trigger_tick();
  lockoutTimer_tick();
  // adc functions
  isr_addDataToAdcBuffer(interrupts_getAdcData());
}

// test the que in the isr to make sure its functioning correctly
void isr_queueTest() {
  isr_init();
  if (adcEmpty())
    printf("empty as it should be\n");
  else
    printf("why is it full\n");
  isr_addDataToAdcBuffer(10);
  isr_addDataToAdcBuffer(15);
  isr_addDataToAdcBuffer(100);
  isr_addDataToAdcBuffer(101);

  uint32_t first = isr_removeDataFromAdcBuffer();
  uint32_t second = isr_removeDataFromAdcBuffer();
  uint32_t thrird = isr_removeDataFromAdcBuffer();
  uint32_t fourth = isr_removeDataFromAdcBuffer();
  if (first == 10 && second == 15 && thrird == 100 && fourth == 101)
    printf("sucess with the queue input and output\n");
  else
    printf("something failed in the queue\n");

  if (adcEmpty())
    printf("empty as it should be\n");
  else
    printf("why is it full\n");
}