/*!
 * @file TinyDHT.cpp
 *
 * @mainpage Adafruit TinyDHT Sensor Library
 *
 * @section intro_sec Introduction
 *
 * Integer version of the Adafruit DHT library for the
 * Trinket and Gemma mini microcontrollers
 *
 * @section license License
 *
 * MIT license
 *
 * @section author Author
 *
 * Written by Adafruit Industries
 */

#include "NonBTinyDHT.h"

DHT::DHT(uint8_t pin, uint8_t type, uint8_t count) {
  _pin = pin;
  _type = type;
  _count = count;
  firstreading = true;
  lastReadResult = false;
}

void DHT::begin(void) {
  // set up the pins!
  pinMode(_pin, INPUT);
  digitalWrite(_pin, HIGH);
  _lastreadtime = 0;
  currentState = DHWAITING;
}

// boolean S == Scale.  True == Farenheit; False == Celcius
int16_t DHT::readTemperature(bool S) {
  int16_t f;

  if (read()) {
    switch (_type) {
    case DHT11:
      f = (int16_t)data[2];
      if (S)
        f = convertCtoF(f);

      return f;
    case DHT22:
    case DHT21:
      f = (int16_t)(data[2] & 0x7F);
      f *= 256;
      f += (int16_t)data[3];
      //f /= 10;
      if (data[2] & 0x80)
        f *= -1;
      if (S)
        f = convertCtoF(f);

      return f;
    }
  }
  /* Serial.print("Read fail"); */
  return BAD_TEMP; // Bad read, return value (from TinyDHT.h)
}

int16_t DHT::convertCtoF(int16_t c) { return (c * 9) / 5 + 32; }

uint16_t DHT::readHumidity(void) { //  0-1000 %
  uint16_t f2; // bigger to allow for math operations
  if (read()) {
    switch (_type) {
    case DHT11:
      f2 = data[0];
      return f2;
    case DHT22:
    case DHT21:
      f2 = (uint16_t)data[0];
      f2 *= 256;
      f2 += data[1];
      //f2 /= 10;
      return f2;
    }
  }
  /* Serial.print("Read fail"); */
  return BAD_HUM; // return bad value (defined in TinyDHT.h)
}

// Returns true if there is valid data available. False if none is available.
boolean DHT::read(void) {

  return lastReadResult;
  /*
  Serial.println(j, DEC);
  Serial.print(data[0], HEX); Serial.print(", ");
  Serial.print(data[1], HEX); Serial.print(", ");
  Serial.print(data[2], HEX); Serial.print(", ");
  Serial.print(data[3], HEX); Serial.print(", ");
  Serial.print(data[4], HEX); Serial.print(" =? ");
  Serial.println(data[0] + data[1] + data[2] + data[3], HEX);
  */
}

// This function calls the DHT update function regularly within the main loop. 
// This will handle reading from the DHT in a non-blocking way as fast as it will
// allow.
void DHT::update(void) {
  uint8_t laststate = HIGH;
  uint8_t counter = 0;
  uint8_t j = 0, i;
  unsigned long currenttime;

  // Our only risk here is having a rollover event. That could result in our
  // timer waiting up to 50 days before taking a sample.
  currenttime = millis();
  if (currenttime < _lastreadtime) {
    // ie there was a rollover
    _lastreadtime = 0;    // This could result in slightly longer delays for the high and low state but the probability is so insanely low that there is no point in accounting.
  }
  
  switch (currentState)
  {
  case DHWAITING:   // Waiting for the next read slot after 2s.
    digitalWrite(_pin, HIGH);   // The pin should be pulled up while we wait
    if (firstreading) 
    {
      currentState = DHPREPHIGH;  // If it is the first reading we just jump ahead.
      firstreading = false;
    }

    if (currenttime - _lastreadtime >= 1800)
    {
      currentState = DHPREPHIGH;  // 1800 instead of 2000 because of the other delays
      _lastreadtime = currenttime;  // Marker for the start of the next state.
    }
    break;
  
  case DHPREPHIGH:
    digitalWrite(_pin, HIGH);   // The pin should be pulled up while we wait
    if (currenttime - _lastreadtime >= 250)
    {
      currentState = DHPREPLOW;  // 1800 instead of 2000 because of the other delays
      _lastreadtime = currenttime;  // Marker for the start of the next state.
    }
    break;

  case DHPREPLOW:
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);  // Force the pin low now.
    if (currenttime - _lastreadtime >= 20)
    {
      currentState = DHREADING;  // 1800 instead of 2000 because of the other delays
      _lastreadtime = currenttime;  // Marker for the start of the next state.
      data[0] = data[1] = data[2] = data[3] = data[4] = 0;
    }
    break;
  
  case DHREADING:

    // This is super time sensitive so we will do this in a blocking manner.
    noInterrupts();
    digitalWrite(_pin, HIGH);
    delayMicroseconds(40);
    pinMode(_pin, INPUT);

    // read in timings
    for (i = 0; i < MAXTIMINGS; i++) {
      counter = 0;
      while (digitalRead(_pin) == laststate) {
        counter++;
        delayMicroseconds(1);
        if (counter == 255) {
          break;
        }
      }
      laststate = digitalRead(_pin);

      //if (counter == 255) // Superflous
        //break;

      // ignore first 3 transitions
      if ((i >= 4) && (i % 2 == 0)) {
        // shove each bit into the storage bytes
        data[j / 8] <<= 1;
        if (counter > _count)
          data[j / 8] |= 1;
        j++;
      }
    }

    interrupts();
    _lastreadtime = currenttime;

    // check we read 40 bits and that the checksum matches
    if ((j >= 40) &&
        (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF))) {
      lastReadResult = true;
    } else 
    {
      lastReadResult = false;
    }
    currentState = DHWAITING;
    break;

  default:
    currentState = DHWAITING;
    break;
  }
}
