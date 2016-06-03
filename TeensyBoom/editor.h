#ifndef _EDITOR_H_
#define _EDITOR_H_

#pragma once

#include <stdint.h>

// fwd decl for other headers
class Editor;

#include "panel-scanner.h"
#include "editor-modes.h"

class Editor
{
public: 

  enum EditorMode
  {
    eMODE_STEP_EDIT = 0,
    eMODE_STEP_ACCENT,
    eMODE_VOICE_SEL,
    eMODE_MUTE_SEL,
    eMODE_PATT_SEL,
    eMODE_CHAIN_EDIT,
    eMODE_UTILITY
  };

  Editor();

  void receiveKey(uint32_t keynum, bool pressed);
  void setMode(EditorMode);
  void forceLEDs();
  
private:

  pvEditorMode* current_mode_p;

};



#endif // keepout
