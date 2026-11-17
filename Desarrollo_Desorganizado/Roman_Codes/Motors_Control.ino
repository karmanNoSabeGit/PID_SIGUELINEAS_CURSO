const int MOT_AIN1 = 15;
const int MOT_AIN2 = 16;
const int MOT_BIN1 = 17;
const int MOT_BIN2 = 19;
const int RGB_PIN  = 48;


void setup() 
{
  neopixelWrite(RGB_PIN, 5, 5, 0);
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\nSetting up");
  pinMode(MOT_AIN1, OUTPUT);
  pinMode(MOT_AIN2, OUTPUT);
  pinMode(MOT_BIN1, OUTPUT);
  pinMode(MOT_BIN2, OUTPUT);

  digitalWrite(MOT_AIN1, LOW);
  digitalWrite(MOT_BIN1, LOW);
  digitalWrite(MOT_AIN2, LOW);
  digitalWrite(MOT_BIN2, LOW);
  delay(1000);


  neopixelWrite(RGB_PIN, 0, 5, 0);

  Serial.println("\nUART Protocol Communication Esteblished");
}


void loop() 
{
  set_motor_currents(100, 100);
  delay(5000);
  set_motor_currents(-100, -100);
  delay(5000);
  set_motor_currents(100, -100);
  delay(5000);
  set_motor_currents(-100, 100);
  delay(5000);
}



/*
Functions fot controlling motors
First one is a tool function used by second one
*/

void set_motor_pwm(int pwm, int IN1_PIN, int IN2_PIN)
{
  pwm = constrain(pwm, -255, 255);

  if (pwm < 0) {
    analogWrite(IN1_PIN, -pwm);
    analogWrite(IN2_PIN, 0);

  } else {
    analogWrite(IN1_PIN, 0);
    analogWrite(IN2_PIN, pwm);
  }
}


//Setting up PWM from -255 to +255

void set_motor_currents(int pwm_A, int pwm_B)
{
  set_motor_pwm(pwm_A, MOT_AIN1, MOT_AIN2);
  set_motor_pwm(pwm_B, MOT_BIN1, MOT_BIN2);

  Serial.print("Set motor A PWM = ");
  Serial.print(pwm_A);
  Serial.print(" motor B PWM = ");
  Serial.println(pwm_B);
}
