const int IN1 = 16;
const int IN2 = 17;
const int IN3 = 18;
const int IN4 = 19;

int interval = 2000;

/*void moverMotorIzquierdo(int velocidad);
void moverMotorDerecho(int velocidad);*/

void setup() {
  // Inicialización explícita del modo de salida para los terminales de potencia
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  moverMotorIzquierdo(0);
  moverMotorDerecho(0);
}



void loop() {

  //FASE 1 IZQUIERDO -> WORKS
  moverMotorIzquierdo(255);
  moverMotorDerecho(0);
  delay(interval);

  moverMotorIzquierdo(155);
  delay(interval);

  moverMotorIzquierdo(0);
  delay(interval*2);

  /*
  //FASE 2 DERECHO -> WORKS
  moverMotorDerecho(255);
  moverMotorIzquierdo(0);
  delay(interval);

  moverMotorDerecho(155);
  delay(interval);

  moverMotorDerecho(0);
  delay(interval*2);

  //FASE 3 DELANTE -> WORKS
  moverMotorDerecho(255);
  moverMotorIzquierdo(255);
  delay(interval);

  moverMotorDerecho(155);
  moverMotorIzquierdo(155);
  delay(interval);
  
  moverMotorDerecho(0);
  moverMotorIzquierdo(0);
  delay(interval*2);*/

  //FASE 4 DETRÁS -> this is where the fun begins

  moverMotorIzquierdo(-255);
  delay(interval);

  moverMotorIzquierdo(0);
  delay(interval*2);
  /*
  // FASE 1: IZQUIERDO
  moverMotorIzquierdo(255);
  moverMotorDerecho(0);
  delay(interval);

  moverMotorIzquierdo(155);
  delay(interval);

  moverMotorIzquierdo(-155);
  moverMotorDerecho(0);
  delay(interval);

  moverMotorIzquierdo(-255);
  delay(interval);
  
  moverMotorDerecho(0);
  moverMotorIzquierdo(0);
  delay(interval*2);
  
  //FASE 2 DERECHO
  
  moverMotorIzquierdo(0);
  moverMotorDerecho(255);
  delay(interval);

  moverMotorDerecho(155);
  delay(interval);

  moverMotorDerecho(-155);
  delay(interval);

  moverMotorDerecho(-255);
  delay(interval);

  moverMotorDerecho(0);
  moverMotorIzquierdo(0);
  delay(interval);

  //FASE 3 ADELANTE

  moverMotorIzquierdo(255);
  moverMotorDerecho(255);
  delay(interval);

  moverMotorIzquierdo(155);
  moverMotorDerecho(155);
  delay(interval);

  moverMotorDerecho(0);
  moverMotorIzquierdo(0);
  delay(interval*2);

  //FASE 4 ATRÁS

  moverMotorIzquierdo(-155);
  moverMotorDerecho(-155);
  delay(interval);

  moverMotorIzquierdo(-255);
  moverMotorDerecho(-255);
  delay(interval);

  //FASE 5 PARAR
  
  moverMotorIzquierdo(0);
  moverMotorDerecho(0);
  delay(interval*2);
  */
} 

void moverMotorIzquierdo(int velocidad) {
  if (velocidad >= 0) {
    // Si la velocidad es positiva, se avanza
    analogWrite(IN1, velocidad);
    digitalWrite(IN2, LOW);
  } else {
    // Si es negativa, se invierte la polaridad para frenar girando hacia atrás
    // (Se multiplica por -1 para pasar la velocidad a un valor positivo válido para el PWM)
    digitalWrite(IN1, LOW);
    analogWrite(IN2, velocidad * -1);
  }
}

void moverMotorDerecho(int velocidad) {
  if (velocidad >= 0) {
    analogWrite(IN3, velocidad);
    digitalWrite(IN4, LOW);
  } else {
    digitalWrite(IN3, LOW);
    analogWrite(IN4, velocidad * -1);
  }
}
