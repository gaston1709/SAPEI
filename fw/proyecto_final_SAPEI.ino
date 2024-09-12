#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN   9
#define SS_PIN    10
#define red       5
#define yellow    6
#define green     7

MFRC522 mfrc522(SS_PIN, RST_PIN);

unsigned long lastScanTime = 0;  // Almacena el tiempo del último escaneo
const unsigned long scanInterval = 2500;  // Intervalo entre escaneos en milisegundos

bool greenBlinking = false;
bool yellowBlinking = false;
bool redBlinking = false;

unsigned long blinkStartTime = 0;
const unsigned long blinkInterval = 500;  // Intervalo de parpadeo en milisegundos

void setup() {
  Serial.begin(115200);
  while (!Serial);

  SPI.begin();
  mfrc522.PCD_Init();
  
  pinMode(red, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(green, OUTPUT);
}

void loop() {
  unsigned long currentTime = millis();
  clearSerialBuffer();  // Limpiar el buffer del Serial antes de comenzar el escaneo

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    if (currentTime - lastScanTime >= scanInterval) {  // Verificar si ha pasado el intervalo
      mostrarUID(mfrc522.uid.uidByte, mfrc522.uid.size);
      lastScanTime = currentTime;  // Actualizar el tiempo del último escaneo
    }
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }

  handleLedCommands();
  handleBlinking(currentTime);
}

void mostrarUID(byte *buffer, byte bufferSize) {
  String uidString = "";
  for (byte i = 0; i < bufferSize; i++) {
    uidString += String(buffer[i] < 0x10 ? "0" : "") + String(buffer[i], HEX);
  }
  uidString.toUpperCase();  // Convertir a mayúsculas para consistencia
  Serial.println(uidString);
}

void clearSerialBuffer() {
  while (Serial.available() > 0) {
    Serial.read();  // Leer y descartar datos del buffer
  }
}

void handleLedCommands() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command == "green on") {
      turnOnLed(green);
    } else if (command == "red on") {
      turnOnLed(red);
    } else if (command == "yellow on") {
      turnOnLed(yellow);
    } else if (command == "green p") {
      blinkLed(green, 6);
    } else if (command == "yellow p") {
      blinkLed(yellow, 6);
    } else if (command == "red p") {
      blinkLed(red, 6);
    } else if (command == "green p on") {
      greenBlinking = true;
      blinkStartTime = millis();
    } else if (command == "green p off") {
      greenBlinking = false;
      digitalWrite(green, LOW);
    } else if (command == "yellow p on") {
      yellowBlinking = true;
      blinkStartTime = millis();
    } else if (command == "yellow p off") {
      yellowBlinking = false;
      digitalWrite(yellow, LOW);
    } else if (command == "red p on") {
      redBlinking = true;
      blinkStartTime = millis();
    } else if (command == "red p off") {
      redBlinking = false;
      digitalWrite(red, LOW);
    }
  }
}

void turnOnLed(int pin) {
  digitalWrite(pin, HIGH);
  unsigned long onStartTime = millis();
  while (millis() - onStartTime < 500) {
    // Esperar sin bloquear otras operaciones
  }
  digitalWrite(pin, LOW);
}

void blinkLed(int pin, int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(125);
    digitalWrite(pin, LOW);
    delay(125);
  }
}

void handleBlinking(unsigned long currentTime) {
  if (greenBlinking) {
    if (currentTime - blinkStartTime >= blinkInterval) {
      digitalWrite(green, !digitalRead(green));
      blinkStartTime = currentTime;
    }
  }
  
  if (yellowBlinking) {
    if (currentTime - blinkStartTime >= blinkInterval) {
      digitalWrite(yellow, !digitalRead(yellow));
      blinkStartTime = currentTime;
    }
  }
  
  if (redBlinking) {
    if (currentTime - blinkStartTime >= blinkInterval) {
      digitalWrite(red, !digitalRead(red));
      blinkStartTime = currentTime;
    }
  }
}
