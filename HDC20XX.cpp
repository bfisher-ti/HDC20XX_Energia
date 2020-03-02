/*
	HDC20XX.cpp
	Created By: Brandon Fisher. March 2020
	
	This code is release AS-IS into the public domain, no guarantee or warranty is given.
	
	Description: This library facilitates communication with, and configuration of,
	the HDC20XX Family of Temperature and Humidity Sensors. It makes extensive use of the 
	Wire.H library, and was designed and tested in Energia. 
*/


#include <HDC20XX.h>
#include <Wire.h>

//Define Register Map
	#define TEMP_LOW 0x00
	#define TEMP_HIGH 0x01
	#define HUMID_LOW 0x02
	#define HUMID_HIGH 0x03
	#define INTERRUPT_DRDY 0x04
	#define TEMP_MAX 0x05
	#define HUMID_MAX 0x06
	#define INTERRUPT_CONFIG 0x07
	#define TEMP_OFFSET_ADJUST 0x08
	#define HUM_OFFSET_ADJUST 0x09
	#define TEMP_THR_L 0x0A
	#define TEMP_THR_H 0x0B
	#define HUMID_THR_L 0x0C
	#define HUMID_THR_H 0x0D
	#define CONFIG 0x0E
	#define MEASUREMENT_CONFIG 0x0F
	#define MID_L 0xFC
	#define MID_H 0xFD
	#define DEVICE_ID_L 0xFE
	#define DEVICE_ID_H 0xFF
	
HDC20XX::HDC20XX(uint8_t addr)
{
  _addr = addr;
  
}

void HDC20XX::begin(void)
{
	Wire.begin();
}

float HDC20XX::readTemp(void)
{
	uint8_t byte[2];
	uint16_t temp;
	byte[0] = readReg(TEMP_LOW);
	byte[1] = readReg(TEMP_HIGH);
	byte[0] = byte[0] & 0xFC;	
	temp = (unsigned int)byte[1] << 8 | byte[0];
	
	return (float)(temp) * 165 / 65536 - 40;
	
}

float HDC20XX::readHumidity(void)
{
	uint8_t byte[2];
	uint16_t humidity;
	byte[0] = readReg(HUMID_LOW);
	byte[1] = readReg(HUMID_HIGH);
	
	humidity = (unsigned int)byte[1] << 8 | byte[0];
	
	return (float)(humidity)/( 65536 )* 100;
	
}

void HDC20XX::enableHeater(void)
{
	uint8_t configContents;	//Stores current contents of config register
	
	configContents = readReg(CONFIG);
	
	//set bit 3 to 1 to enable heater
	configContents = (configContents | 0x08);
	
	writeReg(CONFIG, configContents);
	
}

void HDC20XX::disableHeater(void)
{
	uint8_t configContents;	//Stores current contents of config register
	
	configContents = readReg(CONFIG);
	
	//set bit 3 to 0 to disable heater (all other bits 1)
	configContents = (configContents & 0xF7);
	writeReg(CONFIG, configContents);
	
}

void HDC20XX::openReg(uint8_t reg)
{
  Wire.beginTransmission(_addr); 		// Connect to HDC20XX
  Wire.write(reg); 						// point to specified register
  Wire.endTransmission(); 				// Relinquish bus control
}

uint8_t HDC20XX::readReg(uint8_t reg)
{
	openReg(reg);
	uint8_t reading; 					// holds byte of read data
	Wire.requestFrom(_addr, 1); 		// Request 1 byte from open register
	Wire.endTransmission();				// Relinquish bus control
	
	if (1 <= Wire.available())
	{
		reading = (Wire.read());			// Read byte
	}
	
	return reading;
}

void HDC20XX::writeReg(uint8_t reg, uint8_t data)
{
	
  Wire.beginTransmission(_addr);		// Open Device
  Wire.write(reg);						// Point to register
  Wire.write(data);						// Write data to register 
  Wire.endTransmission();				// Relinquish bus control
  
}

void HDC20XX::setLowTemp(float temp)
{
	uint8_t temp_thresh_low;
	
	// Verify user is not trying to set value outside bounds
	if (temp < -40)
	{
		temp = -40;
	}
	else if (temp > 125)
	{
		temp = 125;
	}
	
	// Calculate value to load into register
	temp_thresh_low = (uint8_t)(256 * (temp + 40)/165);
	
	writeReg(TEMP_THR_L, temp_thresh_low);
	
}

void HDC20XX::setHighTemp(float temp)
{ 
	uint8_t temp_thresh_high;
	
	// Verify user is not trying to set value outside bounds
	if (temp < -40)
	{
		temp = -40;
	}
	else if (temp > 125)
	{
		temp = 125;
	}
	
	// Calculate value to load into register
	temp_thresh_high = (uint8_t)(256 * (temp + 40)/165);
	
	writeReg(TEMP_THR_H, temp_thresh_high);
	
}

void HDC20XX::setHighHumidity(float humid)
{
	uint8_t humid_thresh;
	
	// Verify user is not trying to set value outside bounds
	if (humid < 0)
	{
		humid = 0;
	}
	else if (humid > 100)
	{
		humid = 100;
	}
	
	// Calculate value to load into register
	humid_thresh = (uint8_t)(256 * (humid)/100);
	
	writeReg(HUMID_THR_H, humid_thresh);
	
}

void HDC20XX::setLowHumidity(float humid)
{
	uint8_t humid_thresh;
	
	// Verify user is not trying to set value outside bounds
	if (humid < 0)
	{
		humid = 0;
	}
	else if (humid > 100)
	{
		humid = 100;
	}
	
	// Calculate value to load into register
	humid_thresh = (uint8_t)(256 * (humid)/100);
	
	writeReg(HUMID_THR_L, humid_thresh);
	
}

//  Return humidity from the low threshold register
float HDC20XX::readLowHumidityThreshold(void)
{
	uint8_t regContents;
	
	regContents = readReg(HUMID_THR_L);
	
	return (float)regContents * 100/256;
	
}

//  Return humidity from the high threshold register
float HDC20XX::readHighHumidityThreshold(void)
{
	uint8_t regContents;
	
	regContents = readReg(HUMID_THR_H);
	
	return (float)regContents * 100/256;
	
}

//  Return temperature from the low threshold register
float HDC20XX::readLowTempThreshold(void)
{
	uint8_t regContents;
	
	regContents = readReg(TEMP_THR_L);
	
	return (float)regContents * 165/256 - 40;
	
}

//  Return temperature from the high threshold register
float HDC20XX::readHighTempThreshold(void)
{
	uint8_t regContents;
	
	regContents = readReg(TEMP_THR_H);
	
	return (float)regContents * 165/256 - 40;
	
}


/* Upper two bits of the MEASUREMENT_CONFIG register controls
   the temperature resolution*/
void HDC20XX::setTempRes(int resolution)
{ 
	uint8_t configContents;
	configContents = readReg(MEASUREMENT_CONFIG);
	
	switch(resolution)
	{
		case FOURTEEN_BIT:
			configContents = (configContents & 0x3F);
			break;
			
		case ELEVEN_BIT:
			configContents = (configContents & 0x7F);
			configContents = (configContents | 0x40);  
			break;
			
		case NINE_BIT:
			configContents = (configContents & 0xBF);
			configContents = (configContents | 0x80); 
			break;
			
		default:
			configContents = (configContents & 0x3F);
	}
	
	writeReg(MEASUREMENT_CONFIG, configContents);
	
}
/*  Bits 5 and 6 of the MEASUREMENT_CONFIG register controls
    the humidity resolution*/
void HDC20XX::setHumidRes(int resolution)
{ 
	uint8_t configContents;
	configContents = readReg(MEASUREMENT_CONFIG);
	
	switch(resolution)
	{
		case FOURTEEN_BIT:
			configContents = (configContents & 0xCF);
			break;
			
		case ELEVEN_BIT:
			configContents = (configContents & 0xDF);
			configContents = (configContents | 0x10);  
			break;
			
		case NINE_BIT:
			configContents = (configContents & 0xEF);
			configContents = (configContents | 0x20); 
			break;
			
		default:
			configContents = (configContents & 0xCF);
	}
	
	writeReg(MEASUREMENT_CONFIG, configContents);	
}

/*  Bits 2 and 1 of the MEASUREMENT_CONFIG register controls
    the measurement mode  */
void HDC20XX::setMeasurementMode(int mode)
{ 
	uint8_t configContents;
	configContents = readReg(MEASUREMENT_CONFIG);
	
	switch(mode)
	{
		case TEMP_AND_HUMID:
			configContents = (configContents & 0xF9);
			break;
			
		case TEMP_ONLY:
			configContents = (configContents & 0xFC);
			configContents = (configContents | 0x02);  
			break;
			
		case HUMID_ONLY:
			configContents = (configContents & 0xFD);
			configContents = (configContents | 0x04); 
			break;
			
		default:
			configContents = (configContents & 0xF9);
	}
	
	writeReg(MEASUREMENT_CONFIG, configContents);
}

/*  Bit 0 of the MEASUREMENT_CONFIG register can be used
    to trigger measurements  */
void HDC20XX::triggerMeasurement(void)
{ 
	uint8_t configContents;
	configContents = readReg(MEASUREMENT_CONFIG);

	configContents = (configContents | 0x01);
	writeReg(MEASUREMENT_CONFIG, configContents);
}

/*  Bit 7 of the CONFIG register can be used to trigger a 
    soft reset  */
void HDC20XX::reset(void)
{
	uint8_t configContents;
	configContents = readReg(CONFIG);

	configContents = (configContents | 0x80);
	writeReg(CONFIG, configContents);
	delay(50);
}

/*  Bit 2 of the CONFIG register can be used to enable/disable 
    the interrupt pin  */
void HDC20XX::enableInterrupt(void)
{
	uint8_t configContents;
	configContents = readReg(CONFIG);

	configContents = (configContents | 0x04);
	writeReg(CONFIG, configContents);
}

/*  Bit 2 of the CONFIG register can be used to enable/disable 
    the interrupt pin  */
void HDC20XX::disableInterrupt(void)
{
	uint8_t configContents;
	configContents = readReg(CONFIG);

	configContents = (configContents & 0xFB);
	writeReg(CONFIG, configContents);
}


/*  Bits 6-4  of the CONFIG register controls the measurement 
    rate  */
void HDC20XX::setRate(int rate)
{ 
	uint8_t configContents;
	configContents = readReg(CONFIG);
	
	switch(rate)
	{
		case MANUAL:
			configContents = (configContents & 0x8F);
			break;
			
		case TWO_MINS:
			configContents = (configContents & 0x9F);
			configContents = (configContents | 0x10);  
			break;
			
		case ONE_MINS:
			configContents = (configContents & 0xAF);
			configContents = (configContents | 0x20); 
			break;
		
		case TEN_SECONDS:
			configContents = (configContents & 0xBF);
			configContents = (configContents | 0x30); 
			break;
		
		case FIVE_SECONDS:
			configContents = (configContents & 0xCF);
			configContents = (configContents | 0x40); 
			break;
		
		case ONE_HZ:
			configContents = (configContents & 0xDF);
			configContents = (configContents | 0x50); 
			break;
		
		case TWO_HZ:
			configContents = (configContents & 0xEF);
			configContents = (configContents | 0x60); 
			break;
		
		case FIVE_HZ:
			configContents = (configContents | 0x70); 
			break;
			
		default:
			configContents = (configContents & 0x8F);
	}
	
	writeReg(CONFIG, configContents);
}

/*  Bit 1 of the CONFIG register can be used to control the  
    the interrupt pins polarity */
void HDC20XX::setInterruptPolarity(int polarity)
{
	uint8_t configContents;
	configContents = readReg(CONFIG);
	
	switch(polarity)
	{
		case ACTIVE_LOW:
			configContents = (configContents & 0xFD);
			break;
			
		case ACTIVE_HIGH:
			configContents = (configContents | 0x02);  
			break;
			
		default:
			configContents = (configContents & 0xFD);
	}
	
	writeReg(CONFIG, configContents);	
}

/*  Bit 0 of the CONFIG register can be used to control the  
    the interrupt pin's mode */
void HDC20XX::setInterruptMode(int mode)
{
	uint8_t configContents;
	configContents = readReg(CONFIG);
	
	switch(mode)
	{
		case LEVEL_MODE:
			configContents = (configContents & 0xFE);
			break;
			
		case COMPARATOR_MODE:
			configContents = (configContents | 0x01);  
			break;
			
		default:
			configContents = (configContents & 0xFE);
	}
	
	writeReg(CONFIG, configContents);	
}


uint8_t HDC20XX::readInterruptStatus(void)
{
	uint8_t regContents;
	regContents = readReg(INTERRUPT_DRDY);
	return regContents;
	
}

//  Clears the maximum temperature register
void HDC20XX::clearMaxTemp(void)
{ 
	writeReg(TEMP_MAX, 0x00);
}

//  Clears the maximum humidity register
void HDC20XX::clearMaxHumidity(void)
{ 
	writeReg(HUMID_MAX, 0x00);
}

//  Reads the maximum temperature register
float HDC20XX::readMaxTemp(void)
{
	uint8_t regContents;
	
	regContents = readReg(TEMP_MAX);
	
	return (float)regContents * 165/256 - 40;
	
}

//  Reads the maximum humidity register
float HDC20XX::readMaxHumidity(void)
{
	uint8_t regContents;
	
	regContents = readReg(HUMID_MAX);
	
	return (float)regContents /256 * 100;
	
}


// Enables the interrupt pin for comfort zone operation
void HDC20XX::enableThresholdInterrupt(void)
{
	
	uint8_t regContents;
	regContents = readReg(INTERRUPT_CONFIG);

	regContents = (regContents | 0x78);

	writeReg(INTERRUPT_CONFIG, regContents);	
}

// Disables the interrupt pin for comfort zone operation
void HDC20XX::disableThresholdInterrupt(void)
{
	uint8_t regContents;
	regContents = readReg(INTERRUPT_CONFIG);

	regContents = (regContents & 0x87);

	writeReg(INTERRUPT_CONFIG, regContents);	
}

// enables the interrupt pin for DRDY operation
void HDC20XX::enableDRDYInterrupt(void)
{
	uint8_t regContents;
	regContents = readReg(INTERRUPT_CONFIG);

	regContents = (regContents | 0x80);

	writeReg(INTERRUPT_CONFIG, regContents);	
}

// disables the interrupt pin for DRDY operation
void HDC20XX::disableDRDYInterrupt(void)
{
	uint8_t regContents;
	regContents = readReg(INTERRUPT_CONFIG);

	regContents = (regContents & 0x7F);

	writeReg(INTERRUPT_CONFIG, regContents);	
}
