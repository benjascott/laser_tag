/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

#include "trigger.h"
#include "./include/utils.h"
#include "buttons.h"
#include "mio.h"
#include "my_libs/buttons.h"
#include "switches.h"
#include "transmitter.h"
#include <stdbool.h>
#include <stdio.h>

#define TRIGGER_MIO_OUTPUT_PIN 10
#define BUTTONS_BTN0_MASK 0x1
#define PRESSED "D\n"
#define RELEASED "U\n"
#define TRANSMITTER_TEST_STATEMENT "run transmitter\n"

#define DEBOUNCE_TIME 5000
#define RESET 0
#define BTN_MSK_QUIT 0x4

volatile static trigger_shotsRemaining_t shotsRemaining;
volatile static bool enabled;
static uint16_t debounceTicks;
static bool lastPrintUp;

static enum trigger_st_t {
  disabled_st,
  debounceHigh_st, // waiting for debounce time
  triggerHigh_st,  //
  debounceLow_st,  // waiting for debounce time
  triggerLow_st,
  pressDetected_st
} currentState,
    lastState, lastDebounceState, currentDebounceState;

// The trigger state machine debounces both the press and release of gun
// trigger. Ultimately, it will activate the transmitter when a debounced press
// is detected.

typedef uint16_t trigger_shotsRemaining_t;

// Init trigger data-structures.
// Determines whether the trigger switch of the gun is connected (see discussion
// in lab web pages). Initializes the mio subsystem.
void trigger_init() {
  mio_init(false);
  lastPrintUp = false;
  currentState = disabled_st;
  enabled = false;
  buttons_init();
  mio_setPinAsInput(TRIGGER_MIO_OUTPUT_PIN);
}

// Enable the trigger state machine. The trigger state-machine is inactive until
// this function is called. This allows you to ignore the trigger when helpful
// (mostly useful for testing).
void trigger_enable() { enabled = true; }

// Disable the trigger state machine so that trigger presses are ignored.
void trigger_disable() { enabled = false; }

// Returns the number of remaining shots.
trigger_shotsRemaining_t trigger_getRemainingShotCount() {
  return shotsRemaining;
}

// Sets the number of remaining shots.
void trigger_setRemainingShotCount(trigger_shotsRemaining_t count) {
  shotsRemaining = count;
}

// returns read trigger state
bool trigger_readTrigger() {
  return ((mio_readPin(TRIGGER_MIO_OUTPUT_PIN)) ||
          ((buttons_read() & BUTTONS_BTN0_MASK)));
}

// logic for state control and transitions
void trigger_control() {
  lastState = currentState;
  // control statements
  switch (currentState) {
  case disabled_st:
    if (enabled)
      currentState = triggerLow_st;
    break;
  case triggerLow_st:
    // if you have a non press detected when the last debounced posistion was a
    // press then the trigger was relaced and a press detected
    if (lastDebounceState == triggerHigh_st &&
        currentDebounceState == triggerLow_st)
      currentState = pressDetected_st;
    else if (trigger_readTrigger())
      currentState = triggerHigh_st;
    break;
  case triggerHigh_st:
    if (!trigger_readTrigger())
      currentState = triggerLow_st;
    break;
  case pressDetected_st:
    if (!enabled)
      currentState = disabled_st;
    else {
      currentState = triggerLow_st;
      // reset debounce states
      lastDebounceState = pressDetected_st;
      currentDebounceState = pressDetected_st;
    }
    break;
  }
}

// logic for state actions
void trigger_actions() {
  // actions for each state
  switch (currentState) {
  case disabled_st:
    // reset debounce states
    lastDebounceState = disabled_st;
    currentDebounceState = disabled_st;
    // begin reading
    if (enabled && (shotsRemaining > RESET))
      currentState = triggerLow_st;
    break;
  case triggerLow_st:
    // melay action
    if (lastState != triggerLow_st)
      debounceTicks = RESET;
    // updates debounce states
    else if (debounceTicks == DEBOUNCE_TIME) {
      lastDebounceState = currentDebounceState;
      currentDebounceState = triggerLow_st;
      debounceTicks = RESET;
      // set button to released
      if (lastPrintUp) {
        //printf(RELEASED);
        lastPrintUp = false;
      }
    }
    debounceTicks++;
    break;
  case triggerHigh_st:
    // melay action
    if (lastState != triggerHigh_st)
      debounceTicks = RESET;
    // update debounce states
    else if (debounceTicks == DEBOUNCE_TIME) {
      lastDebounceState = currentDebounceState;
      currentDebounceState = triggerHigh_st;
      debounceTicks = RESET;
      // update trigger pressed
      if (!lastPrintUp) {
        //printf(PRESSED);
        lastPrintUp = true;
      }
    }
    debounceTicks++;
    break;
  case pressDetected_st:
    // activate the trasmitter state machine
    transmitter_setFrequencyNumber(switches_read());
    transmitter_run();
    //printf(TRANSMITTER_TEST_STATEMENT);
    shotsRemaining--;
    break;
  }
}

// Standard tick function.
void trigger_tick() {
  trigger_control();
  trigger_actions();
}

// Runs the test continuously until BTN1 is pressed.
// The test just prints out a 'D' when the trigger or BTN0
// is pressed, and a 'U' when the trigger or BTN0 is released.
void trigger_runTest() {
  trigger_enable();
  // infinite loop for test
  while (!(buttons_read() & BTN_MSK_QUIT))
    ;
}
