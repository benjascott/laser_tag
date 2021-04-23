#include "intervalTimer.h"
#include "display.h"
#include "utils.h"
#include "xparameters.h"
#include <stdint.h>
#include <stdio.h>
#include <xil_io.h>

#define ZERO 0

#define TCSR0 0x00
#define TLR0 0x04
#define TCR0 0x08

#define TCR0_OFFSET 0x08 // register offset for TCR0
#define TCR1_OFFSET 0x18 // register offset for TCR1

#define TCSR1 0x10
#define TLR1 0x14
#define TCR1 0x18
#define MASK_ENT0_ENABLE 0x00000080
#define MASK_ENT0_DISABLE 0xFFFFFF7F

#define SHORT_DELAY 1000
#define DELAY_FOR_EMULATOR_CLOCK 1000
#define SHIFT_SECOND_REG 32

#define BIT_32_RESET 0x00000000

#define CASC_MASK 0x00000800
#define LOAD_MASK 0x00000020

// writes the givin value to the given register address
void intervalTimer_write(uint32_t address, uint32_t value) {
  Xil_Out32(address, value);
}

// reads the register value at the specified address
uint64_t intervalTimer_read(uint32_t address) { return Xil_In32(address); }

// writes zero to the registers TLR0 and TCSR1
// then writes a 1 to the CASC bit in TCSR0 register keeping the rest the same
// argument base address (base address of the counter you want to call this on)
void intervalTimer_init_computation(uint32_t baseAddress) {
  intervalTimer_write(baseAddress + TLR0, ZERO);
  intervalTimer_write(baseAddress + TCSR1, ZERO);
  uint32_t currentState = intervalTimer_read(baseAddress + TCSR0);
  uint32_t writeValue = (currentState | CASC_MASK);
  intervalTimer_write(baseAddress + TCSR0, writeValue);
}

// Initializes a specific timer 0, 1, or 2
// returns 1 if successful and 0 if the timer number isn't a usable timer
// argument timerNumber which timer to use 0, 1, 2
intervalTimer_status_t intervalTimer_init(uint32_t timerNumber) {
  // picks the correct baseaddress the computation fuction for that address
  switch (timerNumber) {
  case INTERVAL_TIMER_TIMER_0:
    intervalTimer_init_computation(XPAR_AXI_TIMER_0_BASEADDR);
    break;
  case INTERVAL_TIMER_TIMER_1:
    intervalTimer_init_computation(XPAR_AXI_TIMER_1_BASEADDR);
    break;
  case INTERVAL_TIMER_TIMER_2:
    intervalTimer_init_computation(XPAR_AXI_TIMER_2_BASEADDR);
    break;
  default:
    return INTERVAL_TIMER_STATUS_FAIL;
    break;
  }
  return INTERVAL_TIMER_STATUS_OK;
}

// changes the start bit ENT0 in TCSR0 to a 1 keeping everything else the same
// argument base address (base address of the counter you want to call this on)
void intervalTimer_start_computation(uint32_t baseAddress) {
  uint32_t currentState = intervalTimer_read(baseAddress + TCSR0);
  uint32_t writeValue = (currentState | MASK_ENT0_ENABLE);
  intervalTimer_write(baseAddress + TCSR0, writeValue);
}

// starts the specified timer running
// argument timerNumber which timer to use 0, 1, 2
void intervalTimer_start(uint32_t timerNumber) {
  // picks the correct baseaddress the computation fuction for that address
  switch (timerNumber) {
  case INTERVAL_TIMER_TIMER_0:
    intervalTimer_start_computation(XPAR_AXI_TIMER_0_BASEADDR);
    break;
  case INTERVAL_TIMER_TIMER_1:
    intervalTimer_start_computation(XPAR_AXI_TIMER_1_BASEADDR);
    break;
  case INTERVAL_TIMER_TIMER_2:
    intervalTimer_start_computation(XPAR_AXI_TIMER_2_BASEADDR);
    break;
  }
}

// writes a 0 to the ENT0 bit of the TCSR0 register to stop the timer
// argument base address (base address of the counter you want to call this on)
void intervalTimer_stop_computation(uint32_t baseAddress) {
  uint32_t currentState = intervalTimer_read(baseAddress + TCSR0);
  uint32_t writeValue = (currentState & MASK_ENT0_DISABLE);
  intervalTimer_write(baseAddress + TCSR0, writeValue);
}

// stops the specified timer
// argument timerNumber which timer to use 0, 1, 2
void intervalTimer_stop(uint32_t timerNumber) {
  // picks the correct baseaddress the computation fuction for that address
  switch (timerNumber) {
  case INTERVAL_TIMER_TIMER_0:
    intervalTimer_stop_computation(XPAR_AXI_TIMER_0_BASEADDR);
    break;
  case INTERVAL_TIMER_TIMER_1:
    intervalTimer_stop_computation(XPAR_AXI_TIMER_1_BASEADDR);
    break;
  case INTERVAL_TIMER_TIMER_2:
    intervalTimer_stop_computation(XPAR_AXI_TIMER_2_BASEADDR);
    break;
  }
}

// loads zero into the TLR0 address,
// sets the load bit to 1 while keeping the other bits in TCSR0 the same
// then writes the load bit back to 0
// repeats for TCSR1
// argument base address (base address of the counter you want to call this on)
void intervalTimer_reset_computation(int32_t baseAddress) {
  uint32_t currentState = intervalTimer_read(baseAddress + TCSR0);
  uint32_t writeValue = (currentState | LOAD_MASK);
  intervalTimer_write(baseAddress + TLR0, ZERO);
  intervalTimer_write(baseAddress + TCSR0, writeValue);
  intervalTimer_write(baseAddress + TCSR0, currentState);

  currentState = intervalTimer_read(baseAddress + TCSR1);
  writeValue = (currentState | LOAD_MASK);
  intervalTimer_write(baseAddress + TLR1, ZERO);
  intervalTimer_write(baseAddress + TCSR1, writeValue);
  intervalTimer_write(baseAddress + TCSR1, currentState);
}

// resets the spicified timer to 0
// argument timerNumber which timer to use 0, 1, 2
void intervalTimer_reset(uint32_t timerNumber) {
  // picks the correct baseaddress the computation fuction for that address
  switch (timerNumber) {
  case INTERVAL_TIMER_TIMER_0:
    intervalTimer_reset_computation(XPAR_AXI_TIMER_0_BASEADDR);
    break;
  case INTERVAL_TIMER_TIMER_1:
    intervalTimer_reset_computation(XPAR_AXI_TIMER_1_BASEADDR);
    break;
  case INTERVAL_TIMER_TIMER_2:
    intervalTimer_reset_computation(XPAR_AXI_TIMER_2_BASEADDR);
    break;
  }
}

// milestone 2

// initializes all timers
// returns 1 for success and 0 for a failier
intervalTimer_status_t intervalTimer_initAll() {
  intervalTimer_init(INTERVAL_TIMER_TIMER_0);
  intervalTimer_init(INTERVAL_TIMER_TIMER_1);
  intervalTimer_init(INTERVAL_TIMER_TIMER_2);
}

// resets all the timers to 0
void intervalTimer_resetAll() {
  intervalTimer_reset(INTERVAL_TIMER_TIMER_0);
  intervalTimer_reset(INTERVAL_TIMER_TIMER_1);
  intervalTimer_reset(INTERVAL_TIMER_TIMER_2);
}

// get the uper register and shift by 32
// add to the lower register
// then divide by the clock frequency to get time elapsed in seconds
// returns a double of the time in seconds
double intervalTimer_getTotalDurationInSeconds_computation() {
  return (double)((intervalTimer_read(XPAR_AXI_TIMER_0_BASEADDR + TCR1_OFFSET)
                   << SHIFT_SECOND_REG) +
                  intervalTimer_read(XPAR_AXI_TIMER_0_BASEADDR + TCR0_OFFSET)) /
         XPAR_AXI_TIMER_0_CLOCK_FREQ_HZ;
}

// gets the elapsed time in seconds for the spicified timer
// returns a double of the time in seconds
// argument timerNumber which timer to use 0, 1, 2
double intervalTimer_getTotalDurationInSeconds(uint32_t timerNumber) {
  // picks the correct baseaddress the computation fuction for that address
  switch (timerNumber) {
  case INTERVAL_TIMER_TIMER_0:
    return intervalTimer_getTotalDurationInSeconds_computation(
        XPAR_AXI_TIMER_0_BASEADDR);
    break;
  case INTERVAL_TIMER_TIMER_1:
    return intervalTimer_getTotalDurationInSeconds_computation(
        XPAR_AXI_TIMER_0_BASEADDR);
    break;
  case INTERVAL_TIMER_TIMER_2:
    return intervalTimer_getTotalDurationInSeconds_computation(
        XPAR_AXI_TIMER_0_BASEADDR);
    break;
  default:
    // return 0 if an invalid timer number was called
    return 0.0;
    break;
  }
}

// test function to make sure the specified timer is working
// returns 1 is success and 0 if a fail
// argument timerNumber which timer to use 0, 1, 2
intervalTimer_status_t intervalTimer_test(uint32_t timerNumber) {
  intervalTimer_status_t timer_status = INTERVAL_TIMER_STATUS_OK;
  // reset
  intervalTimer_init(timerNumber);
  intervalTimer_reset(timerNumber);
  double time0 = intervalTimer_getTotalDurationInSeconds(timerNumber);
  // make sure timer was initialized to 0
  if (time0 != ZERO)
    timer_status = INTERVAL_TIMER_STATUS_FAIL;
  // start that timer
  intervalTimer_start(timerNumber);
  // read compare to 0
  utils_msDelay(SHORT_DELAY);
  double time1 = intervalTimer_getTotalDurationInSeconds(timerNumber);
  // make sure the timer is not 0 anymore
  if (time1 == ZERO)
    timer_status = INTERVAL_TIMER_STATUS_FAIL;

  // read compare to last value should have increased
  utils_msDelay(SHORT_DELAY);
  double time2 = intervalTimer_getTotalDurationInSeconds(timerNumber);
  if (time1 >= time2)
    timer_status = INTERVAL_TIMER_STATUS_FAIL;
  // read compare to last value should have increased
  utils_msDelay(SHORT_DELAY);
  double time3 = intervalTimer_getTotalDurationInSeconds(timerNumber);
  if (time2 >= time3)
    timer_status = INTERVAL_TIMER_STATUS_FAIL;
  // stop the timer
  utils_msDelay(SHORT_DELAY);
  intervalTimer_stop(timerNumber);
  // read two times
  double time4 = intervalTimer_getTotalDurationInSeconds(timerNumber);
  utils_msDelay(SHORT_DELAY);
  double time5 = intervalTimer_getTotalDurationInSeconds(timerNumber);
  // make sure both times are the same
  if (time4 != time5)
    timer_status = INTERVAL_TIMER_STATUS_FAIL;
  // output time results
  // these are dubugging statements
  printf("Testing Timer %u\n", timerNumber);
  printf("Timer Val0 should be 0: %lf\n", time0);
  printf("Timer Val1 should be increasing: %lf\n", time1);
  printf("Timer Val2 should be increasing: %lf\n", time2);
  printf("Timer Val3 should be increasing: %lf\n", time3);
  printf("Timer Val4 timer now stoped: %lf\n", time4);
  printf("Timer Val5 should match Val4: %lf\n", time5);
  return timer_status;
}

// tests all timers to make sure there working
// returns 0 if failed and 1 if success
intervalTimer_status_t intervalTimer_testAll() {
  // run tests for each of the 3 timers
  intervalTimer_status_t TIMER_0 = intervalTimer_test(INTERVAL_TIMER_TIMER_0);
  intervalTimer_status_t TIMER_1 = intervalTimer_test(INTERVAL_TIMER_TIMER_1);
  intervalTimer_status_t TIMER_2 = intervalTimer_test(INTERVAL_TIMER_TIMER_2);
  // make sure all results are a success
  if (TIMER_0 && TIMER_1 && TIMER_2)
    return INTERVAL_TIMER_STATUS_OK;
  else
    return INTERVAL_TIMER_STATUS_FAIL;
}

/*void isr_function() {
  // Empty for now.
}*/