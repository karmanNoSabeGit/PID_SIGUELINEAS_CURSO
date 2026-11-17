#include <Wire.h>
#include <VL53L1X.h>
const int RGB_PIN  = 48;

VL53L1X sensor;

void setup() 
{
  neopixelWrite(RGB_PIN, 5, 5, 0);
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\nSetting up");
  
  Wire.begin(8, 9);
  
  sensor.setTimeout(500);
  if (!sensor.init()) 
  {
    Serial.println("Failed to detect and initialize sensor!");
    neopixelWrite(RGB_PIN, 5, 0, 0);
    while (1) {} // Halt execution
  }

  sensor.setDistanceMode(VL53L1X::Short);
  sensor.setMeasurementTimingBudget(50000);
  sensor.startContinuous(50);


  delay(1000);


  neopixelWrite(RGB_PIN, 0, 5, 0);

  Serial.println("\nSensor initialized and running.");
}

void loop() 
{
  Serial.print("Distance (mm): ");
  Serial.println(sensor.read());

  if (sensor.timeoutOccurred()) 
  { 
    Serial.print("XXX [TIMEOUT]XXX"); 
  }

  delay(1000);
}
