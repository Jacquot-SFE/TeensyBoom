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


#define SECOND

// waveforms.c
extern "C" {
extern const int16_t AudioWaveformSine[257];
}


//static bool trap;

void AudioSynthDrumHeart::noteOn(int16_t topval)
{
  //__disable_irq();

  //if(env_lin_current < 0x0ffff)
  {
    wav_phasor = 0;
    wav_phasor2 = 0;
  }

  env_lin_current = (topval << 16);
  
  //trap = false;
  
  //__enable_irq();
}

void AudioSynthDrumHeart::pitchMod(int32_t depth)
{
  int32_t calc;

  // Depth is 10-bit ADC value
  // call it 1.9 fixed pt fmt
  // Lets turn it into 2.14, in range between -0.75 and ~2.9999
  // It becomes the scalar for the modulation component of the phasor increment.
  if(depth < 0x200)
  {
    // 0 to 0x200 becomes
    // -0x3000 (AKA 0xffffCfff) to 0 ()

    // TBD - do I have a sign/scaling problem?  do I need signed 3.13?
    calc = ((0x200 - depth) * 0x3000 )>> 9;
    calc = -calc;
  }
  else
  {
    // 0x200 to 0x3ff becomes
    // 0x00 to 0xbfa0

    // Again - mind the sign bit better??
    calc = ((depth - 0x200) * 0xc000)>> 9;
  }

#if   0
  // can't do this if called by global c'tor: Serial isn't there until 
  // setup() initializes it.  
  Serial.print("calc: ");
  Serial.println(calc, HEX);

  // Call result 2.14 format (max of ~3.99...approx 4)
#endif  

  wav_pitch_mod = calc;
}



void AudioSynthDrumHeart::update(void)
{
  audio_block_t *block_wav, *block_env;
  int16_t *p_wave, *p_env, *end;
  int32_t sin_l, sin_r, interp, mod, mod2;
  int32_t interp2;
  uint32_t index, scale;


  block_env = allocate();
  if (!block_env) return;
  p_env = (block_env->data);
  end = p_env + AUDIO_BLOCK_SAMPLES;

  block_wav = allocate();
  if (!block_wav) return;
  p_wave = (block_wav->data);

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

    // do wave second;
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

#ifdef SECOND
    if(wav_second)
    {
      // For a perfect fifth above, the second phasor increment
      // should be 1.5X the base.
#if 1
      wav_phasor2 += wav_increment;
      wav_phasor2 += (wav_increment >> 1);
      wav_phasor2 += mod2;
      wav_phasor2 += (mod2 >> 1);
      wav_phasor2 &= 0x7fffffff;
#endif      
    }
#endif    
    // then interp'd waveshape
    switch(wav_shape)
    {
      case SINE:
        index = wav_phasor >> 23; // take top valid 8 bits
        sin_l = AudioWaveformSine[index];
        sin_r = AudioWaveformSine[index+1];

        scale = (wav_phasor >> 7) & 0xFFFF;
        sin_r *= scale;
        sin_l *= 0xFFFF - scale;
        interp = (sin_l + sin_r) >> 16;
#ifdef SECOND
        if(wav_second)
        {
#if 1          
          index = wav_phasor2 >> 23; // take top valid 8 bits
          sin_l = AudioWaveformSine[index];
          sin_r = AudioWaveformSine[index+1];

          scale = (wav_phasor2 >> 7) & 0xFFFF;
          sin_r *= scale;
          sin_l *= 0xFFFF - scale;
          interp2 = (sin_l + sin_r) >> 16;
#endif          
        }
#endif        
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
#ifdef SECOND
    if(wav_second)
    {
      *p_wave = (interp + interp2) >> 1;
    }
    else
#endif    
    {
      *p_wave = interp ;
    }

    p_env++;
    p_wave++;
 }

  transmit(block_env, 1);
  release(block_env);
  
  transmit(block_wav, 0);
  release(block_wav);

}

