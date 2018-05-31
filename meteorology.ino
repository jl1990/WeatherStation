#include "DHTesp.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <Adafruit_BME280.h>
#include <ESP8266HTTPClient.h>


const char* ssid = "";             //Set your wifi network name(ssid)
const char* password = "";  //Set your router password
const int LDRPin = 17;
#define SEALEVELPRESSURE_HPA (1013.25)
HTTPClient http;

Adafruit_BME280 bme;
String SERVER_URL = "http://192.168.1.141:6789/weather";
/** Initialize DHT sensor */
DHTesp dht;

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LDRPin, INPUT);
  // Connect D0 to RST to wake up
  pinMode(D0, WAKEUP_PULLUP);
  enableLed(false);
  connectToWifi();
  //setupOTA();
  bool status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
}

void loop() 
{
  Serial.println();
  Serial.print("Free heap: ");
  Serial.print(ESP.getFreeHeap());
  int sleepSeconds = 900;
  enableLed(true);
  sendPost(SERVER_URL, createJSONFromSensors());
  enableLed(false);
  Serial.println();
  Serial.println("Going to sleep! I will wake up in ");
  Serial.print(sleepSeconds);
  Serial.print(" seconds");
  deepSleep(sleepSeconds);
}

void sendPost(String URL, String JSONBodyData){
  Serial.println();
  Serial.println("Sending data to server: ");
  Serial.print(URL);
  Serial.println();
  Serial.println(JSONBodyData);
  http.begin(URL);
  http.addHeader("Content-Type", "application/json");
  http.POST(JSONBodyData);
  //http.writeToStream(&Serial);
  http.end();
}

void connectToWifi()
{
  // Connect to WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Print the IP address
  Serial.print("IP: ");
  Serial.print(WiFi.localIP());
}

void deepSleep(int seconds)
{
  ESP.deepSleep(seconds * 1000000);
}

String createJSONFromSensors(){
  String result = "{\"temperature\": ";
  result+=bme.readTemperature();
  result+=",\"humidity\": ";
  result+=bme.readHumidity();
  result+=",\"pressure\": ";
  result = result + (bme.readPressure() / 100.0F);
  result+=",\"altitude\": ";
  result+=bme.readAltitude(SEALEVELPRESSURE_HPA);
  result+=",\"lightLevel\": ";
  result+= analogRead(LDRPin);
  result+="}";
  return result;
}

void reconnect(){
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);   // this is a temporary line, to be removed after SDK update to 1.5.4
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
}

void enableLed(boolean value){
  digitalWrite(LED_BUILTIN, !value);
}
