/*
   DHT Temperature and humidity monitoring using ESP8266 and the askSensors
   Description: This examples connects the ESP to wifi, and sends Temperature and humidity to askSensors IoT platfom over HTTPS GET Request.
    Author: https://asksensors.com, 2018 - 2019
    github: https://github.com/asksensors
   InstructableS: https://www.instructables.com/id/DHT11-Temperature-and-Humidity-Monitoring-Using-th/
*/

// includes
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// user config: TODO
const char* wifi_ssid = "ZTE_5FB71E";             // SSID
const char* wifi_password = "67315888";         // WIFI
const char* apiKeyIn = "29pUZB1wwcO6qntHvQTdVIZrDFwc27wh";      // API KEY IN
const unsigned int writeInterval = 25000; // write interval (in ms)

// ASKSENSORS config.
String uri = "http://192.168.0.198:3000";         // ASKSENSORS host name
const char* email = "test.user@mail.com";
const char* password = "test_test";
char * myToken;

// DHT config.
#define DHTPIN            2         // Pin which is connected to the DHT sensor.
// Uncomment the type of sensor in use:
#define DHTTYPE           DHT11     // DHT 11 
//#define DHTTYPE           DHT22     // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;
int status = WL_IDLE_STATUS;
float myTemperature = 0, myHumidity = 0;

void login() {
Serial.println("login to server...");
  myToken = "";
  // Check WiFi Status
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("not connected!");
    return;
  }

  // Block until we are able to connect to the WiFi access point
    HTTPClient http;

    http.begin(uri + "/auth/login/");
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> doc;
    // Add values in the document
    //
    doc["email"] = email;
    doc["password"] = password;

    String requestBody;
    serializeJson(doc, requestBody);

    int httpResponseCode = http.POST(requestBody);
    Serial.println(httpResponseCode);

    if (httpResponseCode == 200) {
      DynamicJsonDocument res(5000);
       String payload = http.getString();  
      
     // Parse JSON object
     auto error = deserializeJson(res, payload);
      if (error) {
        Serial.println(F("Parsing failed!"));
        return;
      }

      const char *amyToken = res["token"].as<char*>();
      myToken = (char*)amyToken;

      Serial.println(myToken);
      
      Serial.println(payload);

    }
    else {

      Serial.printf("Error occurred while sending HTTP POST: %s\n", http.errorToString(httpResponseCode).c_str());

    }
    http.end();

}

void postDataToServer() {
  login();  
  
  Serial.println("Posting JSON data to server...");
  // Block until we are able to connect to the WiFi access point
 

    HTTPClient http;

    http.begin(uri + "/v1/dhts");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("x-access-token", myToken);

    StaticJsonDocument<200> doc;
    // Add values in the document
    //
    doc["temperature"] = String(myTemperature);
    doc["humidity"] = String(myHumidity);

    String requestBody;
    serializeJson(doc, requestBody);

    int httpResponseCode = http.POST(requestBody);
    Serial.println(httpResponseCode);

    if (httpResponseCode == 201) {

      String response = http.getString();

      
      Serial.println(response);

    }
    else {

      Serial.printf("Error occurred while sending HTTP POST: %s\n", http.errorToString(httpResponseCode).c_str());

    }

  http.end();
}


//
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("********** connecting to WIFI : ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("-> WiFi connected");
  Serial.println("-> IP address: ");
  Serial.println(WiFi.localIP());
  // Initialize device.
  dht.begin();
  Serial.println("DHTxx Unified Sensor Example");
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Temperature");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" *C");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" *C");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" *C");
  Serial.println("------------------------------------");
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Humidity");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println("%");
  Serial.println("------------------------------------");
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;
}

void loop() {
  // Read data from DHT
  // Delay between measurements.
  delay(delayMS);
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("Error reading temperature!");
  }
  else {
    // Update temperature and humidity
    myTemperature = (float)event.temperature;
    Serial.print("Temperature: ");
    Serial.print(myTemperature);
    Serial.println(" C");
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println("Error reading humidity!");
  }
  else {
    myHumidity = (float)event.relative_humidity;
    Serial.print("Humidity: ");
    Serial.print(myHumidity);
    Serial.println("%");
  }

  postDataToServer();

  delay(writeInterval );     // delay in msec
}
