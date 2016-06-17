#include <Arduino.h>

#include "editor-modes.h"
#include "editor.h"
#include "panel-scanner.h"
#include "player.h"
#include "pattern.h"

extern PanelScanner theScanner;
extern Player       thePlayer;
extern Pattern      thePattern;
extern Editor       theEditor;

// Local definitions

static const int32_t PATTERN_SEL_INDICATOR = 0x10;
static const int32_t PATTERN_CHAIN_INDICATOR = 0x11;
static const int32_t STEP_EDIT_INDICATOR = 0x12;
static const int32_t STEP_ACCENT_INDICATOR = 0x13;
static const int32_t VOICE_SEL_INDICATOR = 0x14;
static const int32_t MUTE_SEL_INDICATOR = 0x15;
static const int32_t UTILITY_SEL_INDICATOR = 0x16;
static const int32_t PLAY_INDICATOR = 0x17;

///////////////////////////////////////////////////////////////////////////////////////
// common implementation because chain display 
// is used by chain and patt sel modes.
///////////////////////////////////////////////////////////////////////////////////////

static void doChainLeds()
{
  if(thePlayer.chainIsActive())
  {
    theScanner.clearAllBlinkingLEDs();
    
    for(uint32_t i = 0; i < Pattern::NUM_PATTERNS; i++)
    {
      theScanner.setBlinkingLED(i, thePlayer.checkChainMembership(i));
    }
    // blinking overrides background, so
    // to mark current pattern, unset blinking, 
    // set background
    theScanner.clearBlinkingLED(thePattern.getCurrentPattern());
    theScanner.setBackgroundLED(thePattern.getCurrentPattern());
  }
}

static void doPlayingLed()
{
  theScanner.clearAllOverlayLEDs();
  
  if(thePlayer.isPlaying())
  {
    theScanner.setOverlayLED(thePlayer.getCurrentStep());
    theScanner.setBackgroundLED(PLAY_INDICATOR);
  }
  else
  {
    theScanner.clearBackgroundLED(PLAY_INDICATOR);
  }

 
}

///////////////////////////////////////////////////////////////////////////////////////
//////// Base Class
///////////////////////////////////////////////////////////////////////////////////////

// constructor...
pvEditorMode::pvEditorMode()
{
}

// Common handling for the start/stop key
void pvEditorMode::HandlePlayButton(bool pressed)
{
  if(pressed )
  {
    if(thePlayer.isPlaying())
    {
      thePlayer.stop();
    }
    else
    {
      thePlayer.start();
    }
  }
}


///////////////////////////////////////////////////////////////////////////////////////
//////// Step Editor
///////////////////////////////////////////////////////////////////////////////////////

StepEdit::StepEdit(): pvEditorMode()
{
  //current_voice = 0;
  
}

void StepEdit::HandleKey(uint32_t keynum, bool pressed)
{
  bool setting;

  if (keynum == PLAY_INDICATOR) //start/stop key
  {
    HandlePlayButton(pressed);
  }
  else if((keynum >= 0) && (keynum <= 15))
  {
    if(pressed)
    {
      setting = thePattern.toggleBit(keynum);

      // If we're clearing a bit, lose it's accent, too.
      if(!setting)
      {
        if(thePattern.getAccentBit(keynum))
          thePattern.toggleAccentBit(keynum);
      }
      
      setLEDs(true);
    }
  }
  else if(keynum == VOICE_SEL_INDICATOR) // voice select mode
  {
    if(pressed)
    {
      theEditor.setMode(Editor::eMODE_VOICE_SEL);
    }
  }
  else if(keynum ==   STEP_ACCENT_INDICATOR) // voice select mode
  {
    if(pressed)
    {
      theEditor.setMode(Editor::eMODE_STEP_ACCENT);
    }
  }

  else if(keynum == MUTE_SEL_INDICATOR)
  {
    if(pressed)
    {
      theEditor.setMode(Editor::eMODE_MUTE_SEL);
    }
  }
  else if(keynum == PATTERN_SEL_INDICATOR) // pattern select mode
  {
    if(pressed)
    {
      theEditor.setMode(Editor::eMODE_PATT_SEL);
    }
  }
  else if(keynum == UTILITY_SEL_INDICATOR)
  {
    if(pressed)// && (!thePlayer.isPlaying()))
    {
      theEditor.setMode(Editor::eMODE_UTILITY);
    }
  }

#if 0
  else// other, unmapped keys, just show they work.
  {
    theScanner.setBackgroundLED(keynum, pressed);
  }
#endif    
  
}

void StepEdit::setLEDs(bool entry)
{
  // Initialize LEDs 
  if(entry)
  {
    doPlayingLed();
    
    theScanner.setBackgroundLED(STEP_EDIT_INDICATOR);
    
    for(uint32_t i = 0; i < Pattern::PATTERN_LEN; i++)
    {
      theScanner.setBackgroundLED(i, thePattern.getVoiceBit(i));
    }
  }
  else // on exit
  {
    theScanner.clearAllHalfLEDs();
    theScanner.clearAllBackgroundLEDs();
    theScanner.clearAllBlinkingLEDs();
  }
  
}

///////////////////////////////////////////////////////////////////////////////////////
//////// Accent Editor
///////////////////////////////////////////////////////////////////////////////////////

StepAccent::StepAccent(): pvEditorMode()
{
  //current_voice = 0;
  
}

void StepAccent::HandleKey(uint32_t keynum, bool pressed)
{
  bool setting;

  if(keynum == STEP_ACCENT_INDICATOR) 
  {
    if(!pressed)
    {
      theEditor.setMode(Editor::eMODE_STEP_EDIT);
    }
  }
  else if (keynum == PLAY_INDICATOR) //start/stop key
  {
    HandlePlayButton(pressed);
  }
  else if((keynum >= 0) && (keynum <= 15))
  {
    if(pressed)
    {
      setting = thePattern.toggleAccentBit(keynum);

      // If we just added an accent, but note isn't triggered, trigger it.
      // Likewise, if we cleared and accent, clear the trigger.
      // Don't leave stranded accents, because they're an ugly surprise later...
      if( (setting && !thePattern.getVoiceBit(keynum) ) ||
          (!setting && thePattern.getVoiceBit(keynum)))
      {
        thePattern.toggleBit(keynum);
      }
      
      setLEDs(true);
    }
  }
}

void StepAccent::setLEDs(bool entry)
{
  // Initialize LEDs 
  if(entry)
  {
    doPlayingLed();
    
    theScanner.setBackgroundLED(STEP_ACCENT_INDICATOR);
    
    for(uint32_t i = 0; i < Pattern::PATTERN_LEN; i++)
    {
      theScanner.setBackgroundLED(i, thePattern.getAccentBit(i));
      theScanner.setHalfLED(i, thePattern.getVoiceBit(i));
    }
  }
  else // on exit
  {
    theScanner.clearAllHalfLEDs();
    theScanner.clearAllBackgroundLEDs();
    theScanner.clearAllBlinkingLEDs();
  }
  
}


///////////////////////////////////////////////////////////////////////////////////////
//////// Voice Selector
///////////////////////////////////////////////////////////////////////////////////////

VoiceSelect::VoiceSelect(): pvEditorMode()
{
  
}

void VoiceSelect::HandleKey(uint32_t keynum, bool pressed)
{
  if(keynum == VOICE_SEL_INDICATOR) // voice select mode
  {
    if(!pressed)
    {
      theEditor.setMode(Editor::eMODE_STEP_EDIT);
    }
  }
  else if (keynum == PLAY_INDICATOR) //start/stop key
  {
    HandlePlayButton(pressed);
  }
  else if((keynum >= 0) && (keynum <= 10))
  {
    // TBD - more voices means accept more input
    if(pressed)
    {
      thePattern.setCurrentVoice(keynum);
      setLEDs(true);
    }
  }
}

void VoiceSelect::setLEDs(bool entry)
{
  uint32_t bitnum;
  
  if(entry)
  {
    theScanner.clearAllBackgroundLEDs();
    
    doPlayingLed();
    
    // Mode indication
    theScanner.setBackgroundLED(VOICE_SEL_INDICATOR);

    // Present voice indication
    bitnum = thePattern.getCurrentVoice();
    theScanner.setBackgroundLED(bitnum);
  }
  else
  {
    theScanner.clearAllBackgroundLEDs();
  }
}


///////////////////////////////////////////////////////////////////////////////////////
//////// Mute Selector
///////////////////////////////////////////////////////////////////////////////////////

MuteSelect::MuteSelect(): pvEditorMode()
{
  
}

void MuteSelect::HandleKey(uint32_t keynum, bool pressed)
{
  bool setting;
  
  if(keynum == STEP_EDIT_INDICATOR) // voice select mode
  {
    if(pressed)
    {
      theEditor.setMode(Editor::eMODE_STEP_EDIT);
    }
  }
  else if(keynum == PATTERN_SEL_INDICATOR) // pattern select mode
  {
    if(pressed)
    {
      theEditor.setMode(Editor::eMODE_PATT_SEL);
    }
  }
  else if (keynum == PLAY_INDICATOR) //start/stop key
  {
    HandlePlayButton(pressed);
  }
  else if(keynum == UTILITY_SEL_INDICATOR)
  {
    if(pressed)// && (!thePlayer.isPlaying()))
    {
      theEditor.setMode(Editor::eMODE_UTILITY);
    }
  }

  else if((keynum >= 0) && (keynum <= 10))
  {
    if(pressed)
    {
      setting = thePlayer.toggleMuteBit(keynum);
      setLEDs(true);
      
    }
  }
}

void MuteSelect::setLEDs(bool entry)
{
  if(entry)
  {
    doPlayingLed();
    
    // set mode indicator
    theScanner.setBackgroundLED(MUTE_SEL_INDICATOR);

    // and display data on editor buttons
    for(uint32_t i = 0; i < 12; i++)
    {
      theScanner.setBackgroundLED(i, thePlayer.getMuteBit(i));
      // blinking overrides background
      theScanner.setBlinkingLED(i, thePlayer.getPendingMuteBit(i));
    }

    
  }
  else
  {
    theScanner.clearAllBackgroundLEDs();
    theScanner.clearAllBlinkingLEDs();
  }
}

///////////////////////////////////////////////////////////////////////////////////////
//////// Pattern Selector
///////////////////////////////////////////////////////////////////////////////////////


PatternSelect::PatternSelect()
{
  
}
  
void PatternSelect::HandleKey(uint32_t keynum, bool pressed)
{
  if(keynum == STEP_EDIT_INDICATOR)
  {
    if(pressed)
    {
      theEditor.setMode(Editor::eMODE_STEP_EDIT);
    }
  }
  else if(keynum == MUTE_SEL_INDICATOR)
  {
    if(pressed)
    {
      theEditor.setMode(Editor::eMODE_MUTE_SEL);
    }
  }
  else if(keynum == UTILITY_SEL_INDICATOR)
  {
    if(pressed)
    {
      theEditor.setMode(Editor::eMODE_UTILITY);
    }
  }
  else if(keynum == PATTERN_CHAIN_INDICATOR)
  {
    if(pressed)
    {
      thePlayer.initChain();
      theEditor.setMode(Editor::eMODE_CHAIN_EDIT);
    }
  }
  else if (keynum == PLAY_INDICATOR) //start/stop key
  {
    HandlePlayButton(pressed);
  }
  else if((keynum >= 0) && (keynum <= 15))
  {
    if(pressed)
    {
      // if there was a chain, end it
      if(thePlayer.chainIsActive())
      {
        thePlayer.initChain();
      }

      thePlayer.setNextPattern(keynum);
      setLEDs(true);
    }
  }
}

void PatternSelect::setLEDs(bool entry)
{
  int32_t pending;
  
  if(entry)
  {
    theScanner.clearAllBackgroundLEDs();
    theScanner.clearAllBlinkingLEDs();

    doPlayingLed();
    
    // set mode indicator
    theScanner.setBackgroundLED(PATTERN_SEL_INDICATOR);

    if(!thePlayer.chainIsActive())
    {
      theScanner.setBackgroundLED(thePattern.getCurrentPattern());
      pending = thePlayer.getPendingPattern();
      if(pending != -1)
      {
        theScanner.setBlinkingLED(pending);
      }
    }
    else
    {
      doChainLeds();
    }
  }
  else
  {
    theScanner.clearAllBackgroundLEDs();
    theScanner.clearAllBlinkingLEDs();
  }

}

///////////////////////////////////////////////////////////////////////////////////////
//////// Chain Editor
///////////////////////////////////////////////////////////////////////////////////////


ChainEdit::ChainEdit()
{
  
}
  
void ChainEdit::HandleKey(uint32_t keynum, bool pressed)
{
  if(keynum == PATTERN_CHAIN_INDICATOR)
  {
    if(!pressed)
    {
      theEditor.setMode(Editor::eMODE_PATT_SEL);
    }
  }
  else if(keynum == PLAY_INDICATOR) //start/stop key
  {
    HandlePlayButton(pressed);
  }
  else if((keynum >= 0) && (keynum <= 15))
  {
    if(pressed)
    {
      thePlayer.addToChain(keynum);
    }
  }
}

void ChainEdit::setLEDs(bool entry)
{
  if(entry)
  {
    doPlayingLed();
    
    doChainLeds();

    // set mode indicator
    theScanner.setBackgroundLED(PATTERN_CHAIN_INDICATOR);
  }
  else
  {
    theScanner.clearAllBackgroundLEDs();
    theScanner.clearAllBlinkingLEDs();
  }

}


///////////////////////////////////////////////////////////////////////////////////////
//////// Utility mode
///////////////////////////////////////////////////////////////////////////////////////

UtilityMode::UtilityMode(): pvEditorMode()
{
  
}

void UtilityMode::HandleKey(uint32_t keynum, bool pressed)
{
  if(keynum == STEP_EDIT_INDICATOR) // voice select mode
  {
    if(pressed)
    {
      theEditor.setMode(Editor::eMODE_STEP_EDIT);
    }
  }
  else if(keynum == MUTE_SEL_INDICATOR)
  {
    if(pressed)
    {
      theEditor.setMode(Editor::eMODE_MUTE_SEL);
    }
  }
  else if(keynum == PATTERN_SEL_INDICATOR) // pattern select mode
  {
    if(pressed)
    {
      theEditor.setMode(Editor::eMODE_PATT_SEL);
    }
  }
  else if (keynum == PLAY_INDICATOR) //start/stop key
  {
    HandlePlayButton(pressed);
  }
  else if((keynum >= 0) && (keynum <= 15))
  {
    doUtilMode(keynum, pressed);
    
  }
}

void UtilityMode::setLEDs(bool entry)
{
  //uint32_t bitnum;
  
  if(entry)
  {
    doPlayingLed();
    
    // Mode indication
    theScanner.setBackgroundLED(UTILITY_SEL_INDICATOR);

    // Present data indication
    theScanner.setBackgroundLED(0x2, thePlayer.getSwing());
  }
  else
  {
    theScanner.clearAllBackgroundLEDs();
  }
}

void UtilityMode::doUtilMode(uint32_t keynum, bool pressed)
{
  bool playing = thePlayer.isPlaying();
  
  if(!pressed)
  {
    return;
  }
  
  switch(keynum)
  {
    case 0:
    {
      if(!playing)
      {
        thePattern.clearCurrentPattern();
      }
    }
    break;
    case 1:
    {
      if(!playing)
      {
        thePattern.randomizeCurrentPattern();
      }
    }
    break;
    case 2:
    {
      thePlayer.toggleSwing();
      setLEDs(true);
    }
    break;
    case 14:
    {
      if(!playing)
      {
        delay(100);
        
        thePattern.writeToCard();

        delay(100);
      }
    }
    break;
    case 15:
    {
      if(!playing)
      {
        delay(100);

        thePattern.readFromCard();

        delay(100);
      }
    }
    break;
    
  }
}


