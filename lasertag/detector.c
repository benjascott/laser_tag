
/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

#include "detector.h"
#include "buttons.h"
#include "filter.h"
#include "hitLedTimer.h"
#include "include/interrupts.h"
#include "isr.h"
#include "lockoutTimer.h"
#include "queue.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define NUMBER_OF_PLAYERS 10
#define INIT_FLOAT 0.0
#define INIT_INT 0
#define ADC_DIVISOR_VAL 2047.5
#define ADC_SCALED_VAL_SHIFTER 1.0
#define BIGGEST_HIT_PLAYER 9
#define PLAYER_5 4
#define FALSE_INIT false
#define RESET 0
#define NEXT_ITEM 1
#define LAST_ITEM 1

volatile static bool detector_hitDetectedFlag = false;
volatile static bool detector_ignoreAllHitsFlag = false;
static bool fromScratch = true;
static uint32_t sampleCount = 0;
static uint32_t fudgeFactor = 400;
static uint32_t elementCount = 0;
static uint32_t rawAdcValue = 0;
static double scaledAdcValue = 0;
static uint32_t filter_addedInputCount = 0;
static uint32_t lastHitFrequency = 0;
static double detector_threshold = 0;
static uint32_t maxPowerFreqNo = -2;

// hitPowerVals
static double detector_powerVals[NUMBER_OF_PLAYERS] = {
    INIT_FLOAT, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT,
    INIT_FLOAT, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT};

// number of hits for each player array
static uint16_t detector_hitArray[NUMBER_OF_PLAYERS] = {
    INIT_INT, INIT_INT, INIT_INT, INIT_INT, INIT_INT,
    INIT_INT, INIT_INT, INIT_INT, INIT_INT, INIT_INT};

// ignored frequency array
static bool detector_ignoreFrequencies[NUMBER_OF_PLAYERS] = {
    FALSE_INIT, FALSE_INIT, FALSE_INIT, FALSE_INIT, FALSE_INIT,
    FALSE_INIT, FALSE_INIT, FALSE_INIT, FALSE_INIT, FALSE_INIT};

// Always have to init things.
// bool array is indexed by frequency number, array location set for true to
// ignore, false otherwise. This way you can ignore multiple frequencies.
void detector_init(bool ignoredFrequencies[]) {
  filter_init();
  fromScratch = true;
  // needs to initialize data structures here as well
  for (uint32_t i = RESET; i < NUMBER_OF_PLAYERS; i++) {
    detector_ignoreFrequencies[i] = ignoredFrequencies[i];
  }
}

// detect if there is a hit
bool detector_runHitDetection() {
  // sorted power values array
  double detector_sortedPowerVals[NUMBER_OF_PLAYERS] = {
      INIT_FLOAT, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT,
      INIT_FLOAT, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT};
  // run sort on power values
  detector_sort(&lastHitFrequency, detector_powerVals,
                detector_sortedPowerVals);
  /**/
  // comparge against a scaled version of the median power
  detector_threshold = detector_sortedPowerVals[PLAYER_5] * fudgeFactor;
  // if the strongest power level is more than the threshold, hit detected
  if (detector_sortedPowerVals[BIGGEST_HIT_PLAYER] >= detector_threshold) {
    return true;
  }
  return false;
}

// Runs the entire detector: decimating fir-filter, iir-filters,
// power-computation, hit-detection. if interruptsNotEnabled = true, interrupts
// are not running. If interruptsNotEnabled = true you can pop values from the
// ADC queue without disabling interrupts. If interruptsNotEnabled = false, do
// the following:
// 1. disable interrupts.
// 2. pop the value from the ADC queue.
// 3. re-enable interrupts if interruptsNotEnabled was true.
// if ignoreSelf == true, ignore hits that are detected on your frequency.
// Your frequency is simply the frequency indicated by the slide switches
void detector(bool interruptsCurrentlyEnabled) {
  elementCount = isr_adcBufferElementCount();
  // run for all the values in the adc
  for (uint32_t i = RESET; i < elementCount; i++) {
    // temporarily stop interupts so you can run the filter code
    if (interruptsCurrentlyEnabled) {
      interrupts_disableArmInts();
      rawAdcValue = isr_removeDataFromAdcBuffer();
      interrupts_enableArmInts();
    }
    // just get the value
    else {
      rawAdcValue = isr_removeDataFromAdcBuffer();
    }
    scaledAdcValue = detector_getScaledAdcValue(rawAdcValue);
    filter_addNewInput(scaledAdcValue);
    filter_addedInputCount++;

    // every 10 inputs, run the filters and compute power
    if (filter_addedInputCount == filter_getDecimationValue()) {
      filter_firFilter();
      // filter for each of the filters
      for (uint32_t filterNumber = RESET; filterNumber < NUMBER_OF_PLAYERS;
           filterNumber++) {
        filter_iirFilter(filterNumber);
        detector_powerVals[filterNumber] =
            filter_computePower(filterNumber, fromScratch, false);
        fromScratch = false;
      }
      // only detect a hit if the lockout hit timer is not running
      if (!lockoutTimer_running()) {
        // response to detecting a hit
        // detector_hitDetected() runs the detector_runHitDetection()
        if (detector_runHitDetection() &&
            (!detector_ignoreFrequencies[lastHitFrequency])) {
          lockoutTimer_start();
          hitLedTimer_start();
          detector_hitArray[lastHitFrequency]++;
          detector_hitDetectedFlag = true;
        }
      }
      // reset to zero again after running filters
      filter_addedInputCount = RESET;
    }
  }
}

// Returns true if a hit was detected.
bool detector_hitDetected() { return detector_hitDetectedFlag; }

// Returns the frequency number that caused the hit.
uint16_t detector_getFrequencyNumberOfLastHit() { return lastHitFrequency; }

// Clear the detected hit once you have accounted for it.
void detector_clearHit() { detector_hitDetectedFlag = false; }

// Ignore all hits. Used to provide some limited invincibility in some game
// modes. The detector will ignore all hits if the flag is true, otherwise will
// respond to hits normally.
///////////////// CHECK ME /////////////////////
void detector_ignoreAllHits(bool flagValue) {
  // not ignoring all hits
  if (!flagValue) {
    detector_runHitDetection();
  } else { // ignoring all hits
    detector_hitDetectedFlag = false;
  }
}

// Get the current hit counts.
// Copy the current hit counts into the user-provided hitArray
// using a for-loop.
void detector_getHitCounts(detector_hitCount_t hitArray[]) {
  for (uint32_t player = RESET; player < NUMBER_OF_PLAYERS; player++)
    hitArray[player] = detector_hitArray[player];
}

// Allows the fudge-factor index to be set externally from the detector.
// The actual values for fudge-factors is stored in an array found in detector.c
void detector_setFudgeFactorIndex(uint32_t newFactor) {
  fudgeFactor = newFactor;
}

// This function sorts the inputs in the unsortedArray and
// copies the sorted results into the sortedArray. It also
// finds the maximum power value and assigns the frequency
// number for that value to the maxPowerFreqNo argument.
// This function also ignores a single frequency as noted below.
// if ignoreFrequency is true, you must ignore any power from frequencyNumber.
// maxPowerFreqNo is the frequency number with the highest value contained in
// the unsortedValues. unsortedValues contains the unsorted values. sortedValues
// contains the sorted values. Note: it is assumed that the size of both of the
// array arguments is 10.
detector_status_t detector_sort(uint32_t *maxPowerFreqNo,
                                double unsortedValues[],
                                double sortedValues[]) {
  // new sorting array
  double oldVals[NUMBER_OF_PLAYERS] = {
      INIT_FLOAT, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT,
      INIT_FLOAT, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT};
  // keep old array vals
  for (uint32_t player = RESET; player < NUMBER_OF_PLAYERS; player++) {
    oldVals[player] = unsortedValues[player];
  }

  uint32_t i, j;
  double key;

  /////////// actual sort algorithm ////////////
  for (i = NEXT_ITEM; i < NUMBER_OF_PLAYERS; i++) {
    double sortingVal = unsortedValues[i];
    key = unsortedValues[i];
    j = i - LAST_ITEM;
    // loop through each of the items
    while ((j >= RESET) && (unsortedValues[j] > key)) {
      unsortedValues[j + NEXT_ITEM] = unsortedValues[j];
      j = j - LAST_ITEM;
    }
    unsortedValues[j + NEXT_ITEM] = key;
  }

  // transfer sorted values into the sorted array
  for (uint32_t player = RESET; player < NUMBER_OF_PLAYERS; player++) {
    sortedValues[player] = unsortedValues[player];
  }

  // restore original values
  for (uint32_t player = RESET; player < NUMBER_OF_PLAYERS; player++) {
    unsortedValues[player] = oldVals[player];
  }

  // assign lastHitFrequency value
  for (uint32_t player = RESET; player < NUMBER_OF_PLAYERS; player++) {
    if (unsortedValues[player] == sortedValues[BIGGEST_HIT_PLAYER])
      *maxPowerFreqNo = player;
  }
  return DETECTOR_STATUS_OK;
}

// Encapsulate ADC scaling for easier testing.
double detector_getScaledAdcValue(isr_AdcValue_t adcValue) {
  // divide by half the adc range (4095/2), then subtract 1 to center on 0 (-1
  // to 1)
  double doubleADCVal = (double)adcValue;
  return (doubleADCVal / ADC_DIVISOR_VAL) - ADC_SCALED_VAL_SHIFTER;
}

/*******************************************************
 ****************** Test Routines **********************
 ******************************************************/

// Students implement this as part of Milestone 3, Task 3.
void detector_runTest() {
  // first set of data
  detector_powerVals[0] = 10.0;
  detector_powerVals[1] = 11.0;
  detector_powerVals[2] = 14.0;
  detector_powerVals[3] = 13.0;
  detector_powerVals[4] = 8.0;
  detector_powerVals[5] = 60004.0;
  detector_powerVals[6] = 5.0;
  detector_powerVals[7] = 12.0;
  detector_powerVals[8] = 6.0;
  detector_powerVals[9] = 19.0;
  fudgeFactor = 400;
  printf("running hit detection on data set 1\n");
  // run the detector and compare result
  if (detector_runHitDetection()) {
    printf("Hit was detected in first set of data\n");
    printf("Hot for player %d", lastHitFrequency);
  } else { // no hit
    printf("no hit was detected in first set of data\n");
  }

  detector_clearHit();

  // second set of data
  detector_powerVals[0] = 10.0;
  detector_powerVals[1] = 11.0;
  detector_powerVals[2] = 12.0;
  detector_powerVals[3] = 13.0;
  detector_powerVals[4] = 14.0;
  detector_powerVals[5] = 15.0;
  detector_powerVals[6] = 16.0;
  detector_powerVals[7] = 17.0;
  detector_powerVals[8] = 18.0;
  detector_powerVals[9] = 19.0;
  fudgeFactor = 400;
  // running detector
  printf("running hit detection on data set 2\n");
  // detector_runHitDetection();
  if (detector_runHitDetection()) {
    printf("Hit was detected in the second data set\n");
  } else { // no hit
    printf("no hit was detected in the second set of data\n");
  }
  printf("end of detector_runtest()\n");

  detector_clearHit();
}

// Returns 0 if passes, non-zero otherwise.
// if printTestMessages is true, print out detailed status messages.
detector_status_t detector_testAdcScaling() {
  printf("testing adc scaling for lowest value (0), output should be -1\n");
  printf("output: %f\n", detector_getScaledAdcValue(0));

  printf("testing adc scaling for highest value (4095), output should be 1\n");
  printf("output: %f\n", detector_getScaledAdcValue(4095));

  printf("testing adc scaling for lower middle value (2047), output should be "
         "just under 0\n");
  printf("output: %f\n", detector_getScaledAdcValue(2047));

  printf("testing adc scaling for higher middle value (2048), output should be "
         "just above 0\n");
  printf("output: %f\n", detector_getScaledAdcValue(2048));

  printf("testing adc scaling for random number (3103), output should be about "
         ".51550672\n");
  printf("output: %f\n", detector_getScaledAdcValue(3103));

  printf("testing adc scaling for random number (472), output should be about "
         "-.76947497\n");
  printf("output: %f\n", detector_getScaledAdcValue(472));

  printf("end of adc scaling tests\n");
}
