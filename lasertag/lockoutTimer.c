/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

#include "lockoutTimer.h"
#include "./include/utils.h"
#include "./my_libs/intervalTimer.h"
#include "buttons.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define LOCKOUT_TIMER_EXPIRE_VALUE 50000 // Defined in terms of 100 kHz ticks.
#define INTERVAL_TIMER_2 2
#define MS_TEST_TIME 1
#define RESET 0
#define START_TIMER 1
#define BTN_MSK_QUIT 0x4

volatile static bool startTimer;
static uint32_t timerCount;

// state machine vars
static enum lockout_st_t { startTimer_st, runTimer_st } currentState;

// Calling this starts the timer.
void lockoutTimer_start() {
  startTimer = true;
  timerCount = START_TIMER;
}

// Perform any necessary inits for the lockout timer.
void lockoutTimer_init() {
  startTimer = false;
  timerCount = RESET;
  currentState = startTimer_st;
  intervalTimer_init(INTERVAL_TIMER_2);
}

// Returns true if the timer is running.
bool lockoutTimer_running() { return (startTimer); }

// state machine flow controls
void lockoutTimer_control() {
  // state machine flow controls
  switch (currentState) {
  case startTimer_st:
    if (startTimer)
      currentState = runTimer_st;
    break;
  case runTimer_st:
    if (!startTimer)
      currentState = startTimer_st;
    break;
  }
}

// state machine actions
void lockoutTimer_actions() {
  // state machine actions
  switch (currentState) {
  case startTimer_st:
    // no action
    break;
  case runTimer_st:
    timerCount++;
    if (timerCount >= LOCKOUT_TIMER_EXPIRE_VALUE)
      lockoutTimer_init();
    break;
  }
}

// Standard tick function.
void lockoutTimer_tick() {
  lockoutTimer_control();
  lockoutTimer_actions();
}

// Test function assumes interrupts have been completely enabled and
// lockoutTimer_tick() function is invoked by isr_function().
// Prints out pass/fail status and other info to console.
// Returns true if passes, false otherwise.
// This test uses the interval timer to determine correct delay for
// the interval timer.
bool lockoutTimer_runTest() {
  printf("Starting Lockout Timer test\n");
  // lockoutTimer_init();
  // start interval timer
  printf("Starting lockout timer now\n");
  intervalTimer_start(INTERVAL_TIMER_2);
  lockoutTimer_start();
  // loop through test
  while (lockoutTimer_running()) {
  }
  // stop interval timer
  intervalTimer_stop(INTERVAL_TIMER_2);
  printf("Timer ended\n");
  double duration = intervalTimer_getTotalDurationInSeconds(INTERVAL_TIMER_2);
  printf("Time: %f\n", duration);
  return true;
}
