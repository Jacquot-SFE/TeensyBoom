#include <Synth-Clatter.h>
#include <Synth-Decay.h>
#include <Synth-DrumHeart.h>
#include <synth_simple_drum.h>
#include <TeensyBoomVoiceLibrary.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>


#include "panel-scanner.h"
#include "editor.h"
#include "pattern.h"
#include "player.h"
#include "voice.h"


// Appp construct singletons
PanelScanner theScanner;
Editor       theEditor;
Pattern      thePattern; // TBD: multidimensional...
Player       thePlayer;


void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  delay(1500);

  Serial.println("Setup");

  pinMode(15, INPUT); // Volume pot pin?

  // SD CARD & general SPI inits
  // Initialize the SD card
  SPI.setMOSI(7);
  SPI.setSCK(14);

  // This sets the chip selects for the panel
  // to OFF.  Gotta do it before SD init, else the panels contend!
  theScanner.initScanning();


  if (!(SD.begin(10)))
  {
    Serial.println("Unable to access the SD card");
  }
  else
  {
    Serial.println("SD card begin worked");
  }

  if (SD.exists("test.txt"))
  {
    Serial.println("found test.txt file");
  }
  else
  {
    Serial.println("Didn't find test file?");
  }


  theEditor.setMode(Editor::eMODE_PATT_SEL);

  // audio library init
  AudioMemory(20);

  //next = millis() + 1000;

  // read panel before we start to run
  //paramUpdate();

  voiceInit();

  delay(500);

  Serial.println("Setup Complete");
}

void loop()
{
  uint32_t now = millis();
  static uint32_t then;

#if 1
  // put your main code here, to run repeatedly:

  paramUpdate1();// kik,snr,hat
  paramUpdate2();// toms, shaker
  paramUpdate3();// master volume & tempo

  if (now > then)
  {
    thePlayer.tick();
  }

  if (now % 5 == 0)
  {
    theScanner.tick();
  }

  if (now % 5000 == 0)
  {
    //theScanner.dumpLEDs();


    Serial.print("Diagnostics: ");
    Serial.print(" max, buffs: ");
    Serial.print(AudioProcessorUsageMax());
    Serial.print(" ");
    Serial.println(AudioMemoryUsageMax());
    AudioProcessorUsageMaxReset();
  }

  then = now;
#endif
}
