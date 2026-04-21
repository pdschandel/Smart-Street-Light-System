#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

// MQTT broker (Mocking TagoIO for simulation or public broker)
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

const int ldrPin = 34;
const int pirPin = 27;
const int ledPin = 5;

void setup() {
  Serial.begin(115200);
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
  ledcAttach(ledPin, 5000, 8);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  client.setServer(mqttServer, mqttPort);
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32_StreetLight")) {
      Serial.println("MQTT Connected");
    } else {
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int ldrValue = analogRead(ldrPin);
  int motion = digitalRead(pirPin);
  
  int brightness = 0;
  String statusMsg;
  
  if (ldrValue > 2000) { // Dark
    if (motion == HIGH) {
      brightness = 255; // 100%
      statusMsg = "Active (Motion)";
    } else {
      brightness = 51; // 20%
      statusMsg = "Dim (No Motion)";
    }
  } else { // Light
    brightness = 0; // OFF
    statusMsg = "OFF (Daylight)";
  }
  
  ledcWrite(ledPin, brightness);
  
  Serial.println("LDR: " + String(ldrValue) + " | PIR: " + String(motion) + " | Status: " + statusMsg);
  
  // Send data to Cloud
  String payload = "{\"ldr\":" + String(ldrValue) + ",\"motion\":" + String(motion) + ",\"brightness\":" + String((brightness*100)/255) + "}";
  client.publish("smart_streetlight_data", payload.c_str());
  
  delay(2000);
}
