#include "./include/leds.h"
#include "./include/utils.h"
#include "./my_libs/intervalTimer.h"
#include "hitLedTimer.h"
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

volatile static bool enabled;
// volatile static bool startHitDisplay;
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
  printf("on\n");
}

// Turns the gun's hit-LED off.
void hitLedTimer_turnLedOff() {
  mio_writePin(HIT_LED_TIMER_OUTPUT_PIN, LED_WRITE_LOW);
  leds_write(LED_WRITE_LOW);
  printf("off\n");
}

// Need to init things.
void hitLedTimer_init() {
  enabled = false;
  running = false;
  hitTimerCount = 0;
  currentState = low_st;
  lastState = low_st;

  mio_init(false);
  leds_init(false);
  intervalTimer_init(INTERVAL_TIMER_2);
  mio_setPinAsOutput(HIT_LED_TIMER_OUTPUT_PIN);
  hitLedTimer_turnLedOff();
}

bool hitLedTimer_running() { return running; }

// Enables the hitLedTimer.
void hitLedTimer_enable() { enabled = true; }

bool hitLedTimer_isEnabled() { return enabled; }

void hitLedTimer_control() {
  // if (lastState != currentState)
  //  printf("%d\n", currentState);
  lastState = currentState;
  switch (currentState) {
  case low_st:
    if (running && enabled) {
      currentState = high_st;
    }
    break;
  case high_st:
    if (hitTimerCount >= HIT_LED_TIMER_EXPIRE_VALUE) {
      currentState = low_st;
    }
    break;
  }
}

void hitLedTimer_actions() {
  switch (currentState) {
  case low_st:
    printf("L\n");
    // run only as melay action
    // turn LEDS off, reset hit count
    if (lastState == high_st) {
      hitLedTimer_turnLedOff();
      running = false;
    }

    break;
  case high_st:
    prinf("H\n");
    // to check later if debugging -> increment counter before or after checking
    if (lastState == low_st) {
      hitLedTimer_turnLedOn();
      hitTimerCount = 0;
    }
    hitTimerCount += 1;
    break;
  }
}

void hitLedTimer_tick() {
  hitLedTimer_control();
  hitLedTimer_actions();
}

void hitLedTimer_runTest() {
  // printf("running LED test\n");
  // leds_runTest();
  printf("enter HitLedTimer run test\n");
  // hitLedTimer_init();
  printf("enabling timer\n");
  hitLedTimer_enable();
  printf("Entering eternal while loop\n");
  while (true) {
    while (true) {
      // printf("1\n");
      // hitLedTimer_init();
      printf("S\n");
      hitLedTimer_start();
      printf("F\n");
      // printf("wait while timer runs\n");
      // intervalTimer_start(INTERVAL_TIMER_2);
      while (hitLedTimer_running()) {
      }
      // intervalTimer_stop(INTERVAL_TIMER_2);
      // printf("hit timer actually ran for %f, wait 300 ms delay\n",
      // intervalTimer_getTotalDurationInSeconds(INTERVAL_TIMER_2));
      // intervalTimer_reset(INTERVAL_TIMER_2);
      // intervalTimer_start(INTERVAL_TIMER_2);
      // printf("2\n");
      printf("D\n");
      utils_msDelay(TEST_DELAY_PERIOD);
      printf("F\n");
      // intervalTimer_stop(INTERVAL_TIMER_2);
      // printf("ms delay actually ran for %f\n\n",
      // intervalTimer_getTotalDurationInSeconds(INTERVAL_TIMER_2));
    }
  }
}