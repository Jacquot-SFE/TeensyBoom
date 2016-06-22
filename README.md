TEENSYBOOM
---
Teensy 3.2 analog emulation drum machine.

# Organization

Supposedly all of the design artifacts will wind up in here.

If it's not here, go check the ol' [synth-drum]() repo.  It's a mess, be warned.

* **/chassis/** files for the cheek blocks & control panel
* **/TeensyBoomVoiceLibrary/** source files for the Teensy-audio-based library
* **/TeensyBoom/** the sketch files for the sequencer and stuff
* **/demo-patterns/** Some pattern files captured after playing with the sequencer a bit.  For use when you get sick of the default pattern.
* **/Documentation/** ha ha.  

## Setup & Configuration

Developed using Arduino IDE 1.6.5.  Last time I tried a newer built, it rebuilt everything everytime I clicked 'go.'

Loading using Teensy Loader 1.28.

### Installation:

Copy the _TeensyVoiceLibrary_ files to your local Arduino library directory (for me , it's C:\Users\redacted\Documents\Arduino\libraries\TeensyBoomVoiceLibrary).

Remove or rename the AudioSynthSimpleDrum in the teensy audio directory (C:\Users\byron.jacquot\Documents\Arduino\libraries\Audio\synth_simple_drum.cpp and .h).  The Boom library contains a newer version.

### Compile and Load:

I've been building for 72 MHz, non-optimized, with the assumption that we can always bump up to optimization & overclocking if we run out of resources.  It's harder to go the other way!




License Information
-------------------

This product is _**open source**_!

Please review the LICENSE.md file for license information.

Distributed as-is; no warranty is given.

It heavily leverages all of the infrastructure developed by [PJRC.com](PJRC.com).  Please support them by purchasing genuine Teensy components!  _Alibaba Teensys aren't Teensies!_
