#include "hitLedTimer.h"
#include "./include/leds.h"
#include "./include/utils.h"
#include "./my_libs/intervalTimer.h"
#include "buttons.h"
#include "mio.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define HIT_LED_TIMER_EXPIRE_VALUE 50000 // Defined in terms of 100 kHz ticks.
#define HIT_LED_TIMER_OUTPUT_PIN 11      // JF-3
#define LED_WRITE_HIGH 1
#define LED_WRITE_LOW 0
#define TEST_DELAY_PERIOD 300
#define INTERVAL_TIMER_2 2
#define RESET 0
#define LOW "L\n"
#define HIGH "H\n"
#define ON "on\n"
#define OFF "of\n"
#define BTN_MSK_QUIT 0x4

volatile static bool enabled;
volatile static uint32_t hitTimerCount;
volatile static bool running;

volatile static enum hitLedTimer_st_t {
  low_st,
  high_st
} currentState,
    lastState;

void hitLedTimer_start() { running = true; }

// Turns the gun's hit-LED on.
void hitLedTimer_turnLedOn() {
  mio_writePin(HIT_LED_TIMER_OUTPUT_PIN, LED_WRITE_HIGH);
  leds_write(LED_WRITE_HIGH);
  //printf(ON);
}

// Turns the gun's hit-LED off.
void hitLedTimer_turnLedOff() {
  mio_writePin(HIT_LED_TIMER_OUTPUT_PIN, LED_WRITE_LOW);
  leds_write(LED_WRITE_LOW);
  //printf(OFF);
}

// Need to init things.
void hitLedTimer_init() {
  enabled = false;
  running = false;
  hitTimerCount = RESET;
  currentState = low_st;
  lastState = low_st;

  mio_init(false);
  leds_init(false);
  intervalTimer_init(INTERVAL_TIMER_2);
  mio_setPinAsOutput(HIT_LED_TIMER_OUTPUT_PIN);
  hitLedTimer_turnLedOff();
}

// returns if the sm is running aka the led is on
bool hitLedTimer_running() { return running; }

// Enables the hitLedTimer.
void hitLedTimer_enable() { enabled = true; }

// returns if the SM is enabled
bool hitLedTimer_isEnabled() { return enabled; }

// controls logic for the sm
void hitLedTimer_control() {
  lastState = currentState;
  // control logic
  switch (currentState) {
  case low_st:
    if (running && enabled)
      currentState = high_st;
    break;
  case high_st:
    if (hitTimerCount >= HIT_LED_TIMER_EXPIRE_VALUE)
      currentState = low_st;
    break;
  }
}

// controls the SM actions both meeley and moore
void hitLedTimer_actions() {
  // SM actions
  switch (currentState) {
  case low_st:
    // printf(LOW);
    // run only as melay action
    // turn LEDS off, reset hit count
    if (lastState == high_st) {
      hitLedTimer_turnLedOff();
      running = false;
    }

    break;
  case high_st:
    // printf(HIGH);
    // to check later if debugging -> increment counter before or after checking
    if (lastState == low_st) {
      hitLedTimer_turnLedOn();
      hitTimerCount = RESET;
    }
    hitTimerCount++;
    break;
  }
}

// SM tick statement
void hitLedTimer_tick() {
  hitLedTimer_control();
  hitLedTimer_actions();
}

// test to make sure its working correctly
void hitLedTimer_runTest() {
  // printf("running LED test\n");
  // leds_runTest();
  printf("enter HitLedTimer run test\n");
  // hitLedTimer_init();
  printf("enabling timer\n");
  hitLedTimer_enable();
  printf("Entering eternal while loop\n");
  // infinite test loop
  while (!(buttons_read() & BTN_MSK_QUIT)) {
    // printf("S\n");
    hitLedTimer_start();
    // printf("F\n");
    // run the led on
    while (hitLedTimer_running()) {
    }
    // printf("D\n");
    utils_msDelay(TEST_DELAY_PERIOD);
    // utils_msDelay(600 /*TEST_DELAY_PERIOD*/);
    // printf("F\n");
  }
}