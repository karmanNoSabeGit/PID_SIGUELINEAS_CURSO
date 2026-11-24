// Pines del ADC1 recomendados en ESP32
int PINES[6] = {36, 39, 32, 33, 34, 35}; // VP 36, VN 39

// Pines del Driver DRV8833
const int IN1 = 16; // Motor Izquierdo Adelante
const int IN2 = 17; // Motor Izquierdo Atrás
const int IN3 = 18; // Motor Derecho Adelante
const int IN4 = 19; // Motor Derecho Atrás

// Velocidad base modifiable en caliente (usa v0 en el monitor serie para pruebas en mesa)
int velBase = 180;

// Calibración ADC de 12 bits (0-4095)
int maximos[6] = {0, 0, 0, 0, 0, 0};
int minimos[6] = {4095, 4095, 4095, 4095, 4095, 4095};

int32_t pesos[6] = {-24, -16, -8, 8, 16, 24};//{-100, -60, -20, 20, 60, 100}; OTROS VALORES
int valores_lecturas[6];
bool resultado[6];

// Temporizadores no bloqueantes
unsigned long prevControlMillis = 0;
unsigned long prevTelemetryMillis = 0;
const unsigned long INTERVALO_CONTROL = 10;     // PID cada 10 ms (100 Hz)
const unsigned long INTERVALO_TELEMETRIA = 35;  // Gráfica cada 35 ms (~28 FPS)

// Variables de control y telemetría
int32_t error = 0;
int32_t prev_error = 0;
int32_t propoError = 0;
int32_t derivError = 0;
int32_t valor_velocidad = 0;
int velMotIzq = 0;
int velMotDer = 0;

// Constantes PID en coma flotante para permitir ajustes finos
float kP = 9.5 ;
float kD = 1.0;

// Prototipos de funciones
void leerLinea();
int32_t calcularError();
int32_t PIDControl(int32_t errorTotal, int32_t errorPrevio);
void motorHandle(int32_t correctVal);
void moverMotorIzquierdo(int velocidad);
void moverMotorDerecho(int velocidad);
void leerComandosSerial();
void enviarTelemetriaPlotter();

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 6; i++) {
    pinMode(PINES[i], INPUT);
  }

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  int valor_sensor = 0;

  // Fase de calibración: 100 muestras * 50 ms = 5 segundos
  for (int j = 0; j < 100; j++) {
    for (int k = 0; k < 6; k++) {
      valor_sensor = analogRead(PINES[k]);

      if (valor_sensor < minimos[k]) {
        minimos[k] = valor_sensor;
      }

      if (valor_sensor > maximos[k]) {
        maximos[k] = valor_sensor;
      }
    }
    delay(50);
  }

  // Resumen inicial (sin usar ':' para no interferir con el Serial Plotter)
  Serial.println("# Calibracion completada");
  Serial.print("# Minimos = ");
  for (int a = 0; a < 6; a++) {
    Serial.print(minimos[a]);
    Serial.print(" ");
  }
  Serial.println();

  Serial.print("# Maximos = ");
  for (int b = 0; b < 6; b++) {
    Serial.print(maximos[b]);
    Serial.print(" ");
  }
  Serial.println();
  Serial.println("# Comandos -> p<num> (kP) | d<num> (kD) | v<num> (velBase) | r (parar)");
}

void loop() {
  // 1. Escucha continua de comandos desde el PC
  leerComandosSerial();

  unsigned long currentMillis = millis();

  // 2. Bucle de control PID a frecuencia fija (100 Hz)
  if (currentMillis - prevControlMillis >= INTERVALO_CONTROL) {
    prevControlMillis = currentMillis;

    leerLinea();
    prev_error = error;
    error = calcularError();
    valor_velocidad = PIDControl(error, prev_error);
    motorHandle(valor_velocidad);
  }

  // 3. Envío de datos al Serial Plotter (~28 Hz)
  if (currentMillis - prevTelemetryMillis >= INTERVALO_TELEMETRIA) {
    prevTelemetryMillis = currentMillis;
    enviarTelemetriaPlotter();
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

int32_t calcularError() {
  int32_t sumaResultadosPesados = 0;
  int sensoresActivos = 0;

  for (int i = 0; i < 6; i++) {
    sumaResultadosPesados += resultado[i] * pesos[i];
    sensoresActivos += resultado[i];
  }

  // Si se pierde la línea, conserva el último error conocido
  if (sensoresActivos == 0) {
    return prev_error;
  }

  return (sumaResultadosPesados / sensoresActivos);
}

int32_t PIDControl(int32_t errorTotal, int32_t errorPrevio) {
  propoError = kP * errorTotal;
  derivError = kD * (errorTotal - errorPrevio);
  return (propoError + derivError);
}

void motorHandle(int32_t correctVal) {
  velMotIzq = constrain(velBase + correctVal, -255, 255);
  velMotDer = constrain(velBase - correctVal, -255, 255);

  moverMotorIzquierdo(velMotIzq);
  moverMotorDerecho(velMotDer);
}

// --- TELEMETRÍA Y CONTROL EN CALIENTE ---

void enviarTelemetriaPlotter() {
  Serial.print("Error:");
  Serial.print(error);
  Serial.print(",P:");
  Serial.print(propoError);
  Serial.print(",D:");
  Serial.print(derivError);
  Serial.print(",MotIzq:");
  Serial.print(velMotIzq);
  Serial.print(",MotDer:");
  Serial.println(velMotDer);
}

void leerComandosSerial() {
  if (Serial.available() > 0) {
    char cmd = tolower(Serial.read());

    // Ignorar saltos de línea residuales
    if (cmd == '\n' || cmd == '\r' || cmd == ' ') {
      return;
    }

    if (cmd == 'p') {
      kP = Serial.parseFloat();
    } else if (cmd == 'd') {
      kD = Serial.parseFloat();
    } else if (cmd == 'v') {
      velBase = constrain(Serial.parseInt(), 0, 255);
    } else if (cmd == 'r') {
      velBase = 0;
    }

    // Limpiar el resto de caracteres hasta el salto de línea
    while (Serial.available() > 0 && Serial.read() != '\n') {}

    // Confirmación usando '=' en lugar de ':' para no ensuciar las gráficas
    Serial.print("# Actualizado -> kP=");
    Serial.print(kP);
    Serial.print(" | kD=");
    Serial.print(kD);
    Serial.print(" | velBase=");
    Serial.println(velBase);
  }
}

// --- FUNCIONES FÍSICAS DEL DRV8833 ---

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
