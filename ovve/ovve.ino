//#include <i2c_t3.h>
#include <OctoWS2811.h>
#include <NRF24.h>
#include <SPI.h>
#include <EEPROM.h>
#include "common.h"

boolean nrfInit() {
  //Set up wireless
  Serial.println("NRF24: Init...");
  if (!nrf24.init()) {
    Serial.println("NRF24: init failed");
    return false;
  }
  if (!nrf24.setChannel(NRF_DEFAULT_CHANNEL)) {
    Serial.println("NRF24: setChannel failed");
    return false;
  }
  if (!nrf24.setThisAddress((uint8_t*)"serv1", 5)) {
    Serial.println("NRF24: setThisAddress failed");
    return false;
  }
  if (!nrf24.setPayloadSize(NRF_PACKET_SIZE)) {
    Serial.println("NRF24: setPayloadSize failed");
    return false;
  }
  if (!nrf24.setRF(NRF24::NRF24DataRate2Mbps, NRF24::NRF24TransmitPower0dBm)) {
    Serial.println("NRF24: setRF failed");
    return false;
  }
  Serial.println("NRF24: Initialised!");
  return true;
}

void eqInit() {
  //Set up EQ pins
  Serial.print("EQ: Init... ");
  pinMode(PIN_EQ_RESET, OUTPUT); // reset
  pinMode(PIN_EQ_STROBE, OUTPUT); // strobe
  digitalWrite(PIN_EQ_RESET,LOW); // reset low
  digitalWrite(PIN_EQ_STROBE,HIGH); //pin 5 is RESET on the shield
  Serial.print("EQ: Initialised!");
}

void setup() {
  Serial.begin(9600);  //Anything goes, USB! :P
  delay(5000);
  readConfig();
  
  //lcdUpdateTimer.begin(UpdateLEDS, 40000);
  
  //Set up LEDs
  Serial.println("LED: Init... ");
  leds.begin();
  Serial.println("LED: Initialised!");
  
  #ifdef EQ_ENABLED
  eqInit();
  #endif
  
  #ifdef NRF_ENABLED
  if(!nrfInit()) {
    //Blinka error!!!
  }
  #endif
  
  Serial.println("All systems GO!");
}

void readConfig() {
  int address = 0;
  Serial.println("Reading EEPOROM config");
  config_data.mode = (uint8_t)EEPROM.read(address++);
  config_data.colorwheel.change_rate = ((uint8_t)EEPROM.read(address++)) << 8;
  config_data.colorwheel.change_rate |= (uint8_t)EEPROM.read(address++);
  config_data.single_color.red = (uint8_t)EEPROM.read(address++);
  config_data.single_color.green = (uint8_t)EEPROM.read(address++);
  config_data.single_color.blue = (uint8_t)EEPROM.read(address++);
  config_data.eq_thres = ((uint8_t)EEPROM.read(address++)) << 8;
  config_data.eq_thres |= (uint8_t)EEPROM.read(address++);
}

void writeConfig() {
  int address = 0;
  Serial.println("Writing EEPOROM config");
  EEPROM.write(address++, (byte)config_data.mode); 
  EEPROM.write(address++, (byte)(config_data.colorwheel.change_rate >> 8)); 
  EEPROM.write(address++, (byte)(config_data.colorwheel.change_rate & 0xFF)); 
  EEPROM.write(address++, (byte)config_data.single_color.red); 
  EEPROM.write(address++, (byte)config_data.single_color.green); 
  EEPROM.write(address++, (byte)config_data.single_color.blue);
  EEPROM.write(address++, (byte)(config_data.eq_thres >> 8)); 
  EEPROM.write(address++, (byte)(config_data.eq_thres & 0xFF));
}

void defaultConfig() {
  config_data.mode = 0;
  config_data.single_color.red = 0xAA;
  config_data.single_color.green = 0xAA;
  config_data.single_color.blue = 0xAA;
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

void UpdateLEDS() {
  //Serial.println("+");
  leds.show(); 
}

void loop() {
  uint8_t r, g, b;
  uint8_t data[8];
  
  #ifdef EQ_ENABLED
  Serial.println("EQ: Reading...");
  readMSGEQ7(audio_data);
  for(int i=0;i<7;i++) {
    Serial.print("B");
    Serial.print(i+1);
    Serial.print(": ");
    Serial.print(audio_data[i]);
    Serial.print(";"); 
  }
  Serial.println("");
  #endif
  
  #ifdef NRF_ENABLED

  //Serial.println("NRF: ");
  if(nrf24.available()) {
    data[0]=0;data[1]=0;data[2]=0;data[3]=0;data[4]=0;data[5]=0;data[6]=0;data[7]=0;
    Serial.println("Available!");
    uint8_t len = sizeof(data);
    if (!nrf24.recv((uint8_t*)&data, &len)) {
      Serial.println("read failed");
    }
    else {
      Serial.print("Bytes recieved: ");
      Serial.println(len);
      for(int i = 0; i < len; i++) {
        Serial.print(data[i], HEX);
        Serial.print("  ");
      }
      Serial.println("");
      HandleMess(data, len);
    }
  }
  #endif
  
  

  
  switch(config_data.mode){
    //Single color mode
    case 0:
      for(int j = 0; j< 185;j++) {
        leds.setPixel(j, config_data.single_color.red, config_data.single_color.green, config_data.single_color.blue);
      }
    case 1:  
      
      
      #ifndef NRF_ENABLED
      for(int i = 0; i < 360; i++) {
        for(int j = 0; j< 185;j++) {
          leds.setPixel(j, i, 100, 100);
        }
        delay(10);
      }
      #endif
      UpdateLEDS();
  }
}
  
// Function to read 7 band equalizers
void readMSGEQ7(int *audio_data) {
  int band;
  digitalWrite(PIN_EQ_RESET, HIGH);
  digitalWrite(PIN_EQ_RESET, LOW);
  for(band=0; band < 7; band++) {
    digitalWrite(PIN_EQ_STROBE,LOW);   // strobe pin - kicks the IC up to the next band 
    delayMicroseconds(30);             // Wait for MSGEQ to set output level
    audio_data[band] = analogRead(PIN_EQ_SIGNAL);  // store band reading
    digitalWrite(PIN_EQ_STROBE,HIGH); 
  }
}

// algorithm from: http://www.easyrgb.com/index.php?X=MATH&H=19#text19
int makeColor(unsigned int hue, unsigned int saturation, unsigned int lightness) {
  unsigned int red, green, blue;
  unsigned int var1, var2;
  
  if (hue > 359) 
    hue = hue % 360;
  if (saturation > 100) 
    saturation = 100;
  if (lightness > 100) 
    lightness = 100;
  if (saturation == 0) {
    red = green = blue = lightness * 255 / 100;
  } 
  else {
    if (lightness < 50) {
      var2 = lightness * (100 + saturation);
    } 
    else {
      var2 = ((lightness + saturation) * 100) - (saturation * lightness);
    }
    var1 = lightness * 200 - var2;
    red = h2rgb(var1, var2, (hue < 240) ? hue + 120 : hue - 240) * 255 / 600000;
    green = h2rgb(var1, var2, hue) * 255 / 600000;
    blue = h2rgb(var1, var2, (hue >= 120) ? hue - 120 : hue + 240) * 255 / 600000;
  }
  return (red << 16) | (green << 8) | blue;
}

unsigned int h2rgb(unsigned int v1, unsigned int v2, unsigned int hue) {
  if (hue < 60) 
    return v1 * 60 + (v2 - v1) * hue;
  if (hue < 180) 
    return v2 * 60;
  if (hue < 240) 
    return v1 * 60 + (v2 - v1) * (240 - hue);
  return v1 * 60;
}
