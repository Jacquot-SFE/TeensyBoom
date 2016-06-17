#include <Arduino.h>

#include "player.h"
#include "pattern.h"
#include "editor.h"
#include "voice.h"
#include "voice.h"

// Similar, for the pattern...
extern Pattern      thePattern;
extern PanelScanner theScanner;
extern Editor       theEditor;

// constructor...
Player::Player()
{
  playing = false;
  swing   = false;
  current_step = 0;

  active_mutes = 0;
  pending_mutes = 0;

  active_pattern = 0;
  pending_pattern = -1;

  pause_len = 125;// milliseconds, 125 mS = 120 bpm
}

void Player::start()
{
  playing = true;
  current_step = 0;

  if(chain_active)
  {
    chain_play_idx = 0;
    active_pattern = chain_array[0];
    thePattern.setCurrentPattern(active_pattern);
  }
  
  //theScanner.setOverlayLED(0x17);
  theEditor.forceLEDs();
}

void Player::stop()
{
  playing = false;
  theEditor.forceLEDs();
}

bool Player::isPlaying()
{
  return playing;
}

void Player::setPause(uint32_t millisec)
{
  pause_len = millisec;
}

bool Player::toggleSwing()
{
  swing = !swing;

  Serial.print("Swing is now: ");
  Serial.println(swing);

  return swing;
}

bool Player::getSwing()
{
  return swing;
}

bool Player::toggleMuteBit(uint32_t bit)
{
  if(bit > 12)
    return false;

  if(playing)
  {
    pending_mutes ^= (1 << bit);

    return (pending_mutes & (1 << bit));
  }
  else
  {
    active_mutes ^= (1 << bit);

    return (active_mutes & (1 << bit));
  }
}
 
bool Player::getMuteBit(uint32_t bit)
{
  if(bit > 12)
    return false;

  return (active_mutes & (1 << bit));
}

bool Player::getPendingMuteBit(uint32_t bit)
{
  if(bit > 12)
    return false;

  return (pending_mutes & (1 << bit));
}



bool Player::setNextPattern(int32_t next)
{
  if(next > Pattern::NUM_PATTERNS)
    return false;

  if(next == active_pattern)
  {
    return false;
  }

  if(playing)
  {
    pending_pattern = next;

    return true;// (pending_pattern);
  }
  else
  {
    active_pattern = next;
    thePattern.setCurrentPattern(active_pattern);

    return true;  //(active_mutes & (1 << bit));
  }
}

int32_t Player::getActivePattern()
{
  return active_pattern;
}

int32_t Player::getPendingPattern()
{
  return pending_pattern;
}


int32_t Player::getCurrentStep()
{
  return current_step;
}

bool Player::getBlinkPhase()
{
  uint32_t now = millis();
  
  if(now < (next_time - (pause_len>>1)))
  {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////////
// chain stuff
////////////////////////////////////////////////////////////////

bool  Player::chainIsActive()
{
  return chain_active;
}

void  Player::initChain()
{
  chain_active = false;
  chain_insert_idx = 0;
  chain_play_idx = 0;
}

void Player::addToChain(int32_t patt_num)
{
  if(patt_num >= Pattern::NUM_PATTERNS)
  {
    Serial.println("addToChain patt_num out of bounds!");
    return;
  }

  Serial.print("Adding patt ");
  Serial.print(patt_num);
  Serial.print("to chain at idx ");
  Serial.println(chain_insert_idx);

  if(!chain_active)
  {
    // the first piece added to chain
    chain_active = true;
    chain_array[0] = patt_num;
    chain_insert_idx = 1;
    chain_play_idx = -1;// one before the first step - it'll increment and be at beginning
    theEditor.forceLEDs();
    return;
  }

  if(chain_insert_idx < CHAIN_LEN)
  {
    chain_array[chain_insert_idx] = patt_num;
    chain_insert_idx++;
    theEditor.forceLEDs();
  }

}

int32_t Player::getNextChainVal()
{
  chain_play_idx++;
  if(chain_play_idx >= chain_insert_idx)
  {
    chain_play_idx = 0;
  }

#if 0
  Serial.print("Getting next chain pattern, (idx, val):");
  Serial.print(chain_play_idx);
  Serial.print(" ");
  Serial.println(chain_array[chain_play_idx]);
#endif

  return chain_array[chain_play_idx];
}

bool   Player::checkChainMembership(int32_t patt)
{
  // is patt # active within chain?
  for(int32_t i = 0; i < chain_insert_idx; i++)
  {
    if(patt == chain_array[i])
    {
      //Serial.println("found chain member");
      return true;
    }
  }
  return false;
}

void Player::tick()
{
  uint32_t now = millis();
  
  if(now < next_time)
  {
    return;
  }

  if(!swing)
  {
    next_time = now + pause_len;
  }
  else
  {
    if(!(current_step & 0x01))
    {
      next_time = now + pause_len + (pause_len/3);
    }
    else
    {
      next_time = now + ((2*pause_len)/3);
    }
  }

  if(playing)
  {
    uint32_t trigdata = thePattern.getStepData(current_step);

    // Apply mutes
    trigdata &= (~active_mutes);

    theEditor.forceLEDs();

#if 0
   Serial.print("Trigger: step#");
   Serial.print(current_step);
   Serial.print(" bitmap:");
   Serial.println(trigdata, HEX);
#endif

    AudioNoInterrupts();

    if (trigdata & 0x01)
    {
      triggerKick(trigdata & 0x010000);
    }
    if (trigdata & 0x02)
    {
      triggerSnare(trigdata & 0x020000);
    }
    if (trigdata & 0x04)
    {
      // closed hat trumps open hat.
      triggerHat(false, trigdata & 0x040000);
    }
    else if (trigdata & 0x08)
    {
      triggerHat(true, trigdata & 0x080000);  
    }
    if (trigdata & 0x10)
    {
      triggerTom(1, trigdata & 0x100000);
    }
    else if (trigdata & 0x20)
    {
      triggerTom(2, trigdata & 0x200000);
    } 
    else if (trigdata & 0x40)
    {
      triggerTom(3, trigdata & 0x400000);
    }
    if (trigdata & 0x80)
    {
      triggerBell(trigdata & 0x800000);
    }
    if (trigdata & 0x100)
    {
      triggerShaker(trigdata & 0x1000000);
    }
    if (trigdata & 0x200)
    {
      triggerCymbal(trigdata & 0x2000000);
    }
    if (trigdata & 0x400)
    {
      triggerClap(trigdata & 0x4000000);
    }
    AudioInterrupts();
  }

  current_step++;
  if(current_step >= 0x10)
  {
    // this block of code is setting up the NEXT invocation, so we can change 
    // patterns without overhead before the next set of triggers.
    // It means that come instenal state is ahead of the actual playback state - 
    // the takeaway: don't call theEditor.forceLEDs(), or the display gets out of sync
    
    current_step = 0;
    if(pending_mutes)
    {
      active_mutes ^= pending_mutes;
      pending_mutes = 0;
    }

    if(!chain_active)
    {
      if(pending_pattern != -1)
      {
        active_pattern = pending_pattern;
        thePattern.setCurrentPattern(pending_pattern);
        pending_pattern = -1;
      }
      //else, just keep current pattern...
    }
    else//chain_active
    {
      active_pattern = getNextChainVal();
      thePattern.setCurrentPattern(active_pattern);
    }

    // do this after the LED updates, or they get the wrong current step!
    current_step = 0;
  }
}

