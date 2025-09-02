#include <Wire.h>
#include <U8g2lib.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522Debug.h>

#ifndef TARJETAS_H
#define TARJETAS_H

#include <Arduino.h>

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

struct Tarjeta {
  String uid;
  String nombre;
  String grupo;
};

Tarjeta tarjetas[] = {
  {"04845693C42A81", "Computadora", "Facultad"},
  {"04855693C42A81", "Cuaderno", "Facultad"},
  {"04865693C42A81", "Cartuchera", "Facultad"},
  {"047A5693C42A81", "Calculadora", "Facultad"},
  {"047B5693C42A81", "Yerbera", "Matera"},
  {"048B5693C42A81", "Mate", "Matera"},
  {"048C5693C42A81", "Termo", "Matera"},
  {"048D5693C42A81", "Bombilla", "Matera"},
  {"048E5693C42A81", "Llaves", "Personal"},
  {"04835693C42A81", "Billetera", "Personal"}
};

int totalTarjetas = sizeof(tarjetas) / sizeof(tarjetas[0]);

#endif

// ---------- PINES ----------
#define BUTTON_UP    25
#define BUTTON_DOWN  26
#define BUTTON_OK    27
#define LED_PIN      2
#define BUZZER_PIN   13

// ---------- VARIABLES MENU ----------
String grupos[] = {"Facultad", "Matera", "Personal"};
int seleccion = 0;
String grupoSeleccionado = "";

// ---------- RFID ----------
MFRC522DriverPinSimple ss_pin(5);
MFRC522DriverSPI driver{ss_pin};
MFRC522 mfrc522{driver};

// ---------- DETECTADOS ----------
bool detectados[sizeof(tarjetas)/sizeof(tarjetas[0])] = {false};

// ---------- FUNCIONES ----------

String leerUID() {
  String uidStr = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) uidStr += "0";
    uidStr += String(mfrc522.uid.uidByte[i], HEX);
  }
  uidStr.toUpperCase();
  return uidStr;
}

void mostrarMenu() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "Selecciona grupo:");
    for (int i = 0; i < 3; i++) {
      String line = (i == seleccion ? "> " : "  ") + grupos[i];
      u8g2.drawStr(0, 25 + i*12, line.c_str());
    }
  } while (u8g2.nextPage());
}

void leerBotones() {
  if (digitalRead(BUTTON_UP) == LOW) {
    seleccion = (seleccion + 2) % 3;
    delay(200);
  }
  if (digitalRead(BUTTON_DOWN) == LOW) {
    seleccion = (seleccion + 1) % 3;
    delay(200);
  }
  if (digitalRead(BUTTON_OK) == LOW) {
    grupoSeleccionado = grupos[seleccion];
    Serial.print("Grupo seleccionado: "); Serial.println(grupoSeleccionado);
    delay(500);
  }
}

void identificarTarjeta(String uidLeido) {
  bool encontrada = false;
  for (int i = 0; i < totalTarjetas; i++) {
    if (tarjetas[i].uid.equalsIgnoreCase(uidLeido) &&
        tarjetas[i].grupo == grupoSeleccionado) {
      Serial.print("Tarjeta detectada: ");
      Serial.println(tarjetas[i].nombre);
      detectados[i] = true;
      encontrada = true;
      break;
    }
  }
  if (!encontrada) {
    Serial.println("Tarjeta NO pertenece al grupo o NO registrada.");
  }
}

void revisarGrupos() {
  bool completo = true;
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, ("Grupo: " + grupoSeleccionado).c_str());
    u8g2.drawStr(0, 22, "Objetos faltantes:");
    int y = 34;
    for (int i = 0; i < totalTarjetas; i++) {
      if (tarjetas[i].grupo == grupoSeleccionado && !detectados[i]) {
        u8g2.drawStr(0, y, ("- " + tarjetas[i].nombre).c_str());
        y += 12;
        completo = false;
      }
    }
    if (completo) {
      u8g2.drawStr(0, 50, "✅ Todos los objetos detectados!");
    }
  } while (u8g2.nextPage());

  digitalWrite(LED_PIN, completo ? LOW : HIGH);
  digitalWrite(BUZZER_PIN, completo ? LOW : HIGH);

  if (!completo) Serial.println("⚠ Faltan objetos en el grupo!");
  else Serial.println("✅ Todos los objetos detectados!");
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  while (!Serial);

  u8g2.begin();

  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_OK, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  mfrc522.PCD_Init();
  MFRC522Debug::PCD_DumpVersionToSerial(mfrc522, Serial);
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

// ---------- LOOP ----------
void loop() {
  if (grupoSeleccionado == "") {
    mostrarMenu();
    leerBotones();
    return;
  }

  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  String uidLeido = leerUID();
  identificarTarjeta(uidLeido);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  revisarGrupos();
  delay(2000);
}
