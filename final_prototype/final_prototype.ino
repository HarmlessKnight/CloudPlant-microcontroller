#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <U8g2lib.h>
#include <WiFiManager.h> 

const char* host = "";
const int httpsPort = 443;
String deviceCode = "699";
bool deviceRegistered = false;


WiFiClientSecure client;
HTTPClient http;

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 14, 12, U8X8_PIN_NONE);



void setup() 
{
  Serial.begin(9600);
  delay(10);

  WiFiManager wifiManager;
  wifiManager.setTimeout(180);

  if(!wifiManager.autoConnect("AutoConnectAP")) 
  {
    Serial.println("Failed to connect and hit timeout");
    // Could reset or do something else if needed
    ESP.restart();
    delay(1000);
  }
  Serial.println("Connected to WiFi");

  client.setInsecure(); 
  connectToHost();

  u8g2.begin();
}



void connectToHost() 
{
  Serial.print("Connecting to ");
  Serial.println(host);

  if (!client.connect(host, httpsPort)) 
  {
    Serial.println("Connection failed");
    return;
  }

  bool response = dbDeviceCheck();

  if (response)
  {
    deviceRegistered = true; 
    delay(1000);
  }
  else
  {
    createNewDevice(deviceCode);
    delay(1000);
    
  }
    
}

bool dbDeviceCheck() 
{
  
  String url = "/Device/IsCreated?code=" + deviceCode;
  Serial.print("Requesting URL: ");
  Serial.println(url);

  
  http.begin(client, host, httpsPort, url);

  int httpResponseCode = http.GET();
  if (httpResponseCode == HTTP_CODE_OK) 
  {
    String payload = http.getString();
    Serial.println("Response payload:");
    Serial.println(payload);
    return (payload == "true");
  } else 
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    return false;
  }
  http.end();
}

void createNewDevice(String deviceCode) 
{
  Serial.println("Setting up a new device in the database");
  String url = "/Device/CreateDevice?code=" + deviceCode;
  
  http.begin(client, host, httpsPort, url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  int httpResponseCode = http.POST(""); // No payload needed

  if (httpResponseCode > 0) 
  {
    String payload = http.getString();
    Serial.println("Response payload:");
    Serial.println(payload);
    deviceRegistered = true; 
  } else 
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
}


void sendMeasurements(float lightSensor, float temperature, float humidity, float moistureSensor1, float moistureSensor2, float moistureSensor3) 
{
  
  if (!deviceRegistered) 
  {
    Serial.println("Device is not registered. Skipping measurements.");
    return;
  }
  
  String path = "/Measurement";
  String body = "{\n";
  body += "  \"deviceCode\": \"" + String(deviceCode) + "\",\n";
  body += "  \"lightIntensity\": " + String(lightSensor) + ",\n";
  body += "  \"temperature\": " + String(temperature) + ",\n";
  body += "  \"humidity\": " + String(humidity) + ",\n";
  body += "  \"sensorMoisture1\": " + String(moistureSensor1) + ",\n";
  body += "  \"sensorMoisture2\": " + String(moistureSensor2) + ",\n";
  body += "  \"sensorMoisture3\": " + String(moistureSensor3) + "\n";
  body += "}";

  Serial.print("Connecting to ");
  Serial.println(host);

  http.begin(client, host, httpsPort, path);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(body);

  if (httpResponseCode > 0) 
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    String response = http.getString();
    Serial.println("Response from server:");
    Serial.println(response);
  } else 
  {
    Serial.print("HTTP POST request failed, error: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

void processSerialData(String data) 
{
    Serial.println("Received data: " + data);
    float temperature, humidity, lightSensor, moistureSensor1, moistureSensor2, moistureSensor3;

    if (sscanf(data.c_str(), "T:%f,H:%f,LS:%f,MS1:%f,MS2:%f,MS3:%f", &temperature, &humidity, &lightSensor, &moistureSensor1, &moistureSensor2, &moistureSensor3) == 6) 
    {
        sendMeasurements(lightSensor, temperature, humidity, moistureSensor1, moistureSensor2, moistureSensor3);
    } else 
    {
        Serial.println("Error parsing data!");
    }
}

void loop() 
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14B_tr);
  
  const char* label = "Device Code:";
  const char* code = deviceCode.c_str();
  
  int16_t xLabel = (u8g2.getDisplayWidth() - u8g2.getStrWidth(label)) / 2;
  int16_t xCode = (u8g2.getDisplayWidth() - u8g2.getStrWidth(code)) / 2;
  
  u8g2.drawStr(xLabel, 10, label);
  u8g2.drawStr(xCode, 30, code);
  u8g2.sendBuffer();

    if (Serial.available() > 0) 
    {
        String data = Serial.readStringUntil('\n');
        processSerialData(data); // Assuming processSerialData is defined elsewhere
        delay(3000);
    }  
}
