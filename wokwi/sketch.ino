/*
 * Edge Smart Sensor Node
 * ESP32 reads vibration, computes RMS and peak on-device,
 * classifies severity, publishes only alerts via MQTT.
 * Simulated in Wokwi. Broker: broker.hivemq.com
 */

#include <WiFi.h>
#include <PubSubClient.h>

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
const char* MQTT_BROKER = "broker.hivemq.com";
const int MQTT_PORT = 1883;
const char* MQTT_TOPIC = "edge/sensor/vibration/alerts";

const float RMS_NORMAL = 50.0;
const float RMS_WARNING = 100.0;

const unsigned long SAMPLE_INTERVAL_MS = 100;
const unsigned long WINDOW_SIZE = 50;
const unsigned long PUBLISH_INTERVAL_MS = 2000;

WiFiClient espClient;
PubSubClient client(espClient);

float sampleBuffer[WINDOW_SIZE];
int sampleIndex = 0;
unsigned long lastSampleTime = 0;
unsigned long lastPublishTime = 0;

float currentRMS = 0.0;
float currentPeak = 0.0;
int currentSeverity = 0;

float readVibrationSensor() {
  return (float)analogRead(34);
}

void computeFeatures() {
  float sumSquares = 0.0;
  float peak = 0.0;
  for (int i = 0; i < WINDOW_SIZE; i++) {
    float v = sampleBuffer[i];
    sumSquares += v * v;
    if (v > peak) peak = v;
  }
  currentRMS = sqrt(sumSquares / WINDOW_SIZE);
  currentPeak = peak;
  if (currentRMS < RMS_NORMAL) currentSeverity = 0;
  else if (currentRMS < RMS_WARNING) currentSeverity = 1;
  else currentSeverity = 2;
}

void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void connectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    String clientId = "EdgeSensor-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      delay(5000);
    }
  }
}

void publishAlert() {
  String severityStr;
  switch (currentSeverity) {
    case 0: severityStr = "NORMAL"; break;
    case 1: severityStr = "WARNING"; break;
    case 2: severityStr = "CRITICAL"; break;
  }
  String payload = "{";
  payload += "\"severity\":\"" + severityStr + "\",";
  payload += "\"rms\":" + String(currentRMS, 2) + ",";
  payload += "\"peak\":" + String(currentPeak, 2) + ",";
  payload += "\"uptime\":" + String(millis() / 1000);
  payload += "}";
  client.publish(MQTT_TOPIC, payload.c_str());
  Serial.print("Published: ");
  Serial.println(payload);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=================================");
  Serial.println("Edge Smart Sensor Node");
  Serial.println("=================================\n");
  connectWiFi();
  client.setServer(MQTT_BROKER, MQTT_PORT);
  for (int i = 0; i < WINDOW_SIZE; i++) sampleBuffer[i] = 0.0;
}

void loop() {
  unsigned long now = millis();
  if (!client.connected()) connectMQTT();
  client.loop();

  if (now - lastSampleTime >= SAMPLE_INTERVAL_MS) {
    lastSampleTime = now;
    sampleBuffer[sampleIndex] = readVibrationSensor();
    sampleIndex = (sampleIndex + 1) % WINDOW_SIZE;
  }

  if (now - lastPublishTime >= PUBLISH_INTERVAL_MS) {
    lastPublishTime = now;
    computeFeatures();
    publishAlert();
    Serial.print("RMS=");
    Serial.print(currentRMS);
    Serial.print(" Peak=");
    Serial.print(currentPeak);
    Serial.print(" Severity=");
    Serial.println(currentSeverity);
  }
}