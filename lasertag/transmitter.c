/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

#include "transmitter.h"
#include "./include/utils.h"
#include "buttons.h"
#include "filter.h"
#include "mio.h"
#include "stdio.h"
#include "switches.h"
#include <stdbool.h>

#define TRANSMITTER_OUTPUT_PIN 13
#define MS_200 20000
#define RESET 0
#define SET_PIN_HIGH 1
#define SET_PIN_LOW 0
#define HALF_CYCLE 2
#define NONCONTINUOUS_TEST_DELAY_TIME 400
#define MAX_PLAYERS 9
#define BTN_MSK_QUIT 0x4

/*static const uint16_t filter_frequencyTickTable[FILTER_FREQUENCY_COUNT] = {
    68, 58, 50, 44, 38, 34, 30, 28, 26, 24};*/

// The transmitter state machine generates a square wave output at the chosen
// frequency as set by transmitter_setFrequencyNumber(). The step counts for the
// frequencies are provided in filter.h

// States

// disabled
// trasmitter high
// trasmitter low
// update frequency

static enum transmitter_st_t {
  disabled_st,
  transmitterHigh_st,
  transmitterLow_st,
  updateFrequency_st
} currentState,
    lastState;

static int16_t frequencyTemp;
static int16_t frequency;
volatile static bool transmitterRunning;
volatile static bool continuousMode;
volatile static bool enabled;
static int16_t cyclesHighOrLow;
static int16_t currentCycleCount;
static int32_t totalCycles;
static bool updateFreq;

// Standard init function.
void transmitter_init() {
  transmitterRunning = false;
  currentState = disabled_st;
  cyclesHighOrLow = filter_frequencyTickTable[RESET];
  enabled = false;
  mio_init(false);
  updateFreq = false;
  mio_setPinAsOutput(TRANSMITTER_OUTPUT_PIN);
}

// Starts the transmitter.
void transmitter_run() {
  enabled = true;
  currentCycleCount = RESET;
  totalCycles = RESET;
  updateFreq = true;
}

// Returns true if the transmitter is still running.
bool transmitter_running() { return enabled; }

// Sets the frequency number. If this function is called while the
// transmitter is running, the frequency will not be updated until the
// transmitter stops and transmitter_run() is called again.
void transmitter_setFrequencyNumber(uint16_t frequencyNumber) {
  if (frequencyNumber > MAX_PLAYERS)
    return;
  updateFreq = true;
  frequencyTemp = frequencyNumber;

  // continuous mode, update freq numb immidiatly, and update cycles high or low
  if (continuousMode) {
    frequency = frequencyNumber;
    cyclesHighOrLow = filter_frequencyTickTable[frequency] / HALF_CYCLE;
    //printf("cycles high / low : %d\n", cyclesHighOrLow);
    updateFreq = false;
  }
}

// Returns the current frequency setting.
uint16_t transmitter_getFrequencyNumber() { return frequency; }

// sets the transmitter pin high or low
void transmitter_setPin(bool pinStateHigh) {
  if (pinStateHigh)
    mio_writePin(TRANSMITTER_OUTPUT_PIN, SET_PIN_HIGH);
  else
    mio_writePin(TRANSMITTER_OUTPUT_PIN, SET_PIN_LOW);
}

// logic for state control and transitions
void transmitter_control() {
  lastState = currentState;
  // control statements
  switch (currentState) {
  case disabled_st:
    // if trigure pushed transition to high state
    if (/*triggerPressed()*/ enabled)
      currentState = transmitterHigh_st;
    break;
  case transmitterHigh_st:
    // if period reached trasition to low state
    if (currentCycleCount >= cyclesHighOrLow)
      currentState = transmitterLow_st;
    break;
  case transmitterLow_st:
    // if 200 ms transition to update frequency
    if (totalCycles >= MS_200 && !continuousMode)
      currentState = updateFrequency_st;
    // if period reached transition to high state
    else if (currentCycleCount >= cyclesHighOrLow)
      currentState = transmitterHigh_st;
    break;
  case updateFrequency_st:
    currentState = disabled_st;
    break;
  }
}

// actions the state machine performs
void transmitter_actions() {
  switch (currentState) {
  case disabled_st:
    // frequency changed
    if (updateFreq) {
      frequency = frequencyTemp;
      cyclesHighOrLow = filter_frequencyTickTable[frequency] / HALF_CYCLE;
      //printf("cycles high / low : %d\n", cyclesHighOrLow);
      updateFreq = false;
    }
    break;
  case transmitterHigh_st:

    // if last state low or disabled, go high
    // set the pin and reset cycles high or low
    if (lastState != transmitterHigh_st) {
      transmitter_setPin(true);
      currentCycleCount = RESET;
    }
    currentCycleCount++;
    totalCycles++;
    break;
  case transmitterLow_st:
    // if last state high, go low
    // set the pin and reset cycles high or low
    if (lastState != transmitterLow_st) {
      transmitter_setPin(false);
      currentCycleCount = RESET;
    }
    currentCycleCount++;
    totalCycles++;
    break;
  case updateFrequency_st:
    /*printf("transmistion complete \n ticks :%d compared to %d\n", totalCycles,
           MS_200);*/
    totalCycles = RESET;
    // printf("\n");
    // update the frequency
    if (updateFreq) {
      frequency = frequencyTemp;
      cyclesHighOrLow = filter_frequencyTickTable[frequency] / HALF_CYCLE;
      updateFreq = false;
    }
    enabled = false;
    break;
  }
}

// Standard tick function.
void transmitter_tick() {
  transmitter_control();
  transmitter_actions();
}

// Tests the transmitter.
void transmitter_runTest() {
  switches_init();
  transmitter_runContinuousTest();
  transmitter_runNoncontinuousTest();
}

// Runs the transmitter continuously.
// if continuousModeFlag == true, transmitter runs continuously, otherwise,
// transmits one pulse-width and stops. To set continuous mode, you must invoke
// this function prior to calling transmitter_run(). If the transmitter is in
// currently in continuous mode, it will stop running if this function is
// invoked with continuousModeFlag == false. It can stop immediately or wait
// until the last 200 ms pulse is complete. NOTE: while running continuously,
// the transmitter will change frequencies at the end of each 200 ms pulse.
void transmitter_setContinuousMode(bool continuousModeFlag) {
  continuousMode = continuousModeFlag;
}

// Tests the transmitter in non-continuous mode.
// The test runs until BTN1 is pressed.
// To perform the test, connect the oscilloscope probe
// to the transmitter and ground probes on the development board
// prior to running this test. You should see about a 300 ms dead
// spot between 200 ms pulses.
// Should change frequency in response to the slide switches.
void transmitter_runNoncontinuousTest() {
  transmitter_setContinuousMode(false);
  while (!(buttons_read() & BTN_MSK_QUIT)) {
    int16_t switches_val = switches_read();
    frequency = switches_val;
    printf("freq: %d\n", frequency);
    transmitter_setFrequencyNumber(frequency);
    transmitter_run();
    while (transmitter_running())
      ;
    printf("finished\n");
    utils_msDelay(NONCONTINUOUS_TEST_DELAY_TIME);
  }
}

// Tests the transmitter in continuous mode.
// To perform the test, connect the oscilloscope probe
// to the transmitter and ground probes on the development board
// prior to running this test.
// Transmitter should continuously generate the proper waveform
// at the transmitter-probe pin and change frequencies
// in response to changes to the changes in the slide switches.
// Test runs until BTN1 is pressed.
void transmitter_runContinuousTest() {
  transmitter_init();
  int16_t switches_val_new;
  int16_t switches_val_old;
  switches_val_new = switches_read();
  transmitter_setFrequencyNumber(switches_val_new);
  switches_val_old = switches_val_new;
  transmitter_setContinuousMode(true);
  transmitter_run();
  // test while loop
  while (!(buttons_read() & BTN_MSK_QUIT)) {
    switches_val_new = switches_read();
    // update the frequency as needed
    if (switches_val_new != switches_val_old) {
      transmitter_setFrequencyNumber(switches_val_new);
      switches_val_old = switches_val_new;
    }
  }
}
