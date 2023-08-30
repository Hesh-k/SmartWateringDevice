#define BLYNK_TEMPLATE_ID           "TMPL6tijVHgxM"
#define BLYNK_DEVICE_NAME           "Quickstart Template"
#define BLYNK_AUTH_TOKEN            "EzJlsm5nK64IDDoA0MZ6RxpltK9Fuxzh"

#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

char auth[] = BLYNK_AUTH_TOKEN;
int timeout = 120; // seconds to run the configuration portal
int triggerPin = 0; // Pin to trigger the configuration portal

BlynkTimer timer;

int inPin = 7;
int modePin = 9;
int Control = 12;
const int AirValue = 4000;
const int WaterValue = 0;
int intervals = (AirValue - WaterValue) / 3;
int soilMoistureValue, humidity, humiditys = 0;
bool condition = false;

void myTimerEvent() {
  Blynk.virtualWrite(V6, humiditys);
  if (digitalRead(inPin) == 0 && !condition) {
    Blynk.virtualWrite(V3, "Water shortage");
    condition = true;
  }
  if (digitalRead(inPin) == 1) {
    condition = false;
  }
}

void Soilmoisture() {
  soilMoistureValue = analogRead(A0);
  humidity = map(soilMoistureValue, WaterValue, AirValue, 0, 100);
  humiditys = 100 - humidity;
  Serial.print("soil moisture: ");
  Serial.print(humiditys);
  Serial.print(" %");

  if (soilMoistureValue > WaterValue && soilMoistureValue < (WaterValue + intervals)) {
    Serial.println(" —— Very Wet");
    digitalWrite(Control, LOW);
  } else if (soilMoistureValue > (WaterValue + intervals) && soilMoistureValue < (AirValue - intervals)) {
    Serial.println(" —— Wet");
    digitalWrite(Control, LOW);
  } else if (soilMoistureValue < AirValue && soilMoistureValue > (AirValue - intervals)) {
    Serial.println(" —— Dry");
    digitalWrite(Control, HIGH);
  }
  delay(100);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  pinMode(triggerPin, INPUT_PULLUP);

  if (digitalRead(triggerPin) == LOW) {
    WiFiManager wm;
    wm.setConfigPortalTimeout(timeout);

    if (!wm.startConfigPortal("OnDemandAP")) {
      Serial.println("Failed to connect and hit timeout");
      delay(3000);
      ESP.restart();
      delay(5000);
    }

    Serial.println("Connected to WiFi");
    Serial.print("Local IP: ");
    Serial.println(WiFi.localIP());
  }

  Blynk.config(auth);
  Blynk.connect();

  timer.setInterval(1000L, myTimerEvent);
  pinMode(inPin, INPUT);
  pinMode(modePin, OUTPUT);
  pinMode(Control, OUTPUT);
  digitalWrite(modePin, HIGH);
  digitalWrite(Control, LOW);
}

void loop() {
  Blynk.run();
  timer.run();
  Soilmoisture();
}
