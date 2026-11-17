const int RGB_PIN  = 48;
const int sensorPins[8] = {3, 4, 5, 6, 7, 8, 9, 10};
const int irEnablePin = 46;

int sensorValues[8];
int sensor_min[8];
int sensor_max[8];
int umbral[8];
bool on_line[8];
bool calibration = true;


void setup() 
{
  neopixelWrite(RGB_PIN, 5, 5, 0);
  Serial.begin(115200);
  delay(1000);

  pinMode(irEnablePin, OUTPUT);
  digitalWrite(irEnablePin, HIGH);

  Serial.println("\n\nSetting up");

  for (int i = 0; i < 8; i++) 
  {
    sensor_min[i] = 4095;
    sensor_max[i] = 0;
  }

  
  Serial.println("Calibrating... Move the sensor over black and white for 5 seconds!");
  
  neopixelWrite(RGB_PIN, 5, 0, 0);

  unsigned long startTime = millis();
  while (millis() - startTime < 5000) 
  {
    for (int i = 0; i < 8; i++) 
    {
      int val = analogRead(sensorPins[i]);
      if (val < sensor_min[i]) sensor_min[i] = val;
      if (val > sensor_max[i]) sensor_max[i] = val;
    }
    delay(5);
  }
  
  neopixelWrite(RGB_PIN, 5, 5, 0);
  
  for (int i = 0; i < 8; i++) {
    umbral[i] = (sensor_min[i] + sensor_max[i]) / 2;
  }
  Serial.println("Calibration and Umbral calculation complete!");

  delay(1000);


  neopixelWrite(RGB_PIN, 0, 5, 0);

  Serial.println("\nUART Protocol Communication Esteblished");
}

void loop() 
{
  for (int i = 0; i < 8; i++) 
  {
    int raw = analogRead(sensorPins[i]);

    int calibrated = map(raw, sensor_min[i], sensor_max[i], 0, 1000);
    sensorValues[i] = constrain(calibrated, 0, 1000);

    if (raw > umbral[i]) 
    {
      on_line[i] = 1;
    } 
    
    else 
    {
      on_line[i] = 0;
    }

    Serial.print(sensorValues[i]);
    Serial.print("[");
    Serial.print(on_line[i]);
    Serial.print("]\t");
  }
  Serial.println();
  delay(50);

}
