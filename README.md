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

## 1.3 Software Design
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
- Submit well-documented **C code** (or C-compatible) to **GitHub**.
- Ensure:
  - Clear folder structure.
  - Commented code and meaningful function/variable names.
- Highlight:
  - Key functions and algorithms.
  - Any custom libraries or drivers used.

### 2.3 Technical Documentation
Include:
- **Block diagrams** (system-level architecture).
- **Circuit schematics** (MCU, tuner, display, audio stage, power stage).
- Justifications for:
  - Major hardware choices.
  - Major software design decisions.

### 2.4 Pitch Poster

