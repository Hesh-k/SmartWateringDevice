#define BLYNK_TEMPLATE_ID           "TMPLxxxxxx"
#define BLYNK_DEVICE_NAME           "Device"
#define BLYNK_AUTH_TOKEN            "YourAuthToken"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <WiFiManager.h>
#include <Button2.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[40];
char pass[40];

Button2 physicalButton = Button2(0, INPUT_PULLUP);  // Change '0' to the actual button pin

BlynkTimer timer;

int inPin = D7;
int modePin = D9;
int Control = D12;

const int AirValue = 4000;
const int WaterValue = 0;
int intervals = (AirValue - WaterValue) / 3;
int soilMoistureValue, humidity, humiditys = 0;
bool condition = false;

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode. You can reset settings here.");
}

void setupWiFiManager() {
  WiFiManager wm;
  wm.setConfigPortalTimeout(180);  // Config portal stays open for 3 minutes
  wm.setAPCallback(configModeCallback);
  
  if (!wm.autoConnect("AutoConnectAP")) {
    Serial.println("Failed to connect and hit timeout. Resetting...");
    ESP.restart();
    delay(1000);
  }
  
  strcpy(ssid, wm.getWiFiSSID());
  strcpy(pass, wm.getWiFiPass());
}

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
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  physicalButton.setPressedHandler([](Button2 &btn) {
    Serial.println("Button pressed! Resetting WiFi settings...");
    WiFiManager wm;
    wm.resetSettings();
    ESP.restart();
  });

  setupWiFiManager();
  
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L, myTimerEvent);
  pinMode(inPin, INPUT);
  pinMode(modePin, OUTPUT);
  pinMode(Control, OUTPUT);
  digitalWrite(modePin, HIGH);
  digitalWrite(Control, LOW);
}

void loop() {
  physicalButton.loop();
  Blynk.run();
  timer.run();
  Soilmoisture();
}
