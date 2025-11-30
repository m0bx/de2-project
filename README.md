# FM Radio Receiver with Digital Tuning

Build an FM receiver using a tuner module (e.g., Si4703). Include a display to show frequency and simple buttons for tuning. Consider an RDS (Radio Data System).

---

## 1. Problem Statement & Solution Overview

### 1.1 Problem Statement
Our project is the development of an FM radio receiver controlled by an Arduino UNO microcontroller and an Si4703 FM tuner module.

The user will be able to adjust the desired radio frequency using buttons or a rotary encoder, with the selected frequency displayed on an OLED screen. The display will also show RDS (Radio Data System) information and the signal strength of the currently tuned station.

Additionally, we aim to add the functionality to save several favorite stations as presets, which could then be selected using other dedicated buttons.

### 1.2 Hardware Components

#### List of Components
Provide a list of:
- **MCU board** (Arduino UNO) - Serves as the radio's control unit, managing communication between all components.
- **FM tuner module** (Si4703) - A module used to receive radio signals in the 76-108 MHz range, including RDS data. It requires an antenna (headphones) to receive the signal.
- **Display** (OLED) - Used to display information about the current frequency and RDS data.
- **Input controls** (buttons, rotary encoder) - These are used for setting the desired frequency, initiating a search for the next available station, or muting/unmuting current station.
- **Audio output** (headphone jack) - Serves as the audio output and simultaneously functions as the antenna.

#### Inputs
- **Reset button** - Integrated into the rotary encoder; resets the radio system and restores the default frequency.
- **Rotary encoder** - Used for manual tuning; allows the user to adjust the frequency in 0.1 MHz increments.
- **Seek up button** - Initiates an automatic scan to find the next available station with sufficient signal strength in the higher frequency range.
- **Seek down button** - Initiates an automatic scan to find the next available station with sufficient signal strength in the lower frequency range.
- **Mute button** - Toggles the audio output state (mute/unmute).

#### Outputs
- **OLED display** - Visualizes real-time data including the current frequency, RDS information, signal strength (RSSI), volume level, and mute status.
- **Headphone** - Delivers the audio output from the Si4703 module to the user.

---

### 1.3 Software Design
- **System-level block diagram**
<div align="center">
<img width="400" height="3024" alt="image" src="https://github.com/m0bx/de2-project/blob/main/Block_diagram.png" />
</div>


---

## 2. Build (Prototype Development)

### 2.1 Demonstration video and function description
https://github.com/m0bx/de2-project/blob/main/Demonstration_video.mp4

Our project utilizes an Arduino as the main control unit, acting as the Master on the I2C bus to communicate with two Slave devices: the Si4703 FM radio module and an OLED display. The Si4703 module handles signal reception, audio playback, and the decoding of RDS (Radio Data System) data.

Tuning is performed either via a rotary encoder, which allows for manual frequency adjustments in 0.1 MHz increments, or using 'Seek Up' and 'Seek Down' buttons. The seek function triggers the Si4703 to automatically search for the nearest station with sufficient signal strength. The rotary encoder also features an integrated push-button used to reset the system and restore the default frequency. Additionally, a dedicated button allows the user to mute or unmute the audio output.

The OLED display provides essential real-time information, including the current frequency and radio status. During playback, it displays the volume level and signal strength; if the audio is muted, a status indicator appears on the screen. Furthermore, RDS text is displayed whenever successfully received from the station

### 2.2 Source Code Submission
All source files are available here: https://github.com/m0bx/de2-project/tree/main/FM_radio_receiver

The project utilizes libraries introduced during the DE2 course semester. Specifically, we used the 'timer', 'gpio', 'oled', 'twi', and 'uart' libraries.

We also developed a custom 'si4703' library to control the Si4703 module. This library was implemented based on the manufacturer's [datasheet](https://github.com/m0bx/de2-project/blob/main/datasheets/Si4703-B16.pdf ) and [programming guide](https://github.com/m0bx/de2-project/blob/main/datasheets/AN230.PDF). Artificial intelligence tools, specifically Gemini and ChatGPT, assisted in the development process.

Additionally, we created the 'rotaryencoder' library to handle the rotary encoder, which is used for fine frequency tuning. This library is based on [this existing library](https://github.com/mhx/librotaryencoder), with development assistance from ChatGPT and Gemini.

The main program loop is located in the 'main' source file. It contains the core application logic, utilizing the aforementioned libraries to manage display output, communicate with the Si4703 module, and respond to button inputs. Artificial intelligence tools, specifically ChatGPT and Gemini, were used during the development process.

#### Project structure

 ```c
      FM_radio_receiver            // PlatfomIO project
      ├── include                  // Included file(s)
      │   └── timer.h
      ├── lib                      // Libraries
      │   ├── qpio                 // Tomas Fryza's GPIO library
      │   │   ├── gpio.c
      │   │   └── gpio.h
      │   ├── oled                 // Michael Köhler's OLED library
      │   │   ├── font.h
      │   │   ├── oled.c
      │   │   └── oled.h
      │   ├── rotaryencoder        // Our rotary encoder library
      │   │   ├── example.txt
      │   │   ├── rotary_encoder.c
      │   │   └── rotary_encoder.h
      │   ├── si4703               // Our Si4703 library
      │   │   ├── si4703.c
      │   │   └── si4703.h
      │   ├── twi                  // Tomas Fryza's TWI/I2C library
      │   │   ├── twi.c
      │   │   └── twi.h
      │   └── uart                 // Peter Fleury's UART library
      │       ├── uart.c
      │       └── uart.h
      ├── src                      // Source file(s)
      │   └── main.c
      ├── test           
      └── platformio.ini           // Project Configuration File
```

### 2.3 Technical Documentation
#### Circuit schematics
The wiring diagram is shown in https://github.com/m0bx/de2-project/blob/main/FM_receiver_scheme.pdf

#### Pseudocode

<div align="center">
<img width="400" height="3024" alt="image" src="https://github.com/m0bx/de2-project/blob/main/Pseudocode.png" />
</div>

### 2.4 Pitch Poster

