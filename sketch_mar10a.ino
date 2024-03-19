#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>


#include <ESP8266WiFi.h>


#include <DNSServer.h>
#include <ESP8266WebServer.h>


#include <WiFiManager.h>



//Ova e bibliotekata za oledot 
U8G2_SSD1306_128X64_NONAME_F_SW_I2C
u8g2 (U8G2_R0,14,12,U8X8_PIN_NONE);


void setup()
{
  //neka sedi sea serial ke ni treba za testiranje
  Serial.begin(9600);

  //da se vidi tochno kako raboti bibliotekata 
  WiFiManager wifiManager;
  wifiManager.setTimeout(180);
  //probuva da se zakachi za poslednata zapamtena mrezha ako ne projde toa se startuva AP i se cheka da mu se dadat novi params
  if (!wifiManager.autoConnect("ConnectToMeAP")) {
      Serial.println("Failed to connect and hit timeout");
      delay(3000);
      ESP.reset();
      delay(5000);
 }


 Serial.println("Connected...yeey :)");
 u8g2.begin();
}


void loop()
{
  //vaka se ispishuva na ekran -- podelen e na dva dena 1/3 e zholt 2/3 e plav -- 0-20, 20-? (y-axis)
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(0,10,"The current");
  u8g2.drawStr(0,30,"IP address is:");


  String ipStr = WiFi.localIP().toString();
  u8g2.drawStr(0,50,ipStr.c_str());
  u8g2.sendBuffer();
  delay(1000);
}




