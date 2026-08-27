#include <WiFi.h>

#include <WebServer.h>



// --- WiFi Credentials ---

const char* ssid = "YOUR_WIFI_SSID";

const char* password = "YOUR_WIFI_PASSWORD";



// --- Relay Pins Mapping ---

// const int FAN1_PIN = 2; // Physical D2 (GPIO 2)

// const int FAN2_PIN = 4; // Physical D4 (GPIO 4)

// const int BULB1_PIN = 12; // Physical D12 (GPIO 12)

// const int BULB2_PIN = 13; // Physical D13 (GPIO 13)

// --- Safe Relay Pins Mapping ---
const int FAN1_PIN  = 16; // Changed from GPIO 2
const int FAN2_PIN  = 17; // Changed from GPIO 4
const int BULB1_PIN = 25; // Changed from GPIO 12 (Fixes boot failure)
const int BULB2_PIN = 26; // Changed from GPIO 13


#define RELAY_ON LOW

#define RELAY_OFF HIGH



WebServer server(80);



void handleSet() {

if (server.hasArg("appliance") && server.hasArg("state")) {

String appliance = server.arg("appliance");

bool state = (server.arg("state") == "1");

int pin = -1;



if (appliance == "fan1") pin = FAN1_PIN;

else if (appliance == "fan2") pin = FAN2_PIN;

else if (appliance == "bulb1") pin = BULB1_PIN;

else if (appliance == "bulb2") pin = BULB2_PIN;



if (pin != -1) {

digitalWrite(pin, state ? RELAY_ON : RELAY_OFF);

server.send(200, "text/plain", "OK");

return;

}

}

server.send(400, "text/plain", "Bad Request");

}



void setup() {

Serial.begin(115200);



int pins[] = {FAN1_PIN, FAN2_PIN, BULB1_PIN, BULB2_PIN};

for (int p : pins) {

pinMode(p, OUTPUT);

digitalWrite(p, RELAY_OFF);

}



WiFi.begin(ssid, password);

while (WiFi.status() != WL_CONNECTED) {

delay(500);

Serial.print(".");

}



Serial.println("\nESP32 Room Controller Online!");

Serial.print("Assign this IP to Flask ROOM_CONFIG: ");

Serial.println(WiFi.localIP());



server.on("/set", handleSet);

server.begin();

}



void loop() {

server.handleClient();

}



