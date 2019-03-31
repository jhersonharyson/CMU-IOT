#define BLYNK_PRINT Serial

#include <SPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

#define DHTPIN 23
#define DHTTYPE DHT11

DHT_Unified dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
WidgetRTC rtc;
uint32_t delayMS;

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "1a2960a5e3a04f57a6473b15a4dd7f9b";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "INTELBRAS";
char pass[] = "32335096";


const char* server = "big-city-server.herokuapp.com/api/v1/ws";

BLYNK_WRITE(V1) {

  GpsParam gps(param);

  // Print 6 decimal places for Lat, Lon
  Serial.print("Lat: ");
  Serial.println(gps.getLat(), 7);

  Serial.print("Lon: ");
  Serial.println(gps.getLon(), 7);

  // Print 2 decimal places for Alt, Speed
  Serial.print("Altitute: ");
  Serial.println(gps.getAltitude(), 2);

  Serial.print("Speed: ");
  Serial.println(gps.getSpeed(), 2);

  // Delay between measurements.
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
  }
  clockDisplay();
  Serial.println("Dat sent!");
  Serial.println();
  String msg = "{\"sensor\": {\"lat\": "+String(gps.getLat())+",\"log\": "+String(gps.getLon())+"},\"lat\": "+String(gps.getLat())+",\"log\": "+String(gps.getLon())+"}";
  Blynk.virtualWrite(V10, msg);
  
//  Blynk.virtualWrite(V10, "http://big-city-server.herokuapp.com/api/v1/ws/iot");
  
  
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Inicializando...");
  Blynk.begin(auth, ssid, pass);
  dht.begin();
  Serial.println(F("DHTxx Unified Sensor Example"));
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;



  

  //  setSyncInterval(900000);
  setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)
}

void loop()
{
  Blynk.run();
  timer.run();
}

void clockDisplay()
{
  // You can call hour(), minute(), ... at any time
  // Please see Time library examples for details

  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  Serial.print("Current time: ");
  Serial.print(currentTime);
  Serial.print(" ");
  Serial.print(currentDate);
  Serial.println();
  
}

BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin();
  
}
