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

#include "Synth-DrumHeart.h"


// waveforms.c
extern "C" {
extern const int16_t AudioWaveformSine[257];
}


//static bool trap;

void AudioSynthDrumHeart::noteOn(void)
{
  __disable_irq();

  wav_phasor = 0;
  wav_phasor2 = 0;

  env_lin_current = 0x7fff0000;
  
  __enable_irq();
}

void AudioSynthDrumHeart::secondMix(float level)
{
  // As level goes from 0.0 to 1.0,
  // second goes from 0 to 1/2 scale
  // first goes from full scale to half scale. 

  if(level < 0)
  {
    level = 0;
  }
  else if(level > 1.0)
  {
    level = 1.0;
  }

  __disable_irq();
  wav_amplitude2 = level * 0x3fff;
  wav_amplitude1 = 0x7fff - wav_amplitude2;
  __enable_irq();
}


void AudioSynthDrumHeart::pitchMod(float depth)
{
  int32_t intdepth, calc;

  // Validate parameter
  if(depth < 0)
  {
    depth = 0;
  }
  else if(depth > 1.0)
  {
    depth = 1.0;
  }

  // Depth is float, 0.0..1.0
  // turn 0.0 to 1.0 into
  // 0x0 to 0x3fff;
  intdepth = depth * 0x7fff;

  // Lets turn it into 2.14, in range between -0.75 and 2.9999, woth 0 at 0.5
  // It becomes the scalar for the modulation component of the phasor increment.
  if(intdepth < 0x4000)
  {
    // 0 to 0.5 becomes
    // -0x3000 (0xffffCfff) to 0 ()
    calc = ((0x4000 - intdepth) * 0x3000 )>> 14;
    calc = -calc;
  }
  else
  {
    // 0.5 to 1.0 becomes
    // 0x00 to 0xbfa0
    calc = ((intdepth - 0x4000) * 0xc000)>> 14;
  }

  // Call result 2.14 format (max of ~3.99...approx 4)
  // See note in update().
  wav_pitch_mod = calc;
}


void AudioSynthDrumHeart::update(void)
{
  audio_block_t *block_wav, *block_env;
  int16_t *p_wave, *p_env, *end;
  int32_t sin_l, sin_r, interp, mod, mod2, delta;
  int32_t interp2;
  int32_t index, scale;
  bool do_second;

  block_env = allocate();
  if (!block_env) return;
  p_env = (block_env->data);
  end = p_env + AUDIO_BLOCK_SAMPLES;

  block_wav = allocate();
  if (!block_wav) return;
  p_wave = (block_wav->data);

  do_second =  (wav_amplitude2 > 50);

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


    // then do waveforms 
    wav_phasor  += wav_increment;

    // modulation changes how we use the increment
    // the increment will be scaled by the modulation amount.

    // Don't put data in the sign bits unless you mean it!
    mod = signed_multiply_32x16b((env_sqr_current), (wav_pitch_mod>>1)) >> 13;
    mod2 = signed_multiply_32x16b(wav_increment<<3, mod>>1);
#if   0
    if(!trap)
    {
      Serial.print(wav_increment, HEX);
      Serial.print(' ');
      Serial.print(mod, HEX);
      Serial.print(' ');
      Serial.println(mod2, HEX);
      trap = true;
    }
#endif
    
    wav_phasor += (mod2);
    wav_phasor &= 0x7fffffff;

    if(do_second)
    {
      // For a perfect fifth above, the second phasor increment
      // should be 1.5X the base.
      wav_phasor2 += wav_increment;
      wav_phasor2 += (wav_increment >> 1);
      wav_phasor2 += mod2;
      wav_phasor2 += (mod2 >> 1);
      wav_phasor2 &= 0x7fffffff;
    }

    switch(wav_shape)
    {
      case SINE:
        index = wav_phasor >> 23; // take top valid 8 bits
        sin_l = AudioWaveformSine[index];
        sin_r = AudioWaveformSine[index+1];

        //scale = (wav_phasor >> 7) & 0xFFFF;
        //sin_r *= scale;
        //sin_l *= 0xFFFF - scale;
        //interp = (sin_l + sin_r) >> 16;
        delta = sin_r - sin_l;
        scale = (wav_phasor >> 7) & 0xFFFF;
        delta = (delta * scale)>>16;
        interp = sin_l + delta;

        if(do_second)
        {
          index = wav_phasor2 >> 23; // take top valid 8 bits
          sin_l = AudioWaveformSine[index];
          sin_r = AudioWaveformSine[index+1];

          delta = sin_r - sin_l;
          scale = (wav_phasor2 >> 7) & 0xFFFF;
          delta = (delta * scale)>>16;
          interp2 = sin_l + delta;
        }
        else
          interp2 = 0;
      break;
      case TRIANGLE:
      {
        int8_t phaz = wav_phasor >> 29;
        if(phaz == 0)
        {
          interp = wav_phasor >> 14;
        }
        else if(phaz == 0x01)
        {
          interp =  0x8000l - ((wav_phasor >> 14) - 0x8000l);
        }
        else if(phaz == 0x02)
        {
          interp = - (wav_phasor >> 14);
        }
        else // == 0x03
        {
          interp = (wav_phasor >> 14);          
        }
      }
      break;
      case SAW:
        interp = wav_phasor >> 15;
      break;
      case SQUARE:
        if(wav_phasor & 0x40000000)
          interp = 0x7fff;
        else
          interp = 0x8000;
      break;
    }
    if(do_second)
    {
      // Then scale and add the two waves
      interp2 = (interp2 * wav_amplitude2 ) >> 15;
      interp = (interp * wav_amplitude1) >> 15;
      interp = interp + interp2;
    }
    *p_wave = interp ;

 
    p_env++;
    p_wave++;
 }

  transmit(block_env, 1);
  release(block_env);
  
  transmit(block_wav, 0);
  release(block_wav);

}

