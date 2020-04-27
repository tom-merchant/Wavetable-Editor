#About
This is a GUI application for editing wavetables for use with my Oscillator library.

![GUI Preview](https://i.imgur.com/nSp7n2o.png)

The interface is mostly self explanatory. In the top middle it has a preview of the wave shape,
below that an interface for editing the harmonics present in the current wavetable.
Wavetables can be added, selected, deleted and individually saved on the left.

On the right there is the interface for creating spectrally morphed wavetables. There are two play buttons below the oscilloscope,
the left hand one is for previewing the current wavetable and the right hand one is for previewing the
morph wavetable.

There are buttons for zooming in on the harmonics for fine control as well as generating harmonics
from a javascript expression. There are also presets for 4 basic wave shapes (sine, square, ramp and triangle).

The top right graphic is a 3D surface of the current state of the morph wavetable.

##Building

In order to build this project you will need a copy of muJS placed in the source tree, 
you will need my Oscillator, Envelope and AudioUtils libraries placed in the parent directory. You will also need
the gtkmm-3.0, fftw3, OpenGL, readline, and PortAudio headers.

Once you have these you should be able to use cmake to generate a build script for your build environment.