int val_der = 0;
int val_izd = 0;
const int IN1 = 16;
const int IN2 = 17;
const int IN3 = 18;
const int IN4 = 19;
const int s_der = 33;
const int s_izd = 34;
const int led_pin = 5;

bool change_dir = false;

int umbral = 3000;

int state = 0;
int prev_state = 0;
unsigned long prev_millis = 0; // Corrección 1: Tipo sin signo para prevenir overflow
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

  prev_state = state;
  if (val_der >= umbral) {
    if (!change_dir) {
      state = 1;
    } else {
      state = 2;
    }
    prev_millis = current_millis; // Corrección 3: Marca de tiempo sin riesgo de overflow
  } else if (val_izd >= umbral) { 
    if (!change_dir) {
      state = 2;
    } else {
      state = 1;
    }
    prev_millis = current_millis; // Corrección 3: Marca de tiempo sin riesgo de overflow
  } 

  if (prev_state != state){
    digitalWrite(led_pin, LOW);
  } else {
    digitalWrite(led_pin, HIGH);
  }

  Serial.print(val_izd);
  Serial.print("    ");
  Serial.print(val_der);
  Serial.print("    ");
  Serial.print(state);
  Serial.print("    ");
  Serial.println(prev_state);

  // Corrección 6: Evaluación de temporización acoplada con conmutación física de estado
  if (current_millis - prev_millis >= (unsigned long)change_dir_interval) {
    prev_millis = current_millis;
    
    // Corrección 5 y 7: Asignación efectiva y retorno explícito
    change_dir = !change_dir;
    
    if (state == 1) {
      state = 2;
    } else if (state == 2) {
      state = 1;
    }
    for (int i = 0; i < 10; i++){
      Serial.println("***************************");
    }   
  }

  switch (state) {
    case 0: // Avance recto
      moverMotorIzquierdo(215);
      moverMotorDerecho(200);
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
    analogWrite(IN1, velocidad);
    digitalWrite(IN2, LOW);
  } else {
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

// Corrección 7: Inversión lógica pura con retorno explícito
bool cambiarDireccion(bool dir) {
  return !dir;
}
