// Sustituimos por los pines del ADC1 recomendados
int PINES[6] = {36, 39, 32, 33, 34, 35}; 

// Pines del Driver DRV8833
const int IN1 = 16; // Motor Izquierdo Adelante
const int IN2 = 17; // Motor Izquierdo Atrás
const int IN3 = 18; // Motor Derecho Adelante
const int IN4 = 19; // Motor Derecho Atrás

// Velocidad base cuando el robot va recto por la línea (0 a 255)
const int VEL_BASE = 150;

// trabajamos con cada sensor de manera independiente
int maximos[6] = {4096, 4096, 4096, 4096, 4096, 4096};
int minimos[6] = {0, 0, 0, 0, 0, 0};
int pesos[6] = {-3000, -2000, -1000, 1000, 2000, 3000};
bool resultado[6];

int sumaResultados = 0;

long prevMillis = 0;


int valor = 0;
int errorTotal = 0;
int errorPrevio = 0;

int proError = 0;
int derError = 0;

int correctVal = 0;

float kD = 0;
float kP = 0.0;

// Calibración del módulo
void setup() {
  Serial.begin(115200); // INICIALIZAMOS SERIAL PARA PODER VER LOS PRINT

  for(int i = 0; i < 6; i++){ // Configuramos los pines de forma ordenada
    // CORREGIDO: Usamos PINES[i] en lugar de i
    pinMode(PINES[i], INPUT); 
  }

  for(int j = 0; j < 100; j++){ // 100 lecturas
    for(int k = 0; k < 6; k++){ // en cada lectura, hacemos lectura de los 8 sensores
      // CORREGIDO: Usamos PINES[k] en lugar de k
      valor = analogRead(PINES[k]); 
      
      // CORREGIDO: La asignación original estaba al revés (valor = minimos[k])
      if(valor < minimos[k]) minimos[k] = valor;  // actualizamos el minimo
      if(valor > maximos[k]) maximos[k] = valor;  // actualizamos el maximo
    }
    delay(50); // el delay funciona en milisegundos, 100*50=5000=5 segundos para calibrar el robot
  }

  // Yo recomiendo agregar que muestre el valor de calibración, y recuerdo, calibramos sobre fondo blanco, cualquier valor mayor que de mayor que el de la medición,
  // lo detectará como HIGH

  Serial.println("Calibración completada. Valores límite:"); // print con salto de linea
  
  Serial.print("Mínimos: ");
  for(int a = 0; a < 6; a++){
    Serial.print(minimos[a]);
    Serial.print(" ");
  }
  Serial.println(""); // salto de linea
  
  Serial.print("Máximos: ");
  for(int b = 0; b < 6; b++){
    Serial.print(maximos[b]);
    Serial.print(" ");
  }
  Serial.println("");



  // Configuración de motores como salidas
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}

void loop() {
  
  leerLinea();
  
  calcularError();
  PIDControl();
  motorHandle();

  long currentTime = millis();

  if (currentTime-prevMillis >= 1500){
    mostrarValores();
    
    prevMillis = millis();
    
  }
}

void leerLinea(){  // lectura, y valoramos si es fondo o linea blanca

  for (int i = 0; i < 6; i++) {
    // CORREGIDO: Usamos PINES[i] en lugar de i
    int estado = analogRead(PINES[i]); 
    int umbral = minimos[i] + ((maximos[i] - minimos[i]) / 2); // un umbral para permitir que tenga cierto imperfecto. Podeis elegir cuanto porcien
    resultado[i] = (estado > umbral) ? 1 : 0;  // es una variable condicional, si se cumple, elige el valor a la izquierda de los dos puntos, sino el de la derecha
  }
}

void mostrarValores(){
    // muestreo
  Serial.print("Estado actual: ");
  for(int j = 0; j < 6; j++){
    Serial.print(resultado[j]);
    Serial.print(" ");
  }
  Serial.println(" ");

  Serial.print("Suma resultados: ");
  Serial.println(sumaResultados);
  
  Serial.print("Error actual: ");
  Serial.println(errorTotal);

  Serial.println();

  Serial.print("CorrectVal");
  Serial.println(correctVal);
}

void calcularError(){
  // 1. Reiniciamos a 0 en cada ciclo para no acumular basura de lecturas anteriores
  sumaResultados = 0;
  errorTotal = 0;

  for(int i = 0; i<6; i++){
    sumaResultados += resultado[i];
  }
  

  
  // 2. Si ningún sensor detecta la línea, salimos de la función para evitar dividir por 0
  if (sumaResultados == 0) {
    return; 
  }

  for(int i = 0; i<6; i++){
    if (resultado[i]){ // Si el sensor ve la línea (es 1/true)
      // 3. Usamos pesos[i] para coger el valor correspondiente a este sensor
      errorTotal += (pesos[i] / sumaResultados); 
    }
  }
  
  // Opcional para depurar: ver el error calculado

}

void PIDControl (){

  proError = kP*errorTotal;
  derError = kD*(errorTotal-errorPrevio);
  correctVal = proError+derError;

  errorPrevio = errorTotal;
  
  }


void motorHandle() {
  // 1. Calculamos la velocidad objetivo de cada rueda
  int velMotIzq = VEL_BASE + correctVal;
  int velMotDer = VEL_BASE - correctVal;

  // 2. Acotamos matemáticamente para no salirnos del límite del PWM (-255 a 255)
  velMotIzq = constrain(velMotIzq, -255, 255);
  velMotDer = constrain(velMotDer, -255, 255);

  // 3. Enviamos las órdenes físicas al driver
  moverMotorIzquierdo(velMotIzq);
  moverMotorDerecho(velMotDer);
}

// --- FUNCIONES FÍSICAS DEL DRV8833 ---

void moverMotorIzquierdo(int velocidad) {
  if (velocidad >= 0) {
    // Si la velocidad es positiva, avanzamos
    analogWrite(IN1, velocidad);
    digitalWrite(IN2, LOW);
  } else {
    // Si es negativa, invertimos la polaridad para frenar girando hacia atrás
    // (Multiplicamos por -1 para pasar la velocidad a un valor positivo válido para el PWM)
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
