#include <arduino.h>

#include "editor.h"
#include "player.h"

extern PanelScanner theScanner;
extern Player       thePlayer;


// decls for the mode objects
static StepEdit stepEditor;
static StepAccent accentEditor;
static ChainEdit chainEditor;
static VoiceSelect voiceSelector;
static MuteSelect muteSelector;
static PatternSelect patternSelector;
static UtilityMode utilityMode;

// constructor...
Editor::Editor()
{
  // don't refer to other classes here, they may not yet be constructed/init'd
  // IE: the pattern may not be constructed, to pull initial LED modes from.
  // We'll call setMode from setup() to get it to register.
  
  current_mode_p = &patternSelector;
}

void Editor::setMode(EditorMode newmode)
{
  switch(newmode)
  {
  case eMODE_VOICE_SEL:
    {
      current_mode_p->setLEDs(false);
      current_mode_p = &voiceSelector;
      current_mode_p->setLEDs(true);
      break;
    }
  case eMODE_MUTE_SEL:
    {
      current_mode_p->setLEDs(false);
      current_mode_p = &muteSelector;
      current_mode_p->setLEDs(true);
      break;
    }
  case eMODE_PATT_SEL:
    {
      current_mode_p->setLEDs(false);
      current_mode_p = &patternSelector;
      current_mode_p->setLEDs(true);
      break;
    }
  case eMODE_UTILITY:
    {
      current_mode_p->setLEDs(false);
      current_mode_p = &utilityMode;
      current_mode_p->setLEDs(true);
      break;
    }
  case eMODE_CHAIN_EDIT:
    {
      current_mode_p->setLEDs(false);
      current_mode_p = &chainEditor;
      current_mode_p->setLEDs(true);
      break;
    }
  case eMODE_STEP_ACCENT:
    {
      current_mode_p->setLEDs(false);
      current_mode_p = &accentEditor;
      current_mode_p->setLEDs(true);
      break;
    }

  default:
    {
      current_mode_p->setLEDs(false);
      current_mode_p = &stepEditor;
      current_mode_p->setLEDs(true);
      break;
    }
  }
  
}

void Editor::receiveKey(uint32_t keynum, bool pressed)
{
#if 0
  Serial.print("Ed Key: ");
  Serial.print(keynum, HEX);
  Serial.print(" ");
  Serial.println(pressed);
#endif

  current_mode_p->HandleKey(keynum, pressed);
}

void Editor::forceLEDs()
{
  current_mode_p->setLEDs(true);  
}

