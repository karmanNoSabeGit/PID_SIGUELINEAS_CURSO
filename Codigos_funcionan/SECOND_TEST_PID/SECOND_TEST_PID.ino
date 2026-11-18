/*
 * El robot tiene pesos que, como máximo, alteran 24 unidades
 * Si el valor de la kP es pequeño, como 1 o 2,5; entonces 
 * la alteración máxima que puede hacer el PID es de apenas 
 * 25 o 50 unidades de PWM. El robot trabaja con un PWM que va
 * desde 0 a 255 pulsos, con el valor de trabajo entre 150 y 255, aprox
 * Matemáticamente, una kP de alrededor de 10 es la más ópitima
 * porque 255/24 (rango de posibilidades/modificación cada vez) ~= 10
 * 
 * Si queremos una kP de ~1, entonces los pesos se deben multiplicar por 10
*/
// Pines del ADC1 recomendados en ESP32
int PINES[6] = {36, 39, 32, 33, 34, 35}; // VP 36, VN 39

// Pines del Driver DRV8833
const int IN1 = 16; // Motor Izquierdo Adelante
const int IN2 = 17; // Motor Izquierdo Atrás
const int IN3 = 18; // Motor Derecho Adelante
const int IN4 = 19; // Motor Derecho Atrás

// Velocidad base cuando el robot va recto por la línea (0 a 255)
const int VEL_BASE = 180;

// Corrección 1: Inicialización en extremos opuestos para captura real en ADC de 12 bits (0-4095)
int maximos[6] = {0, 0, 0, 0, 0, 0};
int minimos[6] = {4095, 4095, 4095, 4095, 4095, 4095};

int32_t pesos[6] = {-24, -16, -8, 8, 16, 24};
int valores_lecturas[6];
bool resultado[6];

// Corrección 2: Tipo sin signo para registros temporales
unsigned long prevMillis = 0;

int32_t error = 0;
int32_t prev_error = 0;
int valor_velocidad = 0;

int kD = 1;
int kP = 15;

// Prototipos de funciones
void leerLinea();
void mostrarvalor_sensores(int32_t error, int correctVal);
int32_t calcularError();
int32_t PIDControl(int32_t errorTotal, int32_t errorPrevio);
void motorHandle(int32_t correctVal);
void moverMotorIzquierdo(int velocidad);
void moverMotorDerecho(int velocidad);

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 6; i++) {
    pinMode(PINES[i], INPUT);
  }

  // Configuración de motores como salidas
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  int valor_sensor = 0;

  // Fase de calibración: 100 muestras * 50 ms = 5000 ms (5 segundos)
  for (int j = 0; j < 100; j++) {
    for (int k = 0; k < 6; k++) {
      valor_sensor = analogRead(PINES[k]);

      if (valor_sensor < minimos[k]) {
        minimos[k] = valor_sensor;
      }

      if (valor_sensor > maximos[k]) {
        maximos[k] = valor_sensor;
      }
    } // Corrección 3: Cierre explícito del bucle interno de sensores
    
    // Corrección 4: Retardo ubicado en el bucle principal de muestreo
    delay(50);
  }

  Serial.println("Calibración completada. Valores límite:");

  Serial.print("Mínimos: ");
  for (int a = 0; a < 6; a++) {
    Serial.print(minimos[a]);
    Serial.print(" ");
  }
  Serial.println();

  Serial.print("Máximos: ");
  for (int b = 0; b < 6; b++) {
    Serial.print(maximos[b]);
    Serial.print(" ");
  }
  Serial.println();
}

void loop() {
  leerLinea();
  prev_error = error;
  error = calcularError();
  valor_velocidad = PIDControl(error, prev_error);
  motorHandle(valor_velocidad);

  // Corrección 2: Tipo sin signo
  unsigned long currentTime = millis();

  if (currentTime - prevMillis >= 1500) {
    mostrarvalor_sensores(error, valor_velocidad);
    prevMillis = currentTime;
  }
}

void leerLinea() {
  for (int i = 0; i < 6; i++) {
    int estado = analogRead(PINES[i]);
    valores_lecturas[i] = estado;
    int umbral = minimos[i] + ((maximos[i] - minimos[i]) / 2);
    resultado[i] = (estado > umbral) ? 1 : 0;
  }
}

void mostrarvalor_sensores(int32_t errorActual, int correctVal) {
  Serial.print("Estado actual: ");
  for (int j = 0; j < 6; j++) {
    Serial.print(resultado[j]);
    Serial.print(" ");
  }
  Serial.println();

  

  Serial.print("Lecturas: ");
  for (int w = 0; w < 6; w++) {
    Serial.print(valores_lecturas[w]);
    Serial.print(" ");
  }
  Serial.println();
  
  Serial.print("Error: ");
  Serial.println(errorActual);
  Serial.println();
  
  Serial.print("CorrectVal (se suma o resta a la velocidad base: ");
  Serial.println(correctVal);
  Serial.println();

  int velMotIzq = VEL_BASE + correctVal;
  int velMotDer = VEL_BASE - correctVal;

  velMotIzq = constrain(velMotIzq, -255, 255);
  velMotDer = constrain(velMotDer, -255, 255);

  Serial.print("velMotIzq: ");
  Serial.print(velMotIzq);
  Serial.print("  velMotDer: ");
  Serial.println(velMotDer);
  Serial.println();
}

int32_t calcularError() {
  int32_t sumaResultadosPesados = 0;
  int sensoresActivos = 0;

  for (int i = 0; i < 6; i++) {
    sumaResultadosPesados += resultado[i] * pesos[i];
    sensoresActivos += resultado[i];
  }

  // Corrección 5: Protección efectiva contra división entre cero.
  // Si se pierde la línea (0 sensores activos), se conserva el último error conocido
  // para mantener la dirección de búsqueda.
  if (sensoresActivos == 0) {
    return prev_error;
  }

  return (sumaResultadosPesados / sensoresActivos);
}

int32_t PIDControl(int32_t errorTotal, int32_t errorPrevio) {
  int32_t propoError = kP * errorTotal;
  int32_t derivError = kD * (errorTotal - errorPrevio);
  
  // Corrección 6: Eliminada asignación redundante sobre parámetro pasado por valor
  return (propoError + derivError);
}

void motorHandle(int32_t correctVal) {
  int velMotIzq = VEL_BASE + correctVal;
  int velMotDer = VEL_BASE - correctVal;

  velMotIzq = constrain(velMotIzq, -255, 255);
  velMotDer = constrain(velMotDer, -255, 255);

  moverMotorIzquierdo(velMotIzq);
  moverMotorDerecho(velMotDer);
}

// --- FUNCIONES FÍSICAS DEL DRV8833 ---

void moverMotorIzquierdo(int velocidad) {
  if (velocidad >= 0) {
    analogWrite(IN1, velocidad);
    digitalWrite(IN2, LOW);
  } else {
    // Corrección 7: Habilitada inversión de giro para velocidades negativas
    digitalWrite(IN1, LOW);
    analogWrite(IN2, velocidad * -1);
  }
}

void moverMotorDerecho(int velocidad) {
  if (velocidad >= 0) {
    analogWrite(IN3, velocidad);
    digitalWrite(IN4, LOW);
  } else {
    // Corrección 7: Habilitada inversión de giro para velocidades negativas
    digitalWrite(IN3, LOW);
    analogWrite(IN4, velocidad * -1);
  }
}
