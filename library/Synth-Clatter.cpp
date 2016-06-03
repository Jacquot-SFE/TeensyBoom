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

#include "Synth-Clatter.h"



void AudioSynthClatter::update(void)
{
  audio_block_t *out_block_p, *bell_block_p;
  int16_t *p_wave, *p_bell, *end;
  //bool a, b;

  out_block_p = allocate();
  if (!out_block_p) 
  {
    return;
  }
  p_wave = (out_block_p->data);
  end = p_wave + AUDIO_BLOCK_SAMPLES;

  bell_block_p = allocate();
  if(bell_block_p)
  {
    p_bell = bell_block_p->data;
  }

  while(p_wave < end)
  {
    count++;

    for(uint32_t i = 0; i < 6; i++)
    {
      if(count == next_trip[i])
      {
#if 1
        //808
        values[i] ^= 0x0800;
#else        
        // synbal
        values[i] ^= 0x01;
#endif        
        next_trip[i] += half_waves[i];
      }
    }

#if 1
    // additive from TR808
    *p_wave = values[0] + values[1] + values[2] - values[3] - values[4] - values[5];

    if(bell_block_p)
    {
      *p_bell++ = values[0] + values[1];
    }
#else
    // cascaded XOR from Cynbal...
    // easy 3-bit xor: add the three values.  LSB reflects the xor of the values.
    a = (values[0] + values[1] + values[2]) & 0x01;
    b = (values[3] + values[4] + values[5]) & 0x01;
    *p_wave = ((a * 0x4000) + (b * 0x4000 )) - 0x4000;
#endif

    p_wave++;
    
  }

  if(bell_block_p)
  {
    transmit(bell_block_p, 1);
    release(bell_block_p);
  }

  transmit(out_block_p, 0);
  release(out_block_p);
  
}

