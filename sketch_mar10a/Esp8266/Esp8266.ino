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

#define MUX_A 14
#define MUX_B 12
#define MUX_C 13

#define ANALOG_INPUT A0
#define DHTTYPE DHT22
#define DHTPIN 5

DHT dht(DHTPIN, DHTTYPE); // Initialize DHT object

const char* serverAddress = "192.168.43.217";  
const int serverPort = 8080;
const String deviceIdentifier = "Poplava";

float SoilMoisture1value;
float SoilMoisture2value;
float SoilMoisture3value;
float LightSensorValue;

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2 (U8G2_R0, 14, 12, U8X8_PIN_NONE);

WiFiClient wifiClient;
HTTPClient httpClient;

void changeMux(int c, int b, int a) {
  digitalWrite(MUX_A, a);
  digitalWrite(MUX_B, b);
  digitalWrite(MUX_C, c);
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


void setup() {
  //Deifne output pins for Mux
  Serial.begin(9600);
  dht.begin();
  pinMode(MUX_A, OUTPUT);
  pinMode(MUX_B, OUTPUT);     
  pinMode(MUX_C, OUTPUT);     

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
  

  changeMux(LOW, LOW, LOW);
  SoilMoisture1value = analogRead(ANALOG_INPUT); 
  Serial.print("(0 0 0)Sensor 1 Moisture:");
  Serial.println(SoilMoisture1value);


  changeMux(LOW, LOW, HIGH);
  LightSensorValue = analogRead(ANALOG_INPUT);  
  Serial.print("light sensor 0 0 1 :");
  float lightpercentage = 100.0 - ((float)LightSensorValue / 1023.0 * 100.0);
  Serial.print(lightpercentage);
  Serial.println("%");



  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  Serial.print("Temperature:");
  Serial.print(temperature);
  Serial.print(" Humidity:");
  Serial.println(humidity);


  changeMux(LOW, HIGH, HIGH);
  SoilMoisture2value = analogRead(ANALOG_INPUT); //Value of the sensor connected Option 3 pin of Mux
  Serial.print("(0 1 1)Sensor 2 Moisture:");
  Serial.println(SoilMoisture2value);


  changeMux(HIGH, LOW, LOW);
  SoilMoisture3value = analogRead(ANALOG_INPUT); //Value of the sensor connected Option 3 pin of Mux
  Serial.print("(1 0 0)Sensor 3 Moisture:");
  Serial.println(SoilMoisture3value);

  sendToReceiver(lightpercentage,temperature,humidity,SoilMoisture1value,SoilMoisture2value,SoilMoisture3value);

  delay(7000);
}