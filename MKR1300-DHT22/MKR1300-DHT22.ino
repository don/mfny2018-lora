// Send Temperature and Humidity from DHT-22 to The Things Network
// Demo for Maker Faire NY 2018
// Don Coleman

// Hardware:
//   Arduino MKRWAN 1300 https://store.arduino.cc/usa/mkr-wan-1300
//   DHT22 Temperature and Humidity Sensor
//
// Libraries:
//   MKRWAN https://github.com/arduino-libraries/MKRWAN
//   CayenneLPP https://github.com/sabas1080/CayenneLPP
//   Adafruit DHT https://github.com/adafruit/DHT-sensor-library

#include <MKRWAN.h>
#include <CayenneLPP.h>
#include <DHT.h>

#include "arduino_secrets.h" 
String appEui = SECRET_APP_EUI;
String appKey = SECRET_APP_KEY;

LoRaModem modem;

#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

CayenneLPP lpp(51);

void setup() {
  Serial.begin(115200);
  // Uncomment the next line to wait for a serial connection when debugging
  //while (!Serial);
  
  // change this to your regional band (eg. US915, AS923, ...)
  if (!modem.begin(US915)) {
    Serial.println("Failed to start LoRa");
    while (1) {}
  };
  
  Serial.print("Your module version is: ");
  Serial.println(modem.version());
  Serial.print("Your device EUI is: ");
  Serial.println(modem.deviceEUI());

  Serial.println("Attempting to join LoRaWAN network with OTAA");
  int connected = modem.joinOTAA(appEui, appKey);
  if (!connected) {
    Serial.println("Something went wrong; are you indoor? Move near a window and retry");
    while (1) {}
  }
  Serial.println("Join Successful");

  modem.minPollInterval(60);
}

void loop() {
  // read values from DHT
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  // Check if any reads failed and exit early (to try again)
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(1000);
    return;
  }

  // Prepare Cayenne LPP
  lpp.reset();
  lpp.addTemperature(1, temperature);
  lpp.addRelativeHumidity(2, humidity);

  // Send the data
  modem.beginPacket();
  modem.write(lpp.getBuffer(), lpp.getSize());
  int err = modem.endPacket(true);
  if (err > 0) {
    Serial.println("Message sent.");
  } else {
    Serial.println("Error sending data.");
  }

  // Wait 2.5 minutes between transmissions
  delay(2.5 * 60 * 1000);
}
