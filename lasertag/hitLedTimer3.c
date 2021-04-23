/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

#include "./include/leds.h"
#include "./include/utils.h"
#include "./my_libs/intervalTimer.h"
#include "hitLedTimer.h"
#include "mio.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// The lockoutTimer is active for 1/2 second once it is started.
// It is used to lock-out the detector once a hit has been detected.
// This ensure that only one hit is detected per 1/2-second interval.

#define HIT_LED_TIMER_EXPIRE_VALUE 50000 // Defined in terms of 100 kHz ticks.
#define HIT_LED_TIMER_OUTPUT_PIN 11      // JF-3
#define LED_WRITE_HIGH 1
#define LED_WRITE_LOW 0
#define TEST_DELAY_PERIOD 300
#define INTERVAL_TIMER_2 2

volatile static bool enabled;
volatile static bool startHitDisplay;
volatile static uint32_t hitTimerCount;

// state machine vars
static enum hitLedTimer_st_t {
  disabled_st,
  enabled_st,
  displayHit_st
} currentState,
    lastState;

// Need to init things.
void hitLedTimer_init() {
  enabled = false;
  startHitDisplay = false;
  hitTimerCount = 0;
  currentState = disabled_st;
  // running = false;
  mio_init(false);
  leds_init(false);
  intervalTimer_init(INTERVAL_TIMER_2);
  mio_setPinAsOutput(HIT_LED_TIMER_OUTPUT_PIN);
  hitLedTimer_turnLedOff();
}

// Calling this starts the timer.
void hitLedTimer_start() {
  startHitDisplay = true;
  enabled = true;
  hitTimerCount = 0;
}

// Returns true if the timer is currently running.
// just return currentState == displayHit_st??
// bool hitLedTimer_running() {return ((hitTimerCount <=
// HIT_LED_TIMER_EXPIRE_VALUE) && (hitTimerCount > 0));}
bool hitLedTimer_running() {
  return ((currentState == displayHit_st) || startHitDisplay);
} ///*(hitTimerCount != 0);*/ running; }

// make resetting hitTimerCount a meelay action, or add a seperate running
// variable or something for the state trasition
// https://prod.liveshare.vsengsaas.visualstudio.com/join?116F99EFD9887B4A9B9A5C37BC6D05D408D2
void hitLedTimer_control() {
  // if (lastState != currentState)
  //  printf("%d\n", currentState);
  lastState = currentState;
  switch (currentState) {
  case disabled_st:
    if (enabled)
      currentState = enabled_st;
    break;
  case enabled_st:
    if (startHitDisplay) {
      currentState = displayHit_st;
    } else if (!enabled)
      currentState = disabled_st;
    break;
  case displayHit_st:
    if (hitTimerCount >= HIT_LED_TIMER_EXPIRE_VALUE) {
      if (enabled)
        currentState = enabled_st;
      else
        currentState = disabled_st;
    }
    break;
  }
}

void hitLedTimer_actions() {
  switch (currentState) {
  case disabled_st:
    if (lastState != disabled_st) {
      hitLedTimer_turnLedOff();
      hitTimerCount = 0;
    }
    break;
  case enabled_st:
    // run only as melay action
    // turn LEDS off, reset hit count
    if (lastState == displayHit_st) {
      hitLedTimer_turnLedOff();
      hitTimerCount = 0;
    }

    break;
  case displayHit_st:
    // to check later if debugging -> increment counter before or after checking
    if (lastState == enabled_st) {
      hitLedTimer_turnLedOn();
      startHitDisplay = false;
    }
    hitTimerCount += 1;
    break;
  }
}

// Standard tick function.
void hitLedTimer_tick() {
  hitLedTimer_control();
  hitLedTimer_actions();
}

// Turns the gun's hit-LED on.
void hitLedTimer_turnLedOn() {
  mio_writePin(HIT_LED_TIMER_OUTPUT_PIN, LED_WRITE_HIGH);
  leds_write(LED_WRITE_HIGH);
  // printf("on\n");
}

// Turns the gun's hit-LED off.
void hitLedTimer_turnLedOff() {
  mio_writePin(HIT_LED_TIMER_OUTPUT_PIN, LED_WRITE_LOW);
  leds_write(LED_WRITE_LOW);
  // printf("off\n");
}

// Disables the hitLedTimer.
void hitLedTimer_disable() { enabled = false; }

// Enables the hitLedTimer.
void hitLedTimer_enable() { enabled = true; }

bool hitLedTimer_isEnabled() { return enabled; }

// Runs a visual test of the hit LED.
// The test continuously blinks the hit-led on and off.
void hitLedTimer_runTest() {
  // printf("running LED test\n");
  // leds_runTest();
  printf("enter HitLedTimer run test\n");
  // hitLedTimer_init();
  printf("enabling timer\n");
  hitLedTimer_enable();
  printf("Entering eternal while loop\n");
  while (true) {
    printf("1\n");
    // hitLedTimer_init();
    hitLedTimer_start();
    // printf("wait while timer runs\n");
    // intervalTimer_start(INTERVAL_TIMER_2);
    while (hitLedTimer_running()) {
    }
    // intervalTimer_stop(INTERVAL_TIMER_2);
    // printf("hit timer actually ran for %f, wait 300 ms delay\n",
    // intervalTimer_getTotalDurationInSeconds(INTERVAL_TIMER_2));
    // intervalTimer_reset(INTERVAL_TIMER_2);
    // intervalTimer_start(INTERVAL_TIMER_2);
    utils_msDelay(TEST_DELAY_PERIOD);
    // intervalTimer_stop(INTERVAL_TIMER_2);
    // printf("ms delay actually ran for %f\n\n",
    // intervalTimer_getTotalDurationInSeconds(INTERVAL_TIMER_2));
  }
}
