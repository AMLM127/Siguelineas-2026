#include <QTRSensors.h>
#include <EEPROM.h>
#include <MeMegapi.h>

QTRSensors qtr;
const uint8_t SensorCount = 7;
uint16_t sensorValues[SensorCount];
const uint8_t sensorPins[] = {A8, A9, A10, A11, A12, A13, A14};

void setup() {
  Serial.begin(9600);
  
  qtr.setTypeRC();
  qtr.setSensorPins(sensorPins, SensorCount);

  // Inicializar memoria interna
  qtr.calibrate(); 
  
  cargarCalibracion();
}

void loop() {
  // readLineBlack devuelve la posición (0 a 6000) y llena el array sensorValues
  uint16_t position = qtr.readLineBlack(sensorValues);

  Serial.print("SENSORES: ");
  for (uint8_t i = 0; i < SensorCount; i++) {
    Serial.print(sensorValues[i]);
    Serial.print("\t"); // Tabulación para que se vea ordenado
  }
  
  Serial.print("| POSICION: ");
  Serial.println(position);
  
  delay(50); // Un poco más rápido para ver la respuesta
}

void cargarCalibracion() {
  int direccion = 0;
  Serial.println("\n--- CARGANDO DESDE EEPROM ---");

  for (uint8_t i = 0; i < SensorCount; i++) {
    EEPROM.get(direccion, qtr.calibrationOn.minimum[i]);
    direccion += sizeof(uint16_t);
  }
  for (uint8_t i = 0; i < SensorCount; i++) {
    EEPROM.get(direccion, qtr.calibrationOn.maximum[i]);
    direccion += sizeof(uint16_t);
  }

  // Confirmación visual
  Serial.print("MINS: ");
  for (uint8_t i = 0; i < SensorCount; i++) {
    Serial.print(qtr.calibrationOn.minimum[i]); Serial.print("\t");
  }
  Serial.print("\nMAXS: ");
  for (uint8_t i = 0; i < SensorCount; i++) {
    Serial.print(qtr.calibrationOn.maximum[i]); Serial.print("\t");
  }
  Serial.println("\n--- CARGA COMPLETADA ---\n");
}