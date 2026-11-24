// Pines del ADC1 recomendados en ESP32 (no usar ADC2 si luego metemos WiFi)
int PINES[6] = {36, 39, 32, 33, 34, 35}; // VP 36, VN 39

// Pines del Driver DRV8833 (puente en H para los motores)
const int IN1 = 16; // Motor Izquierdo Adelante
const int IN2 = 17; // Motor Izquierdo Atrás
const int IN3 = 18; // Motor Derecho Adelante
const int IN4 = 19; // Motor Derecho Atrás

// Velocidad de crucero cuando el robot va recto por la línea (0 a 255)
// Modificable en caliente desde el monitor serie (ej: v0 para pruebas en mesa)
int velBase = 180;

// Calibración individual por sensor (ADC de 12 bits: 0 a 4095)
// Se inicializan al revés para que la primera lectura real los sobrescriba
int maximos[6] = {0, 0, 0, 0, 0, 0};
int minimos[6] = {4095, 4095, 4095, 4095, 4095, 4095};

// Pesos normalizados en porcentaje de desviación (-100% izquierda a +100% derecha)
// Distancia constante de 40 unidades entre cada pareja de sensores
float pesos[6] = {-100.0, -60.0, -20.0, 20.0, 60.0, 100.0};

int valores_lecturas[6];     // Lectura bruta del ADC (0 - 4095)
int lectura_normalizada[6];  // Lectura calibrada de cada sensor (0 = blanco, 1000 = negro)

// Temporizadores no bloqueantes
unsigned long prevControlMillis = 0;
unsigned long prevTelemetryMillis = 0;
const unsigned long INTERVALO_CONTROL = 10;     // PID cada 10 ms (100 Hz)
const unsigned long INTERVALO_TELEMETRIA = 35;  // Gráfica cada 35 ms (~28 FPS)

// Variables de control en coma flotante para tener curvas suaves sin escalones
float error = 0.0;
float prev_error = 0.0;
float propoError = 0.0;
float derivError = 0.0;
float valor_velocidad = 0.0;
int velMotIzq = 0;
int velMotDer = 0;

// Constantes PID escaladas para el rango [-100, +100]
// Con error máximo de 100 y kP = 2.5, la corrección máxima es 250 (casi el tope de 255 PWM)
float kP = 2 ;
float kD = 6.0;

// Prototipos de funciones
void leerLinea();
float calcularError();
float PIDControl(float errorTotal, float errorPrevio);
void motorHandle(float correctVal);
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

  // Fase de calibración: pasamos el robot de lado a lado sobre la línea (5 segundos)
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

  // Resumen inicial (usamos '=' y no ':' para no interferir con el Serial Plotter)
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

    // Protección por si algún sensor no se calibró bien (evita división por 0 dentro de map)
    if (maximos[i] <= minimos[i]) {
      lectura_normalizada[i] = 0;
    } else {
      // Estiramos el rango real de cada sensor a una escala común de 0 (blanco) a 1000 (negro)
      int norm = map(estado, minimos[i], maximos[i], 0, 1000);
      // Recortamos por si durante la carrera aparece un blanco más brillante o un negro más oscuro
      lectura_normalizada[i] = constrain(norm, 0, 1000);
    }
  }
}

float calcularError() {
  float sumaPonderada = 0.0;
  float sumaIntensidades = 0.0;

  for (int i = 0; i < 6; i++) {
    sumaPonderada += lectura_normalizada[i] * pesos[i];
    sumaIntensidades += lectura_normalizada[i];
  }

  // Detección real de esquina de 90° (tapa extremo + intermedio + centro a la vez)
  // Si solo pisa el extremo al rebotar de una curva, NO entra aquí y usa el PID suave
  /* if (lectura_normalizada[0] > 700 && lectura_normalizada[1] > 500 && lectura_normalizada[2] > 400) {
    return -100.0;
  } 
  if (lectura_normalizada[5] > 700 && lectura_normalizada[4] > 500 && lectura_normalizada[3] > 400) {
    return 100.0;
  } */

  // Filtro de pérdida de línea: si se sale del todo, mantiene el giro máximo hacia el último lado
  if (sumaIntensidades < 250.0) {
    if (prev_error > 20.0) return 100.0;
    if (prev_error < -20.0) return -100.0;
    return prev_error;
  }

  // Centro de gravedad continuo para rectas y curvas normales
  return (sumaPonderada / sumaIntensidades);
}

float PIDControl(float errorTotal, float errorPrevio) {
  propoError = kP * errorTotal;
  derivError = kD * (errorTotal - errorPrevio);
    
  return (propoError + derivError);
}

void motorHandle(float correctVal) {
  velMotIzq = constrain((int)(velBase + correctVal), -255, 255);
  velMotDer = constrain((int)(velBase - correctVal), -255, 255);

  moverMotorIzquierdo(velMotIzq);
  moverMotorDerecho(velMotDer);
}

// --- TELEMETRÍA Y CONTROL EN CALIENTE ---

void enviarTelemetriaPlotter() {
  Serial.print("Error:");
  Serial.print(error, 1); // 1 decimal para ver la suavidad de la curva
  Serial.print(",P:");
  Serial.print(propoError, 1);
  Serial.print(",D:");
  Serial.print(derivError, 1);
  Serial.print(",MotIzq:");
  Serial.print(velMotIzq);
  Serial.print(",MotDer:");
  Serial.println(velMotDer);
}

void leerComandosSerial() {
  if (Serial.available() > 0) {
    char cmd = tolower(Serial.read());

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

    while (Serial.available() > 0 && Serial.read() != '\n') {}

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
