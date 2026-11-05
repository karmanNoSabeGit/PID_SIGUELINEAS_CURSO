// Sustituimos por los pines del ADC1 recomendados
int PINES[6] = {36, 39, 32, 33, 34, 35}; 

// trabajamos con cada sensor de manera independiente
int minimos[6] = {4096, 4096, 4096, 4096, 4096, 4096};
int maximos[6] = {0, 0, 0, 0, 0, 0};
bool resultado[6];

int valor = 0;

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
}

void loop() {
  // lectura, y valoramos si es fondo o linea blanca
  for (int i = 0; i < 6; i++) {
    // CORREGIDO: Usamos PINES[i] en lugar de i
    int estado = analogRead(PINES[i]); 
    int umbral = minimos[i] + ((maximos[i] - minimos[i]) / 2); // un umbral para permitir que tenga cierto imperfecto. Podeis elegir cuanto porcien
    resultado[i] = (estado > umbral) ? 1 : 0;  // es una variable condicional, si se cumple, elige el valor a la izquierda de los dos puntos, sino el de la derecha
  }
  
  // muestreo
  Serial.print("Estado actual: ");
  for(int j = 0; j < 6; j++){
    Serial.print(resultado[j]);
    Serial.print(" ");
  }
  Serial.println(" ");
  
  delay(50);  // hace una medicion cada 50ms
}
