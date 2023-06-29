# CLSerialUtils

### Set of utilities for communication over Euresys camera link serial port to read/write cam registers (parameters).

A project I worked on involved a Basler linescan camera that communicated with framegrabber via Camera Link.  I used a Euresys Grablink framegrabber and Euresys's Multicam API/lib to develop a C++ application for this under Linux.

The Basler camera comes with a few factory installed settings that were not right for my project: Tap Geometry, Exposure Mode and Trigger Mode.  If the camera used CoaXpress then it would be easy to set these parameters 
from the framegrabber API.  But with Camera Link (old technology) you have to use the manufacturer to do any initial setup of the camera.  Basler has a nice app called Pylon to do this, but unfortunately the Linux version
does not support setting up most camera link settings.  So you either need a Windows system (with the framegrabber installed) around to do the setup, or you can setup paramers directly through the camera's registers by sending
serial commands through the camera link interface.  But you need to know a few things first:  1) The camera register information, and 2) how to use your camera link frame grabber to send low-level serial commands.

Luckily, I found the Basler camera info here:

https://www.baslerweb.com/en/sales-support/downloads/software-downloads/basler-binary-protocol-library/

...and Euresys had sample code showing how to read/write data via the camera link serial port.

But this project is what I ended up with:  Two Linux command line utilities (one for reading, one for writing) that each take JSON files ans input, and reads or writes the registers data according to the Basler docs.
