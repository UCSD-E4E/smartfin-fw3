## Architecture Overview


### Primary Components

The main firwmare logic is stored in `smartfin-fw3.ino`, holding the setup and main loop. A statemachine sits at the core of this logic, handeling the switching between various tasks. A task represents some confined logic the firmware could run at any given time, and executation stays within an internal loop until a condition is met to leave the task and go onto the next state as determened by the state-machine. Each state has three logic components: initialization, running, and exiting.  

#### State Overview

* Deep Sleep
  * Init:
    * Checks for boot behavior, then puts the system to sleep
  * Run:
    * N/A, as the system is asleep, and no logic is being ran
  * Exit:
    * N/A, as there is no exit tasks
* CLI (Command Line Interface)
  * Init: 
    * Initializes checking for power status
  * Run:
    * Manages user input, and runs system and debug commands 
  * Exit
    * Stops the power status checks
* Charge 
  * Init: 
    * Initializes checking for power status
  * Run:
    * Checks for user input to switch to CLI, or if unplugged go to sleep
  * Exit:
    * Stops checking for power status
* Upload
  * Init: 
    * Connects to Particle servers for upload
  * Run:
    * Gets data from recorder and filesystem, and uploads over 4g to Particles servers
  * Exit:
    *  Turns off cellular to save power

### Components

********Utility/Component Classes:********

********Flog********

- Various debugging messages to log

************FileCLI************

- Utiltiies to allow cli acsess of on-storage data and files

********************DataUpload********************

- Uploads data to internet server

**************MFGTest**************

- Manufacturing testing of various sensors

******************Scheduler:******************

- Schedules tasks for callibration and deployment

******************Recorder:******************

- System to store data to onboard from sensors
- Sleep until miliseconds function

************System************

- Main file that initalizes all sensors and components
    - Initalized components:
        - sensors: gps, imu, temperature, electric compass, water sensor, ICM20648 (6-axis MotionTracking device, 3-axis gyroscope, 3-axis accelerometer, and a Digital Motion Processor) , AK09916 (3-axis compass
        - computer components: NVRAM, flash memory, charger, led (battery and water), and clock/timers, i2c