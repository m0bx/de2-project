# FM Radio Receiver with Digital Tuning

Build an FM receiver using a tuner module (e.g., **Si4703**).  
Include a display to show the tuned frequency and simple buttons for tuning.  
Optionally, consider implementing **RDS (Radio Data System)**.

---

## 1. Problem Statement & Solution Overview

### 1.1 Problem Statement
- Clearly describe the problem being addressed.
- Explain why an FM radio receiver with digital tuning is useful or interesting.
- Outline any requirements or constraints (e.g., frequency range, audio quality, UI simplicity).

### 1.2 Proposed Solution (MCU-Based)
- Explain how your proposed solution uses a **microcontroller (MCU)** to:
  - Control the FM tuner module (e.g., via I²C).
  - Handle user input (buttons for tuning, volume, etc.).
  - Drive the display (showing frequency and/or RDS data).
  - Manage system states and overall logic.
- Summarize the main features (e.g., seek/scan, preset stations, mute, RDS text).

### 1.3 Hardware Components

#### List of Components
Provide a list of:
- **MCU board** (e.g., Arduino, STM32, etc.)
- **FM tuner module** (e.g., Si4703)
- **Display** (LCD/OLED/TFT)
- **Input controls** (buttons, rotary encoder, etc.)
- **Audio output** (speaker, headphone jack, amplifier)
- **Power supply** (battery, USB, regulator)
- Any **passive/active components** (resistors, capacitors, op-amps, etc.)

#### Justification
For each component, briefly justify:
- Why it is needed in the system.
- Why this particular choice (e.g., ease of use, availability, performance).

---

## 1.4 Software Design

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
