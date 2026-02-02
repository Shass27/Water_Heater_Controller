# Global-scope variables (Thermocouple_test_2.ino)

This document explains the purpose of the variables and objects declared in global scope in:

- `Components_testing/Thermocouple_test_2/Thermocouple_test_2.ino`

## State machine (AUTO mode)

### `enum AutoState`
```c++
enum AutoState {
  AUTO_SET_TEMP,
  AUTO_WAIT_START,
  AUTO_HEAT_USER,
  AUTO_HEAT_DEFAULT
};
```

Represents the operating mode of the controller.

- `AUTO_SET_TEMP`
  - “Temperature selection” window after reset.
  - Short button presses increment `setTemp`.
  - If no user input happens for 10 seconds, the controller chooses a default range based on current temperature.

- `AUTO_WAIT_START`
  - Transitional wait state before default heating begins.
  - After 3 seconds, moves into `AUTO_HEAT_DEFAULT`.

- `AUTO_HEAT_USER`
  - Heating mode using the user-selected setpoint (`setTemp`).
  - Uses `lowCut` / `highCut` derived from `setTemp`.

- `AUTO_HEAT_DEFAULT`
  - Heating mode using an automatically-chosen range derived from the current temperature at the time of timeout.

### `autoState`
```c++
AutoState autoState = AUTO_SET_TEMP;
```
- Current state of the above state machine.
- Read/updated in `handleButton()`, `handleAutoState()`, and used in `controlRelay()` and `showOLED()`.

## Temperature and control thresholds

### `temperatureC`
```c++
float temperatureC = 0;
```
- Holds the latest measured temperature (°C) from the thermocouple.
- Updated by `readTemperature()` once per second.
- Displayed on OLED and used for relay control decisions.

### `setTemp`
```c++
float setTemp = 0;
```
- User-selected target temperature (°C).
- In `AUTO_SET_TEMP`, each short press increases `setTemp` by 1.
- Also initialized in `resetAuto()` as `setTemp = temperatureC` (start selection from current temperature).

### `lowCut` / `highCut`
```c++
float lowCut = 0;
float highCut = 0;
```
- Hysteresis thresholds used to control the heater relay while heating.
- Behavior in `controlRelay()`:
  - If `temperatureC <= lowCut` → heater ON (relay LOW)
  - If `temperatureC >= highCut` → heater OFF (relay HIGH)
- These values are set depending on mode:
  - User mode: `lowCut = setTemp - 2`, `highCut = setTemp + 2`
  - Default mode: `lowCut = temperatureC + 2`, `highCut = temperatureC + 7` (captured at timeout moment)

## User input tracking

### `userPressed`
```c++
bool userPressed = false;
```
- Tracks whether the user interacted during the initial set-temperature window.
- Used in `handleAutoState()` to decide whether to:
  - enter user heating mode (`AUTO_HEAT_USER`), or
  - fall back to default behavior (timeout to `AUTO_WAIT_START`).

## Timers / scheduling variables (millis-based)

All timers are based on Arduino `millis()` (milliseconds since boot). They let the sketch avoid blocking delays (except the small `delay(50)` in `loop()`).

### `lastTempRead`
```c++
unsigned long lastTempRead = 0;
```
- Timestamp of the last temperature poll.
- Used in `readTemperature()` to read the MAX6675 once every ~1000 ms.

### `stateTimer`
```c++
unsigned long stateTimer = 0;
```
- General-purpose timer used for state transitions.
- Examples:
  - In `AUTO_SET_TEMP` with user input: after 3 seconds of inactivity, confirm selection and enter `AUTO_HEAT_USER`.
  - In `AUTO_WAIT_START`: after 3 seconds, enter `AUTO_HEAT_DEFAULT`.

### `idleTimer`
```c++
unsigned long idleTimer = 0;
```
- Tracks time since last “meaningful interaction” (short button press).
- Used for the “no input for 10 seconds” timeout from `AUTO_SET_TEMP` into default mode.

## Button edge/press detection (debounce-lite + long press)

### `lastButton`
```c++
bool lastButton = HIGH;
```
- Stores previous button reading for edge detection.
- Enables detecting:
  - falling edge: `HIGH -> LOW` (press started)
  - rising edge: `LOW -> HIGH` (press released)

### `buttonPressTime`
```c++
unsigned long buttonPressTime = 0;
```
- Timestamp when a press started (on falling edge).
- Used to measure press duration for long-press detection.

### `longPressState`
```c++
bool longPressState = false;
```
- Prevents long-press action from triggering repeatedly while the button remains held.
- When long press is detected (>= 2000 ms), `resetAuto()` is called and `longPressState` is set to `true`.

## OLED blink behavior (visual cue during set-temp)

### `blinkState`
```c++
bool blinkState = true;
```
- Toggles between true/false to make the `setTemp` value blink on the OLED when the user has not pressed the button yet.
- Used in `showOLED()` only (during `AUTO_SET_TEMP` and `!userPressed`).

### `blinkTimer`
```c++
unsigned long blinkTimer = 0;
```
- Timestamp controlling the blink toggle interval.
- In `handleAutoState()` it flips `blinkState` every ~600 ms.
