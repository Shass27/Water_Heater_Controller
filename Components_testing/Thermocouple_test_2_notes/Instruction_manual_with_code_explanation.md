# Thermocouple Test 2 — Instruction Manual (AUTO Mode)

This document explains how to operate the project and what each action does, followed by a “how it’s implemented in code” section.

---

## 1) User Instruction Manual (non-technical)

### What you see on the screen
The OLED shows:
- **Heater: ON / OFF** (whether the relay is currently powering the heater)
- **T: XX.XC** (the current measured temperature in °C)
- A bottom line that shows either:
  - **Set Temp: ___C** (during setup/selection), or
  - **Range: Low-High C** (during automatic heating control)

### Power on / Reset behavior
When the device starts (or after you reset it), it enters **temperature selection mode**.

- The bottom line shows **“Set Temp:”** and a temperature value (blinking if you have not pressed the button yet).
- The heater remains **OFF** while you are in this selection mode.

### Button actions
This project uses **one button** with two types of presses:

#### A) Short press (tap and release)
- Works only during the **Set Temp** screen.
- Each short press increases the **Set Temp by 1°C**.
- After your last short press, the controller will **confirm your chosen temperature after 3 seconds**, then start automatic heating using your choice.

#### B) Long press (hold)
- Holding the button for **2 seconds** performs a reset of the automatic mode.
- The heater turns **OFF immediately**, and the controller returns to the **Set Temp** screen.
- The Set Temp is set to the **current temperature** at the moment of reset (so you start adjusting from the current reading).

### Two ways AUTO mode starts heating
There are two ways the heater control range gets chosen:

#### Mode 1: User-set temperature (you press the button at least once)
1. On the Set Temp screen, short-press to increase the temperature.
2. After your last press, do not press anything for **3 seconds**.
3. The controller begins heating automatically using a tight control band around your chosen set temperature:
   - Heater turns **ON** when temperature is **2°C below** your Set Temp.
   - Heater turns **OFF** when temperature is **2°C above** your Set Temp.

In other words, it holds temperature roughly in:
- **(Set Temp − 2°C) to (Set Temp + 2°C)**

#### Mode 2: Default automatic mode (you do not press anything)
If you do **not press the button for 10 seconds** on the Set Temp screen:
1. The controller automatically chooses a heating range based on the current temperature.
2. After those 10 seconds, it waits **3 seconds**, then starts heating using the default band:
   - Heater turns **ON** when temperature is **2°C above** the temperature at the time the default mode was chosen.
   - Heater turns **OFF** when temperature is **7°C above** the temperature at the time the default mode was chosen.

So the default range becomes:
- **(Current Temp + 2°C) to (Current Temp + 7°C)**  
  (captured at the moment the 10-second timeout happens)

### Heater ON/OFF rules (automatic control)
Once heating control is active (either user-set or default):
- If temperature is at or below the **lower number** of the range → **Heater turns ON**
- If temperature is at or above the **higher number** of the range → **Heater turns OFF**
- While the temperature stays between those values, the heater keeps its last state (this is normal and prevents rapid switching).

### Temperature update rate
- The displayed temperature updates once every **1 second**.

### Safety / sensor error behavior
- If the temperature reading becomes invalid, the heater is forced **OFF**.

---

## 2) How the Code Implements Each Feature (mapped to functions/variables)

> File: `Components_testing/Thermocouple_test_2/Thermocouple_test_2.ino`

### Display contents (“Water Heater Controller”, AUTO MODE, Heater state, temperature, set temp/range)
- Implemented in: `showOLED()`
- Heater ON/OFF text comes from:
  - `digitalRead(RELAY_PIN) == LOW ? " ON " : " OFF "`
- Current temperature comes from:
  - `temperatureC`
- The bottom line switches based on state:
  - If `autoState == AUTO_SET_TEMP` → shows “Set Temp”
  - Else → shows “Range: lowCut-highCut”

### Reading the thermocouple every 1 second
- Implemented in: `readTemperature()`
- Timing variable:
  - `lastTempRead`
- Interval:
  - `if ((millis() - lastTempRead) >= 1000)`
- Reading:
  - `double t = thermocouple.getCelsius();`
  - If valid: `temperatureC = t;`

### Short press increases Set Temp by 1°C (only in Set Temp screen)
- Implemented in: `handleButton()`
- Edge detection uses:
  - `lastButton`
- Short press condition:
  - `if (lastButton == LOW && btn == HIGH && !longPressState)`
- Only acts in selection state:
  - `if (autoState == AUTO_SET_TEMP) { setTemp++; userPressed = true; stateTimer = millis(); }`

### Long press for 2 seconds resets AUTO mode and turns heater OFF
- Implemented in: `handleButton()` + `resetAuto()`
- Long press condition:
  - `if (btn == LOW && !longPressState && (millis() - buttonPressTime) >= 2000)`
- Action:
  - calls `resetAuto();`
- `resetAuto()` behavior:
  - Forces heater OFF: `digitalWrite(RELAY_PIN, HIGH);`
  - Returns to selection: `autoState = AUTO_SET_TEMP;`
  - Clears user-selection flag: `userPressed = false;`
  - Sets starting setpoint: `setTemp = temperatureC;`
  - Restarts idle timer: `idleTimer = millis();`

### “Confirm set temperature after 3 seconds” (user-set mode)
- Implemented in: `handleAutoState()`
- Condition (user pressed at least once):
  - `autoState == AUTO_SET_TEMP && userPressed && millis() - stateTimer >= 3000`
- When it triggers, it sets the hysteresis band around the chosen set temperature:
  - `lowCut = setTemp - 2;`
  - `highCut = setTemp + 2;`
- Then enters heating state:
  - `autoState = AUTO_HEAT_USER;`

### “No input for 10 seconds → default mode”
- Implemented in: `handleAutoState()`
- Condition:
  - `autoState == AUTO_SET_TEMP && !userPressed && millis() - idleTimer >= 10000`
- When it triggers, it captures default thresholds relative to the current temperature:
  - `lowCut = temperatureC + 2;`
  - `highCut = temperatureC + 7;`
- Then transitions to a waiting state:
  - `autoState = AUTO_WAIT_START;`
  - `stateTimer = millis();`

### “Default mode starts heating after 3 seconds”
- Implemented in: `handleAutoState()`
- Condition:
  - `autoState == AUTO_WAIT_START && millis() - stateTimer >= 3000`
- Transition:
  - `autoState = AUTO_HEAT_DEFAULT;`

### Heater ON/OFF logic using lowCut/highCut (active-low relay)
- Implemented in: `controlRelay()`
- Safety first (invalid reading):
  - `if (isnan(temperatureC)) { digitalWrite(RELAY_PIN, HIGH); return; }`
- Only runs hysteresis control in heating states:
  - `if (autoState == AUTO_HEAT_USER || autoState == AUTO_HEAT_DEFAULT) { ... }`
- ON at/below lowCut:
  - `if (temperatureC <= lowCut) digitalWrite(RELAY_PIN, LOW);`
- OFF at/above highCut:
  - `else if (temperatureC >= highCut) digitalWrite(RELAY_PIN, HIGH);`
- Otherwise (not heating states):
  - heater forced OFF: `digitalWrite(RELAY_PIN, HIGH);`

### Blinking “Set Temp” value before the user presses anything
- Implemented in: `handleAutoState()` + `showOLED()`
- Blink toggling every 600 ms:
  - In `handleAutoState()`:
    - `if (millis() - blinkTimer >= 600) { blinkTimer = millis(); blinkState = !blinkState; }`
- Blink usage:
  - In `showOLED()` inside `AUTO_SET_TEMP`:
    - If `!userPressed` and `blinkState` → prints `setTemp`
    - Else prints spaces (so the value appears to blink)

### Main loop execution order (what runs continuously)
- Implemented in: `loop()`
- Order:
  1. `readTemperature();`
  2. `handleButton();`
  3. `handleAutoState();`
  4. `controlRelay();`
  5. `showOLED();`
  6. `delay(50);` (loop repeats ~every 50 ms, while temperature reads are still limited to 1 second by `lastTempRead`)
