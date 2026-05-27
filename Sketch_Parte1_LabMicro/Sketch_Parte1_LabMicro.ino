#include <Servo.h>
#include <EEPROM.h>

Servo miServo;
const int pinSensor = A0;
const int pinServo = 9;

void setup() {
  // Inicializar comunicación serial a 9600 baudios
  Serial.begin(9600);
  
  // Configurar el servo en el pin 9 y mandarlo a una posición inicial segura (0 grados)
  miServo.attach(pinServo);
  miServo.write(0); 
  
  // Leer y mostrar el mensaje de bienvenida desde la memoria EEPROM
  mostrarBienvenida();
  mostrarPrompt();
}

void loop() {
  // Comprobar si el usuario escribió algo en la terminal
  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');
    comando.trim(); // Limpiar espacios y basura invisible (\r) del texto

    if (comando.equals("help")) {
      Serial.println("\n=== TERMINAL INTERACTIVA ===");
      Serial.println("servo     -> Controlar angulo del servo (Interactivo)");
      Serial.println("sensor X  -> Leer sensor cada X milisegundos (Ej: sensor 500)");
      Serial.println("welcome   -> Cambiar mensaje de bienvenida persistente");
      Serial.println("=============================");
    } 
    else if (comando.equals("servo")) {
      ejecutarServo();
    } 
    else if (comando.startsWith("sensor")) {
      // Buscar el espacio para extraer el parámetro del intervalo
      int espacioIdx = comando.indexOf(' ');
      if (espacioIdx != -1) {
        long intervalo = comando.substring(espacioIdx + 1).toInt();
        if (intervalo > 0) {
          ejecutarSensor(intervalo);
        } else {
          Serial.println("\n[Error] El intervalo debe ser mayor a 0 ms.");
        }
      } else {
        Serial.println("\n[Error] Usa -> sensor <tiempo_ms>. Ejemplo: sensor 1000");
      }
    } 
    else if (comando.equals("welcome")) {
      configurarBienvenida();
    } 
    else if (comando.length() > 0) {
      Serial.print("\nComando '");
      Serial.print(comando);
      Serial.println("' no valido. Escribe 'help' para ver la lista.");
    }
    
    mostrarPrompt();
  }
}

// Imprime la línea de comandos estilo Powershell/Bash
void mostrarPrompt() {
  Serial.print("\nArduino@Bash:$ ");
}

// ====================================================================
// COMANDO 1: ACTUADOR INTERACTIVO (SERVO)
// ====================================================================
void ejecutarServo() {
  Serial.println("\n[MODO SERVO INTERACTIVO]");
  Serial.print("Ingrese el angulo (0-180) y presione Enter: ");
  
  // Limpiar cualquier residuo previo en el buffer
  while(Serial.available() > 0) { Serial.read(); }
  
  // Esperar pacientemente a que el usuario escriba el número
  while (Serial.available() == 0) { 
    delay(10); 
  }
  
  // Leer la entrada como una cadena de texto completa
  String entrada = Serial.readStringUntil('\n');
  entrada.trim();
  
  // Convertir a número entero
  int angulo = entrada.toInt();
  
  // Validar si el texto ingresado es un número real entre 0 y 180
  if ((angulo >= 0 && angulo <= 180) && (entrada == "0" || angulo > 0)) {
    miServo.write(angulo); // Mover el servo físicamente
    Serial.print("-> Exito: Servomotor posicionado en ");
    Serial.print(angulo);
    Serial.println(" grados.");
  } else {
    Serial.print("-> Error: '");
    Serial.print(entrada);
    Serial.println("' no es un angulo valido (0 a 180).");
  }
  
  // Limpiar buffer al salir
  while(Serial.available() > 0) { Serial.read(); }
}

// ====================================================================
// COMANDO 2: SENSOR CON PARÁMETROS Y TIMESTAMP
// ====================================================================
void ejecutarSensor(long intervalo) {
  Serial.print("\n[MODO SENSOR] Leyendo cada ");
  Serial.print(intervalo);
  Serial.println(" ms.");
  Serial.println(">>> IMPORTANTE: Escribe la letra 'q' y presiona Enter para detener.");
  delay(1000);

  while (true) {
    // Comprobar si el usuario quiere salir
    if (Serial.available() > 0) {
      char letra = Serial.read();
      if (letra == 'q' || letra == 'Q') {
        Serial.println("\n[Modo Sensor] Lectura detenida por el usuario.");
        while(Serial.available() > 0) { Serial.read(); }
        break;
      }
    }
    
    int valorSensor = analogRead(pinSensor);
    unsigned long tiempo = millis(); // Timestamp en milisegundos desde que inició el Arduino
    
    Serial.print("[Timestamp: ");
    Serial.print(tiempo);
    Serial.print(" ms] -> Valor Analogico (A0): ");
    Serial.println(valorSensor);
    
    delay(intervalo);
  }
}

// ====================================================================
// COMANDO 3: MENSAJE PERSISTENTE EN EEPROM
// ====================================================================
void configurarBienvenida() {
  Serial.println("\n[CONFIGURAR BIENVENIDA]");
  Serial.println("Escribe el nuevo mensaje (Max 40 letras) y presiona Enter:");
  
  while(Serial.available() > 0) { Serial.read(); }
  while (Serial.available() == 0) { delay(10); }
  
  String texto = Serial.readStringUntil('\n');
  texto.trim(); 
  
  if (texto.length() > 0) {
    // Forzamos la limpieza de las primeras posiciones por seguridad en el simulador
    for(int m = 0; m < 45; m++) {
      EEPROM.write(m, 0);
    }
    
    // Grabar el nuevo texto en la EEPROM
    int i = 0;
    for (i = 0; i < texto.length() && i < 40; i++) {
      EEPROM.write(i, texto[i]);
    }
    EEPROM.write(i, '\0'); // Cierre de la cadena de texto
    
    Serial.println("-> ¡Guardado con exito en la EEPROM!");
    Serial.println("-> PASO PARA PROBAR: Haz clic en el boton ROJO redondo (RESET) arriba a la izquierda del Arduino.");
  } else {
    Serial.println("-> Error: El mensaje no puede estar vacio.");
  }
  
  while(Serial.available() > 0) { Serial.read(); }
}

void mostrarBienvenida() {
  Serial.println("\n******************");
  char primera = EEPROM.read(0);
  
  // Si está limpio de fábrica (0xFF) o vacío, usa el mensaje por defecto
  if (primera == 0xFF || primera == '\0' || primera == '\r' || primera == '\n') {
    Serial.println("Bienvenido al Terminal de Control v1.0 [Default]");
  } else {
    int i = 0;
    char c = EEPROM.read(i);
    
    // Leer carácter por carácter de la EEPROM hasta el fin de cadena
    while (c != '\0' && c != '\r' && c != '\n' && i < 40) {
      Serial.print(c);
      i++;
      c = EEPROM.read(i);
    }
    Serial.println(); 
  }
  Serial.println("Escribe 'help' para ver los comandos.");
  Serial.println("****************");
}

// A

