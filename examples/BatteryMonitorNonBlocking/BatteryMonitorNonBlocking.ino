#include <Battery.h>

/**
 * Check battery state of charge on ESP32 board
 * 1 cell li-ion/li-poly battery wired to A0, continuous sensing, sigmoidal mapping function, cut off at 3400mV
 * https://github.com/rlogiacco/BatterySense#lesser-than-5v-with-voltage-booster
 **/
Battery battery(3400, 4200, 25, 4096);

unsigned long prevTime = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  pinMode(13, OUTPUT);
  battery.onDemand(26, HIGH);
  battery.setUpdateInterval(5000); // update voltage every 5 second
  battery.begin(3300, 1.47, &sigmoidal);
}

void loop() {
  battery.loop(); // continuous update battery voltage reading every 5 seconds
                  // actually update every +3 seconds for stablize ADC (update every 8 seconds in this example)
  
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(500);
  
  unsigned long curTime = millis();
  if (curTime - prevTime >= 1000) { // display every second
    Serial.print("Battery voltage is ");
    Serial.print(battery.lastVoltage);
    Serial.print("mV (");
    Serial.print(battery.level(battery.lastVoltage));
    Serial.println("%)");
    prevTime = curTime;
  }
}
