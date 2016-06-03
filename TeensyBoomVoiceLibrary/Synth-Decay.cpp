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

#include "Synth-Decay.h"


void AudioSynthDecay::noteOn(int16_t topval)
{
  //Serial.println("Decay noteOn");
//  __disable_irq();

  env_lin_current = (topval << 16);
  
//  __enable_irq();
}

void AudioSynthDecay::update(void)
{
  //Serial.println("Decay update");
  
  audio_block_t *block_env;
  int16_t *p_env, *end;

  block_env = allocate();
  if (!block_env) return;
  p_env = (block_env->data);
  end = p_env + AUDIO_BLOCK_SAMPLES;

  while(p_env < end)
  {
    // Do envelope first
    if(env_lin_current < 0x0000ffff)
    {
      *p_env = 0;
    }
    else
    {
      env_lin_current -= env_decrement;
      env_sqr_current = multiply_16tx16t(env_lin_current, env_lin_current) ;
      *p_env = env_sqr_current>>15;
    } 

    p_env++;
  }

  transmit(block_env);
  release(block_env);
}

