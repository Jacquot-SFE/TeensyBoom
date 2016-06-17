/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef effect_clap_env_h_
#define effect_clap_env_h_
#include "AudioStream.h"
#include "utility/dspinst.h"

//
//This thing is kinda tangled.  I just spent a day re-reverse rengineering what
//I'd done.
//
//This module generates a clap envelope, emulating what a Tama Techstar was diung
//(which is akin to the classic Roland clap - very similar circuits).  It looks like
//this (4 shark fins, with small gaps between):
//
//|\   |\   |\   |\_
//| \  | \  | \  |  \_
//|  \_|  \_|  \_|    \___
// A  B C  D E  F  G    H
//
//A,C,E are the "splat" decay times.
//B,D,F are silent gaps in between.
//G is a slower decay, 3x the other splats
//H is the end of the cycle.
//
//It's controlled with a state machine.  In the audible (A, C, E, G) phases, it's
//decrementing the q1.31 phase accumulator, and applying it to the input.
//In the silent phases, it's generating zeros
//for the appropriate number of milliseconds.
//
//For Roland/Tama-type emulation, this is applied directly to white noise.
//It's assumed that there's a decay-only envelope that also passes some BP filtered
//white noise in parallel (the "Reverb" portion of the sound).
//
//The decay and gap times are configurable.  8mS decay, 2 mS gap is pretty good...
//With a 250 mS decay for the reverb component.

class AudioEffectClapEnvelope : public AudioStream
{
public:
  AudioEffectClapEnvelope() : AudioStream(1, inputQueueArray) {
    splatLength(8);
    gapLength(2);

    current_splat = 0;
    state = 7;
  }
  void noteOn();

  void splatLength(int32_t milliseconds)
  {
    if(milliseconds < 0)
      return;
    if(milliseconds > 5000)
      milliseconds = 5000;

    int32_t len_samples = milliseconds*(AUDIO_SAMPLE_RATE_EXACT/1000.0);

    even_increment = (0x7fff0000/len_samples);
  };
  void gapLength(int32_t milliseconds)
  {
    if(milliseconds < 0)
      return;
    if(milliseconds > 5000)
      milliseconds = 5000;

    int32_t len_samples = milliseconds*(AUDIO_SAMPLE_RATE_EXACT/1000.0);

    odd_increment = (0x7fff0000/len_samples);
  };

  using AudioStream::release;
  virtual void update(void);


private:
  audio_block_t *inputQueueArray[1];

  int32_t current_splat; // present value
  uint32_t state;
  int32_t odd_increment; // (actually, decrement, how each sample deviates from previous.)
  int32_t even_increment; // (actually, decrement, how each sample deviates from previous.)

};

#undef SAMPLES_PER_MSEC
#endif
