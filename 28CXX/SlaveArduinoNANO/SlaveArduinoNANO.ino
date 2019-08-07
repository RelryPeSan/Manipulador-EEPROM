/**
 * Manipulador de EEPROM - 28CXX
 * SlaveArduinoNANO
 * 
 * versão: 0.2.1
 */

#include <Wire.h>

#define MyAdress 0x33

unsigned long adressCI;

byte pinos[] = {
  2,  // A0
  3,  // A1
  4,  // A2
  5,  // A3
  6,  // A4
  7,  // A5
  8,  // A6
  9,  // A7
  10, // A8
  11, // A9
  12, // A10
  A0, // A11
  A1, // A12
  NULL
};

void setup() {
  Serial.begin(57600);
  
  byte index = 0;
  while(pinos[index] != NULL){
    pinMode(pinos[index++], OUTPUT); 
  }
  
  Wire.begin(MyAdress);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  adressCI = 0x0;
}

void loop() {
  if(Serial.available()){
    Serial.read();
    Serial.println("READY!");
  }
}

void requestEvent(){
  Serial.println("_BEGIN - requestEvent");
  Wire.write((adressCI >> 8) & 0x0F);
  Wire.write(adressCI & 0x0F);
  Serial.print((adressCI >> 8) & 0x0F);
  Serial.println(adressCI & 0x0F);
  Serial.println("_END - requestEvent\n");
}

void receiveEvent(byte numBytes){
  Serial.println("_BEGIN - receiveEvent");

  // recupera os bytes do Master
  byte bytes[numBytes];
  for(int i = 0; i < numBytes; i++){
    bytes[i] = Wire.read();
//    Serial.print(bytes[i]);
  }

  // converte a string em um valor long hexadecimal
  adressCI = strtoul(bytes, NULL, 16);

  Serial.print("BIN: ");
  Serial.println(adressCI, BIN);
  Serial.print("DEC: ");
  Serial.println(adressCI, DEC);
  Serial.print("HEX: ");
  Serial.println(adressCI, HEX);

  setPins();
  Serial.println("_END - receiveEvent\n");
}

void setPins(){
  for(int index = 0; pinos[index] != NULL; index++){
    digitalWrite(pinos[index], (adressCI >> index) & 0x01);
//    Serial.print("[");
//    Serial.print(pinos[index]);
//    Serial.print("] = ");
//    Serial.println((adressCI >> index) & 0x01);
  }
}
