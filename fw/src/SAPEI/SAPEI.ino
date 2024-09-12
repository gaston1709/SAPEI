#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "pins.h"

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
  
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
}

void loop() {
  unsigned long currentTime = millis();

  while (Serial.available() > 0) Serial.read();  //limpia el buffer del serial
  
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    if (currentTime - lastScanTime >= scanInterval) {  // Verificar si ha pasado el intervalo
      showUID(mfrc522.uid.uidByte, mfrc522.uid.size);
      lastScanTime = currentTime;  // Actualizar el tiempo del último escaneo
    }
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }

  handleLedCommands();
  handleBlinking(currentTime);
}

void showUID(byte *buffer, byte bufferSize) {
  String uidString = "";
  for (byte i = 0; i < bufferSize; i++) {
    uidString += String(buffer[i] < 0x10 ? "0" : "") + String(buffer[i], HEX);
  }
  uidString.toUpperCase();  // Convertir a mayúsculas para consistencia
  Serial.println(uidString);
}

void handleLedCommands() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "green on") {
      turnOnLed(GREEN_LED_PIN);
    } else if (command == "red on") {
      turnOnLed(RED_LED_PIN);
    } else if (command == "yellow on") {
      turnOnLed(YELLOW_LED_PIN);
    } else if (command == "green p") {
      blinkLed(GREEN_LED_PIN, 6);
    } else if (command == "yellow p") {
      blinkLed(YELLOW_LED_PIN, 6);
    } else if (command == "red p") {
      blinkLed(RED_LED_PIN, 6);
    } else if (command == "green p on") {
      greenBlinking = true;
      blinkStartTime = millis();
    } else if (command == "green p off") {
      greenBlinking = false;
      digitalWrite(GREEN_LED_PIN, LOW);
    } else if (command == "yellow p on") {
      yellowBlinking = true;
      blinkStartTime = millis();
    } else if (command == "yellow p off") {
      yellowBlinking = false;
      digitalWrite(YELLOW_LED_PIN, LOW);
    } else if (command == "red p on") {
      redBlinking = true;
      blinkStartTime = millis();
    } else if (command == "red p off") {
      redBlinking = false;
      digitalWrite(RED_LED_PIN, LOW);
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
      digitalWrite(GREEN_LED_PIN, !digitalRead(GREEN_LED_PIN));
      blinkStartTime = currentTime;
    }
  }

  if (yellowBlinking) {
    if (currentTime - blinkStartTime >= blinkInterval) {
      digitalWrite(YELLOW_LED_PIN, !digitalRead(YELLOW_LED_PIN));
      blinkStartTime = currentTime;
    }
  }
  
  if (redBlinking) {
    if (currentTime - blinkStartTime >= blinkInterval) {
      digitalWrite(RED_LED_PIN, !digitalRead(RED_LED_PIN));
      blinkStartTime = currentTime;
    }
  }
}
