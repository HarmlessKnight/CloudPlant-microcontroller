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

const char* serverAddress = "192.168.100.70";  
const int serverPort = 8080;
const String deviceIdentifier = "Poplava";

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2 (U8G2_R0, 14, 12, U8X8_PIN_NONE);

float temperature;
float humidity;
float lightSensor;
float moistureSensor1;
float moistureSensor2;
float moistureSensor3;

WiFiClient wifiClient;
HTTPClient httpClient;

void setup() {
  Serial.begin(9600); // Start serial communication at 9600 baud
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
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(0, 10, "The current");
  u8g2.drawStr(0, 30, "IP address is:");

  String ipStr = WiFi.localIP().toString();
  u8g2.drawStr(0, 50, ipStr.c_str());
  u8g2.sendBuffer();

  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n'); // Read data until newline character is encountered
    processSerialData(data); // Process the received data
  }

}

void processSerialData(String data) {
  // Print the received data
  Serial.println("Received data: " + data);

  if (sscanf(data.c_str(), "T:%f,H:%f,LS:%f,MS1:%f,MS2:%f,MS3:%f", &temperature, &humidity, &lightSensor, &moistureSensor1, &moistureSensor2, &moistureSensor3) == 6) {
    sendToReceiver(lightSensor, temperature, humidity, moistureSensor1, moistureSensor2, moistureSensor3);
  } else {
    // Data parsing failed
    Serial.println("Error parsing data!");
  }
}

void sendToReceiver(float lightIntensity,float temperature,float humidity,
  float Sensor1Moisture,float Sensor2Moisture,float Sensor3Moisture) {
  // Create a JSON object to store the data
  StaticJsonDocument<200> jsonDocument;

  // Add fields to the JSON object
  jsonDocument["deviceID"] = deviceIdentifier; // Use the global deviceIdentifier
  jsonDocument["lightIntensity"] = lightIntensity;
  jsonDocument["temperature"] = temperature;
  jsonDocument["humidity"] = humidity;
  jsonDocument["SensorMoisture1"] = Sensor1Moisture;
  jsonDocument["SensorMoisture2"] = Sensor2Moisture;
  jsonDocument["SensorMoisture3"] = Sensor3Moisture;

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
  Serial.print("Sensor 1 Moisture: ");
  Serial.println(Sensor1Moisture);
  Serial.print("Sensor 2 Moisture: ");
  Serial.println(Sensor2Moisture);
  Serial.print("Sensor 3 Moisture: ");
  Serial.println(Sensor3Moisture);
}
