const int IN1 = 16;
const int IN2 = 17;
const int IN3 = 18;
const int IN4 = 19;

int interval = 3000;

/*void moverMotorIzquierdo(int velocidad);
void moverMotorDerecho(int velocidad);*/

void setup() {
  // Inicialización explícita del modo de salida para los terminales de potencia
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}

void loop() {
  // Pruebas secuenciales de interval ms por fase

  // Fase 1: Ambos motores hacia adelante
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  delay(interval);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  delay(interval);
  
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW );
  delay(interval);

  /*moverMotorIzquierdo(0);
  moverMotorDerecho(0);
  delay(interval);*/
}

/*
void moverMotorIzquierdo(int velocidad) {
  if (velocidad > 0) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW); //NO ES ANALOG ES DIGITAL
  } else if (velocidad < 0){
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    }
}
//poner el digitalwrite


void moverMotorDerecho(int velocidad) {
  if (velocidad > 0) {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  } else if (velocidad < 0){
    digitalWrite(IN3, LOW);
    analogWrite(IN4, HIGH);
  } else {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    }
}*/
