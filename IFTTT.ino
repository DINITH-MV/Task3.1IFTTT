#include <WiFiNINA.h>
#include "secrets.h"
#include <BH1750.h>
#include <Wire.h>

BH1750 lightMeter;

//please enter your sensitive data in the Secret tab
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASSWORD;

WiFiClient client;

char HOST_NAME[] = "maker.ifttt.com";
String SUNLIGHT_HITS = "https://maker.ifttt.com/trigger/Sunlight_hits/with/key/drv-TCDXpcpkpWjtMUJLJQ";
String SUNLIGHT_GOES = "https://maker.ifttt.com/trigger/Sunlight_goes/with/key/drv-TCDXpcpkpWjtMUJLJQ";
String LIGHT_DURATION = "https://maker.ifttt.com/trigger/Sun_Duration/with/key/drv-TCDXpcpkpWjtMUJLJQ";
String queryString = "?value1=";

long hitDuration = 0;
long startTime = 0;
long pausedTime = 0;
bool luxAbove100 = false;
bool sunlightDetected = false;
float currentLux = 0;

int hours = 0;
int minutes = 0;
int seconds = 0;
String timeString = "";

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

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected.");
  }

  delay(1000);

  currentLux = lightMeter.readLightLevel();
  Serial.print("Light level: ");
  Serial.print(currentLux);
  Serial.println(" lux");

  if (client.connect(HOST_NAME, 80)) {  

    // To send Sunlight hit signal
    if (!sunlightDetected && currentLux >= 400) {
      Serial.println("Sunlight hits");
      client.println("GET " + String(SUNLIGHT_HITS) + queryString + " HTTP/1.1");
      client.println("Host: " + String(HOST_NAME));
      client.println("Connection: close");
      client.println();  // End of the HTTP header

      readIFTTTResponse();

      sunlightDetected = true;
  
    // To send Sunlight gone signal  
    } else if (sunlightDetected && currentLux <= 400) {

      Serial.println("Sunlight goes");
      client.println("GET " + String(SUNLIGHT_GOES) + queryString + " HTTP/1.1");
      client.println("Host: " + String(HOST_NAME));
      client.println("Connection: close");
      client.println();  // End of the HTTP header

      readIFTTTResponse();

      sunlightDetected = false;
    }

    lightDurationTimer();
      
    delay(1000);

  } else {
    Serial.println("Failed to connect server!");
  }

  

  delay(4000);
}

void lightDurationTimer() {
    if (currentLux >= 400.00){
      if (!luxAbove100) {
      // First time lux exceeds 220, start the timer
      startTime = millis() - pausedTime;
      luxAbove100 = true;  // Set the flag to indicate we're tracking
      }

      hitDuration = (millis() - startTime) / 1000;

      hours = hitDuration / 3600;
      minutes = (hitDuration % 3600) / 60;
      seconds = hitDuration % 60;

      timeString = String(hours) + " hours, " + String(minutes) + " mins, " + String(seconds) + " seconds";
      String ConcatenatedString = queryString + timeString;
    
      client.println("GET " + String(LIGHT_DURATION) + "?value1=" + String(hours) + "&value2="+ String(minutes) + "&value3=" + String(seconds) + " HTTP/1.1");
      client.println("Host: " + String(HOST_NAME));
      client.println("Connection: close");
      client.println();  // End of the HTTP header

    readIFTTTResponse();
              
    } else {
      if (luxAbove100) {
        luxAbove100 = false;
        pausedTime = millis() - startTime;
      }
    }
}

void readIFTTTResponse() {
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
