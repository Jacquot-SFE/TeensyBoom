#ifndef _PANEL_SCANNER_H_
#define _PANEL_SCANNER_H_

#pragma once

#include "editor.h"

static const uint32_t NUM_PANELS = 3;

class PanelScanner
{
public: 
  PanelScanner();

  void initScanning();

  void tick();

  void clearAllLED();

  void setHalfLED(uint32_t num, bool on = true);
  void clearHalfLED(uint32_t num);
  void clearAllHalfLEDs();

  void setBackgroundLED(uint32_t num, bool on = true);
  void clearBackgroundLED(uint32_t num);
  void clearAllBackgroundLEDs();
  
  void setBlinkingLED(uint32_t num, bool on = true);
  void clearBlinkingLED(uint32_t num);
  void clearAllBlinkingLEDs();
  
  void setOverlayLED(uint32_t num);
  void clearOverlayLED(uint32_t num);
  void clearAllOverlayLEDs();

  void dumpLEDs();


private:

  // internal routines
  void readButtons();
  void parseButtons();

  void doTransaction();

  // internal data

  uint8_t led_half_buffer[NUM_PANELS];
  uint8_t led_background_buffer[NUM_PANELS];
  uint8_t led_blinking_buffer[NUM_PANELS];
  uint8_t led_overlay_buffer[NUM_PANELS];
  uint8_t old_buttons[NUM_PANELS];
  uint8_t new_buttons[NUM_PANELS];

};



#endif // keepout
