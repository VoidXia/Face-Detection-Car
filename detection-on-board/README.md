# Human Face Tracking Robot Car - Detection on board with OpenCV

This folder contains the source code for a human face tracking robot car.

In prior, user should install wiringPi library.

## Build OpenCV

Since there is no official build for Debian OS, we build OpenCV from source and installed all the dynamic linked libraries that this program might need. The OpenCV 4.5.5 source code is downloaded from [here](https://github.com/opencv/opencv/archive/4.5.5.zip). This process usually takes up to 6 hours, depending on machine.

## Build facedetection library

Then, we need to compile and link the `facedetectcnn.h` library. The source code for this module is provided in this [repository](https://github.com/ShiqiYu/libfacedetection). 

In this repository, make a new directory called `build`, `cd` into it, and then run `cmake .. && make`. This builds up the necessary libraries for the face detection part.

## Modifying the `CMakeList.txt`

In this part, user need to modify the `CMakeList.txt` file to link the `facedetectcnn.h` library correctly. Specifically, the path `/home/pi/car/libfacedetection/build/install/lib/libfacedetection.so` must be subsituted for the actual path for the abovementioned face detection library.

## Build and Run!

Finally, we can build and run this program. Same as before, `cmake . && make` should do the job, and you will find the executable `tracking` in the current directory. Note that it might not be so responsible, as described in the report. But it can follow faces and bodies in a specific strategy purely on board, without an Android phone.

## Revised version

We came up a revised version that leverages Simulink and an Android phone. See [here](../detection-on-Android-phone/readme.md).