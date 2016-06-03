#include "voice.h"

#include <TeensyBoomVoiceLibrary.h>

//#include "Synth-Clatter.h"
//#include "Synth-Decay.h"
//#include "Synth-DrumHeart.h"
//#include "synth_simple_drum.h"

#include "player.h"  // required so we can update tempo param from knob update func.

extern Player thePlayer;

#define HAT
#define KICK
#define SNARE
#define TOM
#define SHAKER
#define BELL
#define CYMBAL

// Used by multiple instruments:
AudioSynthNoiseWhite   noise;

#ifdef HAT
// hats
AudioSynthClatter      clat1;
AudioSynthDecay        hatdecay;
AudioFilterBiquad      hatfilter;
AudioEffectMultiply    hatmult;
#endif

#ifdef BELL
AudioSynthDecay        belldecay;
AudioEffectMultiply    bellmult;
#endif

#ifdef KICK
// kick
AudioSynthSimpleDrum     kick;
#endif

#ifdef SNARE
// snare
AudioSynthDrumHeart    snare;
AudioMixer4            snaremix;
AudioEffectMultiply    snaremult;
#endif

#ifdef TOM
// tom
AudioSynthSimpleDrum   tom;
#endif

#ifdef SHAKER
// shaker
AudioSynthDecay        shakedecay;
AudioFilterBiquad      shakefilter;
AudioEffectMultiply    shakemult;
#endif

#ifdef CYMBAL
AudioSynthDecay        cymbaldecay;
AudioFilterBiquad      cymbalfilter;
AudioEffectMultiply    cymbalmult;
#endif

// outputs
AudioMixer4            mixer1;
AudioMixer4            mixer2;

AudioOutputI2S           i2s1;           //xy=968,448

#ifdef HAT
AudioConnection          patchCord01(clat1, 0, hatfilter, 0);
AudioConnection          patchCord02(hatfilter, 0, hatmult, 0);
//AudioConnection          patchCord03(hatfilter, 0, hatmult, 0);
//AudioConnection          patchCord01(hatdecay, 0, hatmult, 0);
AudioConnection          patchCord03(hatdecay, 0, hatmult, 1);
AudioConnection          patchCord90(hatmult, 0, mixer1, 0);
#endif

#ifdef BELL
AudioConnection          patchCord04(clat1, 1, bellmult, 0);
AudioConnection          patchCord05(belldecay, 0, bellmult, 1);
AudioConnection          patchCord96(bellmult, 0,  mixer2, 2);
#endif

#ifdef KICK
AudioConnection          patchCord10(kick, 0, mixer1, 1);
#endif

#ifdef SNARE
AudioConnection          patchCord20(noise, 0, snaremix, 0);
AudioConnection          patchCord21(snare, 0, snaremix, 1);
AudioConnection          patchCord22(snare, 1, snaremult, 1);
AudioConnection          patchCord23(snaremix, 0, snaremult, 0);
AudioConnection          patchCord92(snaremult, 0, mixer1, 2);
#endif

#ifdef TOM
AudioConnection          patchCord30(tom, 0, mixer2, 0);
#endif

#ifdef SHAKER
AudioConnection          patchCord40(noise, 0, shakefilter, 0);
AudioConnection          patchCord41(shakefilter, 0, shakemult, 0);
AudioConnection          patchCord42(shakedecay, 0, shakemult, 1);
AudioConnection          patchCord95(shakemult, 0, mixer2, 1);
#endif

#ifdef CYMBAL
AudioConnection          patchCord50(clat1, 0, cymbalfilter, 0);
AudioConnection          patchCord51(cymbalfilter, cymbalmult);
AudioConnection          patchCord52(cymbaldecay, 0, cymbalmult, 1);
AudioConnection          patchCord97(cymbalmult, 0, mixer2, 3);
#endif

//AudioConnection          patchCord96(noise, 0, mixer2, 2);

AudioConnection          patchCord93(mixer2, 0, mixer1, 3);

AudioConnection          patchCord98(mixer1, 0, i2s1, 0);
AudioConnection          patchCord99(mixer1, 0, i2s1, 1);

AudioControlSGTL5000     sgtl5000_1;




// Globals for params on shared voices
uint16_t openlen, closedlen;
uint16_t t1, t2, t3;

void voiceInit()
{
  AudioNoInterrupts();

  //decay1.length(1000);

  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);
  sgtl5000_1.lineOutLevel(16);

  paramInit();

  AudioInterrupts();

}

void paramInit()
{
  // common to severl instruments
  noise.amplitude(0.5);
  
#ifdef HAT
  // hat stuff
  //clat1; - no params
  //hatdecay.length(150);
  openlen = 300;
  closedlen = 50;
  hatfilter.setHighpass(0, 700, 0.2);
  hatfilter.setLowpass(1, 10000, 0.8);

  //hatdecay.frequency(1500);
  //hatdecay.length(50);
#endif

#ifdef BELL
  belldecay.length(333);
#endif

#ifdef KICK
  // kick
  kick.frequency(60);
  kick.length(100);
  kick.pitchMod(0x2f0); // 0x200 is no mod...
#endif

#ifdef SNARE
  // snare
  snare.frequency(200);
  snare.length(100);
  snare.second(true);
  snare.pitchMod(0x280);
  snaremix.gain(0, 0.75);
  snaremix.gain(1, 0.5);
#endif

#ifdef TOM
  // tom
  //tom.frequency(80);
  t1 = 60;
  t2 = 90;
  t3 = 135;
  tom.secondMix(1.0);
  tom.length(250);
  tom.pitchMod(0.75);
#endif

#ifdef SHAKER  
  // shaker
  shakefilter.setLowpass(0, 3500, 0.7);
  shakefilter.setHighpass(1, 400, 0.3);
  shakedecay.length(50);
#endif

#ifdef CYMBAL
  cymbalfilter.setLowpass(0, 4500, 0.3);
  cymbalfilter.setHighpass(1, 770, 0.7);
  cymbaldecay.length(1000);
#endif

  // Master
  mixer1.gain(0, 0.75);// hat
  mixer1.gain(1, 0.75);// kik
  mixer1.gain(2, 0.65);// snr
  mixer1.gain(3, 0.75);// mix2
  mixer2.gain(0, 0.75);// tom
  mixer2.gain(1, 0.3);//shaker
  mixer2.gain(2, 0.5);// bell
  mixer2.gain(3, 0.5);// cymbal
}

void paramUpdate1()
{
  uint16_t   pitch, mod, len;
  uint16_t   snpitch, snmix, snlen;
  uint16_t   ohdec, chdec;

  pitch = analogRead(A1);
  mod = analogRead(A12);
  len = analogRead(A13);

  snpitch = analogRead(A2);
  snmix = analogRead(A3);
  snlen = analogRead(A14);

  ohdec = analogRead(A6);
  chdec = analogRead(A16);

#ifdef KICK
  kick.frequency(30 + (pitch >> 3));
  kick.pitchMod((float)mod/0x3ff);
  kick.length(len + 10);
#endif
#ifdef SNARE
  float mix = (float)snmix / 1024.0;
  snare.frequency(80 + (snpitch >> 2));
  snaremix.gain(0, 1.0 - mix);
  snaremix.gain(1, mix);
  snare.length((snlen/2) + 10);
#endif
#ifdef HAT
  openlen = (ohdec/2) + 10;
  closedlen = (chdec/2) + 10;
#endif  
}
#if 1

void paramUpdate2()
{
  uint16_t   p1, p2, p3;
  uint16_t  len, mod;
  uint16_t  secondskin;
  //uint16_t  slen;

  p1 = analogRead(A7);
  p2 = analogRead(A10);
  p3 = analogRead(A11);

  len = analogRead(A17);
  mod = analogRead(A18);
  secondskin = analogRead(A19);

  //slen = analogRead(A11);

  t1 = 30 + (p1 >> 1);
  t2 = 30 + (p2 >> 1);
  t3 = 30 + (p3 >> 1);

  tom.length(len + 10);
  tom.pitchMod((float)mod/0x3ff);
  tom.secondMix((float)secondskin/0x3ff);

  //shakedecay.length(slen + 10);
}
#endif


void paramUpdate3()
{
  uint16_t volume;
  uint32_t tempo;

  volume = analogRead(A20);

  sgtl5000_1.volume(((float)volume)/0x3ff);

  tempo = analogRead(A15);

  tempo = 0x3ff - tempo;

  tempo *= 225;
  tempo >>= 10;
  tempo &= 0x3ff;

  //pause = tempo + 75;
  thePlayer.setPause(tempo+75);
}



void triggerKick(bool loud)
{
  if(loud)
  {
    kick.noteOn();
  }
  else
  {
    kick.noteOn(0x6000);
  }
}

void triggerSnare(bool loud)
{
  if(loud)
  {
    snare.noteOn();
  }
  else
  {
    snare.noteOn(0x5000);
  }
}

void triggerTom(int32_t num, bool loud)
{
  if (num == 1)
  {
    tom.frequency(t1);
    tom.noteOn(loud?0x7fff:0x6000);
  }
  else if (num == 2)
  {
    tom.frequency(t2);
    tom.noteOn(loud?0x7fff:0x6000);
  }
  else if (num == 3)
  {
    tom.frequency(t3);
    tom.noteOn(loud?0x7fff:0x6000);
  }
}


void triggerShaker(bool loud)
{
  shakedecay.noteOn(loud?0x7fff:0x6000);
}

void triggerHat(bool open, bool loud)
{
  if (!open)
  {
    hatdecay.length(closedlen);
    hatdecay.noteOn(loud?0x7fff:0x6000);
  }
  else
  {
    hatdecay.length(openlen);
    hatdecay.noteOn(loud?0x7fff:0x6000);
  }
  
}


void triggerBell(bool loud)
{
  belldecay.noteOn(loud?0x7fff:0x6000);
}


void triggerCymbal(bool loud)
{
  Serial.println("cymbal");
  
  cymbaldecay.noteOn(loud?0x7fff:0x6000);
}

