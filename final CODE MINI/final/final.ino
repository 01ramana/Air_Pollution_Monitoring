#define SW_VERSION " ThinkSpeak.com" // SW version will appears at innitial LCD Display
#include "SdsDustSensor.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <SoftwareSerial.h> 
#include <TinyGPS.h> 
#include <DHT.h>
#define DHTPIN D5    // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 22  (AM2302), AM2321)
DHT dht(DHTPIN, DHTTYPE);
float lat = 28.5458,lon = 77.1703; // create variable for latitude and longitude object  
SoftwareSerial gpsSerial(D3,D4);//rx,tx  
TinyGPS gps; // create gps object 
uint32_t tsLastReport = 0;
void onBeatDetected(){
  
}

const char* MY_SSID = "Miniproject";
const char* MY_PWD = "12345678";
 WiFiClient client;
const char* TS_SERVER = "api.thingspeak.com";
String TS_API_KEY = "7OMWQ208WZVFO9OK";
int rxPin = D7;
int txPin = D8;
SdsDustSensor sds(rxPin, txPin);
void connectWifi()
{
  Serial.print("Connecting to " + *MY_SSID);
  WiFi.begin(MY_SSID, MY_PWD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("");
  
}
/*
   Sending Data to Thinkspeak Channel
 **/
void sendDataTS(void)
{ 
  while(gpsSerial.available()){ // check for gps data 
  if(gps.encode(gpsSerial.read()))// encode gps data 
  {  
  gps.f_get_position(&lat,&lon); // get latitude and longitude 
  Serial.print("Position: "); 
  Serial.print("Latitude:"); 
  Serial.print(lat,6); 
  Serial.print(";"); 
  Serial.print("Longitude:"); 
  Serial.println(lon,6);  ; 
  Serial.print(lat); 
  Serial.print(" "); 
 }
} 
String latitude = String(lat,6); 
  String longitude = String(lon,6); 
Serial.println(latitude+";"+longitude); 
delay(1000);
PmResult pm = sds.readPm();
  if (pm.isOk()) {
    Serial.print("PM2.5 = ");
    Serial.print(pm.pm25);
    Serial.print(", PM10 = ");
    Serial.println(pm.pm10);

    // if you want to just print the measured values, you can use toString() method as well
    Serial.println(pm.toString());
  } else {
    // notice that loop delay is set to 0.5s and some reads are not available
    Serial.print("Could not read values from PM sensor, reason: ");
    Serial.println(pm.statusToString());
  }
  delay(500);
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.println(h);
  delay(1000);
   Serial.print(F("Temperature: "));
  Serial.println(t);
  delay(1000); 
 
  if (client.connect(TS_SERVER, 80))
  {
    String postStr = TS_API_KEY;
    postStr += "&field1=";
    postStr += String(h);
     postStr += "&field2=";
    postStr += String(t);
     postStr += "&field3=";
    postStr += String(pm.pm25);
    postStr += "&field4=";
    postStr += String(pm.pm10);
    postStr += "&field5=";
    postStr += String(lat);
    postStr += "&field6=";
    postStr += String(lon);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + TS_API_KEY + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
    delay(1000);
  }
  client.stop();
}
void setup()
{
  Serial.begin(9600);
  delay(10);
  connectWifi();
  sds.begin();
  Serial.println("The GPS Received Signal:");
  gpsSerial.begin(9600); // connect gps sensor
  Serial.println(sds.queryFirmwareVersion().toString()); // prints firmware version
  Serial.println(sds.setActiveReportingMode().toString()); // ensures sensor is in 'active' reporting mode
  Serial.println(sds.setContinuousWorkingPeriod().toString()); // ensures sensor has continuous working period - default but not recommended
  dht.begin();
}
void loop()
{
  sendDataTS();
  delay(10);  
}
