/**
 * CoreArduinoUNO
 * Projeto para manipular um 28c64 (e seus derivados)
 * Funções
 *  - Ler
 * versão: 0.3.0
 */

#include <Wire.h>

template <typename T>
inline Print& operator << (Print& streamer, const T& x) {
  streamer.print(x);
  return streamer;
}

#define adressNANO 0x33

#define READ F("read")
#define READSLAVE F("readslave")
#define LOAD F("load")
#define PROTECT F("protect")
#define ERASE F("erase")

#define NONE_HEX 0x00
#define READ_HEX 0x10
#define READSLAVE_HEX 0x11
#define LOAD_HEX 0x20
#define PROTECT_HEX 0x30
#define ERASE_HEX 0x40

#define PIN_CE 10
#define PIN_OE 11
#define PIN_WE 12

#define PIN_INIT_LOW 2

const char RETURN_CARRY[] = "\n\r";

typedef struct PINS_E {
  byte OE : 1;
  byte CE : 1;
  byte WE : 1;
} Pins_e;

typedef struct BITS_IO {
  byte bit_0 : 1;
  byte bit_1 : 1;
  byte bit_2 : 1;
  byte bit_3 : 1;
  byte bit_4 : 1;
  byte bit_5 : 1;
  byte bit_6 : 1;
  byte bit_7 : 1;
} Bits_io;

typedef union PINS_IO{
  Bits_io io;
  byte value;
} Pins_io;

struct PINS {
  Pins_e e;
  Pins_io io;
} pins;

void setup() {

  pinMode(PIN_CE, OUTPUT);
  pinMode(PIN_OE, OUTPUT);
  pinMode(PIN_WE, OUTPUT);
  setModePinsIO(OUTPUT);
  
  Serial.begin(9600);
  while(!Serial);
  
  Wire.begin();
  
  Serial << F("Iniciando mapeamento do software...\n\r") << "Adress: 0x";
  if(adressNANO < 0x10)
    Serial << "0";
  Serial.println(adressNANO, HEX);
}

void loop() {

  Serial << ">";
  while(!Serial.available());
  String command = Serial.readString();
  Serial << command ;
  
  command.trim();

  Serial << F("Comando: ");
  switch(getCommand(command)){
    case READ_HEX:
      commandRead();
      break;
    case READSLAVE_HEX:
      commandReadSlave();
      break;
    case LOAD_HEX:
      commandLoad();
      break;
    case PROTECT_HEX:
      commandProtect();
      break;
    case ERASE_HEX:
      commandErase();
      break;
    default:
      Serial << F("inválido!") << RETURN_CARRY;
      return;
  }

}

byte getCommand(String command){
  if(command.equalsIgnoreCase(READ)){
    return READ_HEX;
  } else if(command.equalsIgnoreCase(READSLAVE)){
    return READSLAVE_HEX;
  } else if(command.equalsIgnoreCase(LOAD)){
    return LOAD_HEX;
  } else if(command.equalsIgnoreCase(PROTECT)){
    return PROTECT_HEX;
  } else if(command.equalsIgnoreCase(ERASE)){
    return ERASE_HEX;
  } else {
    return NONE_HEX;
  }
}

void adjustPins(){
  // ajustando pinos de enable's
  digitalWrite(PIN_OE, pins.e.OE);
  digitalWrite(PIN_CE, pins.e.CE);
  digitalWrite(PIN_WE, pins.e.WE);

}

void adjustPins(bool io){
  adjustPins();
  
  // ajustando pino de I/O's
  setModePinsIO(io);
}

void setModePinsIO(bool io){
  for(int i = PIN_INIT_LOW; i < (PIN_INIT_LOW + 8); i++){
    pinMode(i, io);
  }
}

void getPinsIO(){
  pins.io.value = digitalRead(PIN_INIT_LOW + 7) << 7 | digitalRead(PIN_INIT_LOW + 6) << 6 |
                  digitalRead(PIN_INIT_LOW + 5) << 5 | digitalRead(PIN_INIT_LOW + 4) << 4 |
                  digitalRead(PIN_INIT_LOW + 3) << 3 | digitalRead(PIN_INIT_LOW + 2) << 2 |
                  digitalRead(PIN_INIT_LOW + 1) << 1 | digitalRead(PIN_INIT_LOW);
}

void setPinsIO(){
  for(byte i = 0; i < 8; i++)
    digitalWrite(PIN_INIT_LOW + i, (pins.io.value >> i) & 0x01);
}

void pulse(int pin, unsigned int delayMicro){
  digitalWrite(pin, !digitalRead(pin));
  delayMicroseconds(delayMicro);
  digitalWrite(pin, !digitalRead(pin));
}

void pulse(int pin){
  pulse(pin, 5);
}

void commandRead(){
  // ajusta os pinos de enable's
  pins.e.OE = LOW;
  pins.e.CE = LOW;
  pins.e.WE = HIGH;
  Serial << READ << RETURN_CARRY;
  
  adjustPins(INPUT);

  String textoOUT = F("Informe o endereço(hexadecimal) que pretende ler\nAdress: ");

  Serial << textoOUT;
  while(!Serial.available());
  String valor = Serial.readString();
  valor.toUpperCase();
  Serial << F("0x") << ((valor.length()-1)%2 == 1 ? "0" : "") << valor;

  // inicia a transmissão com o arduino NANO Slave
  Wire.beginTransmission(adressNANO);

  // converte a string em um array para fazer a transferencia.
  char buf[valor.length()];
  valor.getBytes(buf, valor.length());
  
  Wire.write(buf);

  // lê as portas do arduino, valor que esta armazenado na EEPROM vai para pins.io.*;
  getPinsIO();

  // encerra a transmissão com o arduino Slave
  Wire.endTransmission();

  Serial << "#: ";
  Serial.println(pins.io.value, BIN);

}

void commandReadSlave(){
  Serial << F("BEGIN ") << READSLAVE << RETURN_CARRY;

  Wire.requestFrom(adressNANO, 2);

  while(Wire.available()){
    int tmp1 = Wire.read();
    int tmp2 = Wire.read();
    unsigned int value = tmp1 << 8 | tmp1;
    Serial << tmp1 << " eq " << tmp2 << RETURN_CARRY;
    Serial << F("Endereço em Slave: ") << value << RETURN_CARRY;
  }
  Serial << F("END ") << READSLAVE << RETURN_CARRY;
}

void commandLoad(){
  // ajusta os pinos de enable's
  pins.e.OE = HIGH;
  pins.e.CE = LOW;
  pins.e.WE = HIGH;
  Serial << LOAD << RETURN_CARRY;
  
  adjustPins(OUTPUT);

  // pegar endereço
  String textoOUT = F("Informe o endereço(hexadecimal) em que pretende gravar (0 - 1FFF)\n\rAdress: ");
  Serial << textoOUT;
  while(!Serial.available());
  String endereco = Serial.readString();
  Serial << endereco;

  
  // inicia a transmissão com o arduino NANO Slave
  Wire.beginTransmission(adressNANO);

  // converte a string do endereço em um array para fazer a transferencia.
  char buf[endereco.length()];
  endereco.getBytes(buf, endereco.length());

  // envia o array do endereço para o slave
  Wire.write(buf);

  // encerra a transmissão com o arduino Slave
  Wire.endTransmission();


  // pegar valor a ser gravado
  textoOUT  = F("Informe o valor a ser gravado neste endereço. (00 - FF)\n\rValue: ");
  Serial << textoOUT;
  while(!Serial.available());
  String valor = Serial.readString();
  Serial << valor;

  byte bytes[2];
  for(int i = 0; i < 2; i++){
    bytes[i] = valor[i];
  }

  byte bValue = strtoul(bytes, NULL, 16);

  Serial << "bValue: " << bValue << RETURN_CARRY;
  // transfere somente o primeiro byte para os pinos
//  valor.getBytes(pins.io.value, 1);
  pins.io.value = bValue;

  setPinsIO();

  // lança um pulso em WE para gravar
  pulse(PIN_WE);

}

void commandProtect(){
  notImplemented();
}

void commandErase(){
  notImplemented();
}

void notImplemented(){
  Serial << F("Not implemented yet!") << RETURN_CARRY;
}
