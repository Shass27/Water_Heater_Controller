# Water Heater Controller (Thermocouple_test) — Instruction Manual

This document explains how to operate the project and what to expect on the OLED.  
It has **two sections**:
1. **User instruction manual (non-technical)** — how to use it
2. **Feature-to-code mapping (technical)** — where each feature is implemented in the sketch

---

## 1) Instruction Manual (How to operate it)

### What you have (controls + indicators)
- **Mode switch** with two positions:
  - **MANUAL mode**
  - **AUTO mode**
- **One push button**
- **OLED screen** that shows:
  - Current mode (MANUAL/AUTO)
  - Heater status (ON/OFF)
  - Current temperature (°C)
  - Either a timer screen or temperature-target screen depending on what you’re doing

### Safety note about AUTO mode
In this test sketch, **AUTO mode keeps the heater OFF**. It is effectively a “do nothing / safe” mode.

---

### A) Selecting MANUAL vs AUTO mode
1. Move the switch to **MANUAL**:
   - The controller becomes active and will respond to the button.
2. Move the switch to **AUTO**:
   - The heater is forced **OFF**
   - Manual actions are cancelled/reset

If you change modes while something is running, the system resets to a safe state and stops heating.

---

### B) Manual Mode Feature 1: Timed Heating (add minutes)
This is the “quick start” behavior in manual mode.

**Goal:** Run the heater for a chosen amount of time (in 1-minute increments).

How to use:
1. Ensure the mode is **MANUAL**.
2. **Tap the button** to add time:
   - Each tap adds **+1 minute**
3. After you stop pressing the button, **wait 3 seconds**:
   - The heater will start automatically.
4. While running:
   - The display shows a **countdown timer**
   - Heater status shows **ON**
5. When the timer reaches **00:00**:
   - Heating stops automatically (heater turns OFF)

Notes:
- You can tap the button again while the timer is running to **add another minute** (extends runtime).

---

### C) Manual Mode Feature 2: Temperature Target Heating (set a target °C)
This is the “hands-off” behavior that appears if you don’t choose a timed run.

**Goal:** Heat until the measured temperature reaches a user-set target temperature.

How to enter temperature-target mode:
1. Ensure the mode is **MANUAL**.
2. Do **nothing** (don’t press the button) for **10 seconds** while idle.
3. The display changes to a **Set Temp** screen.

How to set the target temperature:
1. When you see **“Set Temp:”**:
   - The target value will **blink** to show it’s waiting for input.
2. **Tap the button** to increase the target temperature:
   - Each tap increases the target by **+1°C**
   - After your first tap, the target becomes solid (not blinking).
3. After you stop pressing the button, **wait 3 seconds**:
   - Heating begins automatically.

What happens while heating:
- The heater turns **ON** and stays on while heating.
- The display shows the target temperature.

When it finishes:
- When the current temperature reaches the target:
  - The heater turns **OFF**
  - The display shows **“Target Reached”**
- After **5 seconds**, the controller returns to the idle manual screen.

Timeout / cancel behavior:
- If you enter the **Set Temp** screen but **don’t press the button** for **10 seconds**, the controller cancels and returns to idle.

---

### D) Reset / Emergency stop (long press)
At almost any time in MANUAL mode (including while heating):
1. **Press and hold the button for 2 seconds**
2. The controller resets manual operation:
   - Heater turns **OFF**
   - Timer/temperature actions are cleared
   - Returns to the idle manual screen

---

### E) Understanding the OLED display (what it’s telling you)
The screen generally shows:
- **Mode**: MANUAL MODE / AUTO MODE
- **Heater**: ON or OFF
- **T:** current temperature in °C
- Bottom line changes depending on activity:
  - **Time: mm:ss** during timed mode (or before timed run starts)
  - **Set Temp: XXC** when choosing a target temp
  - **Target: XXC** while heating to a set temperature
  - **Target Reached** when finished

---

## 2) Feature-to-code mapping (what code executes each behavior)

This section points to the functions and logic blocks responsible for the behaviors described above.

### Always-running tasks each loop
In `loop()`:
- Temperature sampling:
  - `readTemperature();`
- Mode selection:
  - `readModeSwitch();`
- OLED update:
  - `showOLED();`

---

### A) Selecting MANUAL vs AUTO mode (and stopping heat when switching)
- **Where mode is read:**
  - `readModeSwitch()`
    - Reads `MANUAL_PIN` and `AUTO_PIN` and sets `currentMode`.

- **Where mode changes trigger resets:**
  - In `loop()`:
    - `if (currentMode != lastMode) { resetManual(); digitalWrite(RELAY_PIN, HIGH); lastMode = currentMode; }`
  - This ensures switching modes:
    - Cancels manual state
    - Forces heater OFF immediately

- **AUTO mode forces heater OFF:**
  - In `loop()`:
    - `else { digitalWrite(RELAY_PIN, HIGH); }`

---

### B) Timed Heating (tap to add minutes, auto-start after 3s, countdown, auto-stop)
- **Tap button adds time:**
  - `handleButton()`
    - On short press release, if state is `MANUAL_S1_IDLE` / `MANUAL_S1_INPUT` / `MANUAL_S1_RUN`:
      - `remainingSeconds += 60;`
      - If not already running: sets `manualState = MANUAL_S1_INPUT; stateTimer = millis();`

- **Auto-start after 3 seconds of no input:**
  - `handleManualState()`
    - If `manualState == MANUAL_S1_INPUT` and `(millis() - stateTimer) >= 3000`:
      - `manualState = MANUAL_S1_RUN;`
      - `lastSecond = millis();`

- **Countdown once per second:**
  - `handleManualState()` in `MANUAL_S1_RUN`:
    - Uses `lastSecond` to decrement `remainingSeconds--` every 1000ms.

- **Auto-stop at 00:00:**
  - `handleManualState()`:
    - When `remainingSeconds == 0`:
      - `manualState = MANUAL_S1_DONE; stateTimer = millis();`
  - Heater ON/OFF decision is centralized in:
    - `controlRelay()`
      - Heater ON when `manualState == MANUAL_S1_RUN`

- **Timer display:**
  - `showOLED()`:
    - Default case prints `Time:` and calls `printTime(remainingSeconds)`.

---

### C) Temperature Target Heating (idle 10s → set temp, wait 3s after confirm, stop at target, show done 5s)
- **Enter temperature setting after 10 seconds of idle:**
  - `handleManualState()`:
    - If `manualState == MANUAL_S1_IDLE` and `(millis() - situation1StartTime >= 10000)`:
      - Initializes `targetTemp = (int) temperatureC;`
      - `tempConfirmed = false;`
      - `manualState = MANUAL_TEMP_SET;`
      - `idleTimer = millis();`

- **Button increases target temp and confirms:**
  - `handleButton()` when `manualState == MANUAL_TEMP_SET`:
    - `targetTemp++;`
    - `tempConfirmed = true;`
    - `stateTimer = millis();`

- **Timeout if user never presses button (10 seconds):**
  - `handleManualState()`:
    - If `manualState == MANUAL_TEMP_SET` and `!tempConfirmed` and `(millis() - idleTimer) >= 10000`:
      - `resetManual();`

- **Start heating 3 seconds after confirmation:**
  - `handleManualState()`:
    - If `manualState == MANUAL_TEMP_SET && tempConfirmed && millis() - stateTimer >= 3000`:
      - `manualState = MANUAL_TEMP_HEAT;`

- **Stop heating when target reached:**
  - `handleManualState()` in `MANUAL_TEMP_HEAT`:
    - If `temperatureC >= targetTemp`:
      - `manualState = MANUAL_TEMP_DONE;`
      - `stateTimer = millis();`

- **Show “Target Reached” for 5 seconds then reset:**
  - `handleManualState()` in `MANUAL_TEMP_DONE`:
    - If `millis() - stateTimer >= 5000`:
      - `resetManual();`

- **Relay control during temperature heating:**
  - `controlRelay()`:
    - Heater ON when:
      - `manualState == MANUAL_TEMP_HEAT && temperatureC < targetTemp`

- **OLED behavior for set temp (blink / solid):**
  - `showOLED()` in `case MANUAL_TEMP_SET:`:
    - `blinkTimer` + `blinkState` toggle every **600ms**
    - If `!tempConfirmed`, it alternates printing value vs blanks

---

### D) Reset / long-press stop (2 seconds)
- Implemented in `handleButton()`:
  - When button is held LOW for **more than 2000ms**:
    - Calls `resetManual();`
    - Sets `longPressState = true;` so release doesn’t count as a short press.

- `resetManual()` does the safety-critical cleanup:
  - `manualState = MANUAL_S1_IDLE;`
  - `remainingSeconds = 0;`
  - `tempConfirmed = false;`
  - Resets timing references
  - Forces heater OFF:
    - `digitalWrite(RELAY_PIN, HIGH);`

---

### E) Temperature reading and refresh rate (every 0.5 seconds)
- `readTemperature()`:
  - Reads the MAX6675 every `TEMP_READ_INTERVAL` = **500ms**.
  - Updates global `temperatureC` if reading is valid.

---
