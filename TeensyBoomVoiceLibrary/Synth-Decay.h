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

#ifndef synth_decay_h_
#define synth_decay_h_

#include "AudioStream.h"
#include "utility/dspinst.h"

//#define SAMPLES_PER_MSEC (AUDIO_SAMPLE_RATE_EXACT/1000.0)

class AudioSynthDecay : public AudioStream
{
public:

  AudioSynthDecay() : AudioStream(0, NULL) // 0 for No inputs
  {
    env_lin_current = 0;
    env_sqr_current = 0;
    //length(1000);
  }
  void noteOn(int16_t topval = 0x7fff);

  void length(int32_t milliseconds)
  {
    //Serial.println("decay length");
    
    if(milliseconds < 0)
      return;
    else if(milliseconds == 0) // avoid div by 0.
      milliseconds = 1;
    else if(milliseconds > 5000) // arbitrary limit...
      milliseconds = 5000;

    int32_t len_samples = milliseconds*(AUDIO_SAMPLE_RATE_EXACT/1000.0);

//    __disable_irq();    

    env_decrement = (0x7fff0000/len_samples);
    
//    __enable_irq();    
  };

  using AudioStream::release;
  virtual void update(void);

  // public for debug...
  // Envelope params
  int32_t env_lin_current; // present value of linear slope.
  int32_t env_sqr_current; // the square of the linear value - quasi exponential..
  int32_t env_decrement;   // how each sample deviates from previous.

private:
  //audio_block_t *inputQueueArray[1];

};

#undef SAMPLES_PER_MSEC
#endif

