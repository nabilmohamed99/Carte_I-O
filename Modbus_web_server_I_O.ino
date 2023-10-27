#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoModbus.h>

byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x7A, 0x08 };
IPAddress ip(192, 168, 1, 221);
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

  EthernetClient webClient = ethServer.available();
  
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
          webClient.println("<body>");
          webClient.println("<h1>Change IP Address</h1>");
          webClient.println("<form action='/setip' method='get'>");
          webClient.println("New IP Address: <input type='text' name='ip' id='ip' required><br><br>");
          webClient.println("<input type='submit' value='Set IP'>");
          webClient.println("</form>");
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
