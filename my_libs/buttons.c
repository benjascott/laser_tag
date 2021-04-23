#include "buttons.h"
#include "display.h"
#include <stdint.h>
#include <xil_io.h>
#include <xparameters.h>

#define OFF 0x0
#define BASE_BUTTON 0
#define LOOP 1
#define INIT_BUTTONS 0x00
#define ALL_ON 0xF

#define TOP_LEFT_X DISPLAY_PORTRAIT_MODE_ORIGIN_LOWER_LEFT
#define TOP_LEFT_Y DISPLAY_PORTRAIT_MODE_ORIGIN_LOWER_LEFT

#define QUARTER_DISPLAY_WIDTH DISPLAY_WIDTH / 4
#define HALF_DISPLAY_HEIGHT DISPLAY_HEIGHT / 2
#define QUARTER_DISPLAY_HEIGHT DISPLAY_HEIGHT / 4

#define BTN0_TEXT_POS                                                          \
  TOP_LEFT_X + TEXT_PIXEL_OFFSET + (BTN_0_POS * (QUARTER_DISPLAY_WIDTH))
#define BTN1_TEXT_POS                                                          \
  TOP_LEFT_X + TEXT_PIXEL_OFFSET + (BTN_1_POS * (QUARTER_DISPLAY_WIDTH))
#define BTN2_TEXT_POS                                                          \
  TOP_LEFT_X + TEXT_PIXEL_OFFSET + (BTN_2_POS * (QUARTER_DISPLAY_WIDTH))
#define BTN3_TEXT_POS TOP_LEFT_X + TEXT_PIXEL_OFFSET

#define BTN0_X TOP_LEFT_X + (BTN_0_POS * (QUARTER_DISPLAY_WIDTH))
#define BTN1_X TOP_LEFT_X + (BTN_1_POS * (QUARTER_DISPLAY_WIDTH))
#define BTN2_X TOP_LEFT_X + (BTN_2_POS * QUARTER_DISPLAY_WIDTH)
#define BTN3_X TOP_LEFT_X

#define BTN_0_POS 3
#define BTN_1_POS 2
#define BTN_2_POS 1
#define LAST_LINE_NOT_WORKING 1

#define BTN0_TEXT "BTN0"
#define BTN1_TEXT "BTN1"
#define BTN2_TEXT "BTN2"
#define BTN3_TEXT "BTN3"
#define TEXT_PIXEL_OFFSET 16

#define TEXT_SIZE 2

// reads and returns the contents of the buttons GPIO register
int32_t buttons_readGpioRegister(int32_t offset) {
  return Xil_In32(XPAR_PUSH_BUTTONS_BASEADDR + offset);
}

// returns the current value of the buttons (first 4 bits)
int32_t buttons_read() { return Xil_In32(XPAR_PUSH_BUTTONS_BASEADDR); }

// initializes the buttons
int32_t buttons_init() {
  Xil_Out32(XPAR_PUSH_BUTTONS_BASEADDR, INIT_BUTTONS);
  return buttons_read();
}

// writes a value to the buttons register with a given offset
void buttons_writeGpioRegister(int32_t offset, int32_t value) {
  Xil_Out32(XPAR_PUSH_BUTTONS_BASEADDR + offset, value);
}

// draws boxes on the screen for the currently pushed buttons the values of
// which must be passed in
void buttons_drawButton(int8_t buttonsOn) {
  display_fillScreen(DISPLAY_BLACK);
  // BTN3 box and text display to the screen
  if ((buttonsOn & BUTTONS_BTN3_MASK) != OFF) {
    display_fillRect(BTN3_X, TOP_LEFT_Y, QUARTER_DISPLAY_WIDTH,
                     HALF_DISPLAY_HEIGHT, DISPLAY_BLUE);
    display_setCursor(BTN3_TEXT_POS, QUARTER_DISPLAY_HEIGHT);
    display_println(BTN3_TEXT);
  }
  // BTN2 box and text display to the screen
  if ((buttonsOn & BUTTONS_BTN2_MASK) != OFF) {
    display_fillRect(BTN2_X, TOP_LEFT_Y, QUARTER_DISPLAY_WIDTH,
                     HALF_DISPLAY_HEIGHT, DISPLAY_RED);
    display_setCursor(BTN2_TEXT_POS, QUARTER_DISPLAY_HEIGHT);
    display_println(BTN2_TEXT);
  }
  // BTN1 box and text display to the screen
  if ((buttonsOn & BUTTONS_BTN1_MASK) != OFF) {
    display_fillRect(BTN1_X, TOP_LEFT_Y, QUARTER_DISPLAY_WIDTH,
                     HALF_DISPLAY_HEIGHT, DISPLAY_GREEN);
    display_setCursor(BTN1_TEXT_POS, QUARTER_DISPLAY_HEIGHT);
    display_println(BTN1_TEXT);
  }
  // BTN0 box and text display to the screen
  if ((buttonsOn & BUTTONS_BTN0_MASK) != OFF) {
    display_fillRect(BTN0_X, TOP_LEFT_Y,
                     (QUARTER_DISPLAY_WIDTH)-LAST_LINE_NOT_WORKING,
                     HALF_DISPLAY_HEIGHT, DISPLAY_YELLOW);
    display_setCursor(BTN0_TEXT_POS, QUARTER_DISPLAY_HEIGHT);
    display_println(BTN0_TEXT);
  }
}

// loop that will run till all the buttons are pressed simultaniously
// reads the current values of the buttons then displays output to the screen
// depending on which are pressed
void buttons_runTest() {
  // initialization
  display_init();
  display_fillScreen(DISPLAY_BLACK);
  display_setTextColor(DISPLAY_WHITE);
  display_setTextSize(TEXT_SIZE);
  // Blank the screen.
  int8_t lastButtons = buttons_init();
  while (LOOP) {
    int8_t buttonsOn =
        buttons_readGpioRegister(BASE_BUTTON); // get button values
    // write new display contests to the screen if the buttons pressed chages
    if (lastButtons != buttonsOn) {
      lastButtons = buttonsOn;
      buttons_drawButton(buttonsOn);
    }
    // exits if all buttons are pressed
    if (buttonsOn == ALL_ON) {
      display_fillScreen(DISPLAY_BLACK);
      return;
    }
  }
}
