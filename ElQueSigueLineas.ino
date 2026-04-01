#include <QTRSensors.h>
#include <EEPROM.h>
#include "MeMegaPi.h"

class CarroSiguelineas {
  private:
    MeMegaPiDCMotor motorIzquierdo;
    MeMegaPiDCMotor motorDerecho;
  public:
    CarroSiguelineas() : motorIzquierdo(PORT3B), motorDerecho(PORT3A) {}
    void setSpeeds(int leftSpeed, int rightSpeed) {
      motorIzquierdo.run(leftSpeed); 
      motorDerecho.run(rightSpeed);  
    }
    void detenerse() {
      motorIzquierdo.stop(); 
      motorDerecho.stop();   
    }
};

class RobotSeguidor {
  private:
    CarroSiguelineas motores;
    QTRSensors qtr;
    static const uint8_t SensorCount = 7;
    uint16_t sensorValues[SensorCount];
    const uint8_t sensorPins[7] = {A8, A9, A10, A11, A12, A13, A14}; 

    float proportional = 0.5; 
    float derivative = 0.8;   
    int baseSpeed = 50;      
    int maxSpeed = 65;       
    int minSpeed = 30;        
    
    float lastError = 0;
    float position, error;

    // --- Control de Inercia ---
    unsigned long tiempoPerdido = 0;
    const int margenInercia = 225; // Milisegundos que sigue recto en blanco

  public:
    void inicializar() {
      Serial.begin(9600);
      qtr.setTypeRC(); 
      qtr.setSensorPins(sensorPins, SensorCount); 
      qtr.calibrate(); 
      cargarDesdeEEPROM();
    }

    void cargarDesdeEEPROM() {
      int direccion = 0;
      for (uint8_t i = 0; i < SensorCount; i++) {
        EEPROM.get(direccion, qtr.calibrationOn.minimum[i]);
        direccion += sizeof(uint16_t);
      }
      for (uint8_t i = 0; i < SensorCount; i++) {
        EEPROM.get(direccion, qtr.calibrationOn.maximum[i]);
        direccion += sizeof(uint16_t);
      }
    }

    void ejecutar() {
      position = qtr.readLineBlack(sensorValues); 
      error = position - 3000; 

      int sumaValores = 0;
      for (uint8_t i = 0; i < SensorCount; i++) {
        sumaValores += sensorValues[i]; 
      }

      if (sumaValores < 400) { // Si detecta blanco total
        // Si ha pasado poco tiempo, sigue recto por inercia
        if (millis() - tiempoPerdido < margenInercia) {
          motores.setSpeeds(baseSpeed, baseSpeed);
        } else {
          // Si ya pasó el margen, ahora sí busca la línea
          buscarLinea();
        }
        return; 
      }

      // Si ve la línea, reseteamos el cronómetro y aplicamos PID
      tiempoPerdido = millis();

      if (abs(error) > 2900) { //
        buscarLinea(); //
      } else {
        aplicarPID(); //
      }
      delay(10); //
    }

    void aplicarPID() {
      float speedDifference = (error * proportional / 256) + ((error - lastError) * derivative / 256); 
      lastError = error; 

      int leftSpeed = baseSpeed - speedDifference; 
      int rightSpeed = baseSpeed + speedDifference; 

      leftSpeed = constrain(leftSpeed, minSpeed, maxSpeed); 
      rightSpeed = constrain(rightSpeed, minSpeed, maxSpeed); 

      motores.setSpeeds(leftSpeed, rightSpeed); 
    }

    void buscarLinea() {
      // Bajamos un poco la velocidad de giro (de 60 a 50) para que no sea tan brusco
      if (lastError > 0) {
        motores.setSpeeds(60, -40); 
      } else {
        motores.setSpeeds(-40, 60); 
      }
    }
};

RobotSeguidor miAffenbot;
void setup() { miAffenbot.inicializar(); }
void loop() { miAffenbot.ejecutar(); }