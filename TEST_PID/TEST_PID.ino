// Sustituimos por los pines del ADC1 recomendados
int PINES[6] = {36, 39, 32, 33, 34, 35}; //VP 36, VN 39 

// Pines del Driver DRV8833
const int IN1 = 16; // Motor Izquierdo Adelante
const int IN2 = 17; // Motor Izquierdo Atrás
const int IN3 = 18; // Motor Derecho Adelante
const int IN4 = 19; // Motor Derecho Atrás

// Velocidad base cuando el robot va recto por la línea (0 a 255)
const int VEL_BASE = 150;

// trabajamos con cada sensor de manera independiente
int minimos[6] = {4096, 4096, 4096, 4096, 4096, 4096};
int maximos[6] = {0, 0, 0, 0, 0, 0};
int32_t pesos[6] = {-24, -16, -8, 8, 16, 24};
int valores_lecturas[6];
bool resultado[6];


long prevMillis = 0;

int32_t error;
int32_t prev_error;
int valor_velocidad;


int kD = 0; //variables
int kP = 10;

// Calibración del módulo
void setup() {
  Serial.begin(115200); // INICIALIZAMOS SERIAL PARA PODER VER LOS PRINT

  for(int i = 0; i < 6; i++){ // Configuramos los pines de forma ordenada
    // CORREGIDO: Usamos PINES[i] en lugar de i
    pinMode(PINES[i], INPUT); 
  }
  int valor_sensor = 0;

  for(int j = 0; j < 100; j++){ // 100 lecturas
    for(int k = 0; k < 6; k++){ // en cada lectura, hacemos lectura de los 8 sensores
      // CORREGIDO: Usamos PINES[k] en lugar de k
      valor_sensor = analogRead(PINES[k]); 
      
      
      // CORREGIDO: La asignación original estaba al revés (valor_sensor = minimos[k])
      if(valor_sensor < minimos[k]){
        minimos[k] = valor_sensor;  // actualizamos el minimo
      }
      
      if(valor_sensor > maximos[k]) {
        maximos[k] = valor_sensor;  // actualizamos el maximo
      }
    }
    delay(50); // el delay funciona en milisegundos, 100*50=5000=5 segundos para calibrar el robot
  }

  // Yo recomiendo agregar que muestre el valor_sensor de calibración, y recuerdo, calibramos sobre fondo blanco, cualquier valor_sensor mayor que de mayor que el de la medición,
  // lo detectará como HIGH

  Serial.println("Calibración completada. valor_sensores límite:"); // print con salto de linea
  
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
  prev_error = error;
  error = calcularError();
  valor_velocidad = PIDControl(error, prev_error);
  motorHandle(valor_velocidad);

  long currentTime = millis();

  if (currentTime-prevMillis >= 1500){
    mostrarvalor_sensores(error, valor_velocidad);
    
    prevMillis = millis();
    
  }
}

void leerLinea(){  // lectura, y valor_sensor miramos si es fondo o linea blanca

  for (int i = 0; i < 6; i++) {
    // CORREGIDO: Usamos PINES[i] en lugar de i
    int estado = analogRead(PINES[i]);
    valores_lecturas[i] = estado; 
    int umbral = minimos[i] + ((maximos[i] - minimos[i]) / 2); // un umbral para permitir que tenga cierto imperfecto. Podeis elegir cuanto porcien
    resultado[i] = (estado > umbral) ? 1 : 0;  // es una variable condicional, si se cumple, elige el valor_sensor a la izquierda de los dos puntos, sino el de la derecha
  }
}

void mostrarvalor_sensores(int32_t error, int correctVal){
    // muestreo
  Serial.print("Estado actual: ");
  for(int j = 0; j < 6; j++){
    Serial.print(resultado[j]);
    Serial.print(" ");
  }
  Serial.println(" ");

  
  Serial.print("Error: ");
  Serial.println(error);
  Serial.println();

  Serial.print("Lecturas: ");
  for(int w = 0; w < 6; w++){
    Serial.print(valores_lecturas[w]);
    Serial.print(" ");
}
  Serial.println();

  Serial.print("CorrectVal: ");
  Serial.println(correctVal);
  Serial.println();

  
}

int32_t calcularError(){ //corregido sin probar
  int32_t error;
  // 1. Reiniciamos a 0 en cada ciclo para no acumular basura de lecturas anteriores
  int32_t sumaResultadosPesados = 0;
  int sensoresActivos = 0;

  for(int i = 0; i<6; i++){
    sumaResultadosPesados += resultado[i]*pesos[i]; //sensores activos
    sensoresActivos += resultado[i];
  }
  
  if (sensoresActivos != 0){
    return error; //error es el valor de error, no que lance una excepción
  }

  error = (sumaResultadosPesados/sensoresActivos);

  return error;
  
  // Opcional para depurar: ver el error calculado

}

int32_t PIDControl (int32_t errorTotal, int32_t errorPrevio){

  int32_t propoError = kP*errorTotal;
  int32_t derivError = kD*(errorTotal-errorPrevio);
  int32_t correctVal = propoError+derivError;

  errorPrevio = errorTotal;

  return correctVal;
  
  }


void motorHandle(int32_t correctVal) {
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
  } /*else {
    // Si es negativa, invertimos la polaridad para frenar girando hacia atrás
    // (Multiplicamos por -1 para pasar la velocidad a un valor_sensor positivo válido para el PWM)
    digitalWrite(IN1, LOW);
    analogWrite(IN2, velocidad * -1);
  }*/
}

void moverMotorDerecho(int velocidad) {
  if (velocidad >= 0) {
    analogWrite(IN3, velocidad);
    digitalWrite(IN4, LOW);
  } /*else {
    digitalWrite(IN3, LOW);
    analogWrite(IN4, velocidad * -1);
  }*/
}
