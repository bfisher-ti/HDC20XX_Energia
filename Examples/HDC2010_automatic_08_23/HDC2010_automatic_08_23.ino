#include <Wire.h>

int val2 = 0;
int inPin2 = A1; //P6.1 on MSP430F5529LP
int ledPin2 = P4_7; //GREEN LED

/*  Note: This Sketch does not use the HDC20XX Library, rather it shows how to communicate directly with the HDC2010.

	This sketch operates HDC2010 device over 100kHz I2C bus, being read from in automatic mode.
    It was written for MSP430 LaunchPads and tested with Energia on MSP430F5529 LaunchPad.

    HDC2010 I2C Address(here) is 0x40, Energia appears to require decimal values of the hex, so 0x40 (7 bit) = 0x80 8 bit, here we use 64 decimal

    This code outputs calculated comma separated values on UART and is meant to be used with Energia UART COM, Teraterm, or similar @ 115200 (-F5529) or 9600 (-G2553)
    For *.csv file logging/graphing purposes and to also be (a little) human readable.
    For example, here is what will come over UART at 115kpbs, be displayed on screen and can be written to log file:
    (at program start)

    , HDC2010 RH, HDC2010 T
    ,   44.91,     28.89
    ,   44.84,     28.88
    ,   44.85,     28.89
    ,   44.86,     28.89

    If Teraterm logging is used, a timestamp can be added to each the line entries.

    Soft Resets are commented out as they are not seen to be needed. With some devices, if one is sent, however, a second might also need to be sent - left the code here in case it comes up with OSM

    Josh Wyatt, THS Applications Engineer, Texas Instruments 08/2019
*/
uint16_t HDC2010_INT_REG;
uint8_t  iBuff_INT_REG2[1];
uint16_t HDC2010_hum_MSB;
uint16_t HDC2010_temp_MSB;
uint16_t HDC2010_hum_DEC;
uint16_t HDC2010_temp_DEC;
float HDC2010_hum_percentage;
float HDC2010_temp_celcius;
uint8_t  iBuff_HDC2010[4];

void setup()
{
  pinMode(inPin2, INPUT_PULLDOWN);
  pinMode(ledPin2, OUTPUT);
  digitalWrite(ledPin2, LOW);

  Wire.begin(); // join i2c bus
  Wire.setTimeout(0);
  //  Wire.setClock(400000);  //This should change I2C speed to 400kHz, but Energia does not support this Arduino API, look at Wire.begin function 08/03/2019 JDW
  //Serial.begin(9600); //open MSP430G2553 serial port at 9600 or open with MSP430F5529 to use with the UART logger dongle
  Serial.begin(115200); //open MSP430F5529 serial port at 115200
  Serial.println(",   HDC2010 RH  ,   HDC2010 T");              // Print humidity board header
  delay(300);
  /*
       HDC2010 SETUP
  */
  Wire.beginTransmission(64);                                             // transmit to HDC2010 device address 0x40 (0x80 (write), 0x81 (read))
  Wire.write(14);                                                         // Register 0x0E
  Wire.write(128);                                                        // soft reset of HDC2010
  Wire.endTransmission();                                                 // stop transmitting
  delayMicroseconds(550);
  //  Wire.beginTransmission(64);                                             // transmit to HDC2010 device address 0x40 (0x80 (write), 0x81 (read))
  //  Wire.write(14);                                                         // Register 0x0E
  //  Wire.write(128);                                                        // soft reset of HDC2010
  //  Wire.endTransmission();                                                 // stop transmitting
  //  delayMicroseconds(550);

  Wire.beginTransmission(64);                                               // transmit to HDC2010 device address 0x40 (0x80 (write), 0x81 (read))
  Wire.write(07);                                                           // Register 0x07
  Wire.write(128);                                                          // writing 0x80 into 0x07
  Wire.endTransmission();                                                   // stop transmitting
  delayMicroseconds(550);
  Wire.beginTransmission(64);                                               // transmit to HDC2010 device address 0x40 (0x80 (write), 0x81 (read))
  Wire.write(14);                                                           // Register 0x0E
  //  Wire.write(87);                                                          // writing 0x57 into 0x0E, DRDY enabled, active high with comparator mode, need to research how this would be useful
  Wire.write(118);                                                          // writing 0x76 into 0x0E, this makes DRDY active high - normal setting for 5 reads per sec AMM mode
  //     Wire.write(102);                                                          // writing 0x66 into 0x0E, this makes DRDY active high - normal setting for 2 reads per sec AMM mode
  //  Wire.write(86);                                                          // writing 0x56 into 0x0E, this makes DRDY active high - normal setting for 1 read per sec AMM mode
  //  Wire.write(84);                                                          // writing 0x54 into 0x0E, this makes DRDY active low - normal setting for 1 read per sec AMM mode
  //  Wire.write(83);                                                          // writing 0x53 into 0x0E, DRDY active high, but high Z, comparator mode need to research how this would be useful
  //  Wire.write(82);                                                          // writing 0x52 into 0x0E, DRDY active high, but high Z need to research how this would be useful
  //  Wire.write(81);                                                          // writing 0x51 into 0x0E, DRDY active low, comparator mode, but high Z need to research how this would be useful
  //    Wire.write(80);                                                        // writing 0x50 into 0x0E DRDY active low, level sensitive mode, but high Z, need to research how this would be useful
  Wire.endTransmission();                                                   // stop transmitting
  //      delayMicroseconds(550);
  // read out all functional registers
  Wire.beginTransmission(64);                                               // transmit to HDC2010 device address 0x40 (0x80 (write), 0x81 (read))
  Wire.write(04);                                                           // pointer (0x04)
  Wire.endTransmission();                                                   // stop transmitting
  Wire.requestFrom(64, 12, 1);                                              // request 12 bytes from HDC2010 slave device,  from register 0x04 to 0x0E
  //    delayMicroseconds(550);

  // start conversion
  /*
     0x01 – 14 bit, 0x03 – temp only
     0x51 – 11 bit,  0x53 – temp only
     0xA1 – 9 bit , 0xA3 – temp only
  */
  Wire.beginTransmission(64);                                             // transmit to HDC2010 device address 0x40 (0x80 (write), 0x81 (read))
  Wire.write(15);                                                         // Register 0x0F
  //    Wire.write(161);                                                         // HDC2010 start conversion (0xA1) 9 bit
  //  Wire.write(163);                                                         // HDC2010 start conversion (0xA3) 9 bit, temp only
  Wire.write(81);                                                         // HDC2010 start conversion (0x51) 11 bit
  //  Wire.write(83);                                                         // HDC2010 start conversion (0x53) 11 bit, temp only
  //  Wire.write(01);                                                         // HDC2010 start conversion (0x01) 14 bit
  //  Wire.write(03);                                                         // HDC2010 start conversion (0x03) 14 bit, temp only
  Wire.endTransmission();                                                 // stop transmitting
  //  delayMicroseconds(1600);
}

void loop()
{
  ////////////////HDC2010///////////                                           http://www.ti.com/lit/ds/symlink/hdc2010.pdf
  val2 = digitalRead(inPin2);
  digitalWrite(ledPin2, val2);

  if (val2 == HIGH) //0x0E set for DRDY active high, i.e. 0x56
    //  if (val2 == LOW) //0x0E set for DRDY active low, i.e. 0x54
  {
    Wire.beginTransmission(64);                                             // transmit to HDC2010 device address 0x40 (0x80 (write), 0x81 (read))
    Wire.write(04);                                                         // pointer
    Wire.endTransmission();                                                 // stop transmitting
    Wire.requestFrom(64, 1, 1);                                             // request 1 bytes from HDC2010 slave device, register 0x04 (should be 0x80)
    Wire.readBytes (iBuff_INT_REG2, 1);
    HDC2010_INT_REG = iBuff_INT_REG2[0];
    if  (HDC2010_INT_REG == 128)
    {
      Wire.beginTransmission(64);                                             // transmit to HDC2010 device address 0x40 (0x80 (write), 0x81 (read))
      Wire.write(00);                                                         // pointer
      Wire.endTransmission();                                                 // stop transmitting
      Wire.requestFrom(64, 4, 1);                                             // request 4 bytes from HDC2010 slave device, registers 0x00 through 0x03
      // math here for HDC2010 output
      while (Wire.available())
      {
        Wire.readBytes(iBuff_HDC2010, 4);
        HDC2010_hum_MSB = iBuff_HDC2010[3] << 8;                              // shift left
        HDC2010_hum_DEC = (HDC2010_hum_MSB + iBuff_HDC2010[2]);               // get value for calculation, made from iBuff index array values 3 and 2 for humidity reading.
        HDC2010_hum_percentage = ((float)(HDC2010_hum_DEC) / 65536) * 100;    // do math according to the HDC2010 datasheet, page 18 http://www.ti.com/lit/ds/symlink/hdc2010.pdf
        Serial.print(",    ");
        Serial.print(HDC2010_hum_percentage);                                 // print out %RH to terminal
        Serial.print("      ,");
        HDC2010_temp_MSB = (iBuff_HDC2010[1] << 8);                            // shift left
        HDC2010_temp_DEC = (HDC2010_temp_MSB + iBuff_HDC2010[0]);              // get value for calculation, made from iBuff index array values 0 and 1 for temp reading.
        HDC2010_temp_celcius = ((float)(HDC2010_temp_DEC) / 65536) * 165 - 40; // do math according to the HDC2080 datasheet, page 18 http://www.ti.com/lit/ds/symlink/hdc2010.pdf
        Serial.print("    ");
        Serial.println(HDC2010_temp_celcius);                                    // print out T to terminal
      }
    }
  }
}
