#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte FILAS = 4;
const byte COLUMNAS = 4;
char keys[FILAS][COLUMNAS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte pinesFilas[FILAS] = {9, 8, 7, 6};
byte pinesColumnas[COLUMNAS] = {5, 4, 3, 2};

Keypad teclado = Keypad(makeKeymap(keys), pinesFilas, pinesColumnas, FILAS, COLUMNAS);

bool modoSensorActivo = false;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  lcd.print("Cargando...");
  lcd.setCursor(0, 1);
  lcd.print("D=Saltar");       // ← indica al usuario que D salta la espera

  // ← timeout aumentado a 5000ms, D salta la espera
  unsigned long tiempoEspera = millis();
  while (millis() - tiempoEspera < 5000) {
    char t = teclado.getKey();
    if (t == 'D') break;        // ← D sale de la espera

    if (Serial.available() > 0) {
      char indicador = Serial.read();
      if (indicador == 'W') {
        recibirYMostrarBienvenida();
        break;
      }
    }
  }

  mostrarMenuPrincipal();
}

void loop() {
  char tecla = teclado.getKey();

  // Recibir datos del sensor o mensaje de bienvenida
  if (Serial.available() > 0) {
    char indicador = Serial.read();

    if (modoSensorActivo && indicador == 'S') {
      int valorTransmitido = Serial.parseInt();
      lcd.setCursor(10, 1);
      lcd.print("    ");
      lcd.setCursor(10, 1);
      lcd.print(valorTransmitido);
    }
    // ← recibe la bienvenida actualizada desde el loop también
    else if (!modoSensorActivo && indicador == 'W') {
      recibirYMostrarBienvenida();
      delay(2500);
      mostrarMenuPrincipal();
    }
  }

  if (tecla) {
    if (!modoSensorActivo) {
      if (tecla == 'A') {
        pedirValorServo();
      }
      else if (tecla == 'B') {
        pedirValorSensor();
      }
      else if (tecla == 'C') {
        configurarNuevaBienvenida();
      }
      else if (tecla == 'D') {     // ← D re-solicita la bienvenida desde el menú
        solicitarBienvenida();
      }
    } else {
      Serial.println("STOP_SEN");
      modoSensorActivo = false;
      lcd.clear();
      lcd.print("Sensor Detenido");
      delay(1000);
      mostrarMenuPrincipal();
    }
  }
}

// ← nueva: lee el mensaje 'W' del serial y lo muestra
void recibirYMostrarBienvenida() {
  String msg = Serial.readStringUntil('\n');
  msg.trim();
  lcd.clear();
  lcd.print(msg);
  delay(2500);
}

// ← nueva: envía REQUEST_W y espera respuesta
void solicitarBienvenida() {
  Serial.println("REQUEST_W");
  lcd.clear();
  lcd.print("Solicitando...");

  unsigned long t = millis();
  while (millis() - t < 3000) {
    if (Serial.available() > 0) {
      char ind = Serial.read();
      if (ind == 'W') {
        recibirYMostrarBienvenida();
        break;
      }
    }
  }
  mostrarMenuPrincipal();
}

void mostrarMenuPrincipal() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("A:Srv B:Sen C:Msg");  // ← actualizado para caber
  lcd.setCursor(0, 1);
  lcd.print("D:Ver Bienvenida");   // ← muestra opción D
}

void pedirValorServo() {
  lcd.clear();
  lcd.print("Angulo (0-180):");
  lcd.setCursor(0, 1);

  String valor = "";
  while (true) {
    char t = teclado.getKey();
    if (t >= '0' && t <= '9' && valor.length() < 3) {
      valor += t;
      lcd.print(t);
    }
    if (t == '#') {
      if (valor.length() > 0) {
        Serial.println("SERVO:" + valor);
        lcd.clear();
        lcd.print("Enviando...");
        delay(1000);
      }
      break;
    }
    if (t == '*') break;
  }
  mostrarMenuPrincipal();
}

void pedirValorSensor() {
  lcd.clear();
  lcd.print("Intervalo (ms):");
  lcd.setCursor(0, 1);

  String valor = "";
  while (true) {
    char t = teclado.getKey();
    if (t >= '0' && t <= '9' && valor.length() < 5) {
      valor += t;
      lcd.print(t);
    }
    if (t == '#') {
      if (valor.length() > 0) {
        Serial.println("START_SEN:" + valor);
        lcd.clear();
        lcd.print("Leyendo...");
        lcd.setCursor(0, 1);
        lcd.print("Valor A0: ");
        modoSensorActivo = true;
      }
      break;
    }
    if (t == '*') break;
  }
  if (!modoSensorActivo) mostrarMenuPrincipal();
}

void configurarNuevaBienvenida() {
  lcd.clear();
  lcd.print("Nuevo Msg (16ch):");
  lcd.setCursor(0, 1);

  String nuevoMsg = "";
  while (true) {
    char t = teclado.getKey();
    if (t && t != '#' && t != '*' && nuevoMsg.length() < 16) {
      nuevoMsg += t;
      lcd.print(t);
    }
    if (t == '#') {
      if (nuevoMsg.length() > 0) {
        Serial.println("NEW_WELCOME:" + nuevoMsg);
        lcd.clear();
        lcd.print("Guardando...");
        // ← ya no dice "Reset para ver", espera el 'W' de confirmación
        delay(500);
      }
      break;
    }
    if (t == '*') break;
  }
  mostrarMenuPrincipal();
}