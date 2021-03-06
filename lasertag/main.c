/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

// Uncomment the line below to run a comprehensive test of the filters.
//#define FILTER_TEST_RUN // Just run the filter test.

// Uncomment the line below to run the game in Zombie mode..
//#define ZOMBIE_MODE   // Enable zombie mode with the main for laser tag.

#define TEST_MAIN // Used for general testing.

// Leave uncommented to run the queue test.
// #define QUEUE_TEST_RUN

// Leave uncommented to run the sound test.
// #define SOUND_TEST_RUN

// Leave uncommented to simply dump raw ADC values to the console.
// #define JUST_DUMP_RAW_ADC_VALUES

// Leave uncommented to run two-player mode.
// #define RUNNING_MODES_TWO_TEAMS

// Leave uncommented to run continuous/shooter mode.
 #define DEFAULT_RUNNING_MODE

// The following line enables the main() contained in laserTagMain.c
// Leave this line uncommented unless you want to run some other special test
// main().
#define LASER_TAG_MAIN
//#define M3T3

#ifdef LASER_TAG_MAIN

#include "./include/utils.h"
#include "detector.h"
#include "display.h"
#include "filter.h"
#include "filterTest.h"
#include "gameModes.h"
#include "hitLedTimer.h"
#include "interrupts.h"
#include "isr.h"
#include "leds.h"
#include "lockoutTimer.h"
#include "my_libs/buttons.h"
#include "runningModes.h"
#include "sound.h"
#include "transmitter.h"
#include "trigger.h"
#include <assert.h>
#include <stdio.h>

#define TEST_BREAK_VALUE 1000

// The program comes up in continuous mode.
// Hold BTN2 while the program starts to come up in shooter mode.
int main() {
  #ifdef M3T2
  // isr invocation
  isr_init();

  leds_init(true);
  interrupts_initAll(true);           // Init the interrupt code.
  interrupts_enableTimerGlobalInts(); // Enable interrupts at the timer.
  interrupts_startArmPrivateTimer();  // Start the private ARM timer running.
  interrupts_enableArmInts();
  // inits

  // runtests
  printf("trigger test\n");
  trigger_runTest();
  utils_msDelay(TEST_BREAK_VALUE);
  printf("hit led timer test\n");
  hitLedTimer_runTest();
  utils_msDelay(TEST_BREAK_VALUE);
  printf("lockout timer test\n");
  lockoutTimer_runTest();
  utils_msDelay(TEST_BREAK_VALUE);
  printf("continuous transmitter test\n");
  transmitter_runContinuousTest();
  utils_msDelay(TEST_BREAK_VALUE);
  printf("noncontinuous transmitter test\n");
  transmitter_init();
  transmitter_runNoncontinuousTest();
  printf("done with tests");
  #endif

  #ifdef M3T3
  detector_testAdcScaling();
  detector_runTest();
  isr_queueTest();
  #endif

// Runs a comprehensive filter test if defined.
#ifdef QUEUE_TEST_RUN
  queue_runTest();
#endif

#ifdef FILTER_TEST_RUN
  // display_init();
  filterTest_runTest();

#endif

#ifdef SOUND_TEST_RUN
  sound_runTest();
#endif

#ifdef RUNNING_MODES_TWO_TEAMS
  gameModes_twoTeams();
#endif

#ifdef DEFAULT_RUNNING_MODE
  // btn2 is pressed, which enables shooter mode.
  buttons_init(); // Init the buttons.
  if (buttons_read() &
      BUTTONS_BTN2_MASK) { // Read the buttons to see if BTN2 is drepressed.
    printf("Starting shooter mode\n\r");
    runningModes_shooter(); // Run shooter mode if BTN2 is depressed.
  } else {
    printf("Starting continuous mode\n\r");
    runningModes_continuous(); // Otherwise, go to continuous mode.
  }
#endif
}
#endif // LASER_TAG_MAIN
