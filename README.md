# laser_tag
As a Junior Core cumulative project, I built a laser tag system with a partner that implements concepts from various topics within electrical and computer engineering such as circuitry design, signals and systems, and embedded system programming.

The system functions in the following manner:

Photo-sensors pick up signals transmitted at a certain frequency from other players' shooting LED. The captured signal goes through an analog amplifier that amplifies the signal 5000x in magnitude. The magnified signal goes though a series of filters. First, a low-pass filter to cut out any unwanted frequencies to eliminate aliasing signals. Second, 10 bandpass filters that test the input frequency for any one of the 10 possible player frequencies. If the input signal falls into one of the 10 bandpass filters, then a hit is detected. There is a detection algorithm that compares the output of the 10 bandpass filters. If one of the frequencies is marginally greater than the meadian value, hit is detected and the person shot is disabled for a half second (they cannot shoot). The system includes state machines for the trigger, transmitter, and hit detected LED that run on the isr timer. Sound is also implemented so that when shots are detected, reload is executed, or game over events occur (along with a variety of other possible events in the game), an associated sound file is executed. 

Code is downloaded onto an SD card and serves as memory on a microprocessor board. Speaker, battery pack, and laser tag gun are attached to the pack. Photo-sensors are arrayed on the straps of the pack that serve as input to the system. 

![image of laser tag pack](laser_tag_pack.png)

As an extension to the project, we switched out the gun for a glove that we built, similar to an Iron Man glove. 
Here's a link to a video that shows the implemented design (including the glove):

https://www.youtube.com/watch?app=desktop&v=Q7QozufEk70&feature=youtu.be
