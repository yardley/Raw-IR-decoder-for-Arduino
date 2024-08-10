#include <Arduino.h>

// KY-26 Remote protocol
//
// The KY-26 remote control is used in locally branded air conditioners.
// I used a ZAICON air conditioner, which also seems to be rebranded as
// SACOM, SENCYS, and possibly others.
//
// The remote sends a 4-byte message which contains all possible settings every
// time.
//
// Byte 0 contains the a power signal, operating mode, and fan speed.
// Byte 1 contains the timer setting.
// Byte 2 contains the temperature setting.
// Byte 3 contains a checksum.

bool decodeKY26Remote(byte *bytes, int byteCount) {
  // If this looks like a KY-26 code...
  if (byteCount != 4               //
      || (bytes[0] & 0xC4) != 0x00 // Bits 3, 7, and 8 must be 0
      || (bytes[1] & 0xC0) != 0x80 // Bits 7 must be 0, bit 8 must be 1
      || (bytes[2] & 0xE0) != 0x00 // Bits 6, 7, and 8 must be 0
      || (bytes[3] & 0x80) != 0x80 // Bit 8 must be 1
  ) {
    return false;
  }

  Serial.println(F("Looks like a KY-26 remote control protocol"));

  // Power mode
  // It sends the same signal for on/off, it depends on the unit status.
  if (bytes[0] & 0x08) {
    Serial.println(F("On/Off action"));
  }

  // Operating mode
  Serial.print(F("MODE: "));
  switch (bytes[0] & 0x03) {
  case 0x00:
    Serial.println(F("AUTO"));
    break;
  case 0x01:
    Serial.println(F("COOL"));
    break;
  case 0x03:
    Serial.println(F("FAN"));
    break;
  case 0x02:
    Serial.println(F("DRY"));
    break;
  }

  // Fan speed
  switch (bytes[0] & 0x30) {
  case 0x10:
    Serial.println(F("FAN: 1"));
    break;
  case 0x20:
    Serial.println(F("FAN: 2"));
    break;
  case 0x30:
    Serial.println(F("FAN: 3"));
    break;
  }

  // Timer
  // First 4 bits are hours, last 4 bits are half hours.
  Serial.print(F("Timer: "));
  if (!(bytes[1] & 0x20)) {
    Serial.println(F("OFF"));
  } else {
    Serial.print(bytes[1] & 0x0F);
    if (bytes[1] & 0x10) {
      Serial.print(F(".5"));
    }
    Serial.println(F("h"));
  }

  // Temperature
  Serial.print(F("Temperature: "));
  int temp = bytes[2] & 0x1F;
  Serial.println(temp == 0 ? 15 : temp); // 0 is 15 degrees

  // CheckSum8 Modulo 256
  byte checksum = 0;
  for (int i = 0; i < 3; i++) {
    checksum += bytes[i];
  }
  checksum = checksum % 256;
  if (checksum != bytes[3]) {
    Serial.print(F("Checksum error: "));
    Serial.print(checksum, HEX);
    Serial.print(F(" != "));
    Serial.println(bytes[3], HEX);
  } else {
    Serial.println(F("Checksum matches"));
  }

  return true;
}

/*
      x
Hh 00011000 00000001 00000001 11110101 h POWER
       xx
Hh 00001000 00000001 11101000 11100101 h SPEED 1
Hh 00000100 00000001 11101000 11101101 h SPEED 2
Hh 00001100 00000001 11101000 11100011 h SPEED 3
   xx
Hh 00001000 00000001 11101000 11100101 h MODE 1
Hh 10001000 00000001 11011000 00110101 h MODE 2
Hh 11001000 00000001 11011000 01110101 h MODE 3
Hh 01001000 00000001 10011000 11010101 h MODE 4
                     xxxxxx
Hh 00001000 00000001 00000000 00001001 h TEMP 15
Hh 00001000 00000001 00001000 00000101 h TEMP 16
Hh 00001000 00000001 10001000 10000101 h TEMP 17
Hh 00001000 00000001 01001000 01000101 h TEMP 18
Hh 00001000 00000001 11001000 11000101 h TEMP 19
Hh 00001000 00000001 00101000 00100101 h TEMP 20
Hh 00001000 00000001 10101000 10100101 h TEMP 21
Hh 00001000 00000001 01101000 01100101 h TEMP 22
Hh 00001000 00000001 11101000 11100101 h TEMP 23
Hh 00001000 00000001 00011000 00010101 h TEMP 24
Hh 00001000 00000001 10011000 10010101 h TEMP 25
Hh 00001000 00000001 01011000 01010101 h TEMP 26
Hh 00001000 00000001 11011000 11010101 h TEMP 27
Hh 00001000 00000001 00111000 00110101 h TEMP 28
Hh 00001000 00000001 10111000 10110101 h TEMP 29
Hh 00001000 00000001 01111000 01110101 h TEMP 30
Hh 00001000 00000001 11111000 11110101 h TEMP 31
            xxxxxx
Hh 00001100 00000001 00000000 00001101 h Timer OFF
Hh 00001100 00001101 00000000 00000111 h Timer 0.5h
Hh 00001100 10000101 00000000 10001011 h Timer 1h
Hh 00001100 10001101 00000000 10000111 h Timer 1.5h
Hh 00001100 01000101 00000000 01001011 h Timer 2h
Hh 00001100 01001101 00000000 01000111 h Timer 2.5h
Hh 00001100 11000101 00000000 11001011 h Timer 3h
Hh 00001100 11001101 00000000 11000111 h Timer 3.5h
Hh 00001100 00100101 00000000 00101011 h Timer 4h
Hh 00001100 00101101 00000000 00100111 h Timer 4.5h
Hh 00001100 10100101 00000000 10101011 h Timer 5h
Hh 00001100 10101101 00000000 10100111 h Timer 5.5h
Hh 00001100 01100101 00000000 01101011 h Timer 6h
Hh 00001100 01101101 00000000 01100111 h Timer 6.5h
Hh 00001100 11100101 00000000 11101011 h Timer 7h
Hh 00001100 11101101 00000000 11100111 h Timer 7.5h
Hh 00001100 00010101 00000000 00011011 h Timer 8h
Hh 00001100 00011101 00000000 00010111 h Timer 8.5h
Hh 00001100 10010101 00000000 10011011 h Timer 9h
Hh 00001100 10011101 00000000 10010111 h Timer 9.5h
Hh 00001100 01010101 00000000 01011011 h Timer 10h
Hh 00001100 01011101 00000000 01010111 h Timer 10.5h
Hh 00001100 11010101 00000000 11011011 h Timer 11h
Hh 00001100 11011101 00000000 11010111 h Timer 11.5h
Hh 00001100 00110101 00000000 00111011 h Timer 12h
*/