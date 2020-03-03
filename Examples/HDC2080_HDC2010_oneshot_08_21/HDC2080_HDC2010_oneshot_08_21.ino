#include <Wire.h>

/*  Note: This Sketch does not use the HDC20XX Library, rather it shows how to communicate directly with the HDC2010.

	This sketch operates HDC2080 and HDC2010 devices over 100kHz I2C bus, each being read from sequentially in one shot mode
    and was written for MSP430 LaunchPads and tested with Energia on MSP430F5529 and MSP430G2553 LaunchPads. 
    
    HDC2080 I2C Address(here) is 0x41, Energia appears to require decimal values of the hex, so 0x41 (7 bit) = 0x82 8 bit, here we use 65 decimal
    HDC2010 I2C Address(here) is 0x40, Energia appears to require decimal values of the hex, so 0x40 (7 bit) = 0x80 8 bit, here we use 64 decimal

    This code outputs calculated comma seperated values on UART and is meant to be used with Energia UART COM, Teraterm, or similar @ 115200 (-F5529) or 9600 (-G2553)
    For *.csv file logging/graphing purposes and to also be (a little) human readable.
    For example, here is what will come over UART at 115kpbs, be displayed on screen and can be written to log file:
    (at program start)

    ,HDC2080 RH,HDC2080 T, HDC2010 RH, HDC2010 T
    ,  43.90,    29.38,      44.91,     28.89
    ,  43.87,    29.41,      44.84,     28.88
    ,  43.89,    29.42,      44.85,     28.89
    ,  43.89,    29.42,      44.86,     28.89

    If Teraterm logging is used, a timestamp can be added to each the line entries.

    Overall loop timing is adjusted by constant at end of loop (currently set for 1000mSec (1 Sec), have checked down to 10mSec

    Soft Resets are commented out as they are not seen to be needed. With some devices, if one is sent, however, a second must be sent - left the code here in case it comes up with OSM

    Josh Wyatt, THS Applications Engineer, Texas Instruments 08/2019
*/
uint16_t HDC2080_INT_REG;
uint8_t  iBuff_INT_REG[1];
uint16_t HDC2080_hum_MSB;
uint16_t HDC2080_temp_MSB;
uint16_t HDC2080_hum_DEC;
uint16_t HDC2080_temp_DEC;
float HDC2080_hum_percentage;
float HDC2080_temp_celcius;
uint8_t  iBuff_HDC2080[4];
uint16_t HDC2010_hum_MSB;
uint16_t HDC2010_temp_MSB;
uint16_t HDC2010_hum_DEC;
uint16_t HDC2010_temp_DEC;
float HDC2010_hum_percentage;
float HDC2010_temp_celcius;
uint8_t  iBuff_HDC2010[4];

void setup()
{
  Wire.begin(); // join i2c bus
  Wire.setTimeout(0);
  //  Wire.setClock(400000);  //This should change I2C speed to 400kHz, but Energia does not support this Arduino API, look at Wire.begin function 08/03/2019 JDW
  //Serial.begin(9600); //open MSP430G2553 serial port at 9600 or open with MSP430F5529 to use with the UART logger dongle
  Serial.begin(115200); //open MSP430F5529 serial port at 115200
  Serial.println(",HDC2080 RH,HDC2080 T, HDC2010 RH, HDC2010 T");              // Print humidity board header
  delay(500);

  /////////////////HDC2080 INIT, FOR TEST PURPOSES///////////////////
  //SOFT_RESET - this can be used, but if one reset is sent, a second one should be as well.
  //  Wire.beginTransmission(65);                                             // transmit to HDC2080 device address 0x41 (0x82 (write), 0x83 (read))
  //  Wire.write(14);                                                         // Register 0x0E
  //  Wire.write(128);                                                        // soft reset of HDC2080
  //  Wire.endTransmission();                                                 // stop transmitting
  //  delayMicroseconds(550);                                                 // delay, delay not needed (i think so far)
  //  //
  //  //  //  //Second Reset
  //  Wire.beginTransmission(65);                                             // transmit to HDC2080 device address 0x41 (0x82 (write), 0x83 (read))
  //  Wire.write(14);                                                         // Register 0x0E
  //  Wire.write(128);                                                        // soft reset of HDC2080
  //  Wire.endTransmission();                                                 // stop transmitting
  //  delayMicroseconds(550);                                                 // delay, needed
  //
}
void loop()
{
  ////////////////HDC2080///////////                                         http://www.ti.com/lit/ds/symlink/hdc2080.pdf
  // use this when doing one shot mode
  Wire.beginTransmission(65);                                             // transmit to HDC2080 device address 0x41
  Wire.write(15);                                                         // register 0x0F
  Wire.write(01);                                                         // start conversion
  Wire.endTransmission();                                                 // stop transmitting
  delayMicroseconds(1600);                                              // delay set for 14 bit conversion time for temp and humidity together - typical should be ~1270uSec
  Serial.print(",");
  Wire.beginTransmission(65);                                             // transmit to HDC2080 device address 0x41 (0x82 (write), 0x83 (read))
  Wire.write(00);                                                         // pointer
  Wire.endTransmission();                                                 // stop transmitting
  Wire.requestFrom(65, 4, 1);                                             // request 4 bytes from HDC2080 slave device, registers 0x00 through 0x03

  // math here for HDC2080 output
  while (Wire.available())
  {
    Wire.readBytes(iBuff_HDC2080, 4);
    HDC2080_hum_MSB = iBuff_HDC2080[3] << 8;                              // shift left
    HDC2080_hum_DEC = (HDC2080_hum_MSB + iBuff_HDC2080[2]);               // get value for calculation, made from iBuff index array values 3 and 2 for humidity reading.
    HDC2080_hum_percentage = ((float)(HDC2080_hum_DEC) / 65536) * 100;    // do math according to the HDC2080 datasheet, page 18 http://www.ti.com/lit/ds/symlink/hdc2080.pdf
    Serial.print("  ");
    Serial.print(HDC2080_hum_percentage);                                 // print out %RH to terminal
    Serial.print(",");
    HDC2080_temp_MSB = (iBuff_HDC2080[1] << 8);                            // shift left
    HDC2080_temp_DEC = (HDC2080_temp_MSB + iBuff_HDC2080[0]);              // get value for calculation, made from iBuff index array values 0 and 1 for temp reading.
    HDC2080_temp_celcius = ((float)(HDC2080_temp_DEC) / 65536) * 165 - 40; // do math according to the HDC2080 datasheet, page 18 http://www.ti.com/lit/ds/symlink/hdc2080.pdf
    Serial.print("    ");
    Serial.print(HDC2080_temp_celcius);                                    // print out T to terminal
    Serial.print(",");
  }

  ////////////////HDC2010///////////                                           http://www.ti.com/lit/ds/symlink/hdc2010.pdf
  Wire.beginTransmission(64);                                               // transmit to HDC2010 device address 0x40
  Wire.write(15);                                                           // register 0x0F
  Wire.write(01);                                                           // start conversion
  Wire.endTransmission();                                                   // stop transmitting
  delayMicroseconds(1600);                                              // delay set for 14 bit conversion time for temp and humidity together - typical should be ~1270uSec
  Wire.beginTransmission(64);                                               // transmit to HDC2010 device address 0x40
  Wire.write(00);                                                           // pointer to 0x00
  Wire.endTransmission();                                                   // stop transmitting
  Wire.requestFrom(64, 4, 1);                                               // request 4 bytes from HDC2010 slave device, registers 0x00 through 0x03

  // math here for HDC2010 output
  while (Wire.available())
  {
    Wire.readBytes(iBuff_HDC2010, 4);
    HDC2010_hum_MSB = iBuff_HDC2010[3] << 8;                                // shift left
    HDC2010_hum_DEC = (HDC2010_hum_MSB + iBuff_HDC2010[2]);                 // get value for calculation, made from iBuff index array values 3 and 2 for humidity reading.
    HDC2010_hum_percentage = ((float)(HDC2010_hum_DEC) / 65536) * 100;      // do math according to the HDC2010 datasheet, page 18 http://www.ti.com/lit/ds/symlink/hdc2010.pdf
    Serial.print("\t ");
    Serial.print(HDC2010_hum_percentage);                                   // print out %RH to terminal
    Serial.print(",");
    HDC2010_temp_MSB = (iBuff_HDC2010[1] << 8);                             // shift left
    HDC2010_temp_DEC = (HDC2010_temp_MSB + iBuff_HDC2010[0]);               // get value for calculation, made from iBuff index array values 0 and 1 for temp reading.
    HDC2010_temp_celcius = ((float)(HDC2010_temp_DEC) / 65536) * 165 - 40;  // do math according to the HDC2010 datasheet, page 18 http://www.ti.com/lit/ds/symlink/hdc2010.pdf
    Serial.print("     ");
    Serial.println(HDC2010_temp_celcius);                                     // print out T to terminal
  }
  delay(1000);                              // Systems Engineering requested control of read loop timing of the sensors - this is overall loop delay, comment out or modify to go faster or slower
  //  delay(10);
}
