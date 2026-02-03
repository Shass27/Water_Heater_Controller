# Water Heater Controller

## Abstract

This project presents an embedded system for intelligent water heater control using an ESP32 microcontroller. The system provides dual operational modes—manual and automatic—with real-time temperature monitoring, safety features, and a user-friendly interface. The controller employs a thermocouple sensor for precise temperature measurements and implements hysteresis-based control logic to maintain water temperature within specified ranges while preventing excessive heating cycles.

## 1. Introduction

### 1.1 Overview

Water heating systems consume significant energy in residential and commercial applications. This project addresses the need for efficient, safe, and user-controllable water heating through an embedded control system. The controller offers flexibility through two distinct operating modes while maintaining critical safety thresholds to prevent overheating.

### 1.2 Objectives

- Implement dual-mode operation (manual and automatic) for diverse user requirements
- Provide real-time temperature monitoring and display
- Ensure system safety through maximum temperature limits
- Minimize relay switching cycles through hysteresis control
- Create an intuitive user interface for configuration and monitoring

## 2. System Architecture

### 2.1 Hardware Components

The system comprises the following key components:

| Component | Model/Type | Function |
|-----------|------------|----------|
| Microcontroller | ESP32 | Main processing unit and control logic |
| Temperature Sensor | MAX6675 Thermocouple | High-precision temperature measurement |
| Display | SSD1306 OLED (128×64) | User interface and status display |
| Relay Module | SPDT Relay | Heater power control |
| Mode Switch | SPDT Switch | Manual/Auto mode selection |
| Push Button | Momentary Switch | User input and configuration |

### 2.2 Pin Configuration

```
Temperature Sensor (MAX6675):
  - SO:  GPIO 19
  - CS:  GPIO 5
  - SCK: GPIO 18

Display (I2C):
  - SDA: GPIO 21
  - SCL: GPIO 22

Control Inputs:
  - MANUAL_PIN:  GPIO 14
  - AUTO_PIN:    GPIO 27
  - BUTTON_PIN:  GPIO 26
  - RELAY_PIN:   GPIO 25
```

## 3. Software Design

### 3.1 Control Architecture

The system implements a state-machine architecture with independent state management for manual and automatic modes. Temperature readings occur at 500ms intervals, and the display refreshes continuously to provide real-time feedback.

### 3.2 Safety Features

- **Maximum Temperature Limit**: 70°C hard limit to prevent overheating
- **Sensor Fault Detection**: System halts heating if invalid temperature readings occur
- **Mode Transition Protection**: Safe state reset during mode changes

## 4. Operating Modes

### 4.1 Automatic Mode

The automatic mode provides two operational scenarios:

#### 4.1.1 Default Operation
- System initializes with current temperature as baseline
- After 10-second idle period, establishes asymmetric temperature range: [T_current + 2°C, T_current + 7°C]
- Maintains temperature within this range using hysteresis control
- Asymmetric range provides gradual warming from ambient temperature

#### 4.1.2 User-Configured Operation
- User sets desired temperature via button press (increment by 1°C per press)
- After 3-second confirmation delay, establishes symmetric control range: [T_set - 2°C, T_set + 2°C]
- System maintains temperature within tighter band centered on user preference

**Control Logic**: 
- Heater activates when temperature ≤ lower threshold
- Heater deactivates when temperature ≥ upper threshold

### 4.2 Manual Mode

The manual mode offers two distinct control methods:

#### 4.2.1 Timer-Based Control
- Each button press adds 60 seconds to heating timer
- Input confirmation period: 3 seconds
- Heater operates for specified duration
- Status displays remaining time in MM:SS format

#### 4.2.2 Temperature Target Control
- Activates after 6-second idle period
- User sets target temperature (initialized to current temperature)
- System heats until target temperature achieved
- 10-second timeout for temperature input
- Automatic reset after target reached

### 4.3 User Interaction

- **Short Press**: Increment timer (Manual) or temperature setting
- **Long Press** (>2 seconds): System reset to initial state

## 5. Installation and Usage

### 5.1 Hardware Setup

1. Connect MAX6675 thermocouple module to designated GPIO pins
2. Wire SSD1306 OLED display via I2C interface
3. Connect relay module to GPIO 25
4. Install mode selection switch between GPIO 14 and GPIO 27
5. Attach momentary push button to GPIO 26
6. Ensure all components share common ground with ESP32

### 5.2 Software Installation

#### Prerequisites
- Arduino IDE (version 2.x or later)
- ESP32 board support package

#### Required Libraries
```
- Adafruit_GFX
- Adafruit_SSD1306
- max6675
```

#### Compilation and Upload
1. Open `Final_Code/Final_Code.ino` in Arduino IDE
2. Select board: "ESP32 Dev Module"
3. Configure upload settings and select appropriate COM port
4. Compile and upload to ESP32

### 5.3 Operation

1. Power on the system
2. Select desired mode using SPDT switch (Manual/Auto)
3. Follow on-screen instructions for configuration
4. Monitor temperature and heater status on OLED display

## 6. Display Interface

The OLED display provides comprehensive system status:

### Header
```
     Water Heater
       Controller
     [MODE NAME]
```

### Status Line
```
Heater: [ON/OFF]  T: [XX.X]C
```

### Mode-Specific Information
- **Auto Mode**: Set temperature or operational range
- **Manual Mode**: Remaining time or target temperature

## 7. Technical Specifications

- **Temperature Range**: 0°C to 70°C (safety limited)
- **Temperature Resolution**: 0.25°C (MAX6675 specification)
- **Update Rate**: 500ms for temperature readings
- **Control Method**: Hysteresis-based bang-bang control
- **Power Supply**: 5V DC (ESP32 standard)
- **Relay Rating**: As per relay module specification

## 8. Development and Testing

The `Components_testing` directory contains modular test programs developed during system integration:

- **OLED_initial_test**: Display initialization verification
- **Thermocouple_test**: Temperature sensor validation
- **ESP32_OLED_SPDT**: Mode switching functionality
- **ESP32_OLED_ManualPushbutton_Relay**: Integrated component testing

These test programs facilitate troubleshooting and component verification.

## 9. Future Enhancements

Potential improvements for subsequent iterations:

1. **Data Logging**: Implement temperature history recording to SD card or cloud storage
2. **Wi-Fi Connectivity**: Remote monitoring and control capabilities
3. **PID Control**: Replace bang-bang control with PID for reduced temperature oscillation
4. **Energy Monitoring**: Track power consumption metrics
5. **Multiple Sensor Support**: Redundant temperature sensing for increased reliability
6. **Mobile Application**: Dedicated smartphone app for advanced configuration

## 10. Safety Considerations

⚠️ **Important Safety Notes**:

- This system controls potentially hazardous electrical heating equipment
- Ensure proper electrical isolation between control circuits and heating elements
- Use appropriately rated relay modules for heater power requirements
- Install thermal fuses and other safety devices as per local electrical codes
- Regular maintenance and calibration recommended for thermocouple sensors
- Maximum temperature limit (70°C) prevents scalding but system should not replace manufacturer safety devices

## 11. License and Acknowledgments

This project is provided as-is for educational and development purposes. Users assume all responsibility for implementation and safety compliance.

**Component Libraries**:
- Adafruit Industries (Display Libraries)
- MAX6675 Library Contributors

---

*Project Documentation - Water Heater Controller System*  
*Embedded Systems Development*
