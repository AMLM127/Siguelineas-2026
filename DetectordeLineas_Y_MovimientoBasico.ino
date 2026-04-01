#include <QTRSensors.h>
#include <EEPROM.h>
#include "MeMegaPi.h"

// --- CLASE DE CONTROL DE MOTORES ---
class CarroSiguelineas {
  private:
    MeMegaPiDCMotor motorIzquierdo; 
    MeMegaPiDCMotor motorDerecho;

  public:
    // Ajustado a tus puertos PORT3B y PORT3A
    CarroSiguelineas() : motorIzquierdo(PORT3B), motorDerecho(PORT3A) {} 

    void mover(int velIzq, int velDer) { 
      motorIzquierdo.run(velIzq); 
      motorDerecho.run(velDer); 
    }

    void detenerse() { 
      motorIzquierdo.stop(); 
      motorDerecho.stop(); 
    }
};

// --- INSTANCIAS ---
CarroSiguelineas miBot;
QTRSensors qtr;

// --- CONFIGURACIÓN DEL SENSOR ---
const uint8_t SensorCount = 7;
uint16_t sensorValues[SensorCount];
const uint8_t sensorPins[] = {A8, A9, A10, A11, A12, A13, A14};

void setup() {
  Serial.begin(9600);
  
  // Inicializar sensores
  qtr.setTypeRC();
  qtr.setSensorPins(sensorPins, SensorCount);

  // Inicializar memoria interna de la librería (necesario antes de cargar)
  qtr.calibrate(); 
  
  // Cargar datos de la EEPROM
  cargarCalibracion();
  
  Serial.println("Prueba lista: Coloca el robot en la línea.");
  delay(2000); // Pausa de seguridad antes de arrancar
}

void loop() {
  // Leer posición de la línea
  uint16_t position = qtr.readLineBlack(sensorValues);

  // Lógica de prueba:
  // Si la posición está cerca del centro (aprox 3000 para 7 sensores)
  // O si algún sensor detecta un valor alto (línea negra)
  bool detectaLinea = false;
  for (uint8_t i = 0; i < SensorCount; i++) {
    if (sensorValues[i] > 500) { // Umbral de detección (ajustable)
      detectaLinea = true;
      break;
    }
  }

  if (detectaLinea) {
    // Si ve la línea, avanza lento
    Serial.print("Línea detectada! Pos: ");
    Serial.println(position);
    miBot.mover(20, 20); 
  } else {
    // Si se sale de la línea, se detiene
    Serial.println("Buscando línea...");
    miBot.detenerse();
  }

  delay(50); 
}

void cargarCalibracion() {
  int direccion = 0;
  Serial.println("\n--- CARGANDO CALIBRACION ---");

  for (uint8_t i = 0; i < SensorCount; i++) {
    EEPROM.get(direccion, qtr.calibrationOn.minimum[i]);
    direccion += sizeof(uint16_t);
  }
  for (uint8_t i = 0; i < SensorCount; i++) {
    EEPROM.get(direccion, qtr.calibrationOn.maximum[i]);
    direccion += sizeof(uint16_t);
  }

  // Mostrar valores en monitor serie para confirmar
  Serial.print("MINS: ");
  for (uint8_t i = 0; i < SensorCount; i++) {
    Serial.print(qtr.calibrationOn.minimum[i]); Serial.print("\t");
  }
  Serial.print("\nMAXS: ");
  for (uint8_t i = 0; i < SensorCount; i++) {
    Serial.print(qtr.calibrationOn.maximum[i]); Serial.print("\t");
  }
  Serial.println("\n--- CARGA EXITOSA ---\n");
}