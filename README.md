# Drummer
## Bring the beat to life!
![Presentation1new](https://user-images.githubusercontent.com/73828655/219969945-8496257f-c613-41df-9d27-653ea00909c9.jpg)

## Project Description: 
### An interactive drum that can be used as a tool for musicians to create inspired and full sounding music.
**Features:**
* Calibration Mode – sets the hit sound levels for use
* Metronome Mode – produces a beat at a given frequency
* Playback Mode – records and replays the beat created by the user
* Looper Mode – repeats the beat created by the user in a loop, allowing the user to build on the beat
* Interactive Mode – produces a bar of beats to follow the user’s bar and to continue the rhythm

## List of Content:
### ESP32
Code for the ESP32, which controls the microphone and solenoids according to the mode instructions set.

### Application
Files that are relevant to the application which acts as the user interface to contorl the modes.

### Model
Notebook for creating the neural network for interactive mode.

### Unit Tests
Basic test code for hardware accessories such as the microphone, pedal, and solenoids. These tests can be used in the event of a malfunction in order to test the accessories.

### Project Documentation
PDF files for the documentation of the project.

## System Diagram:
<img width="491" alt="System Diagram" src="https://user-images.githubusercontent.com/73828655/219973078-0f813a41-0396-44d6-991f-2213a4704e01.png">

## Wiring Diagram:
<img width="517" alt="Wiring Diagram" src="https://user-images.githubusercontent.com/73828655/219972676-fe5b2391-525d-4686-9125-0112db343b68.png">

## User Interface – Application:
<img width="667" alt="Screenshot 2023-02-21 172218" src="https://user-images.githubusercontent.com/73828655/220387563-369b0c90-3bf3-4fb0-ba63-2897bf01fc3f.png">

## Libraries Used:
* For the ESP code - BLE libraries, TFLite and TFLite Micro
* For the Appliaction - flutter_blue
