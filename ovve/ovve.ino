//#include <i2c_t3.h>
#include <OctoWS2811.h>
#include <NRF24.h>
#include <SPI.h>
#include <EEPROM.h>
#include "common.h"

void setup() {
  Serial.begin(9600);  //Anything goes, USB! :P
  
  readConfig();
  
  //lcdUpdateTimer.begin(UpdateLEDS, 40000);
  
  //Set up LEDs
  Serial.println("LED: Init... ");
  leds.begin();
  Serial.println("LED: Initialised!");
  
  //Set up EQ pins
  Serial.print("EQ: Init... ");
  pinMode(PIN_EQ_RESET, OUTPUT); // reset
  pinMode(PIN_EQ_STROBE, OUTPUT); // strobe
  digitalWrite(PIN_EQ_RESET,LOW); // reset low
  digitalWrite(PIN_EQ_STROBE,HIGH); //pin 5 is RESET on the shield
  Serial.print("EQ: Initialised!");

  //Set up wireless
  Serial.println("NRF24: Init...");
  if (!nrf24.init())
    Serial.println("NRF24: init failed");
  if (!nrf24.setChannel(2))
    Serial.println("NRF24: setChannel failed");
  if (!nrf24.setThisAddress((uint8_t*)"serv1", 5))
    Serial.println("NRF24: setThisAddress failed");
  if (!nrf24.setPayloadSize(3*sizeof(uint8_t)))
    Serial.println("NRF24: setPayloadSize failed");
  if (!nrf24.setRF(NRF24::NRF24DataRate2Mbps, NRF24::NRF24TransmitPower0dBm))
    Serial.println("NRF24: setRF failed");    
  Serial.println("NRF24: Initialised!");
}

void readConfig(){
  int address = 0;
  Serial.println("Reading EEPOROM config");
  config_data.mode = (uint8_t)EEPROM.read(address++);
  config_data.changetime = ((uint8_t)EEPROM.read(address++)) << 8;
  config_data.changetime |= (uint8_t)EEPROM.read(address++);
  config_data.color.red = (uint8_t)EEPROM.read(address++);
  config_data.color.green = (uint8_t)EEPROM.read(address++);
  config_data.color.blue = (uint8_t)EEPROM.read(address++);
  config_data.eq_thres = ((uint8_t)EEPROM.read(address++)) << 8;
  config_data.eq_thres |= (uint8_t)EEPROM.read(address++);
}

void writeConfig(){
  int address = 0;
  Serial.println("Writing EEPOROM config");
  EEPROM.write(address++, (byte)config_data.mode); 
  EEPROM.write(address++, (byte)(config_data.changetime >> 8)); 
  EEPROM.write(address++, (byte)(config_data.changetime & 0xFF)); 
  EEPROM.write(address++, (byte)config_data.color.red); 
  EEPROM.write(address++, (byte)config_data.color.green); 
  EEPROM.write(address++, (byte)config_data.color.blue);
  EEPROM.write(address++, (byte)(config_data.eq_thres >> 8)); 
  EEPROM.write(address++, (byte)(config_data.eq_thres & 0xFF));
}

void defaultConfig()
{
  config_data.mode = 0;
  config_data.ingle_color.red = 0xAA;
  config_data.color.green = 0xAA;
  config_data.color.blue = 0xAA;
  config_data.eq_thres = 550;
  
  config_data.sparkle.intensity = 0xFF;
  config_data.sparkle.color.red = 0xFF;
  config_data.sparkle.color.green = 0xFF;
  config_data.sparkle.color.blue = 0xFF;
  config_data.sparkle.rate = 10;
  config_data.sparkle.change_rate = 100;
  
  config_data.colorwheel.change_rate = 100;
  config_data.colorwheel.step_size = 1;
  config_data.colorwheel.intensity = 0xFF;
  
  config_data.eq[7];
  config_data.eq_thres;
}

void UpdateLEDS()
{
  Serial.println("+");
  leds.show(); 
}

void loop() {
  uint8_t r8, g8, b8;
  uint8_t data[128];
  
  //data[0] = 0x00;
  //data[1] = 0x00;
  //data[2] = 0xFF;
  
  /*Serial.println("EQ: Reading...");
  readMSGEQ7(audio_data);
  for(int i=0;i<7;i++)
  {
    Serial.print("B");
    Serial.print(i+1);
    Serial.print(": ");
    Serial.print(audio_data[i]);
    Serial.print(";"); 
  }
  Serial.println("");*/
  
  Serial.println("NRF: ");
  if(nrf24.available()){
    // ping_client sends us an unsigned long containing its timestamp
    
    uint8_t len = sizeof(data);
    if (!nrf24.recv((uint8_t*)&data, &len))
      Serial.println("read failed");
    else 
    {
      HandleMess(data, len);
      //Serial.print(data[0], HEX);
      //Serial.print(data[1], HEX);
      //Serial.print(data[2], HEX);
      //int i = 0;  
      //Serial.print((int)r8, HEX); Serial.print((int)g8, HEX); Serial.print((int)b8, HEX);
      //Serial.println(" ");
      //for(i = 0; i< 185;i++)
      //{
      //  leds.setPixel(i, data[0], data[1],data[2]);
      //}
      //leds.show();
    }
  }
  leds.show();
  //nrf24.waitAvailable();
  // ping_client sends us an unsigned long containing its timestamp
  //uint8_t data[3];
  //uint8_t len = sizeof(data);
  //if (!nrf24.recv((uint8_t*)&data, &len))
  //  Serial.println("read failed");
  //else 
  //{
  //  Serial.print(data[0], HEX);
  //  Serial.print(data[1], HEX);
  //  Serial.print(data[2], HEX);
  
//  r8 = (uint8_t)(red/256);
//  g8 = (uint8_t)(green/256);
//  b8 = (uint8_t)(blue/256);
 /*int i = 0;  
  //Serial.print((int)r8, HEX); Serial.print((int)g8, HEX); Serial.print((int)b8, HEX);
  Serial.println(" ");
  for(i = 0; i< 120;i++)
  {
      leds.setPixel(i, data[0], data[1],data[2]);
  }
  leds.show();*/
}

//void colorWipe(int color, int wait)
//{
//  for (int i=0; i < leds.numPixels(); i++) {
//    leds.setPixel(i, color);
//    leds.show();
//    delayMicroseconds(wait);
//  }
//}

// Function to read 7 band equalizers
void readMSGEQ7(int *audio_data)
{
  int band;
  digitalWrite(PIN_EQ_RESET, HIGH);
  digitalWrite(PIN_EQ_RESET, LOW);
  for(band=0; band < 7; band++)
  {
    digitalWrite(PIN_EQ_STROBE,LOW);   // strobe pin - kicks the IC up to the next band 
    delayMicroseconds(30);             // Wait for MSGEQ to set output level
    audio_data[band] = analogRead(PIN_EQ_SIGNAL);  // store band reading
    digitalWrite(PIN_EQ_STROBE,HIGH); 
  }
}

int makeColor(unsigned int hue, unsigned int saturation, unsigned int lightness)
{
	unsigned int red, green, blue;
	unsigned int var1, var2;

	if (hue > 359) hue = hue % 360;
	if (saturation > 100) saturation = 100;
	if (lightness > 100) lightness = 100;

	// algorithm from: http://www.easyrgb.com/index.php?X=MATH&H=19#text19
	if (saturation == 0) {
		red = green = blue = lightness * 255 / 100;
	} else {
		if (lightness < 50) {
			var2 = lightness * (100 + saturation);
		} else {
			var2 = ((lightness + saturation) * 100) - (saturation * lightness);
		}
		var1 = lightness * 200 - var2;
		red = h2rgb(var1, var2, (hue < 240) ? hue + 120 : hue - 240) * 255 / 600000;
		green = h2rgb(var1, var2, hue) * 255 / 600000;
		blue = h2rgb(var1, var2, (hue >= 120) ? hue - 120 : hue + 240) * 255 / 600000;
	}
	return (red << 16) | (green << 8) | blue;
}

unsigned int h2rgb(unsigned int v1, unsigned int v2, unsigned int hue)
{
	if (hue < 60) return v1 * 60 + (v2 - v1) * hue;
	if (hue < 180) return v2 * 60;
	if (hue < 240) return v1 * 60 + (v2 - v1) * (240 - hue);
	return v1 * 60;
}
