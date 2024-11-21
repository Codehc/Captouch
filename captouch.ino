#define NOP() __asm__ __volatile__ ("nop")

#define SENSOR 0
#define LED 12

#define BUFF_LEN 10

#define THRESHOLD 3

void setup() {
  Serial.begin(115200);

  pinMode(LED, OUTPUT);
}

// Buffer to smooth values out over time
int buffer[BUFF_LEN] = {0};

// Wait function to allow for the capacitors to charge up
void wait() {
  NOP(); NOP(); NOP(); NOP();
  NOP(); NOP(); NOP(); NOP();
}

// Push to the average buffer
void push_to_buffer(int value) {
  for (int i = 1; i < BUFF_LEN; i++) {
    buffer[i - 1] = buffer[i];
  }
  buffer[BUFF_LEN - 1] = value;
}

// Gets the average value of the full buffer
int avg_value() {
  int total = 0;
  for (int i = 0; i < BUFF_LEN; i++) {
    total += buffer[i];
  }
  return total / BUFF_LEN;
}

int measure_cap_delay() {
  // Start charging the capacitors
  pinMode(SENSOR, OUTPUT);
  digitalWrite(SENSOR, HIGH);
  wait();

  // Stop charging the capacitors and wait for discharge time
  pinMode(SENSOR, INPUT);

  // Measure discharge time
  int ticks = 0;
  while (digitalRead(SENSOR) == HIGH) {
    ticks++;
  }
  
  // Add discharge time to the average buffer
  push_to_buffer(ticks);
  return avg_value();
}

// 0 -> calibration; 1 -> measuring touch
int state = 0;

int base_discharge_time = 0;

void loop() {
  if (state == 0) {
    Serial.println("Calibrating Sensor");
    // Calibrate base discharge time
    for (int i = 0; i < BUFF_LEN; i++) {
      base_discharge_time = measure_cap_delay();
      delay(5);
    }

    // Transition state
    state = 1;
    Serial.println("Done Calibrating Sensor");
  } else if (state == 1) {
    int discharge_time = measure_cap_delay();

    Serial.print(discharge_time);
    Serial.print(" vs. base discharge time: ");
    Serial.println(base_discharge_time);
    // THRESHOLD must be tuned and depends on the physical build of the "touch capacitor plate"
    if (discharge_time > base_discharge_time + THRESHOLD) {
      digitalWrite(LED, HIGH);
    } else {
      digitalWrite(LED, LOW);
    }
    delay(5);
  }
}
