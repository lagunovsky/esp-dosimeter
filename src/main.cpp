#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "GeigerCounter.h"
#include "main.h"


GeigerCounter counter;
WiFiClient wifiClient;
PubSubClient pubSubClient(wifiClient);

const char *ssid = "";
const char *password = "";
const char *mqttServer = "";

const int ledPin = 2;
const int interruptPin = 4;
const int blinkTime = 10;

unsigned long lastPulseTime = 0;
unsigned long currentPulseTime;

bool currentLedStatus = false;
bool lastLedStatus = false;

unsigned long tickTime = 0;
unsigned long lastsSyncCycle = 0;

void IRAM_ATTR pulseInterrupt() {
  currentPulseTime = millis();

  if (currentPulseTime - lastPulseTime <= 2) {
    return;
  } else {
    lastPulseTime = currentPulseTime;
  }

  counter.addPulse(currentPulseTime);
}

void setup() {
  Serial.begin(1152000);

  pinMode(ledPin, OUTPUT);

  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), pulseInterrupt, FALLING);

  setupWiFi();
  pubSubClient.setServer(mqttServer, 1883);
}

void setupWiFi() {
  delay(500);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin((char *) ssid, password);
  WiFi.setHostname("esp-dosimeter");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!pubSubClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (pubSubClient.connect("ESP32Dosimeter")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(pubSubClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!pubSubClient.connected()) {
    reconnect();
  }
  pubSubClient.loop();

  tickTime = millis();

  // blink
  currentLedStatus = tickTime - lastPulseTime <= blinkTime;
  if (currentLedStatus != lastLedStatus) {
    lastLedStatus = currentLedStatus;
    digitalWrite(ledPin, currentLedStatus);
  }

  if (counter.hasData(tickTime) && tickTime > 10000 && tickTime - lastsSyncCycle > 5000) {
    lastsSyncCycle = tickTime;

    double pulses_s = counter.getPulsesPerSecond(tickTime);
    double microR_h = 0.000004669 * pow(pulses_s, 3) - 0.000384 * pow(pulses_s, 2) + 41.81 * pulses_s; // µR/h (мкР/ч)
    double microSv_h = microR_h / 100; // µSv/h (мкЗв/ч)

    StaticJsonDocument<256> doc;
    doc["pulses/s"] = pulses_s;
    doc["µSv/h"] = microSv_h;

    char buffer[256];
    size_t length = serializeJson(doc, buffer);
    pubSubClient.publish("global/dosimeter", buffer, length);

    serializeJson(doc, Serial);
    Serial.println();
  }
}


