#include <WiFi.h>
#include <WiFiUdp.h>

// --- CONFIGURACIÓN WI-FI (MODO PUNTO DE ACCESO) ---
const char* SSID_AP = "Siguelineas_ESP32";
const char* PASS_AP = "12345678";
const unsigned int PUERTO_UDP = 4210;

WiFiUDP udp;
IPAddress pcIP;
unsigned int pcPort = 0;
bool pcConectado = false;

// --- PINES HARDWARE ---
// Todos en ADC1: 100% compatibles con Wi-Fi activo
int PINES[6] = {36, 39, 32, 33, 34, 35};

const int IN1 = 16; // Motor Izquierdo Adelante
const int IN2 = 17; // Motor Izquierdo Atrás
const int IN3 = 18; // Motor Derecho Adelante
const int IN4 = 19; // Motor Derecho Atrás

// --- VARIABLES DEL PID BINARIO (VERSIÓN ESTABLE) ---
int velBase = 180;
bool enMarcha = false; // Arranca parado por seguridad hasta que pulses 'g' o cambies 'v'

int maximos[6] = {0, 0, 0, 0, 0, 0};
int minimos[6] = {4095, 4095, 4095, 4095, 4095, 4095};
int lectura_binaria[6];
int pesos[6] = {-24, -16, -8, 8, 16, 24};

float kP = 10.0;
float kD = 0.0;

float error = 0.0;
float prev_error = 0.0;
float propoError = 0.0;
float derivError = 0.0;
float valor_velocidad = 0.0;
int velMotIzq = 0;
int velMotDer = 0;

// --- TEMPORIZADORES ---
unsigned long prevControlMillis = 0;
unsigned long prevTelemetryMillis = 0;
const unsigned long INTERVALO_CONTROL = 10;     // PID a 100 Hz
const unsigned long INTERVALO_TELEMETRIA = 20;  // Telemetría a 50 Hz

// Prototipos
void leerLinea();
float calcularError();
float PIDControl(float errorTotal, float errorPrevio);
void motorHandle(float correctVal);
void moverMotorIzquierdo(int velocidad);
void moverMotorDerecho(int velocidad);
void gestionarComandosUDP();
void enviarTelemetriaUDP();

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 6; i++) {
    pinMode(PINES[i], INPUT);
  }
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  moverMotorIzquierdo(0);
  moverMotorDerecho(0);

  // 1. Calibración de sensores (5 segundos moviendo el robot sobre la línea)
  for (int j = 0; j < 100; j++) {
    for (int k = 0; k < 6; k++) {
      int valor = analogRead(PINES[k]);
      if (valor < minimos[k]) minimos[k] = valor;
      if (valor > maximos[k]) maximos[k] = valor;
    }
    delay(50);
  }

  // 2. Arrancamos el punto de acceso Wi-Fi con latencia mínima
  WiFi.mode(WIFI_AP);
  WiFi.softAP(SSID_AP, PASS_AP);
  WiFi.setSleep(false); // Desactiva ahorro de energía para eliminar picos de latencia
  udp.begin(PUERTO_UDP);

  Serial.print("Wi-Fi listo. IP del robot: ");
  Serial.println(WiFi.softAPIP()); // Por defecto: 192.168.4.1
}

void loop() {
  // 1. Escucha no bloqueante de paquetes UDP desde el ordenador
  gestionarComandosUDP();

  unsigned long currentMillis = millis();

  // 2. Bucle de control PID a 100 Hz
  if (currentMillis - prevControlMillis >= INTERVALO_CONTROL) {
    prevControlMillis = currentMillis;

    leerLinea();
    prev_error = error;
    error = calcularError();
    valor_velocidad = PIDControl(error, prev_error);

    if (enMarcha) {
      motorHandle(valor_velocidad);
    } else {
      velMotIzq = 0;
      velMotDer = 0;
      moverMotorIzquierdo(0);
      moverMotorDerecho(0);
    }
  }

  // 3. Envío de telemetría por UDP a 50 Hz (solo si el PC ya se ha presentado)
  if (pcConectado && (currentMillis - prevTelemetryMillis >= INTERVALO_TELEMETRIA)) {
    prevTelemetryMillis = currentMillis;
    enviarTelemetriaUDP();
  }
}

void leerLinea() {
  for (int i = 0; i < 6; i++) {
    int lectura = analogRead(PINES[i]);
    int umbral = (minimos[i] + maximos[i]) / 2;
    lectura_binaria[i] = (lectura > umbral) ? 1 : 0;
  }
}

float calcularError() {
  int sumaPonderada = 0;
  int sensoresActivos = 0;

  for (int i = 0; i < 6; i++) {
    if (lectura_binaria[i] == 1) {
      sumaPonderada += pesos[i];
      sensoresActivos++;
    }
  }

  if (sensoresActivos == 0) {
    return prev_error;
  }

  return (float)sumaPonderada / sensoresActivos;
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

// --- MÓDULO DE TELEMETRÍA WI-FI UDP ---

void enviarTelemetriaUDP() {
  // Usamos char[] fijo y snprintf para no fragmentar la memoria RAM (evita usar String)
  char buffer[64];
  int len = snprintf(buffer, sizeof(buffer), "%lu,%.1f,%.1f,%.1f,%d,%d",
                     millis(), error, propoError, derivError, velMotIzq, velMotDer);

  udp.beginPacket(pcIP, pcPort);
  udp.write((const uint8_t*)buffer, len);
  udp.endPacket();
}

void gestionarComandosUDP() {
  int packetSize = udp.parsePacket();
  if (packetSize <= 0) return;

  char cmdBuffer[64];
  int len = udp.read(cmdBuffer, sizeof(cmdBuffer) - 1);
  if (len <= 0) return;
  cmdBuffer[len] = '\0';

  // Guardamos automáticamente la IP y puerto del ordenador para responder en unicast (máxima velocidad)
  pcIP = udp.remoteIP();
  pcPort = udp.remotePort();
  pcConectado = true;

  char cmd = tolower(cmdBuffer[0]);
  if (cmd == 'h') {
    // Paquete "HELLO" inicial del script de Python: no hace falta cambiar nada
    return;
  } else if (cmd == 'p') {
    kP = atof(cmdBuffer + 1);
  } else if (cmd == 'd') {
    kD = atof(cmdBuffer + 1);
  } else if (cmd == 'v') {
    velBase = constrain(atoi(cmdBuffer + 1), 0, 255);
  } else if (cmd == 'g') {
    enMarcha = true;  // Arrancar motores
  } else if (cmd == 'r') {
    enMarcha = false; // Parada de emergencia real (corta motores a 0)
  }
}

// --- CONTROL FÍSICO DRV8833 ---

void moverMotorIzquierdo(int velocidad) {
  if (velocidad >= 0) {
    analogWrite(IN1, velocidad);
    digitalWrite(IN2, LOW);
  } else {
    digitalWrite(IN1, LOW);
    analogWrite(IN2, -velocidad);
  }
}

void moverMotorDerecho(int velocidad) {
  if (velocidad >= 0) {
    analogWrite(IN3, velocidad);
    digitalWrite(IN4, LOW);
  } else {
    digitalWrite(IN3, LOW);
    analogWrite(IN4, -velocidad);
  }
}
