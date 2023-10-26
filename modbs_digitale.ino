#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoModbus.h>

byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x7A, 0x08 };
IPAddress ip(192, 168, 1, 225);
EthernetServer ethServer(502);
ModbusTCPServer modbusTCPServer;

int inputPins[] = {29, 28, 26, 24, 23, 22, A15, A14, A13, A12, A11, A10, A9, A8, 48, 47, 46, 45, 43, 42, 41, 40};
int outputPins[]={0x1E,0x1f,0x20,0x21,0x22,0x23,0x24,0x24};
int numBrs[] = {30, 31, 32, 33, 34, 35, 36, 37};
void setup() {
     Ethernet.init(53);
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  ethServer.begin();

  if (!modbusTCPServer.begin()) {
    Serial.println("Failed to start Modbus TCP Server!");
    while (true) {
      delay(1); // Rien à faire sans le serveur Modbus
    }
  }

  for (int i = 0; i < sizeof(inputPins) / sizeof(inputPins[0]); i++) {
    pinMode(inputPins[i], INPUT);
  }
  for (int i = 0; i < sizeof(outputPins) / sizeof(outputPins[0]); i++) {
    pinMode(outputPins[i], OUTPUT);
    digitalWrite(outputPins[i], LOW);
  }

  // Configurez les registres Modbus pour stocker les valeurs des entrées
  modbusTCPServer.configureHoldingRegisters(0x00, 50);
}

void loop() {
  EthernetClient client = ethServer.available();

  if (client) {
    Serial.println("Nouveau client");
    modbusTCPServer.accept(client);

    while (client.connected()) {
      modbusTCPServer.poll();
       updateRegisters();
       
    }
    Serial.println("Client déconnecté");
  }

  updateInputs();
}

void updateInputs() {
  // Lire les valeurs des broches numériques en entrée et les stocker dans les registres Modbus
  for (int i = 0; i < sizeof(inputPins) / sizeof(inputPins[0]); i++) {
    modbusTCPServer.holdingRegisterWrite(0x00 + i, digitalRead(inputPins[i]));
  }
}

void updateRegisters() {

  for (int i = 0; i < sizeof(outputPins) / sizeof(outputPins[0]); i++) {
    int modbusAddress = outputPins[i]; 
    int pinNumber = numBrs[i]; 
    Serial.println(pinNumber);

    int registerValue = modbusTCPServer.holdingRegisterRead(modbusAddress);
    Serial.println(registerValue);

    pinMode(pinNumber, OUTPUT);
    if (registerValue == 1) {
      digitalWrite(pinNumber, HIGH);
    } else {
      digitalWrite(pinNumber, LOW);
    }
  }
}
