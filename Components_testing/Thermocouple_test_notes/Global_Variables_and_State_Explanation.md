# Global-scope variables (Thermocouple_test.ino)

This document explains the variables declared in **global scope** in `Thermocouple_test.ino`, including constants/macros, enums, and state variables used throughout the program.

---

## Enums and state variables

### `enum ManualState { ... }`
Defines the finite-state machine (FSM) states for manual behavior.

States:
- `MANUAL_S1_IDLE`
  - **Meaning:** Manual mode idle state (no time has been entered yet).
- `MANUAL_S1_INPUT`
  - **Meaning:** User is entering time (button adds minutes). After a short delay, transitions to RUN.
- `MANUAL_S1_RUN`
  - **Meaning:** Heater runs for `remainingSeconds` and counts down once per second.
- `MANUAL_S1_DONE`
  - **Meaning:** Time-based run finished (used as a terminal/end state before reset or further behavior).

Temperature-setting “Situation 2” states:
- `MANUAL_TEMP_SET`
  - **Meaning:** User sets a target temperature (button increments `targetTemp`).
- `MANUAL_TEMP_WAIT`
  - **Meaning:** Placeholder state (currently not actually entered by logic as written).
- `MANUAL_TEMP_HEAT`
  - **Meaning:** Heater runs until current temperature reaches/exceeds `targetTemp`.
- `MANUAL_TEMP_DONE`
  - **Meaning:** Target reached, message displayed briefly, then reset.

### `ManualState manualState = MANUAL_S1_IDLE;`
- **Purpose:** Current manual FSM state.
- **Used by:** `handleButton()`, `handleManualState()`, `controlRelay()`, `showOLED()`.

---

### `enum SystemMode { MODE_MANUAL, MODE_AUTO };`
- **Purpose:** High-level operating mode of the controller, determined by the mode switch pins.

### `SystemMode currentMode = MODE_MANUAL;`
- **Purpose:** Current selected mode based on switch inputs.
- **Updated by:** `readModeSwitch()`.

### `SystemMode lastMode = MODE_MANUAL;`
- **Purpose:** Remembers previous loop’s mode to detect mode changes.
- **Used by:** `loop()` to reset state when the mode toggles.

---

## Temperature and setpoint globals

### `float temperatureC = 0;`
- **Purpose:** Latest measured temperature in Celsius.
- **Updated by:** `readTemperature()`.
- **Displayed by:** `showOLED()`.
- **Used by:** `handleManualState()` and `controlRelay()`.

### `float targetTemp = 0;`
- **Purpose:** User-selected temperature target for the temperature-based manual mode.
- **Set by:** `handleManualState()` (initializes to current temp) and `handleButton()` (increments).
- **Used by:** `handleManualState()`, `controlRelay()`, `showOLED()`.

---

## Timekeeping globals (seconds, timers, and state timestamps)

### `unsigned long remainingSeconds = 0;`
- **Purpose:** Countdown duration for the time-based manual run (`MANUAL_S1_RUN`).
- **Modified by:**
  - `handleButton()` (+60 seconds per short press)
  - `handleManualState()` (decrement each second while running)

### `unsigned long lastSecond = 0;`
- **Purpose:** Timestamp used to decrement `remainingSeconds` once per 1000ms.
- **Used by:** `handleManualState()` in `MANUAL_S1_RUN`.

### `unsigned long stateTimer = 0;`
- **Purpose:** Generic “entered-state-at” timestamp for delays and transitions.
- **Used by:**
  - transitioning from INPUT → RUN after 3 seconds
  - transitioning after confirming target temp (3 seconds)
  - timing DONE message duration

### `unsigned long idleTimer = 0;`
- **Purpose:** Tracks user inactivity timeouts (especially in temperature-setting state).
- **Used by:** `handleManualState()` to reset if user doesn’t confirm temp within 10 seconds.

### `unsigned long situation1StartTime = 0;`
- **Purpose:** Tracks how long the system has been idle in manual mode to decide when to enter temperature-setting workflow.
- **Used by:** `handleManualState()` (after 10 seconds in IDLE → enter `MANUAL_TEMP_SET`).

---

## Temperature-set confirmation flag

### `bool tempConfirmed = false;`
- **Purpose:** Tracks whether the user has pressed the button to confirm/set a temperature (in `MANUAL_TEMP_SET`).
- **Used by:**
  - `handleButton()` (sets true after incrementing target)
  - `handleManualState()` (controls transition into heating)
  - `showOLED()` (controls blinking vs solid value)

---

## OLED blinking globals (UI feedback)

### `bool blinkState = true;`
- **Purpose:** Toggles between showing/hiding the temperature value to create a blinking effect in the UI.
- **Used by:** `showOLED()` when `manualState == MANUAL_TEMP_SET` and `tempConfirmed == false`.

### `unsigned long blinkTimer = 0;`
- **Purpose:** Timestamp to toggle `blinkState` every ~600ms.
- **Used by:** `showOLED()`.

---

## Button handling globals (edge detection + long press)

### `bool lastButton = HIGH;`
- **Purpose:** Stores previous sampled button level to detect transitions:
  - HIGH → LOW: press start
  - LOW → HIGH: release
- **Used by:** `handleButton()`.

### `unsigned long buttonPressTime = 0;`
- **Purpose:** Timestamp of when the press started, to measure long-press duration.
- **Used by:** `handleButton()`.

### `bool longPressState = false;`
- **Purpose:** Prevents a long-press from also triggering a short-press action on release.
- **Used by:** `handleButton()`.

---

## Temperature read throttling globals

### `unsigned long lastTempRead = 0;`
- **Purpose:** Timestamp of last thermocouple read.
- **Used by:** `readTemperature()` to enforce a minimum read interval.

### `#define TEMP_READ_INTERVAL 500`
- **Purpose:** Minimum time (ms) between temperature reads.
- **Used by:** `readTemperature()`.

---
