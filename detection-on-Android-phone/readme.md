# Human Face Tracking Robot Car - Detection on Android Phone

This folder contains the source code for a human face tracking robot car. It contains three parts: Android Simulink source code, Raspberry Pi C/C++ source code, and Virtual Microphone bash source code.

To make things work, user need to set up the remote server using Docker, set up the Virtual Microphone, build and run the Simulink model on Android phone, and run the C++ program on Raspberry Pi. 

In prior, user should install wiringPi library.

## Android Simulink source code

The `androidFaceDetectionAndTracking.slx` file is the Simulink source for the Android phone. To deploy it, simply open this file with MATLAB/Simulink, connect your phone to MATLAB Simulink, and click "Build, Deploy & Start" button to deploy the app onto the Android phone. You will then see a new app called "AndroidFaceDetection" on the phone. This is our customized app that will detect faces and transmit the face location to the Raspberry Pi.

## Raspberry Pi C/C++ source code

This source code is the core controlling logic of the smart robot car. It is responsible for aggregating face detection data from the Android phone and the voice recognition data from a remote server, and determine the direction of the car corresponding to the two data. By default, the voice commmands have a higher priority.

### Compiling

`g++ -g -o android_car android_car.cpp -pthread -lwiringPi`

This will generate a `android_car` executable file. Make sure it has file `out.txt` and `asrt.py` in the same directory.

### Running

Run this executable file with `./android_car`. You will see the output of the program in the terminal and the car should move corresponding to the voice commands and the face detection inputs from the Android phone if set up right.

## Virtual Microphone bash source code

The executable bash file `VirtualMic.sh` sets up a voice transmitting server. It is responsible for receiving voice commands from the Android phone and send them to the Raspberry Pi. User might need to install Mumble on the host machine first. To run this program, run `sudo bash VirtualMic.sh`. It is suggested that to run this program in GUI Desktop, but it may also work with X11-forwarding properly set up. Then, the Plumble Android client can access the server and provides its microphone inputs.

## Supplementary codes

### `out.txt`

This is the one-to-one mapping from the UDP packets inputs to the corresponding numerical outputs. It solves the problem of not having a linear mapping from the binary input to the location number output. **It shall not be changed in any form.**

### `asrt.py`

It records voice from the Android phone, automatically stops when the speaking is done, saves it to a `wav` file, sends the file to remote server in Base64 encoding, and returns different values according to the voice recognition result.

### Speech recognition server

`docker run --rm -it -p 80:20001 --name asrt-server -d ailemondocker/asrt_service:1.2.0` sets up the remote server dedicated for voice recognition. This can also be set up locally, so that the robot car will not need any Internet connection. However, the Raspberry Pi we've used is in ARM architecture, and the machine learning kit is only optimized for X86 architecture. Docker here is used for modularized management and simplicity. Note that the firewall configuration should be taken care of.

