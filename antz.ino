// ANTZ
#define BLYNK_PRINT Serial // Comment this out to disable prints and save space
#include <EEPROM.h>
#include <Blynk.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <BlynkSimpleEsp8266.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define ONE_WIRE_BUS D3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define SEALEVELPRESSURE_HPA (1013.25)
//Setup connection of the sensor
Adafruit_BME280 bme; // I2C

char auth[] = "515bf65c434942edb612466cd0cc5544";

char ssid[] = "EARTH";
char pass[] = "AbCd1234";

BlynkTimer timer;

//Variables
float pressure; //To store the barometric pressure (Pa)
float airTemp;  //To store the air temperature (oC)
int humidity;   //To store the humidity (%) (you can also use it as a float variable)
float gndTemp;  //To store the ground temperature (oC) GROUND
boolean trigger = false;
int minTemp;
int maxTemp;
int eminTemp;
int emaxTemp;
float aveTemp;

BLYNK_WRITE(V5) {
  minTemp = param.asInt();
  EEPROM.put(0, minTemp); // Write to EEPROM
  EEPROM.commit();
  Serial.print("Written");
}

BLYNK_WRITE(V6) {
  maxTemp = param.asInt();
  EEPROM.put(32, maxTemp); // Write to EEPROM
  EEPROM.commit();
  Serial.print("Written");
}

void setup() {
  bme.begin(0x76);    //Begin the sensor
  Serial.begin(9600); //Begin serial communication at 9600bps
  EEPROM.begin(512);
  Blynk.begin(auth, ssid, pass);

  sensors.begin();

  timer.setInterval(2000L, ReadSensors); // Temperature sensor polling interval (2000L = 5 seconds)
}

void ReadSensors() {
  sensors.requestTemperatures(); // GROUND TEMPERATURE

  //Read values from the sensor:
  pressure = bme.readPressure();        // PRESSURE
  airTemp = bme.readTemperature();      // AIR TEMPERATURE
  humidity = bme.readHumidity();        // HUMIDITY
  gndTemp = sensors.getTempCByIndex(0); // GROUND TEMPERATURE
  aveTemp = (airTemp + gndTemp) / 2;    // AVERAGE TEMPERATURE

  Blynk.virtualWrite(V1, pressure/100); // PRESSURE
  Blynk.virtualWrite(V2, airTemp);      // AIR TEMPERATURE
  Blynk.virtualWrite(V3, humidity);     // HUMIDITY
  Blynk.virtualWrite(V4, gndTemp);      // GROUND TEMPERATURE

  EEPROM.get(0, eminTemp); // Read from EEPROM
  EEPROM.get(32, emaxTemp); // Read from EEPROM

  if (aveTemp <= eminTemp || aveTemp >= emaxTemp && !trigger) { // Trigger if temperature is critical
    //Blynk.notify("Temperature critical, your ants are dying");
    Serial.println("BAD");
    Serial.println(eminTemp);
    Serial.println(emaxTemp);
    delay(2000);
    trigger=true;
  }

  if (aveTemp >= eminTemp || aveTemp <= emaxTemp && trigger) { // Triger if temperature is ok again
    //Blynk.notify("Temperature under control");
    Serial.println("GOOD");
    Serial.println(eminTemp);
    Serial.println(emaxTemp);
    delay(2000);
    trigger=false;
  }

}

void loop() {
  Blynk.run();
  timer.run();
}
