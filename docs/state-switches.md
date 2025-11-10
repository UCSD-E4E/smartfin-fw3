# Firmware State Switch Reference

This document explains how the firmware decides which task/state to run and what variables gate each decision. 

## 1. Architecture Overview

- The main state machine lives in `src/smartfin-fw3.ino:25-185`. `stateMachine[]` maps every `STATES_e` value to a `Task` implementation, and `mainFunc()` repeatedly calls `task->init()`, `task->run()`, and `task->exit()` for the current state.
- `currentState` is set during boot by `initalizeTaskObjects()` (`src/smartfin-fw3.ino:187`) and is subsequently updated with whatever `Task::run()` returns.
- `SF_DEFAULT_STATE` is `STATE_CHARGE` (`src/product.hpp:124`), so any unexpected wake or reset falls back to the charge task unless overridden.
- System-wide inputs (USB power, water detect, recorder contents, battery voltage, etc.) are exposed through the global `pSystemDesc` (`src/system.cpp:101`), whose `SystemFlags_t` members (`src/system.hpp:16`) are kept up to date by timers:
  - `SYS_chargerTask()` debounces USB/charger presence into `systemFlags.hasCharger` (`src/system.cpp:214-259`).
  - `SYS_waterTask()` pushes new samples into the `WaterSensor` FIFO so `getLastStatus()` reflects the "in water" decision window (`src/system.cpp:263-275`, `src/watersensor/waterSensor.cpp`).

## 2. Boot / "Setup" Flow 

1. `SleepTask::loadBootBehavior()` clears the retained "boot behavior" flag unless the previous sleep explicitly staged a different wake mode (`src/sleepTask.cpp:86-108`).
2. `initalizeTaskObjects()` evaluates, in order (`src/smartfin-fw3.ino:195-235`):
   | Condition (first match wins) | Variables / functions | Initial state |
   | --- | --- | --- |
   | Retained boot mode is `BOOT_BEHAVIOR_UPLOAD_REATTEMPT` | `SleepTask::getBootBehavior()` | `STATE_UPLOAD` (if `SF_CAN_UPLOAD`, else default) |
   | Water already detected | `pSystemDesc->pWaterSensor->getCurrentReading()` | `STATE_DEPLOYED` |
   | Recorder has data | `pSystemDesc->pRecorder->hasData()` | `STATE_UPLOAD` (if uploads allowed) |
   | USB/charger present | `pSystemDesc->flags->hasCharger` | `STATE_CHARGE` (default) |
   | None of the above | — | `STATE_DEEP_SLEEP` |
3. Entering `STATE_DEEP_SLEEP` means the device either hibernates until USB or the water sensor wakes it, or it immediately bounces back if USB is already present.

## 3. State Reference

Each subsection lists the critical inputs and the exact transitions that appear in source.

### 3.1 `STATE_DEEP_SLEEP` - Sleep Task 

- **Entry sources**: Boot (no charger/data/water), CLI "Sleep" command, Charge task when USB removed, Ride task on low battery or timeout, DataUpload when it has nothing to send or connection fails.
- **What `init()` does** (`src/sleepTask.cpp:10-72`):
  - Lights the sleep LED theme, checks `digitalRead(SF_USB_PWR_DETECT_PIN)`; if USB is already high the task returns immediately so the scheduler can pick another state.
  - Stores the chosen `BOOT_BEHAVIOR` into NVRAM and calls `SYS_deinitSys()` to stop peripherals.
  - Pulls `WATER_DETECT_EN_PIN` low so the water sensor can trigger `WKP` during hibernate; full hibernate occurs via `System.sleep()`.
- **`run()` behavior**: If we ever execute `run()` (meaning we woke without a reset), we immediately return `SF_DEFAULT_STATE` (`STATE_CHARGE`) to restart the machine (`src/sleepTask.cpp:74-79`).
- **How we leave Sleep**:
  - **USB detect**: hardware wake causes `digitalRead(SF_USB_PWR_DETECT_PIN)` to be true inside `init()`, so the task ends quickly and `mainFunc()` schedules `STATE_CHARGE`.
  - **Water detect**: `WKP` interrupt fires, leading to a cold boot that re-runs `initalizeTaskObjects()`. Because `pSystemDesc->pWaterSensor->getCurrentReading()` now returns true, the next state is `STATE_DEPLOYED`.
  - **Upload reattempt**: CLI can set `BOOT_BEHAVIOR_UPLOAD_REATTEMPT`, forcing the next boot into `STATE_UPLOAD` after a timed soft power-off (`src/sleepTask.cpp:45-68`).

### 3.2 `STATE_DEPLOYED` - Ride Task (with GPS/no-GPS substates)

- **Entry sources**: Boot if water is already detected, Sleep wake due to water, CLI `Set State`, Charge task when `SF_CHARGE_ALLOW_DEPLOY` is enabled and water is detected (debug-only), or DataUpload when water is detected mid-upload (`DataUpload::can_upload()` returns `STATE_DEPLOYED`).
- **Key inputs** (`src/rideTask.cpp`):
  - `pSystemDesc->pWaterSensor->getLastStatus()` - must become true within `SURF_SESSION_GET_INTO_WATER_TIMEOUT_MS` (300 s) or the task aborts back to `STATE_DEEP_SLEEP` (`rideTask.cpp:74-84`).
  - `pSystemDesc->flags->batteryLow` - any assertion forces a sleep transition (`rideTask.cpp:134-166`).
  - `SF_CAN_UPLOAD` - compile-time flag deciding whether exiting the water returns `STATE_UPLOAD` or `STATE_DEEP_SLEEP` (`rideTask.cpp:124-159`).
- **Transitions**:
  | Trigger | Variables | Next state | Code |
  | --- | --- | --- | --- |
  | Timeout waiting to enter water | `millis() - startTime > SURF_SESSION_GET_INTO_WATER_TIMEOUT_MS` | `STATE_DEEP_SLEEP` | `src/rideTask.cpp:74-84` |
  | Out of water while logging | `!pSystemDesc->pWaterSensor->getLastStatus()` | `STATE_UPLOAD` (if allowed) else `STATE_DEEP_SLEEP` | `src/rideTask.cpp:124-159` |
  | Low battery | `pSystemDesc->flags->batteryLow` | `STATE_DEEP_SLEEP` | `src/rideTask.cpp:135-166` |
- **Exit work**: Closes recorder session and restarts charger monitoring (`src/rideTask.cpp:175-198`).

### 3.3 `STATE_UPLOAD` - Data Upload Task

- **Entry sources**: Boot when recorder has data, Ride exits when out of water and uploads enabled, CLI `Set State`/debug menu, Sleep reupload behavior.
- **Gating logic**: `can_upload()` continuously evaluates environment constraints (`src/cellular/dataUpload.cpp:33-72`). The result either keeps us uploading or forces a state change.
  | Condition | Required variables | Resulting state |
  | --- | --- | --- |
  | Recorder empty | `!pSystemDesc->pRecorder->hasData()` + `pSystemDesc->flags->hasCharger` | `STATE_CHARGE` |
  | Recorder empty & no charger | same as above | `STATE_DEEP_SLEEP` |
  | Cellular disconnected and reconnection fails within `SF_CELL_SIGNAL_TIMEOUT_MS` | `sf::cloud::wait_connect` | `STATE_DEEP_SLEEP` |
  | Water sensor asserts | `pSystemDesc->pWaterSensor->getCurrentStatus()` | `STATE_DEPLOYED` |
  | Battery below `SF_BATTERY_UPLOAD_VOLTAGE` (3.6 V) | `pSystemDesc->pBattery->getVCell()` | `STATE_DEEP_SLEEP` |
- **`run()` transitions** (`src/cellular/dataUpload.cpp:74-190`):
  - While `can_upload()` keeps returning `STATE_UPLOAD`, packets are pulled from the recorder, base64 encoded, and published.
  - Failures when fetching or encoding packets are treated as firmware bugs and send the system to `STATE_CLI` for investigation (`lines 100-155`) so an operator can intervene.
  - Cloud publish failures return `STATE_DEEP_SLEEP` after logging counts (`lines 144-165`).
  - When the recorder becomes empty, the loop exits and `run()` returns whatever `can_upload()` decided (Sleep, Charge, or Deploy) (`line 185`).
- **Exit**: Gracefully disconnects from the cloud (`lines 193-199`).

### 3.4 `STATE_CHARGE` - Charge Task

- **Entry sources**: Default boot, DataUpload when no data but charger present, CLI after exiting manual control, Sleep waking via USB, or manual `CLI_setState()`.
- **Inputs**:
  - `systemFlags.hasCharger`, kept fresh by `SYS_chargerTask()` reading `SF_USB_PWR_DETECT_PIN` with hysteresis (`src/system.cpp:214-259`).
  - Serial input buffer used to detect the escape sequence `#CLI` (`src/chargeTask.cpp:32-36`).
  - Optional `SF_CHARGE_ALLOW_DEPLOY` gate to redeploy directly from the charger for debugging (`src/product.hpp:170`, `src/chargeTask.cpp:47-51`).
- **Transitions** (`src/chargeTask.cpp:25-55`):
  | Trigger | Variables | Next state |
  | --- | --- | --- |
  | User types `#CLI` | Serial buffer equals `CLI_INTERRUPT_PHRASE` | `STATE_CLI` |
  | Charger removed | `!pSystemDesc->flags->hasCharger` | `STATE_DEEP_SLEEP` |
  | Water detected while `SF_CHARGE_ALLOW_DEPLOY == 1` | `pSystemDesc->pWaterSensor->getLastReading()` | `STATE_DEPLOYED` |

### 3.5 `STATE_CLI` 

- **Entry sources**: Typing `#CLI` while charging, selecting CLI from other debug contexts, DataUpload or other tasks returning `STATE_CLI` on error.
- **Behavior**:
  - Initializes with charger monitoring active and red LED solid (`src/cli/cli.cpp:87-105`).
  - Keeps a mutable `CLI_nextState`, defaulting to `STATE_CHARGE` (`line 91`).
- **Transitions**:
  | Trigger | Command / variable | Next state |
  | --- | --- | --- |
  | USB terminal disconnect | `MNU_executeMenu()` returns `-1` | `STATE_DEEP_SLEEP` |
  | User selects "sleep" | `CLI_doSleep()` sets `STATE_DEEP_SLEEP` (`src/cli/menuItems/systemCommands.cpp:38-46`) | Sleep |
  | User selects "Set State" | `CLI_setState()` stores any `STATES_e` value in `CLI_nextState` (`src/cli/cli.cpp:145-165`) | Depends on input |
  | Custom commands | Example: manufacturing test, upload trigger, etc., then exit menu to enact `CLI_nextState`. |

