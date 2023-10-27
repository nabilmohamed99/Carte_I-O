#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoModbus.h>
#include <EEPROM.h>
byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x7A, 0x08 };
IPAddress ip(192, 168, 1, 222);
EthernetServer ethServer(502);
EthernetServer ethServer1(80);
ModbusTCPServer modbusTCPServer;

int inputPins[] = {29, 28, 26, 24, 23, 22, A15, A14, A13, A12, A11, A10, A9, A8, 48, 47, 46, 45, 43, 42, 41, 40};
int outputPins[]={0x1E,0x1f,0x20,0x21,0x22,0x23,0x24,0x24};
int numBrs[] = {30, 31, 32, 33, 34, 35, 36, 37};
void setup() {



   Ethernet.init(53);
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  ethServer.begin();
    ethServer1.begin();
    IPAddress storedIP;
    readIPAddressFromEEPROM(storedIP);

  if (storedIP != IPAddress(0, 0, 0, 0)) {
    Serial.println("the");
    ip = storedIP;
    Ethernet.begin(mac, ip);
  }

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

    Serial.print("Server is at ");
  Serial.println(Ethernet.localIP());
}

void loop() {
   EthernetClient modBusClient = ethServer.available();

 

  if (modBusClient) {
    Serial.println("Nouveau client modbus");
 

    modbusTCPServer.accept(modBusClient);

    while (modBusClient.connected()) {
      modbusTCPServer.poll();
       updateRegisters();
       
    }
    Serial.println("Client déconnecté");
  }

  updateInputs();

  EthernetClient webClient = ethServer1.available();
    if (modBusClient){
     int clientPort=modBusClient.remotePort();
     Serial.println(clientPort);
   }
  
  if (webClient) {
    
  
    Serial.println("CLient web");

    boolean currentLineIsBlank = true;
    String newIPString;

    while (webClient.connected()) {
      if (webClient.available()) {
        char c = webClient.read();
        Serial.write(c);

        if (c == '\n' && currentLineIsBlank) {
   webClient.println("HTTP/1.1 200 OK");
webClient.println("Content-Type: text/html");
webClient.println("Connection: close");
webClient.println();
webClient.println("<!DOCTYPE HTML>");
webClient.println("<html>");
webClient.println("<head>");
webClient.println("<style>");
webClient.println("body { font-family: Arial, sans-serif; margin: 0; padding: 0; text-align: center; background-color: #f2f2f2; }");
webClient.println(".navbar { background-color: #0074cc; color: #fff; padding: 10px; }");
webClient.println(".container { display: flex; justify-content: center; align-items: center; height: 70vh; }");
webClient.println("h1 { color: #333; }");
webClient.println("form { width: 350px; padding: 20px; background-color: #fff; border-radius: 5px; box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.2); }");
webClient.println("input[type='text'] { width: 90%; padding: 10px; margin-bottom: 10px; font-size: 16px; border: 1px solid #ccc; border-radius: 3px; }");
webClient.println("input[type='submit'] { width: 100%; padding: 10px 20px; font-size: 18px; background-color: blue; color: #fff; border: none; border-radius: 3px; cursor: pointer; }");
webClient.println("</style>");
webClient.println("</head>");
webClient.println("<body>");
webClient.println("<div class='navbar'>Meier Energy</div>"); // Navbar
webClient.println("<div class='container'>");
webClient.println("<form action='/setip' method='get'>");
webClient.println("<h1>Changer l'adresse IP</h1>");
webClient.println("Nouvelle adresse IP : <input type='text' name='ip' id='ip' required pattern='^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$'><br><br>");
webClient.println("<input type='submit' value='Configurer'>");
webClient.println("</form>");
webClient.println("</div>");
webClient.println("</body>");
webClient.println("</html>");



          break;
        }

        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
          newIPString += c;
        }
      }
    }

    if (newIPString.length() > 0) {
   
    

      if (newIPString.startsWith("GET /setip?ip=")) {
    
     
        String newIPValue = newIPString.substring(14,27);
        Serial.println(newIPValue);
          Serial.println("///////////");
        
        IPAddress newIP;
        if (newIP.fromString(newIPValue)) {
        
          ip = newIP;
          Ethernet.begin(mac, ip);
          writeIPAddressToEEPROM(ip);

        
          
        }
      }

      webClient.stop();
      Serial.println(" Web Client disconnected");
    
    }
    
  }

 
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

void readIPAddressFromEEPROM(IPAddress &ipAddr) {
  int eepromAddr = 0;
  byte ipOctets[4];
  for (int i = 0; i < 4; i++) {
    ipOctets[i] = EEPROM.read(eepromAddr++);
  }
  ipAddr = IPAddress(ipOctets);
}


void writeIPAddressToEEPROM(IPAddress ipAddr) {
  
  int eepromAddr = 0;
  for (int i = 0; i < 4; i++) {
    Serial.println(ipAddr[i]);
    delay(10);
    EEPROM.write(eepromAddr++, ipAddr[i]);
  
  }

}

