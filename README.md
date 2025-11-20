# FM Radio Receiver with Digital Tuning

Build an FM receiver using a tuner module (e.g., Si4703). Include a display to show frequency and simple buttons for tuning. Consider an RDS (Radio Data System).

---

## 1. Problem Statement & Solution Overview

### 1.1 Problem Statement
Our project is the development of an FM radio receiver controlled by an Arduino UNO microcontroller and an Si4703 FM tuner module.

The user will be able to set the desired radio station frequency using buttons, with the selected frequency shown on an OLED display. The display will also show information broadcast by stations using the Radio Data System (RDS).

Additionally, we aim to add the functionality to save several favorite stations as presets, which could then be selected using other dedicated buttons.

### 1.2 Hardware Components

#### List of Components
Provide a list of:
- **MCU board** (Arduino UNO) - Serves as the radio's control unit, managing communication between all components.
- **FM tuner module** (Si4703) - A module used to receive radio signals in the 76-108 MHz range, including RDS data. It requires an antenna (headphones) to receive the signal.
- **Display** (OLED) - Used to display information about the current frequency and RDS data.
- **Input controls** (buttons, rotary encoder) - These are used for setting the desired frequency, initiating a search for the next available station, or saving favorite stations to memory.
- **Audio output** (headphone jack) - Serves as the audio output and simultaneously functions as the antenna.
- Any **passive/active components** (resistors, capacitors)

---

## 1.3 Software Design

Describe the planned software structure:

- **System-level block diagram**
  - Show MCU, tuner module, display, buttons, audio output, etc.
- **Flowcharts / State diagrams**
  - Power-up & initialization
  - Station tuning & scanning
  - Display update logic
  - Optional: RDS data handling
- **Pseudocode**
  - Main loop structure
  - Key functions (e.g., `tuneFrequency()`, `readRDS()`, `updateDisplay()`)

---

## 2. Build (Prototype Development)

### 2.1 Objective
- Implement and test a working prototype based on the approved proposal.
- Use university laboratories and facilities where needed.
- Circuit construction may include:
  - Breadboard with passive and active components.
  - Custom PCB (if available).

### 2.2 Deliverables

#### a) Prototype Demonstration Video
- Maximum **3-minute** video.
- Show the prototype’s **functionality**:
  - Tuning across FM band.
  - Display updates.
  - Audio output.
  - Optional: RDS features.
- Clearly explain:
  - System features.
  - Operation and user interaction.
  - Observed performance.

#### b) Source Code Submission
- Submit well-documented **C code** (or C-compatible) to **GitHub**.
- Ensure:
  - Clear folder structure.
  - Commented code and meaningful function/variable names.
- Highlight:
  - Key functions and algorithms.
  - Any custom libraries or drivers used.

#### c) Technical Documentation
Include:
- **Block diagrams** (system-level architecture).
- **Circuit schematics** (MCU, tuner, display, audio stage, power stage).
- Justifications for:
  - Major hardware choices.
  - Major software design decisions.

#### d) Pitch Poster (A3 or Larger)
A visually appealing poster summarizing:
- Project **concept and motivation**.
- **System design** and main modules.
- **Functionality & features**.
- **Value, applications, and potential impact**.

---

## 3. Demonstration

- Present and defend the project in front of instructors and peers.
- Showcase:
  - Working prototype.
  - Pitch poster.
- Each team member should:
  - Explain their individual contribution.
  - Answer technical questions about:
    - Design decisions.
    - Implementation details.
    - Testing and debugging process.
