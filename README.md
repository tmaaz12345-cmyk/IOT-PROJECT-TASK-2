# IOT-PROJECT-TASK-2
WOKWI LINK
https://wokwi.com/projects/470777393133677569

utomated Irrigation Controller — Wokwi Setup Guide
These files (sketch.ino, diagram.json, libraries.txt) together form a complete Wokwi project. No physical hardware needed.

Important note about the sensor
Wokwi does not have a native "soil moisture sensor" component, so this project uses a Potentiometer to simulate it. Turning the potentiometer knob changes the analog voltage exactly the way a real soil moisture sensor would when soil goes from wet to dry — this is a standard substitution used in simulators for any analog sensor.

Steps
Go to: https://wokwi.com/projects/new/esp32
A new project will open with default files:
sketch.ino
diagram.json
libraries.txt (add via the + button in the left panel if missing)
Copy the content of these three files and paste them into the corresponding files in Wokwi (the ones attached here).
Click the green Play/Run button at the top.
Once the simulation starts:
Click on the potentiometer and drag its knob to simulate dry/wet soil.
Watch the Serial Monitor — it prints raw ADC value, smoothed value, moisture %, and pump state (ON/OFF) every 2 seconds.
Watch the relay module — its indicator LED and switch will flip to "ON" when moisture drops below 30%, and "OFF" once it rises above 45%.
Requirement checklist (as given in the PDF)
 Reads analog input from a soil moisture sensor (via ADC on GPIO 34)
 Explicit threshold logic gate evaluates if soil is too dry
 Digital output pin (GPIO 5) switches a 5V relay module ON/OFF, simulating a water pump
Extra industrial-grade features included (bonus, beyond minimum requirement)
Oversampling: each reading averages 8 ADC samples to cut down noise
EMA (Exponential Moving Average) filter: smooths readings over time
Calibrated normalization: map() + constrain() converts raw ADC into a safe 0–100% moisture value (never goes out of bounds)
Hysteresis (deadband): pump turns ON below 30% and OFF above 45%, so it doesn't rapidly flicker when moisture sits right at one threshold
Safe Active-LOW boot sequence: the relay pin is forced HIGH (OFF) before being set as OUTPUT, so the pump never fires accidentally on power-up or reset
Common issue
If the relay seems to behave backwards (ON when it should be OFF), your relay module might be Active-HIGH instead of Active-LOW. In that case, just swap the logic in this line inside sketch.ino:

digitalWrite(RELAY_PIN, pumpIsOn ? LOW : HIGH);
to

digitalWrite(RELAY_PIN, pumpIsOn ? HIGH : LOW);
