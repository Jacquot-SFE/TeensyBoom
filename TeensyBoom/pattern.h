#ifndef _PATTERN_H_
#define _PATTERN_H_

#pragma once

#include <stdint.h>


class Pattern
{
public: 
  static const uint32_t PATTERN_LEN = 16;
  static const uint32_t NUM_PATTERNS = 16;

  Pattern();

  bool     toggleBit(uint32_t step);
  bool     toggleAccentBit(uint32_t step);

  void     setCurrentVoice(uint32_t);
  uint32_t getCurrentVoice(void);

  void     setCurrentPattern(uint32_t);
  uint32_t getCurrentPattern();

  void     clearCurrentPattern();
  void     randomizeCurrentPattern();

  uint32_t getStepData(uint32_t index);
  bool     getVoiceBit(uint32_t step);
  bool     getAccentBit(uint32_t step);

  bool     writeToCard();
  bool     readFromCard();
  
private:

  uint32_t pattern_data[NUM_PATTERNS][PATTERN_LEN];

  uint32_t current_pattern;
  uint32_t current_voice;
  uint32_t current_voice_mask;
  uint32_t current_accent_mask;
  

};



#endif // keepout
