#include <Arduino.h>
#include <Audio.h>
#include <SPI.h>
#include <SD.h>

#include "pattern.h"

static const uint32_t sequence[Pattern::PATTERN_LEN] =
{
#if 0
  //shaker and toms....
  //S123OCSK
  0b100001000001,
  0b00000000,
  0b10100000,
  0b10000000,
  0b00010001,
  0b00000000,
  0b10100000,
  0b10000000,
  0b01000001,
  0b01000000,
  0b10000000,
  0b10100000,
  0b00100001,
  0b00000000,
  0b10010000,
  0b10010000,
#else
  //kick, snare & hats
  0b100000000101,
  0b00000100,
  0b10000100,
  0b10000101,
  0b00000110,
  0b00000100,
  0b10001001,
  0b10000010,
  0b00000101,
  0b00000100,
  0b10000100,
  0b10000100,
  0b00000110,
  0b00000100,
  0b10010100,
  0b10000100,
#endif
};

static const uint8_t SD_CHIPSEL = 10;


// constructor...
Pattern::Pattern()
{

  // Only init internal data -
  // adding dependencies on other items at this point is tricky, if we aren't really careful
  // about order of construction.

  for (uint32_t i = 0; i < PATTERN_LEN; i++)
  {
    for (uint32_t j = 0; j < NUM_PATTERNS; j++)
    {
      pattern_data[j][i] = sequence[i];
    }
  }

  setCurrentVoice(0);
  setCurrentPattern(0);

  current_voice_mask  = 0x01;
  current_accent_mask = 0x010000;

}


bool Pattern::toggleBit(uint32_t index)
{
  if (index >= PATTERN_LEN)
  {
    Serial.println("Overflow in toggle");
    index %= PATTERN_LEN;
  }

  pattern_data[current_pattern][index] ^= current_voice_mask;

  return (pattern_data[current_pattern][index] & current_voice_mask);
}


bool Pattern::toggleAccentBit(uint32_t index)
{
  if (index >= PATTERN_LEN)
  {
    Serial.println("Overflow in toggle");
    index %= PATTERN_LEN;
  }

  pattern_data[current_pattern][index] ^= current_accent_mask;

  return (pattern_data[current_pattern][index] & current_accent_mask);
}




// Return all of the buts for the current step
uint32_t Pattern::getStepData(uint32_t index)
{
  if (index >= PATTERN_LEN)
  {
    Serial.println("Overflow in pattern req");
    index %= PATTERN_LEN;
  }

  return pattern_data[current_pattern][index];
}

// Get all of the column data for a particular voice
bool Pattern::getVoiceBit(uint32_t step)
{
  if (step >= PATTERN_LEN)
  {
    Serial.println("Overflow in pattern bit");
    step %= PATTERN_LEN;
  }
#if 0
  Serial.print("GVB: curr ")  ;
  Serial.print(current_pattern)  ;
  Serial.print(" step ")  ;
  Serial.println(step)  ;
#endif

  return (pattern_data[current_pattern][step] & current_voice_mask);
}

bool Pattern::getAccentBit(uint32_t step)
{
  if (step >= PATTERN_LEN)
  {
    Serial.println("Overflow in pattern bit");
    step %= PATTERN_LEN;
  }

  return (pattern_data[current_pattern][step] & current_accent_mask);
}


void Pattern::setCurrentVoice(uint32_t num)
{
  current_voice = num;
  current_voice_mask = 1 << num;
  current_accent_mask = 1 << (num + 16);
}

uint32_t Pattern::getCurrentVoice(void)
{
  return current_voice;
}

void Pattern::setCurrentPattern(uint32_t nextpatt)
{
  if (nextpatt < NUM_PATTERNS)
  {
    current_pattern = nextpatt;
  }
}

uint32_t Pattern::getCurrentPattern()
{
  return current_pattern;
}

void     Pattern::clearCurrentPattern()
{
  for (uint32_t i = 0; i < PATTERN_LEN; i++)
  {
    pattern_data[current_pattern][i] = 0;
  }
}

void     Pattern::randomizeCurrentPattern()
{
  for (uint32_t i = 0; i < PATTERN_LEN; i++)
  {
    pattern_data[current_pattern][i] = random(0x0fff) & random(0x0fff) ;
  }
}

////////////////////////////////////////////////////////////////////
////////// file management
////////////////////////////////////////////////////////////////////
bool     Pattern::writeToCard()
{
  File fd;
  size_t len;
  bool success = false;

  // shouldn't reach here if playing.

  Serial.println("***** Attempting pattern file write");

  if (SD.exists("test.txt"))
  {
    Serial.println("** found test file!");
  }
  else
  {
    Serial.println("** NO test file!");
  }


  fd = SD.open("patt.txt", FILE_WRITE);

  if (!fd)
  {
    Serial.println("** no fd!");
    goto CLEANUP;
  }

  fd.seek(0);

  len = sizeof(pattern_data);
  Serial.print("Pattern memoty total length: ");
  Serial.println(len);

  if (fd.write((uint8_t*)pattern_data, len) != len)
  {
    Serial.println("** Bad write length");
    goto CLEANUP;
  }

  success = true;
  Serial.println("***** Pattern file completely written.");

CLEANUP:

  if (fd)
  {
    Serial.println("Cleaning up fd");
    fd.flush();
    fd.close();
  }

  return success;
}


bool     Pattern::readFromCard()
{
  File fd;
  size_t len, avail;
  bool success = false;

  uint8_t val;
  uint8_t* munge = (uint8_t*)pattern_data;


  Serial.println("***** Attempting pattern file read");

  if (!SD.exists("patt.txt"))
  {
    Serial.println("** NO pattern file!");
    goto CLEANUP;
  }

  Serial.println("** found pattern file!");

  fd = SD.open("patt.txt", FILE_READ);

  if (!fd)
  {
    Serial.println("** no fd!");
    goto CLEANUP;
  }

  fd.seek(0);

  len = sizeof(pattern_data);
  Serial.print("Pattern memoty total length: ");
  Serial.println(len);

  avail = fd.available();
  Serial.print("File has bytes: ");
  Serial.println(avail);

  if (len != avail)
  {
    Serial.println("File Read error: length/struct mismatch");
    goto CLEANUP;
  }

  for (uint32_t i = 0; i < len; i++)
  {
    val = fd.read();
    munge[i] = val;
  }

  success = true;

  Serial.println("***** Pattern file completely read.");

CLEANUP:

  if (fd)
  {
    Serial.println("Cleaning up fd");
    fd.flush();
    fd.close();
  }

  return success;
}


