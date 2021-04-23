#include "switches.h"
#include "xil_io.h"
#include <stdint.h>
#include <xparameters.h>

#define BASE_SWITCH 0
#define LOOP 1
#define INIT_SWITCH 0x00
#define ALL_ON 0xF

// reads and returns the contents of the switches GPIO register
int32_t switches_readGpioRegister(int32_t offset) {
  return Xil_In32(XPAR_SLIDE_SWITCHES_BASEADDR + offset);
}

// writes a value to the switches register with a given offset
void switches_writeGpioRegister(int32_t offset, int32_t value) {
  Xil_Out32(XPAR_SLIDE_SWITCHES_BASEADDR + offset, value);
}

// initialize the switches
int32_t switches_init() {
  switches_writeGpioRegister(BASE_SWITCH, INIT_SWITCH);
  return switches_read();
}

// returns the current values of the switches
int32_t switches_read() { return switches_readGpioRegister(BASE_SWITCH); }

// lights up the LEDs above the switches depending on the passed in value
void switches_lightSwitches(int8_t switchesOn) {
  Xil_Out32(XPAR_LEDS_BASEADDR, switchesOn);
}

// loops reading the current value of the switches and lighting up the LEDs
// above depending on the switches current positions
// Terminates when all switches are on
void switches_runTest() {
  // switch initialization
  int8_t lastSwitches = switches_init();
  while (LOOP) {
    // gets the current value of the switches
    int8_t switchesOn = switches_readGpioRegister(BASE_SWITCH);
    // if the switches positions have changed change the LEDs above them
    if (lastSwitches != switchesOn) {
      lastSwitches = switchesOn;
      switches_lightSwitches(switchesOn);
    }
    // End the loop if all switches are pressed
    if (switchesOn == ALL_ON) {
      switches_lightSwitches(INIT_SWITCH);
      return;
    }
  }
}