int val_der = 0;
int val_izd = 0;
const int IN1 = 16;
const int IN2 = 17;
const int IN3 = 18;
const int IN4 = 19;
const int s_der = 36; //vp
const int s_izd = 35;
const int led_pin = 5;

bool girar_izda = false;
int umbral = 3000;

int state = 0;
int prev_state = 0;
unsigned long prev_millis = 0; // Corrección 1: Tipo sin signo para prevenir overflow
unsigned long giro_derecha_millis = 0;
unsigned long change_dir_interval = 750;
unsigned long giro_derecha_interval = 1000;

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
  if (val_der <= umbral) { 
      state = 1;
      girar_izda = true; 
  } else if (val_izd <= umbral and girar_izda) {  
      state = 2;
  } 
  if (prev_state != state){
        prev_millis = current_millis; // Corrección 3: Marca de tiempo sin riesgo de overflow
    }

  // Corrección 6: Evaluación de temporización acoplada con conmutación física de estado
  if (current_millis - prev_millis >= (unsigned long)change_dir_interval) {
    prev_millis = current_millis;
    girar_izda = false;
    // Corrección 5 y 7: Asignación efectiva y retorno explícito    
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

  Serial.print(val_izd);
  Serial.print("    ");
  Serial.print(val_der);
  Serial.print("    ");
  Serial.print(state);
  Serial.print("    ");
  Serial.println(prev_state);
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

/* // Corrección 7: Inversión lógica pura con retorno explícito
bool cambiarDireccion(bool dir) {
  return !dir;
}
*/
