#ifndef _PLAYER_H_
#define _PLAYER_H_

#pragma once

class Player
{
public: 
  Player();

  void start();
  void stop();

  bool isPlaying();

  void setPause(uint32_t millisec);

  bool toggleSwing();
  bool getSwing();

  bool toggleMuteBit(uint32_t bit);
  bool getMuteBit(uint32_t bit);
  bool getPendingMuteBit(uint32_t bit);

  bool setNextPattern(int32_t next);
  int32_t getActivePattern();
  int32_t getPendingPattern();

  int32_t getCurrentStep();
  bool    getBlinkPhase();

  void tick();

  static const uint32_t CHAIN_LEN = 32;

  // Chain Stuff
  bool    chainIsActive();
  void    initChain();
  void    addToChain(int32_t patt_num);
  int32_t getNextChainVal();
  bool    checkChainMembership(int32_t patt);

private:

  bool    playing;
  bool    swing;
  int32_t current_step;
  int32_t prev_step;
  int32_t pause_len;
  int32_t next_time;

  int32_t active_mutes;
  int32_t pending_mutes;

  int32_t active_pattern;
  int32_t pending_pattern;

  bool    chain_active;
  int32_t chain_len;
  int32_t chain_insert_idx;
  int32_t chain_play_idx;
  int8_t  chain_array[CHAIN_LEN];

};



#endif // keepout
