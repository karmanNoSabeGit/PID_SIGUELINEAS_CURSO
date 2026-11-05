int val_der = 0;
int val_izd = 0;
const int IN1 = 16;
const int IN2 = 17;
const int IN3 = 18;
const int IN4 = 19;
const int s_der = 36;
const int s_izd = 39; 

int umbral  = 2048; //el umbral es el valor a partir del cual se define blanco o negro
                    //negro es si la luz reflejada es menos de 2048 (valor arbitrario, 
                    //la mitad de 4096). Se define en la calibración del sensor.

int state = 0;
long prev_millis = 0;
int switch_interval = 100;
int printout_interval = 1000;


void setup() {
  // put your setup code here, to run once:
  pinMode(s_der, INPUT);
  pinMode(s_izd, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  moverMotorIzquierdo(255);
  moverMotorDerecho(255);
  val_der = analogRead(s_der);
  val_izd = analogRead(s_izd);
  Serial.begin(115200);
  

}


void loop() {
  long current_millis = millis(); //millis() es una forma muy común de poder tener dos eventos
                                  //funcionando casi-simultaneamente
    
  if (current_millis-prev_millis >= switch_interval){
      val_der = analogRead(s_der);
      val_izd = analogRead(s_izd);
  
      prev_millis = current_millis;
      
       if (val_der < umbral){
        state = 1;
    
      } else if (val_izd < umbral) { //ell
        state = 2;
    
      } else {
      state = 0;
      }
     }

   switch (state){
    
    case 0: //recto
     moverMotorIzquierdo(255);
     moverMotorDerecho(255);
     break;
     
    case 1: //derecha
     moverMotorIzquierdo(255);
     moverMotorDerecho(155); //reducimos la velocidad en el derecho para girar a la derecha
     break;
    
    case 2: //izquierda
     moverMotorIzquierdo(155); //ídem para el motor izquierdo
     moverMotorDerecho(255);
     break;
   }
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
