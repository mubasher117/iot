#include <WiFi.h>
#include <WebServer.h>

// ==========================================
// 1. YOUR WI-FI CREDENTIALS
// ==========================================
const char* ssid     = "DDS 2.4G";
const char* password = "ExpectoPatronumDDS";

// --- Safe Relay Pins for ESP32-C3 Super Mini ---
const int FAN1_PIN  = 2;  // Safe GPIO
const int FAN2_PIN  = 3;  // Safe GPIO
const int BULB1_PIN = 4;  // Safe GPIO
const int BULB2_PIN = 5;  // Safe GPIO

#define RELAY_ON LOW
#define RELAY_OFF HIGH

WebServer server(80);

// ==========================================
// 2. HANDLE SET COMMAND (/set?appliance=fan1&state=1)
// ==========================================
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

// ==========================================
// 3. HANDLE STATUS POLLED BY FLASK (/status)
// ==========================================
void handleStatus() {
  // Check the physical state of the pins and build a JSON response
  String json = "{";
  json += "\"fan1\":" + String(digitalRead(FAN1_PIN) == RELAY_ON ? "true" : "false") + ",";
  json += "\"fan2\":" + String(digitalRead(FAN2_PIN) == RELAY_ON ? "true" : "false") + ",";
  json += "\"bulb1\":" + String(digitalRead(BULB1_PIN) == RELAY_ON ? "true" : "false") + ",";
  json += "\"bulb2\":" + String(digitalRead(BULB2_PIN) == RELAY_ON ? "true" : "false");
  json += "}";

  server.send(200, "application/json", json);
}

// ==========================================
// 4. SETUP AND LOOP
// ==========================================
void setup() {
  Serial.begin(115200);

  // Initialize pins to OFF before setting as OUTPUT to prevent flickering
  int pins[] = {FAN1_PIN, FAN2_PIN, BULB1_PIN, BULB2_PIN};
  for (int p : pins) {
    digitalWrite(p, RELAY_OFF);
    pinMode(p, OUTPUT);
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nESP32 Room Controller Online!");
  Serial.print("Assign this IP to Flask ROOM_CONFIG: ");
  Serial.println(WiFi.localIP());

  // Register the URL endpoints
  server.on("/set", handleSet);
  server.on("/status", handleStatus);
  server.begin();
}

void loop() {
  server.handleClient();
  delay(10);
}