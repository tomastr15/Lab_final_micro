#include <Servo.h>
#include <EEPROM.h>

Servo miServo;
const int pinSensor = A0;
const int pinServo = 9;
bool transmitiendoSensor = false;
unsigned long ultimoEnvio = 0;
unsigned long intervaloSensor = 1000;

void setup() {
  Serial.begin(9600);
  miServo.attach(pinServo);
  miServo.write(0);

  delay(3000);   // ← aumentado para dar tiempo al Arduino 2
  enviarMensajeEEPROM();
}

void loop() {
  if (transmitiendoSensor && (millis() - ultimoEnvio >= intervaloSensor)) {
    ultimoEnvio = millis();
    int lectura = analogRead(pinSensor);
    Serial.print('S');
    Serial.println(lectura);
  }

  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();

    if (comando.startsWith("SERVO:")) {
      int angulo = comando.substring(6).toInt();
      if (angulo >= 0 && angulo <= 180) {
        miServo.write(angulo);
      }
    }
    else if (comando.startsWith("START_SEN:")) {
      intervaloSensor = comando.substring(10).toInt();
      transmitiendoSensor = true;
      ultimoEnvio = millis();
    }
    else if (comando.equals("STOP_SEN")) {
      transmitiendoSensor = false;
    }
    else if (comando.equals("REQUEST_W")) {       // ← nuevo: re-envía bajo pedido
      enviarMensajeEEPROM();
    }
    else if (comando.startsWith("NEW_WELCOME:")) {
      String nuevoTexto = comando.substring(12);

      for (int m = 0; m < 45; m++) { EEPROM.write(m, 0); }

      int i = 0;
      for (i = 0; i < nuevoTexto.length() && i < 16; i++) {
        EEPROM.write(i, nuevoTexto[i]);
      }
      EEPROM.write(i, '\0');

      delay(100);
      enviarMensajeEEPROM();  // ← nuevo: confirma enviando el mensaje guardado
    }
  }
}

void enviarMensajeEEPROM() {
  char primera = EEPROM.read(0);
  Serial.print('W');

  if (primera == 0xFF || primera == '\0' || primera == '\r' || primera == '\n') {
    Serial.println("Bienvenido! [Def]");
  } else {
    int i = 0;
    char c = EEPROM.read(i);
    while (c != '\0' && c != '\r' && c != '\n' && i < 16) {
      Serial.print(c);
      i++;
      c = EEPROM.read(i);
    }
    Serial.println();
  }
}