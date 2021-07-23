// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, modified for integer use, public domain

#include "NonBTinyDHT.h"

#define DHTPIN 2  // DHT connected to Arduino Uno Digital Pin 2

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11 
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)


// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) 
//    of the sensor

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600); // Output status on Uno serial monitor
  Serial.println("DHTxx test!");
 
  dht.begin();
}

void loop() {
  // dht.update() must be called every main loop cycle. 
  // Using delay functions in the main loop will not break it but
  // it will delay the rate at which samples are taken.
  dht.update();

  // You can read the humidity and temperature values as often as you like
  // However, a new value will only be returned after the sensor produces one.
  // NOTE!!! The returned values are multiplied by 10 and need to be divided by
  // 10 in order to produce the correct results. This allows accuracy to 
  // one decimal point without using floats.
  int16_t h = dht.readHumidity();
  int16_t t = dht.readTemperature(0);

  // check if returns are valid then something went wrong!
  if ( t == BAD_TEMP || h == BAD_HUM ) { // if error conditions          
    // Data is not valid... do something...
    // Remember to divide by 10.
  } else {
    // Do something with the data.
    // Remember to divide by 10.
  }
}
