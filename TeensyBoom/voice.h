#ifndef _VOICE_H_
#define _VOICE_H_

#pragma once

#include <stdint.h>
#include <Audio.h>

void voiceInit();

void paramInit();

void paramUpdate1();
void paramUpdate2();
void paramUpdate3();

void triggerKick(bool loud = true);
void triggerSnare(bool loud = true);
void triggerTom(int32_t num, bool loud = true);
void triggerShaker(bool loud = true);
void triggerHat(bool open, bool loud = true);
void triggerBell(bool loud = true);
void triggerCymbal(bool loud = true);
void triggerClap(bool loud = true);

#endif // keepout
