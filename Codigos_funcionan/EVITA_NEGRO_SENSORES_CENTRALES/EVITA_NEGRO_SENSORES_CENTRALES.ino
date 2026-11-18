int val_der = 0; //ESTÁN AL REVES DER E IZDA
int val_izd = 0;
const int IN1 = 16;
const int IN2 = 17;
const int IN3 = 18;
const int IN4 = 19;
const int s_der = 32; //DERECHA
const int s_izd = 33; //IZDA
const int led_pin = 5;

int umbral = 3000; //calculado a ojo

int state = 0;
unsigned long prev_millis = 0; //Tipo sin signo para prevenir overflow
int change_dir_interval = 1000;
int printout_interval = 1000;

bool cambiarDireccion(bool dir);
void moverMotorIzquierdo(int velocidad);
void moverMotorDerecho(int velocidad);

void setup() {
  pinMode(s_der, INPUT);
  pinMode(s_izd, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  Serial.begin(115200);
}

void loop() {

  unsigned long current_millis = millis();
  val_der = analogRead(s_der);
  val_izd = analogRead(s_izd);

  if (val_der >= umbral) {
      state = 1;
     
  } else if (val_izd >= umbral) { 
      state = 2;
  }

  
  if(current_millis - prev_millis >= printout_interval){ //cada segundo imprime valores 
    prev_millis = current_millis;                        //ayuda a comprobar que funciona
    Serial.print(val_izd);
    Serial.print("    ");
    Serial.print(val_der);
    Serial.print("    ");
    Serial.print(state);
    Serial.println("    ");
  }
  switch (state) { //implementación sencilla de máquina de estados
    case 0: // Avance recto
      moverMotorIzquierdo(215);
      moverMotorDerecho(215);
      break;
      
    case 1: // Giro derecha
      moverMotorIzquierdo(255);
      moverMotorDerecho(0);
      break;
      
    case 2: // Giro izquierda
      moverMotorIzquierdo(0);
      moverMotorDerecho(255);
      break;
  }
}

void moverMotorIzquierdo(int velocidad) {
  if (velocidad >= 0) {
    analogWrite(IN3, velocidad);
    digitalWrite(IN4, LOW);
  } else {
    digitalWrite(IN3, LOW);
    analogWrite(IN4, velocidad * -1);
  }
}

void moverMotorDerecho(int velocidad) {
  if (velocidad >= 0) {
    analogWrite(IN1, velocidad);
    digitalWrite(IN2, LOW);
  } else {
    digitalWrite(IN1, LOW);
    analogWrite(IN2, velocidad * -1);
  }
}

// Corrección 7: Inversión lógica pura con retorno explícito
bool cambiarDireccion(bool dir) {
  return !dir;
}
