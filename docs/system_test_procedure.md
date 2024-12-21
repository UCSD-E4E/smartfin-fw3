# System Test Procedure
This procedure will test the system level functionality.

All tests shall start with the fin plugged into a computer just after a firmware install.

# Charge Test
1. Check that the LEDs are in the Charging mode.
2. Connect a serial terminal at 115200 8N1 lfcrlf.
3. Check that there is no response to any key (do not enter `#CLI`).

# CLI Mode
1. Connect a serial terminal at 115200 8N1 lfcrlf.
2. Enter `#CLI`
3. Immediately after pressing the `I` key, check that a terminal prompt appears and is legible.

# Deploy Mode
1. Disconnect the fin from the charger.
2. Check that all LEDs turn off.
3. Submerge the fin in a seawater bath.
4. Check that the LEDs are in the Deploy mode.
5. Let the fin sit for 30 minutes.
6. Pull the fin out of the water.
7. Check that the LEDs switch to the Upload mode within 1 minute.
8. Check that the fin turns off within 10 minutes.
9. Check that the appropriate amount of data was uploaded.
10. Check that the data is sane.