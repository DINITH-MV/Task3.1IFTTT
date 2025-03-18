#include <WiFiNINA.h>
#include "secrets.h"
#include <BH1750.h>
#include <Wire.h>

BH1750 lightMeter;

//please enter your sensitive data in the Secret tab
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASSWORD;

WiFiClient client;

char   HOST_NAME[] = "maker.ifttt.com";
String SUNLIGHT_HIT   = "https://maker.ifttt.com/trigger/Sunlight_hit/with/key/drv-TCDXpcpkpWjtMUJLJQ"; 
String SUNLIGHT_GONE  = "https://maker.ifttt.com/trigger/Sunlight_gone/with/key/drv-TCDXpcpkpWjtMUJLJQ";
String queryString = "?value1=57&value2=25";

float previousLux = 0;

void setup() {
  // initialize WiFi connection
  WiFi.begin(ssid, pass);

  Serial.begin(9600);
  while (!Serial);

  Wire.begin();
  lightMeter.begin();
  Serial.println(F("BH1750 Test begin"));
}

void loop() {

  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }

  delay(1000);

  float currentLux = lightMeter.readLightLevel();
  Serial.print("Light level: ");
  Serial.print(currentLux);
  Serial.println(" lux");  

    if (client.connect(HOST_NAME, 80)) { // Ensure connection before sending

      if (currentLux - previousLux >= 100.00) {
        Serial.println("Sunlight hit");
        client.println("GET " + String(SUNLIGHT_HIT) + queryString + " HTTP/1.1");
        client.println("Host: " + String(HOST_NAME));
        client.println("Connection: close");
        client.println(); // End of the HTTP header
        
        while (client.connected()) {
            if (client.available()) {
              // read an incoming byte from the server and print it to serial monitor:
              char c = client.read();
              Serial.print(c);
          }
        }

        // the server's disconnected, stop the client:
        client.stop();
        Serial.println();
        Serial.println("disconnected");

      } else if (currentLux - previousLux <= -100.00) {
        Serial.println("Sunlight gone");
        client.println("GET " + String(SUNLIGHT_GONE) + queryString + " HTTP/1.1");
        client.println("Host: " + String(HOST_NAME));
        client.println("Connection: close");
        client.println(); // End of the HTTP header
        
        while (client.connected()) {
            if (client.available()) {
              // read an incoming byte from the server and print it to serial monitor:
              char c = client.read();
              Serial.print(c);
          }
        }    

        // the server's disconnected, stop the client:
        client.stop();
        Serial.println();
        Serial.println("disconnected");
      }

  
    } else {
    Serial.println("Failed to connect server!");
  }

  previousLux = currentLux;
  delay(2000);
}