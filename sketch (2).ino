/*
 * Project 2: Automated Irrigation Controller (Actuator Logic)
 * DecodeLabs IoT Industrial Training Kit - Batch 2026
 *
 * Board    : ESP32 Dev Kit
 * Sensor   : Analog soil moisture sensor -> GPIO 34 (ADC1_CH6, input-only pin)
 *            (Simulated in Wokwi using a Potentiometer, since Wokwi has no
 *             native soil moisture part. Turning the knob = changing moisture.)
 * Actuator : 5V Relay Module (Active-LOW) -> GPIO 5, driving a simulated water pump
 *
 * Key requirements covered:
 *  - Read analog input from soil moisture sensor (ADC)
 *  - Explicit threshold logic to evaluate dry/wet soil
 *  - Digital output pin switches a 5V relay module (simulated water pump)
 *
 * Extra industrial-grade safety features included:
 *  - Oversampling + Exponential Moving Average (EMA) filter to remove ADC noise
 *  - Calibrated normalization (map + constrain) -> converts raw ADC to 0-100% moisture
 *  - Hysteresis (dual threshold) to prevent relay chattering near the boundary
 *  - Safe Active-LOW boot sequence (pin forced HIGH/OFF before being set as OUTPUT)
 */

#define SOIL_PIN   34     // ADC1_CH6 - input-only analog pin on ESP32
#define RELAY_PIN  5      // Digital output -> relay IN pin (Active-LOW module)

// ---- Calibration anchors (adjust these after testing your real sensor) ----
const int ADC_DRY = 3200;   // Raw ADC reading in fully dry soil / open air
const int ADC_WET = 1200;   // Raw ADC reading in fully wet / submerged soil

// ---- Hysteresis thresholds (the "deadband") ----
const float T_ON  = 30.0;   // Turn pump ON when moisture drops below this (%)
const float T_OFF = 45.0;   // Turn pump OFF when moisture rises above this (%)

// ---- EMA (Exponential Moving Average) filter ----
float smoothedADC = 0;
const float alpha = 0.2;    // Smoothing factor (0.1 - 0.3 recommended)
bool firstReading = true;

// ---- State machine ----
bool pumpIsOn = false;

unsigned long previousMillis = 0;
const long interval = 2000;   // Sample every 2 seconds

void setup() {
  Serial.begin(115200);
  Serial.println("Automated Irrigation Controller - Booting...");

  // --- CRITICAL SAFETY SEQUENCE for Active-LOW relays ---
  // Force the pin HIGH (relay OFF) BEFORE declaring it as OUTPUT.
  // This prevents the pump from momentarily firing during boot/reset,
  // because GPIO pins can float LOW for a moment on startup otherwise.
  digitalWrite(RELAY_PIN, HIGH);
  pinMode(RELAY_PIN, OUTPUT);

  analogReadResolution(12);   // ESP32 ADC: 0-4095 (12-bit)
}

// Reads the ADC multiple times and averages (oversampling) to reduce noise
int readOversampledADC(int pin, int samples = 8) {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delayMicroseconds(200);   // tiny gap between samples, negligible vs 2s loop
  }
  return sum / samples;
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // 1) Read raw ADC (oversampled to reduce instantaneous noise)
    int rawADC = readOversampledADC(SOIL_PIN);

    // 2) Apply EMA filter for further smoothing across time
    if (firstReading) {
      smoothedADC = rawADC;
      firstReading = false;
    } else {
      smoothedADC = (alpha * rawADC) + ((1 - alpha) * smoothedADC);
    }

    // 3) Normalize raw ADC into a 0-100% moisture value
    //    NOTE: dry soil -> higher ADC value -> LOWER moisture %
    int moisture = map((int)smoothedADC, ADC_DRY, ADC_WET, 0, 100);
    moisture = constrain(moisture, 0, 100);   // Safety clamp - never go out of bounds

    // 4) Hysteresis-based decision logic (the "deadband")
    if (moisture < T_ON) {
      pumpIsOn = true;             // Too dry -> irrigate
    } else if (moisture > T_OFF) {
      pumpIsOn = false;            // Sufficiently wet -> stop
    }
    // else: moisture is between T_ON and T_OFF -> hold previous state (deadband)

    // 5) Drive the Active-LOW relay
    //    LOW  = relay energized  = pump ON
    //    HIGH = relay de-energized = pump OFF
    digitalWrite(RELAY_PIN, pumpIsOn ? LOW : HIGH);

    // 6) Telemetry
    Serial.print("Raw ADC: ");
    Serial.print(rawADC);
    Serial.print(" | Smoothed: ");
    Serial.print(smoothedADC, 1);
    Serial.print(" | Moisture: ");
    Serial.print(moisture);
    Serial.print("% | Pump: ");
    Serial.println(pumpIsOn ? "ON" : "OFF");
  }

  // No delay() used here - the loop stays non-blocking.
}
