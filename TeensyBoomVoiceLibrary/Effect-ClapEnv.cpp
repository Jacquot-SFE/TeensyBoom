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

#include "Effect-ClapEnv.h"


void AudioEffectClapEnvelope::noteOn(void)
{
  __disable_irq();

  current_splat = 0x7fff0000;
  state = 0;
  
  __enable_irq();
}

void AudioEffectClapEnvelope::update(void)
{
  audio_block_t *block;
  uint32_t *p, *end;
  uint32_t remaining, mul, sample12, tmp1, tmp2;
  int32_t  loc_incr;

  block = receiveWritable();
  if (!block) return;

  p = (uint32_t *)(block->data);
  end = p + AUDIO_BLOCK_SAMPLES/2;

recheck:
  switch(state)
  {
    case 7:
    {
      current_splat = 0;
      while (p < end) 
      {
        *p = pack_16t_16t(current_splat, current_splat);
        p++;
      }
    }
    break;

    case 6:
    { 
      loc_incr = even_increment/3;
      goto calc;
    }
    break;
    case 1:
    case 3:
    case 5:
    {
      loc_incr = odd_increment;

      while(p < end)
      {
        current_splat -= loc_incr;
        current_splat -= loc_incr;

        *p = 0;
        p++;

        if(current_splat < 0x0000ffff)
        {
          state++;
          current_splat = 0x7fff0000;
          goto recheck;
        }
      }
    }
    break;
    case 0:
    case 2:
    case 4:
    {
      loc_incr = even_increment;
calc:     
      while(p < end)
      {
        sample12 = *p;
        current_splat -= loc_incr;
        mul = multiply_16tx16t(current_splat, current_splat) ;
        tmp1 = multiply_16tx16b(mul<<1, sample12);
        
        current_splat -= loc_incr;
        mul = multiply_16tx16t(current_splat, current_splat) ;
        tmp2 = multiply_16tx16t(mul<<1, sample12);
        sample12 = pack_16t_16t(tmp2, tmp1);
        *p = sample12;
        p++;

        if(current_splat < 0x0000ffff)
        {
          state++;
          current_splat = 0x7fff0000;
          goto recheck;
        }
      }
    }
  }
  transmit(block);
  release(block);

}

