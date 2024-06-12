#include <DHT.h>

// Define the type of DHT sensor (DHT11, DHT21, DHT22)
#define DHTTYPE DHT22

// Pin where the DHT sensor is connected
#define DHT_PIN 2

// Initialize DHT sensor
DHT dht(DHT_PIN, DHTTYPE);

// Define the pins for light sensor and moisture sensors
#define LIGHT_SENSOR_PIN 15
#define MOISTURE_SENSOR_PIN_1 16
#define MOISTURE_SENSOR_PIN_2 17
#define MOISTURE_SENSOR_PIN_3 18

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  // Initialize DHT sensor
  dht.begin();
}

void loop() {
  delay(15000);
  // Read temperature and humidity from DHT sensor
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Read analog values from light sensor and moisture sensors
  int lightSensorValue = analogRead(LIGHT_SENSOR_PIN);
  int moistureSensorValue1 = analogRead(MOISTURE_SENSOR_PIN_1);
  int moistureSensorValue2 = analogRead(MOISTURE_SENSOR_PIN_2);
  int moistureSensorValue3 = analogRead(MOISTURE_SENSOR_PIN_3);

  // Convert analog values to percentages
  float lightPercentage = 100.0 - ((lightSensorValue / 1023.0) * 100.0);
  float moisturePercentage1 = 100.0 - ((moistureSensorValue1 / 1023.0) * 100.0);
  float moisturePercentage2 = 100.0 - ((moistureSensorValue2 / 1023.0) * 100.0);
  float moisturePercentage3 = 100.0 - ((moistureSensorValue3 / 1023.0) * 100.0);

  // Send data to ESP
  Serial.print("T:");
  Serial.print(temperature);
  Serial.print(",H:");
  Serial.print(humidity);
  Serial.print(",LS:");
  Serial.print(lightPercentage);
  Serial.print(",MS1:");
  Serial.print(moisturePercentage1);
  Serial.print(",MS2:");
  Serial.print(moisturePercentage2);
  Serial.print(",MS3:");
  Serial.println(moisturePercentage3);
}
