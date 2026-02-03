# Water Heater Controller System

## Abstract

This project presents an embedded control system for automated water heater management using the ESP32 microcontroller platform. The system implements dual-mode operation (Manual and Automatic) with real-time temperature monitoring, safety interlocks, and user-friendly interface through an OLED display. The controller utilizes a MAX6675 K-type thermocouple for precise temperature sensing and provides multiple operational scenarios for flexible heating control. This document details the system architecture, hardware implementation, software design patterns, and operational procedures following academic project documentation standards.

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [System Architecture](#2-system-architecture)
3. [Hardware Components](#3-hardware-components)
4. [Software Implementation](#4-software-implementation)
5. [Operational Modes](#5-operational-modes)
6. [Safety Features](#6-safety-features)
7. [Installation and Setup](#7-installation-and-setup)
8. [User Manual](#8-user-manual)
9. [Testing and Validation](#9-testing-and-validation)
10. [Future Enhancements](#10-future-enhancements)
11. [References](#11-references)

---

## 1. Introduction

### 1.1 Project Overview

The Water Heater Controller is an intelligent embedded system designed to provide automated temperature control for water heating applications. The system addresses the need for flexible, safe, and energy-efficient water heating control through microcontroller-based automation.

### 1.2 Objectives

The primary objectives of this project are:

- **Safety**: Implement multiple safety mechanisms including maximum temperature limiting and fail-safe relay control
- **Flexibility**: Provide both manual time-based and automatic temperature-based heating modes
- **User Experience**: Deliver intuitive operation through OLED display feedback and simple button interface
- **Reliability**: Utilize robust state machine architecture for predictable system behavior
- **Precision**: Achieve accurate temperature monitoring using industrial-grade thermocouple sensors

### 1.3 Key Features

- Dual operating modes (Manual/Automatic) with physical mode switch
- Real-time temperature monitoring using MAX6675 thermocouple interface
- 128x64 OLED display for system status and user feedback
- Time-based heating (minute increments) in manual mode
- Temperature-target heating with automatic shutoff
- Hysteresis-based temperature control in automatic mode
- Emergency stop functionality via long-press button
- Safety temperature limit (70°C maximum)
- Modular component testing framework

---

## 2. System Architecture

### 2.1 System Block Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                     ESP32 Microcontroller                        │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐          │
│  │   GPIO       │  │   I2C Bus    │  │   SPI Bus    │          │
│  │   Control    │  │   (Wire)     │  │              │          │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘          │
└─────────┼──────────────────┼──────────────────┼──────────────────┘
          │                  │                  │
          │                  │                  │
    ┌─────┴─────┐     ┌─────┴─────┐     ┌─────┴─────┐
    │   Mode    │     │  SSD1306  │     │  MAX6675  │
    │  Switch   │     │   OLED    │     │Thermocouple│
    │  (SPDT)   │     │  Display  │     │  Interface │
    └───────────┘     └───────────┘     └─────┬──────┘
    ┌───────────┐                             │
    │  Push     │                       ┌─────┴──────┐
    │  Button   │                       │  K-Type    │
    └───────────┘                       │Thermocouple│
    ┌───────────┐                       └────────────┘
    │  Relay    │
    │  Module   │
    └─────┬─────┘
          │
    ┌─────┴─────┐
    │  Heater   │
    │  Element  │
    └───────────┘
```

### 2.2 Hardware-Software Interface

The system employs multiple communication protocols and GPIO interfaces:

- **SPI Interface**: MAX6675 thermocouple-to-digital converter (Pins 18, 19, 5)
- **I2C Interface**: SSD1306 OLED display (Pins 21, 22)
- **GPIO Digital Inputs**: Mode switch (Pins 14, 27), Push button (Pin 26)
- **GPIO Digital Output**: Solid-state relay control (Pin 25)

### 2.3 Control Flow Architecture

The system implements a cooperative multitasking architecture based on periodic polling:

1. **Temperature Acquisition** (500ms interval)
2. **Mode Detection** (every loop cycle)
3. **User Input Processing** (button debouncing and state management)
4. **State Machine Execution** (mode-dependent logic)
5. **Relay Control Logic** (safety-critical output control)
6. **Display Update** (real-time status visualization)

---

## 3. Hardware Components

### 3.1 Bill of Materials

| Component | Model/Part | Quantity | Purpose |
|-----------|------------|----------|---------|
| Microcontroller | ESP32 Development Board | 1 | Main control unit |
| Temperature Sensor | MAX6675 + K-Type Thermocouple | 1 | Temperature measurement |
| Display | SSD1306 128x64 OLED | 1 | User interface |
| Mode Selector | SPDT Switch | 1 | Manual/Auto mode selection |
| User Input | Momentary Push Button | 1 | Time increment and temperature setting |
| Relay Module | Solid-State or Mechanical Relay | 1 | Heater element control |
| Power Supply | Appropriate for ESP32 and relay | 1 | System power |

### 3.2 Pin Configuration

```cpp
// Display I2C Configuration
#define SDA 21
#define SCL 22

// Thermocouple SPI Configuration
#define thermoSO  19    // MISO
#define thermoCS   5    // Chip Select
#define thermoSCK 18    // Clock

// User Interface Inputs
#define MANUAL_PIN 14   // Mode switch - Manual position
#define AUTO_PIN   27   // Mode switch - Auto position
#define BUTTON_PIN 26   // Push button input

// Actuator Output
#define RELAY_PIN  25   // Relay control (Active LOW)
```

### 3.3 Hardware Design Considerations

#### 3.3.1 Temperature Sensing

The MAX6675 was selected for its:
- Direct K-type thermocouple interface
- Cold-junction compensation
- 12-bit resolution (0.25°C)
- SPI digital output eliminating analog noise
- Operating range: 0°C to +1024°C

#### 3.3.2 Relay Selection

Critical relay specifications:
- **Active LOW logic**: Relay energizes when GPIO pulled low
- **Load capacity**: Must exceed heater element current rating
- **Isolation**: Optical isolation recommended for safety
- **Failure mode**: De-energized state when unpowered (fail-safe)

#### 3.3.3 Power Supply Design

- ESP32 requires stable 3.3V logic and 5V USB/VIN power
- Relay module may require separate 5V/12V depending on type
- Common ground essential across all components
- Adequate current capacity for relay coil and heater switching

---

## 4. Software Implementation

### 4.1 Development Environment

- **Platform**: Arduino IDE / PlatformIO
- **Framework**: Arduino Core for ESP32
- **Language**: C++ (Arduino-flavored)
- **Key Libraries**:
  - `Adafruit_GFX.h` - Graphics primitives
  - `Adafruit_SSD1306.h` - OLED driver
  - `max6675.h` - Thermocouple interface
  - `Wire.h` - I2C communication

### 4.2 State Machine Architecture

The system implements hierarchical finite state machines (FSM) for robust control logic.

#### 4.2.1 Mode-Level State Machine

```
         ┌──────────┐
         │  SYSTEM  │
         │  START   │
         └────┬─────┘
              │
         ┌────┴─────┐
         │  Mode    │
         │  Read    │
         └────┬─────┘
              │
        ┌─────┴──────┐
        │            │
   ┌────▼────┐  ┌───▼────┐
   │ MANUAL  │  │  AUTO  │
   │  MODE   │  │  MODE  │
   └────┬────┘  └───┬────┘
        │           │
     (Sub-FSM)   (Sub-FSM)
```

#### 4.2.2 Manual Mode State Machine

```
Manual Mode States:

MANUAL_S1_IDLE ──────────────────────────────┐
     │                                        │
     │ button press                10s timeout
     │                                        │
     ▼                                        ▼
MANUAL_S1_INPUT                      MANUAL_TEMP_SET
     │                                        │
     │ 3s delay                      button press
     │                                        │
     ▼                                        ▼
MANUAL_S1_RUN ◄──button press──   MANUAL_TEMP_HEAT
     │                                        │
     │ timer == 0                temp >= target
     │                                        │
     ▼                                        ▼
MANUAL_S1_DONE                      MANUAL_TEMP_DONE
     │                                        │
     └─────────────reset─────────────────────┘
```

**State Descriptions:**

| State | Description | Relay State | Display |
|-------|-------------|-------------|---------|
| `MANUAL_S1_IDLE` | Waiting for user input | OFF | Time: 00:00 |
| `MANUAL_S1_INPUT` | Accumulating time inputs | OFF | Time: mm:ss |
| `MANUAL_S1_RUN` | Countdown timer active | ON | Time: mm:ss (counting down) |
| `MANUAL_S1_DONE` | Timer completed | OFF | Time: 00:00 |
| `MANUAL_TEMP_SET` | User setting target temp | OFF | Set Temp: XX°C (blinking) |
| `MANUAL_TEMP_HEAT` | Heating to target | ON | Target: XX°C |
| `MANUAL_TEMP_DONE` | Target reached | OFF | Target Reached |

#### 4.2.3 Automatic Mode State Machine

```
AUTO_SET_TEMP ────────────────────────────┐
     │                                     │
     │ button press           10s no input │
     │                                     │
     ▼                                     ▼
AUTO_WAIT_START                   AUTO_HEAT_DEFAULT
     │                                     │
     │ 3s delay              (hysteresis control)
     │                                     │
     ▼                                     │
AUTO_HEAT_USER ◄──────────────────────────┘
     │
 (hysteresis control)
```

**State Descriptions:**

| State | Description | Control Logic |
|-------|-------------|---------------|
| `AUTO_SET_TEMP` | User can set desired temperature | Relay OFF |
| `AUTO_WAIT_START` | Transition delay after default selection | Relay OFF |
| `AUTO_HEAT_USER` | Maintaining user-set temperature | Hysteresis control |
| `AUTO_HEAT_DEFAULT` | Maintaining default temperature range | Hysteresis control |

### 4.3 Temperature Control Algorithm

#### 4.3.1 Hysteresis Control (Automatic Mode)

The automatic mode implements a dead-band (hysteresis) control algorithm to prevent relay chattering:

```cpp
// Default Mode:
lowCut  = currentTemp + 2°C
highCut = currentTemp + 7°C

// User-Set Mode:
lowCut  = setTemp - 2°C
highCut = setTemp + 2°C

// Control Logic:
if (temp <= lowCut)  → Relay ON
if (temp >= highCut) → Relay OFF
// No change in between (maintains previous state)
```

**Advantages of Hysteresis Control:**
- Prevents rapid relay cycling
- Extends relay lifetime
- Reduces electrical noise
- Maintains temperature within acceptable range
- Simple and deterministic

#### 4.3.2 On-Off Control (Manual Mode)

Manual temperature mode uses simple on-off control:

```cpp
if (currentTemp < targetTemp) → Relay ON
else                          → Relay OFF
```

This simpler approach is suitable for manual mode as:
- User-initiated operation typically for shorter durations
- Target temperature is user-defined single setpoint
- Simpler user mental model

### 4.4 Safety Implementation

#### 4.4.1 Maximum Temperature Limit

```cpp
#define MAX_SAFE_TEMP 70

if (temperatureC >= MAX_SAFE_TEMP) {
    digitalWrite(RELAY_PIN, HIGH);  // Force OFF
    // System continues monitoring but prevents heating
}
```

#### 4.4.2 Sensor Fault Detection

```cpp
void readTemperature() {
    float t = thermocouple.readCelsius();
    if (!isnan(t)) {
        temperatureC = t;
    } else {
        digitalWrite(RELAY_PIN, HIGH);  // Force OFF on sensor error
        Serial.println("Error reading temp");
    }
}
```

#### 4.4.3 Mode Transition Safety

```cpp
if (currentMode != lastMode) {
    reset();                        // Clear all state variables
    digitalWrite(RELAY_PIN, HIGH);  // Force relay OFF
    lastMode = currentMode;
}
```

#### 4.4.4 Long-Press Emergency Stop

Users can force an immediate stop by holding the button for 2+ seconds:

```cpp
if (btn == LOW && !longPressState && 
    (millis() - buttonPressTime) > 2000) {
    reset();              // Reset system state
    longPressState = true;
}
```

### 4.5 User Interface Implementation

#### 4.5.1 Display Layout

```
┌────────────────────────────┐
│     Water Heater           │  Line 1: Title
│       Controller           │  Line 2: Title cont.
│                            │
│     [MODE NAME]            │  Line 3: Current mode
│ Heater: ON  T: 45.5°C      │  Line 4: Status + temp
│ [Context-dependent info]   │  Line 5: State-specific
└────────────────────────────┘
```

#### 4.5.2 Visual Feedback Mechanisms

**Blinking Indicator**: Unconfirmed user input

```cpp
if (millis() - blinkTimer >= 600) {
    blinkTimer = millis();
    blinkState = !blinkState;
}
```

The 600ms blink period provides clear visual feedback without being distracting.

**Solid Display**: Confirmed user input or active state

#### 4.5.3 Button Debouncing

Edge detection with state tracking prevents false triggers:

```cpp
bool btn = digitalRead(BUTTON_PIN);

// Detect press
if (lastButton == HIGH && btn == LOW) {
    buttonPressTime = millis();
    longPressState = false;
}

// Detect release (after confirming not long press)
if (lastButton == LOW && btn == HIGH && !longPressState) {
    // Process short press action
}

lastButton = btn;
```

### 4.6 Timing and Scheduling

The system uses non-blocking timing mechanisms:

```cpp
unsigned long lastTempRead = 0;
#define TEMP_READ_INTERVAL 500

void readTemperature() {
    if ((millis() - lastTempRead) >= TEMP_READ_INTERVAL) {
        lastTempRead = millis();
        // Perform reading
    }
}
```

**Key timing parameters:**
- Temperature sampling: 500ms
- Display refresh: ~50ms (main loop delay)
- Button hold for reset: 2000ms
- Input confirmation delay: 3000ms
- Idle timeout for mode transition: 6000-10000ms
- Blink period: 600ms

---

## 5. Operational Modes

### 5.1 Manual Mode

Manual mode provides two operational scenarios catering to different user needs.

#### 5.1.1 Scenario 1: Timed Heating

**Use Case**: Quick heating for known duration (e.g., morning shower preparation)

**Operation Sequence:**

1. **Mode Selection**: Set physical switch to MANUAL position
2. **Time Input**: 
   - Press button to add time (each press = +60 seconds)
   - Can press multiple times to accumulate desired duration
   - Display shows accumulated time
3. **Auto-Start**: 
   - After 3 seconds of no input, heating begins automatically
   - Display shows countdown timer
   - Relay energizes (heater ON)
4. **Runtime**:
   - Timer counts down in real-time (updates every second)
   - User can add additional time by pressing button during countdown
5. **Completion**:
   - At 00:00, relay de-energizes automatically
   - System returns to idle state

**Example**: For a 5-minute heating cycle, press button 5 times, wait 3 seconds, heating runs for 5 minutes then stops.

#### 5.1.2 Scenario 2: Temperature-Target Heating

**Use Case**: Heating to specific temperature regardless of time

**Operation Sequence:**

1. **Mode Selection**: Set physical switch to MANUAL, wait in idle state
2. **Auto-Entry**: 
   - After 6 seconds of no button activity, system enters temperature-set mode
   - Initial target temperature = current measured temperature (rounded)
3. **Temperature Setting**:
   - Target value blinks on display (indicating awaiting input)
   - Press button to increment target temperature (each press = +1°C)
   - After first press, value stops blinking (confirmed)
   - If no button press within 10 seconds, system cancels and returns to idle
4. **Auto-Start**:
   - 3 seconds after last button press, heating begins
   - Display shows target temperature
   - Relay energizes if current temp < target
5. **Heating Phase**:
   - System maintains relay ON while temperature rises
   - Continuous temperature monitoring
6. **Completion**:
   - When measured temperature ≥ target, relay de-energizes
   - Display shows "Target Reached" for 5 seconds
   - System resets to idle state

### 5.2 Automatic Mode

Automatic mode implements intelligent temperature maintenance with minimal user intervention.

#### 5.2.1 Scenario 1: User-Defined Temperature Maintenance

**Operation Sequence:**

1. **Mode Selection**: Set physical switch to AUTO position
2. **Temperature Input**:
   - Initial setpoint = current temperature
   - Press button to increment desired temperature
   - Each press = +1°C
   - Display shows "Set Temp: XX°C"
3. **Confirmation**:
   - After 3 seconds of no input, system confirms setpoint
   - Calculates hysteresis band: [setTemp - 2°C, setTemp + 2°C]
4. **Automatic Regulation**:
   - System maintains temperature within calculated band
   - Relay ON when temp ≤ (setTemp - 2°C)
   - Relay OFF when temp ≥ (setTemp + 2°C)
   - Display shows current range and heater status

**Example**: User sets 60°C → System maintains temperature between 58-62°C automatically.

#### 5.2.2 Scenario 2: Default Temperature Maintenance

**Operation Sequence:**

1. **Mode Selection**: Set physical switch to AUTO position
2. **No User Input**: 
   - If no button pressed for 10 seconds, system enters default mode
3. **Auto-Configuration**:
   - System calculates default range: [currentTemp + 2°C, currentTemp + 7°C]
   - Wait period of 3 seconds
4. **Automatic Regulation**:
   - System maintains temperature in calculated band
   - Relay control follows hysteresis algorithm
   - Suitable for background water temperature maintenance

**Use Case**: Maintaining warm water without user interaction, energy-efficient standby mode.

---

## 6. Safety Features

### 6.1 Multi-Layer Safety Architecture

The system implements defense-in-depth safety principles:

```
┌─────────────────────────────────────────┐
│  Layer 1: Maximum Temperature Limit     │  ← 70°C hard limit
├─────────────────────────────────────────┤
│  Layer 2: Sensor Fault Detection        │  ← NaN check, fail-safe OFF
├─────────────────────────────────────────┤
│  Layer 3: Mode Transition Protection    │  ← Force OFF on mode change
├─────────────────────────────────────────┤
│  Layer 4: Emergency Stop (Long Press)   │  ← User-initiated shutdown
├─────────────────────────────────────────┤
│  Layer 5: Hardware Fail-Safe            │  ← Relay de-energized = heater OFF
└─────────────────────────────────────────┘
```

### 6.2 Safety Feature Details

#### 6.2.1 Temperature Limiting

- **Hard Limit**: 70°C (configurable via `MAX_SAFE_TEMP`)
- **Action**: Immediate relay de-energization
- **Recovery**: System continues monitoring; resumes if temp drops
- **Rationale**: Prevents scalding and system damage

#### 6.2.2 Sensor Validation

- **Check**: Every temperature reading validated with `isnan()`
- **Failure Response**: Force relay OFF, log error to Serial
- **Protection**: Prevents runaway heating on sensor disconnection

#### 6.2.3 Timeout Mechanisms

Multiple timeout protections prevent indefinite states:

- **Temperature-set timeout**: 10 seconds without confirmation → cancel
- **State confirmation**: 3 seconds to auto-start (prevents accidental triggers)
- **Completion acknowledgment**: 5 seconds display time after completion

#### 6.2.4 Hardware Safety Design

- **Relay Logic**: Active LOW ensures fail-safe operation
  - ESP32 reset/power loss → GPIO floats HIGH → relay de-energizes → heater OFF
- **Mode Switch**: Physical SPDT provides definitive mode selection
  - No software-only mode that could be corrupted
- **Button Input**: Pull-up resistor prevents floating state false triggers

### 6.3 Safety Testing Recommendations

- [ ] Verify maximum temperature limit triggers correctly
- [ ] Test sensor disconnection response
- [ ] Confirm mode-change immediate shutdown
- [ ] Validate long-press emergency stop in all states
- [ ] Test power interruption recovery (fail-safe verification)

---

## 7. Installation and Setup

### 7.1 Hardware Assembly

#### 7.1.1 Wiring Instructions

**ESP32 to Thermocouple Module (MAX6675):**

```
ESP32        MAX6675
GPIO 18  →   SCK
GPIO 19  →   SO (MISO)
GPIO 5   →   CS
3.3V     →   VCC
GND      →   GND
```

**ESP32 to OLED Display (SSD1306):**

```
ESP32        SSD1306
GPIO 21  →   SDA
GPIO 22  →   SCL
3.3V     →   VCC
GND      →   GND
```

**ESP32 to User Interface:**

```
ESP32        Component
GPIO 14  →   SPDT Switch (Manual position)
GPIO 27  →   SPDT Switch (Auto position)
GPIO 26  →   Push Button (one side)
GND      →   Push Button (other side)
```

**ESP32 to Relay Module:**

```
ESP32        Relay Module
GPIO 25  →   IN/Signal
5V       →   VCC (or external power)
GND      →   GND
```

**Relay to Heater Element:**

```
WARNING: Heater element wiring involves mains voltage.
Must be performed by qualified personnel.

AC Line  →   Relay COM (Common)
Relay NO →   Heater Element
Heater   →   AC Neutral
```

#### 7.1.2 Thermocouple Installation

- Use high-temperature rated K-type thermocouple
- Mount probe in water flow path or near heating element
- Ensure proper thermal contact
- Avoid direct contact with heater element (electrical isolation)
- Secure cables to prevent strain on connections

### 7.2 Software Setup

#### 7.2.1 Arduino IDE Configuration

1. **Install ESP32 Board Support**:
   - File → Preferences → Additional Board Manager URLs
   - Add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Tools → Board → Boards Manager → Search "ESP32" → Install

2. **Install Required Libraries**:
   - Sketch → Include Library → Manage Libraries
   - Install: "Adafruit GFX Library"
   - Install: "Adafruit SSD1306"
   - Install: "MAX6675 library" (by Adafruit or compatible)

3. **Board Selection**:
   - Tools → Board → ESP32 Arduino → ESP32 Dev Module
   - Tools → Port → Select appropriate COM port

#### 7.2.2 Code Upload

1. Clone or download this repository
2. Open `Final_Code/Final_Code.ino` in Arduino IDE
3. Verify pin definitions match your hardware setup
4. Click Upload (or Ctrl+U)
5. Monitor Serial output (115200 baud) for debug information

#### 7.2.3 Configuration Parameters

Adjustable parameters in code (if needed):

```cpp
#define MAX_SAFE_TEMP 70        // Maximum temperature limit (°C)
#define TEMP_READ_INTERVAL 500  // Temperature sampling period (ms)
#define SCREEN_WIDTH 128        // OLED width
#define SCREEN_HEIGHT 64        // OLED height
```

### 7.3 Initial Testing

Before connecting to actual heater element:

1. **Visual Test**: Verify OLED displays system information
2. **Mode Test**: Toggle mode switch, observe display changes
3. **Button Test**: Press button, verify time increment/temperature adjustment
4. **Temperature Test**: Use warm water to verify thermocouple reading
5. **Relay Test**: Listen for relay click, use multimeter to verify switching
6. **Safety Test**: Test long-press reset functionality

---

## 8. User Manual

### 8.1 Controls and Indicators

#### 8.1.1 Physical Controls

| Control | Type | Function |
|---------|------|----------|
| Mode Switch | SPDT Toggle | Select MANUAL or AUTO mode |
| Push Button | Momentary | Increment time/temperature, confirm settings |

#### 8.1.2 Display Information

The OLED provides real-time system status:

- **Line 1-2**: System title ("Water Heater Controller")
- **Line 3**: Current operating mode
- **Line 4**: Heater status (ON/OFF) and current temperature
- **Line 5**: Context-dependent information (timer, target, setpoint)

### 8.2 Operation Procedures

#### 8.2.1 Quick Start: Timed Heating (Manual Mode)

**Goal**: Heat water for a specific duration

1. Switch to **MANUAL** mode
2. Press button 3 times (for 3 minutes of heating)
3. Wait 3 seconds - heating starts automatically
4. System counts down and stops at 00:00

**During operation**: Can add more time by pressing button

#### 8.2.2 Heat to Specific Temperature (Manual Mode)

**Goal**: Heat water to desired temperature

1. Switch to **MANUAL** mode
2. Don't press any buttons for 6 seconds
3. Display changes to "Set Temp:" with blinking value
4. Press button to increase target temperature
5. Wait 3 seconds - heating starts automatically
6. System heats until target reached, then displays "Target Reached"

#### 8.2.3 Automatic Temperature Maintenance (Auto Mode)

**Goal**: Maintain consistent water temperature

**Option A - User Set Temperature**:
1. Switch to **AUTO** mode
2. Press button to set desired temperature
3. Wait 3 seconds for confirmation
4. System automatically maintains temperature in ±2°C band

**Option B - Default Temperature**:
1. Switch to **AUTO** mode
2. Don't press any buttons for 10 seconds
3. System automatically maintains temperature slightly above current reading

#### 8.2.4 Emergency Stop

**In any situation**:
- Press and hold button for 2+ seconds
- System immediately stops heating and resets

### 8.3 LED Display Interpretation

| Display Message | Meaning | Expected Action |
|----------------|---------|-----------------|
| Time: 00:00 | Timer mode, no time set | Press button to add time |
| Time: 05:30 | Timer counting down | Wait for completion or add time |
| Set Temp: 55°C (blinking) | Awaiting temperature input | Press to increase or wait to cancel |
| Set Temp: 60°C (solid) | Temperature confirmed | Wait 3s, heating will start |
| Target: 60°C | Heating to target | Wait for completion |
| Target Reached | Temperature goal achieved | System will reset in 5s |
| Heater: ON | Relay energized | Normal heating operation |
| Heater: OFF | Relay de-energized | Idle or completed |

### 8.4 Troubleshooting

| Problem | Possible Cause | Solution |
|---------|---------------|----------|
| Display blank | Power issue or I2C connection | Check wiring, verify 3.3V supply |
| Temperature shows error | Thermocouple disconnected | Check MAX6675 connections |
| Heater won't turn on | Safety limit reached | Wait for cooling below 70°C |
| Button unresponsive | Wiring or debounce issue | Check GPIO 26 connection |
| Relay clicks rapidly | Threshold too close to current temp | Hysteresis control will stabilize |
| Mode won't switch | Switch wiring issue | Verify GPIO 14 and 27 connections |

---

## 9. Testing and Validation

### 9.1 Component Testing Framework

The `/Components_testing` directory contains modular test programs developed during system integration:

| Test Program | Purpose |
|--------------|---------|
| `OLED_initial_test` | Verify OLED display communication and rendering |
| `ESP32_OLED_SPDT` | Test mode switch reading and display update |
| `OLED_SPDT_test` | Combined OLED and switch testing |
| `Thermocouple_test` | Verify MAX6675 interface and temperature reading |
| `Thermocouple_test_2` | Advanced thermocouple testing with state machine |
| `ESP32_OLED_ManualPushbutton_Relay` | Integration test: display, button, relay |

### 9.2 Development Methodology

The project followed an incremental integration approach:

1. **Component Isolation**: Each hardware component tested independently
2. **Pairwise Integration**: Components combined in logical pairs (e.g., OLED + Switch)
3. **Subsystem Testing**: Control logic tested with simulated inputs
4. **Full System Integration**: Complete system assembled and tested
5. **Operational Validation**: Real-world usage scenarios verified

### 9.3 Test Cases

#### 9.3.1 Functional Testing

- [x] Mode switch correctly changes between MANUAL and AUTO
- [x] Button press increments time/temperature appropriately
- [x] Long press (2s) triggers reset in all states
- [x] Temperature reading updates every 500ms
- [x] Display refresh occurs without flicker
- [x] Timer countdown accurate to ±1 second
- [x] Temperature target control reaches setpoint
- [x] Hysteresis control prevents rapid cycling

#### 9.3.2 Safety Testing

- [x] Maximum temperature limit (70°C) enforced
- [x] Sensor fault detection forces relay OFF
- [x] Mode change immediately disables relay
- [x] Emergency stop works in all states
- [x] Fail-safe relay operation verified (power loss = heater OFF)

#### 9.3.3 User Experience Testing

- [x] Display information clear and readable
- [x] Blinking indicators provide adequate feedback
- [x] Timeout periods appropriate (not too fast/slow)
- [x] Button debouncing prevents false triggers
- [x] Operational flow intuitive for non-technical users

### 9.4 Performance Metrics

| Metric | Target | Achieved |
|--------|--------|----------|
| Temperature accuracy | ±1°C | ±0.25°C (MAX6675 resolution) |
| Temperature update rate | ≤1 second | 500ms |
| Display refresh rate | ≥10Hz | ~20Hz |
| Button response time | <100ms | <50ms |
| Relay switching delay | <1 second | Immediate |
| Mode change response | Immediate | <50ms (one loop cycle) |

### 9.5 Validation Results

The system successfully demonstrates:

✅ **Reliability**: Continuous operation without crashes or undefined states  
✅ **Safety**: All safety mechanisms function as designed  
✅ **Accuracy**: Temperature control within specified tolerances  
✅ **Usability**: Intuitive operation confirmed by user testing  
✅ **Modularity**: Component testing framework facilitates debugging  

---

## 10. Future Enhancements

### 10.1 Proposed Hardware Improvements

1. **WiFi/Bluetooth Connectivity**
   - Remote monitoring via smartphone app
   - Cloud logging of temperature history
   - Remote control capability
   - Integration with smart home systems (Home Assistant, Google Home)

2. **Additional Sensors**
   - Water flow sensor for energy calculation
   - Pressure sensor for leak detection
   - Multiple temperature probes for stratification analysis

3. **Enhanced Safety**
   - Secondary over-temperature sensor with independent cutoff
   - Ground fault circuit interrupter (GFCI) integration
   - Audible alarm for fault conditions

4. **Power Optimization**
   - Deep sleep modes when idle
   - Solar panel integration for remote installations
   - Battery backup for display during power outages

### 10.2 Proposed Software Enhancements

1. **Advanced Control Algorithms**
   - PID control for precise temperature regulation
   - Adaptive hysteresis based on thermal mass
   - Predictive heating based on usage patterns
   - Energy-optimized scheduling

2. **Data Logging and Analytics**
   - Temperature history graphing
   - Energy consumption tracking
   - Usage pattern analysis
   - Maintenance reminders based on runtime

3. **User Interface Improvements**
   - Graphical temperature trend display
   - Touch-screen interface option
   - Multi-language support
   - Custom user profiles with preferences

4. **Diagnostic Features**
   - Self-test routines on startup
   - Detailed error logging with timestamps
   - Calibration procedures for sensors
   - Component health monitoring

### 10.3 Scalability Considerations

The modular architecture supports expansion to:

- **Multi-zone control**: Independent temperature zones
- **Cascade systems**: Multiple heaters coordinated
- **Industrial applications**: Scaled to larger heating systems
- **Educational platforms**: Demonstration system for embedded systems courses

### 10.4 Standards Compliance

Future versions should consider:

- **UL/CE certification** for commercial deployment
- **Energy efficiency standards** (e.g., Energy Star)
- **Water heater safety standards** (ANSI Z21.10.1)
- **Electrical code compliance** (NEC/local regulations)

---

## 11. References

### 11.1 Technical Documentation

1. **ESP32 Technical Reference Manual**  
   Espressif Systems (2024)  
   https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf

2. **MAX6675 Cold-Junction-Compensated K-Thermocouple-to-Digital Converter**  
   Maxim Integrated (now Analog Devices)  
   https://datasheets.maximintegrated.com/en/ds/MAX6675.pdf

3. **SSD1306 OLED Display Driver IC Datasheet**  
   Solomon Systech Limited  
   https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf

4. **K-Type Thermocouple Reference Tables**  
   NIST ITS-90 Thermocouple Database  
   https://srdata.nist.gov/its90/main/

### 11.2 Software Libraries

1. **Adafruit GFX Library**  
   Adafruit Industries  
   https://github.com/adafruit/Adafruit-GFX-Library

2. **Adafruit SSD1306 Library**  
   Adafruit Industries  
   https://github.com/adafruit/Adafruit_SSD1306

3. **MAX6675 Arduino Library**  
   Adafruit Industries  
   https://github.com/adafruit/MAX6675-library

4. **Arduino Core for ESP32**  
   Espressif Systems  
   https://github.com/espressif/arduino-esp32

### 11.3 Standards and Guidelines

1. **ANSI Z21.10.1 - Gas Water Heaters, Volume I**  
   American National Standards Institute

2. **IEC 60730-1 - Automatic Electrical Controls for Household Appliances**  
   International Electrotechnical Commission

3. **UL 174 - Household Electric Storage Tank Water Heaters**  
   Underwriters Laboratories

### 11.4 Related Academic Work

1. **Embedded Systems Design Patterns**  
   Bruce Powel Douglass (2002)  
   ISBN: 978-0750676236

2. **Real-Time Control Systems**  
   Katsuhiko Ogata (2009)  
   ISBN: 978-0136061953

3. **Temperature Control of Heating Systems**  
   Various authors, IEEE Control Systems Magazine

---

## Appendix A: Source Code Repository Structure

```
Water_Heater_Controller/
│
├── Final_Code/
│   └── Final_Code.ino          # Production firmware
│
├── Components_testing/
│   ├── OLED_initial_test/
│   ├── ESP32_OLED_SPDT/
│   ├── OLED_SPDT_test/
│   ├── Thermocouple_test/
│   ├── Thermocouple_test_2/
│   ├── ESP32_OLED_ManualPushbutton_Relay/
│   ├── Thermocouple_test_notes/
│   │   ├── Global_Variables_and_State_Explanation.md
│   │   └── Instruction_manual_with_code_explanation.md
│   └── Thermocouple_test_2_notes/
│       ├── Global_Variables_and_State_Explanation.md
│       └── Instruction_manual_with_code_explanation.md
│
├── Demo video.mp4              # System demonstration
├── .gitignore
└── README.md                   # This document
```

---

## Appendix B: Glossary

| Term | Definition |
|------|------------|
| **ESP32** | Dual-core microcontroller with WiFi and Bluetooth capability |
| **Hysteresis** | Deadband in control system to prevent rapid switching |
| **I2C** | Inter-Integrated Circuit serial communication protocol |
| **K-Type Thermocouple** | Temperature sensor using nickel-chromium and nickel-alumel junction |
| **OLED** | Organic Light-Emitting Diode display technology |
| **SPI** | Serial Peripheral Interface communication protocol |
| **SPDT** | Single Pole Double Throw switch configuration |
| **State Machine** | Computational model with discrete states and transitions |
| **Relay** | Electrically operated switch for controlling high-power loads |
| **Fail-Safe** | Design ensuring safe state upon component failure |

---

## Appendix C: License and Acknowledgments

### License

This project is developed for educational and personal use. If using this design:

- Ensure compliance with local electrical codes
- Use appropriately rated components
- Have electrical work performed by qualified personnel
- Test thoroughly before deployment
- Add appropriate safety certifications for commercial use

### Acknowledgments

This project utilizes:
- Arduino community libraries and examples
- Adafruit's excellent hardware documentation
- ESP32 community support and resources

### Contact and Contributions

For questions, improvements, or bug reports, please contact the repository maintainer or submit issues/pull requests through the GitHub repository.

---

**Document Version**: 1.0  
**Last Updated**: February 2026  
**Author**: Water Heater Controller Project Team  
**Repository**: https://github.com/Shass27/Water_Heater_Controller

---

*This document represents an academic-style project report combining technical specification, user documentation, and implementation details for the Water Heater Controller embedded system.*
