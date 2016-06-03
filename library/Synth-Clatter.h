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

#ifndef synth_clatter_h_
#define synth_clatter_h_
#include "AudioStream.h"
#include "utility/dspinst.h"

//#define SAMPLES_PER_MSEC (AUDIO_SAMPLE_RATE_EXACT/1000.0)

class AudioSynthClatter : public AudioStream
{
public:

  AudioSynthClatter() : AudioStream(2, inputQueueArray) 
  {
    next_trip[0] = half_waves[0] = (AUDIO_SAMPLE_RATE_EXACT/(800*2));
    next_trip[1] = half_waves[1] = (AUDIO_SAMPLE_RATE_EXACT/(540*2));
    next_trip[2] = half_waves[2] = (AUDIO_SAMPLE_RATE_EXACT/(522*2));
    next_trip[3] = half_waves[3] = (AUDIO_SAMPLE_RATE_EXACT/(304*2));
    next_trip[4] = half_waves[4] = (AUDIO_SAMPLE_RATE_EXACT/(369*2));
    next_trip[5] = half_waves[5] = (AUDIO_SAMPLE_RATE_EXACT/(205*2));
  }

  using AudioStream::release;
  virtual void update(void);

  // public for debug...
  uint32_t count;
  uint32_t next_trip[6];  
  uint32_t values[6];

private:
  audio_block_t *inputQueueArray[2];

  // Denominators are values calculated from CYNBAL values
  uint32_t half_waves[6];// = {110,125,140,155,189,202};
                                            
};

#endif

