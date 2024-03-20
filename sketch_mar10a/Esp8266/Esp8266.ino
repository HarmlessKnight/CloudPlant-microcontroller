#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <DHT.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define DHTPIN 4
#define DHTTYPE DHT22   
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT object 

const int LightSensorAnalogPin = A0;
const int sensorPowerPin = 5;

const char* serverAddress = "192.168.100.9";  
const int serverPort = 8080;
const String deviceIdentifier = "Poplava";

// OLED library
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2 (U8G2_R0, 14, 12, U8X8_PIN_NONE);

WiFiClient wifiClient;
HTTPClient httpClient;

void sendToReceiver(float lightIntensity, float temperature, float humidity) {
  // Create a JSON object to store the data
  StaticJsonDocument<200> jsonDocument;

  // Add fields to the JSON object
  jsonDocument["deviceID"] = deviceIdentifier; // Use the global deviceIdentifier
  jsonDocument["lightIntensity"] = lightIntensity;
  jsonDocument["temperature"] = temperature;
  jsonDocument["humidity"] = humidity;

  // Convert the JSON object to a string
  String jsonString;
  serializeJson(jsonDocument, jsonString);

  Serial.print("Sending Data: ");
  Serial.println(jsonString);

  // Send the data to the server via HTTP POST request
  String url = "http://" + String(serverAddress) + ":" + String(serverPort) + "/data";


  httpClient.begin(wifiClient, url); // Reuse the existing connection
  httpClient.addHeader("Content-Type", "application/json"); // Set content type to JSON

  int httpResponseCode = httpClient.POST(jsonString);

  if (httpResponseCode > 0) {
    Serial.printf("HTTP Response code: %d\n", httpResponseCode);
    Serial.println("Alles Gut!");
  } else {
    Serial.printf("HTTP POST request failed, error: %s\n", httpClient.errorToString(httpResponseCode).c_str());
  }

  Serial.println("Data Sent:");
  Serial.print("Light Intensity: ");
  Serial.println(lightIntensity);
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Humidity: ");
  Serial.println(humidity);
}

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(sensorPowerPin, OUTPUT);
  digitalWrite(sensorPowerPin, LOW);

  WiFiManager wifiManager;
  wifiManager.setTimeout(180);
  if (!wifiManager.autoConnect("ConnectToMeAP")) {
      Serial.println("Failed to connect and hit timeout");
      delay(3000);
      ESP.reset();
      delay(5000);
  }
  Serial.println("Connected...yeey :)");
  u8g2.begin();
}

void loop() {
  // Code to display IP address on OLED
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(0, 10, "The current");
  u8g2.drawStr(0, 30, "IP address is:");

  String ipStr = WiFi.localIP().toString();
  u8g2.drawStr(0, 50, ipStr.c_str());
  u8g2.sendBuffer();
  
  // Code to measure sensor data
  int lightIntensity = analogRead(LightSensorAnalogPin);
  float percentage = 100.0 - ((float)lightIntensity / 1023.0 * 100.0);
  Serial.print("Light Intensity: ");
  Serial.print(percentage);
  Serial.println("%");

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println("C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");

  // Send data to the server
  sendToReceiver(percentage, temperature, humidity);

  delay(10000); // Adjust delay as needed
}
