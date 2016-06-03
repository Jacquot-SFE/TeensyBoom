#ifndef _EDITOR_MODES_H_
#define _EDITOR_MODES_H_

#pragma once

#include <stdint.h>

class pvEditorMode
{
  public:

    pvEditorMode();

    virtual void HandleKey(uint32_t keynum, bool pressed) = 0;
    virtual void setLEDs(bool entry) = 0;

    void HandlePlayButton(bool pressed);

  private:
};


class StepEdit: public pvEditorMode
{
  public:

    StepEdit();
    virtual void HandleKey(uint32_t keynum, bool pressed);
    virtual void setLEDs(bool entry);
  private:

};

class StepAccent: public pvEditorMode
{
  public:

    StepAccent();
    virtual void HandleKey(uint32_t keynum, bool pressed);
    virtual void setLEDs(bool entry);
  private:

};

class VoiceSelect: public pvEditorMode
{
  public:

    VoiceSelect();
    virtual void HandleKey(uint32_t keynum, bool pressed);
    virtual void setLEDs(bool entry);

  private:
};

class MuteSelect: public pvEditorMode
{
  public:

    MuteSelect();
    virtual void HandleKey(uint32_t keynum, bool pressed);
    virtual void setLEDs(bool entry);

  private:
};

class PatternSelect: public pvEditorMode
{
  public:

    PatternSelect();
    virtual void HandleKey(uint32_t keynum, bool pressed);
    virtual void setLEDs(bool entry);

  private:
};

class ChainEdit: public pvEditorMode
{
  public:

    ChainEdit();
    virtual void HandleKey(uint32_t keynum, bool pressed);
    virtual void setLEDs(bool entry);

  private:
};



class UtilityMode: public pvEditorMode
{
  public:

    UtilityMode();
    virtual void HandleKey(uint32_t keynum, bool pressed);
    virtual void setLEDs(bool entry);

  private:

    void doUtilMode(uint32_t keynum, bool pressed);
};

#endif // keepout
