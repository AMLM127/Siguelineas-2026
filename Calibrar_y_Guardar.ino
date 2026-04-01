#include <QTRSensors.h>
#include <EEPROM.h>

QTRSensors qtr;
const uint8_t SensorCount = 7;
uint16_t sensorValues[SensorCount];
const uint8_t sensorPins[] = {A8, A9, A10, A11, A12, A13, A14};

void setup() {
  qtr.setTypeRC();
  qtr.setSensorPins(sensorPins, SensorCount);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); 

  Serial.begin(9600);
  Serial.println("--- INICIANDO CALIBRACIÓN (Mueve el sensor) ---");

  for (uint16_t i = 0; i < 400; i++) {
    qtr.calibrate();
    delay(10);
  }

  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("Calibración terminada.");

  // Mostrar reporte de valores detectados
  Serial.println("\n--- REPORTE DE CALIBRACION ---");
  Serial.print("MINS: ");
  for (uint8_t i = 0; i < SensorCount; i++) {
    Serial.print(qtr.calibrationOn.minimum[i]);
    Serial.print("\t");
  }
  Serial.print("\nMAXS: ");
  for (uint8_t i = 0; i < SensorCount; i++) {
    Serial.print(qtr.calibrationOn.maximum[i]);
    Serial.print("\t");
  }
  Serial.println("\n------------------------------");

  guardarCalibracion();
}

void loop() {
  // Opcional: ver lectura en tiempo real
  uint16_t position = qtr.readLineBlack(sensorValues);
  Serial.print("Pos: "); Serial.print(position);
  Serial.print(" | Val: ");
  for (uint8_t i = 0; i < SensorCount; i++) {
    Serial.print(sensorValues[i]); Serial.print(" ");
  }
  Serial.println();
  delay(100);
}

void guardarCalibracion() {
  int direccion = 0;
  for (uint8_t i = 0; i < SensorCount; i++) {
    EEPROM.put(direccion, qtr.calibrationOn.minimum[i]);
    direccion += sizeof(uint16_t);
  }
  for (uint8_t i = 0; i < SensorCount; i++) {
    EEPROM.put(direccion, qtr.calibrationOn.maximum[i]);
    direccion += sizeof(uint16_t);
  }
  Serial.println(">>> EEPROM Actualizada con éxito.");
}