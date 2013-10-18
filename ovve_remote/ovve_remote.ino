/*  Copyright (c) 2013 Eric Lind  
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 */
 
#include <i2c_t3.h>
#include <TCS34725_T3.h>
#include <NRF24.h>
#include <SPI.h>
#include <Bounce.h>
#include <DogLcd.h>
#include <Encoder.h>
#include <EEPROM.h>
#include <MenuSystem.h>
#include "globals.h"

void tcsInit(){
  lcd.setCursor(0, 1);
  lcd.print("Color sensor...");
  if (tcs.begin()) {
    lcd.setCursor(0, 2);
    lcd.print("Found!");
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    lcd.setCursor(0, 2);
    lcd.print("Failed! Halting!");
    while (1); // halt!
  }
  tcs.setInterrupt(true);
  delay(100);
}

void lcdInit(){
  //Start 3-line, normal contrast, 3.3v
  if(lcd.begin(DOG_LCD_M163, config_data.lcdContrast, DOG_LCD_VCC_3V3) == 0){
    Serial.println("Found LCD");
  }
  LCDOn();
  
  lcd.noCursor();
  lcd.noBlink();
  
  lcd.setCursor(0, 0);
  lcd.print("Init Ovve...");
  
  lcd.setCursor(0, 1);
  lcd.print("LCD...");
  lcd.setCursor(0,2);
  lcd.print("Initialized!");
  delay(INIT_DELAY);
}

void setup() {
  Serial.begin(9600);
  delay(5000);
  defaultConfig();
  //writeConfig();
  //Read configuration from eeprom
  //readConfig();  
  
  //Initialize LCD first
  lcdInit();
  
  #ifdef TCS_ENABLED
  //If TCS is installed, initialize it
  tcsInit();
  #endif
  
  
  #ifdef NRF_ENABLED
  //If NRF is installed, initialize it.
  nrfInit();
  #endif
  
  //Initialize menus
  menuInit();
  
  Serial.println("Initialised");
  
  //Set up simple gamma correction table
  for (uint16_t i=0; i<256; i++) {
    float x = i;
    x /= 255;
    x = pow(x, 2.5);
    x *= 255;
    
    gammatable[i] = (uint8_t)x;      
  }
  
  //Setup bouncer pins
  pinMode(PIN_SCAN_BUTTON, INPUT);
  pinMode(PIN_ENC_BUTTON, INPUT);
  
  //Reset encoder
  enc.write(0);
  encPos = 0;
  
  LCDOn();
}

void readConfig(){
  int address = 0;
  Serial.println("Reading EEPOROM config");
  config_data.mode = (uint8_t)EEPROM.read(address++);
  config_data.single_color.red = (uint8_t)EEPROM.read(address++);
  config_data.single_color.green = (uint8_t)EEPROM.read(address++);
  config_data.single_color.blue = (uint8_t)EEPROM.read(address++);
  config_data.sparkle.intensity = (uint8_t)EEPROM.read(address++);
  config_data.sparkle.color.red = (uint8_t)EEPROM.read(address++);
  config_data.sparkle.color.green = (uint8_t)EEPROM.read(address++);
  config_data.sparkle.color.blue = (uint8_t)EEPROM.read(address++);
  config_data.sparkle.rate = ((uint8_t)EEPROM.read(address++)) << 8;
  config_data.sparkle.rate |= (uint8_t)EEPROM.read(address++);
  config_data.sparkle.change_rate = ((uint8_t)EEPROM.read(address++)) << 8;
  config_data.sparkle.change_rate |= (uint8_t)EEPROM.read(address++);
  config_data.colorwheel.intensity = (uint8_t)EEPROM.read(address++);
  config_data.colorwheel.change_rate = ((uint8_t)EEPROM.read(address++)) << 8;
  config_data.colorwheel.change_rate |= (uint8_t)EEPROM.read(address++);
  config_data.colorwheel.step_size = (uint8_t)EEPROM.read(address++);
  config_data.lcdBacklight = (uint8_t)EEPROM.read(address++);
  config_data.lcdContrast = (uint8_t)EEPROM.read(address++);
  config_data.lcdBacklightTime = ((uint8_t)EEPROM.read(address++)) << 8;
  config_data.lcdBacklightTime |= (uint8_t)EEPROM.read(address++);
}

void writeConfig(){
  int address = 0;
  Serial.println("Writing EEPOROM config");
  EEPROM.write(address++, (byte)config_data.mode);
  EEPROM.write(address++, (byte)config_data.single_color.red); 
  EEPROM.write(address++, (byte)config_data.single_color.green); 
  EEPROM.write(address++, (byte)config_data.single_color.blue); 
  EEPROM.write(address++, (byte)config_data.sparkle.intensity);
  EEPROM.write(address++, (byte)config_data.sparkle.color.red); 
  EEPROM.write(address++, (byte)config_data.sparkle.color.green); 
  EEPROM.write(address++, (byte)config_data.sparkle.color.blue); 
  EEPROM.write(address++, (byte)(config_data.sparkle.rate >> 8)); 
  EEPROM.write(address++, (byte)(config_data.sparkle.rate & 0xFF));
  EEPROM.write(address++, (byte)(config_data.sparkle.change_rate >> 8)); 
  EEPROM.write(address++, (byte)(config_data.sparkle.change_rate & 0xFF));
  EEPROM.write(address++, (byte)config_data.colorwheel.intensity);
  EEPROM.write(address++, (byte)(config_data.colorwheel.change_rate >> 8)); 
  EEPROM.write(address++, (byte)(config_data.colorwheel.change_rate & 0xFF));
  EEPROM.write(address++, (byte)config_data.colorwheel.step_size);
  EEPROM.write(address++, (byte)config_data.lcdBacklight);
  EEPROM.write(address++, (byte)(config_data.lcdBacklightTime >> 8)); 
  EEPROM.write(address++, (byte)(config_data.lcdBacklightTime & 0xFF));
  EEPROM.write(address++, (byte)config_data.lcdContrast);
}

void defaultConfig(){
  config_data.mode = 0;
  
  config_data.single_color.red = 0xAA;
  config_data.single_color.green = 0xAA;
  config_data.single_color.blue = 0xAA;
  
  config_data.sparkle.intensity = 0xFF;
  config_data.sparkle.color.red = 0xFF;
  config_data.sparkle.color.green = 0xFF;
  config_data.sparkle.color.blue = 0xFF;
  config_data.sparkle.rate = 10;
  config_data.sparkle.change_rate = 100;
  
  config_data.colorwheel.change_rate = 100;
  config_data.colorwheel.step_size = 1;
  config_data.colorwheel.intensity = 0xFF;
  
  config_data.lcdBacklight = 0xE0;
  config_data.lcdBacklightTime = 3000 ; 
  config_data.lcdContrast = 0x28;
}

void LCDOn(){
  lcdLight = true;
  lcd.setBacklight(config_data.lcdBacklight, true);
  ClearLCD();
  lastAction = 0;
}

void LCDOff(){
  lcdLight = false;
  lcd.setBacklight(0, true); 
}

void ClearLCD(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("***** OVVE *****");
}

void loop() {
  uint8_t col[3];
  
  if(lastAction > config_data.lcdBacklightTime && lcdLight == true){
    ClearLCD();
    LCDOff();
  }
  
  /*boolean butBounceChanged = butBounce.update();
  if(butBounce.read() && butBounceChanged) {
    butPressed = true;
    LCDOn();
    ms.select();
  }
  
  if((butBounce.duration() > LONG_PRESS) && (butPressed == true)){
    ActivateMenu();
    butPressed = false;
    ms.select();
  }
  
  long newPosition = enc.read();
  //Serial.println(newPosition);
  if(newPosition != encPos){
    Serial.println("GG");
    if(newPosition > encPos){
      for(int i = 0; i < (newPosition-encPos); i++){
        ms.next(); 
      }
    }
    else {
      for(int i = 0; i < (encPos-newPosition); i++){
        ms.prev(); 
      }
    }
    lcd.setCursor(0, 1);
    lcd.print(ms.get_current_menu()->get_selected()->get_name());
  }*/
  CheckColor();
}



void ActivateMenu() {
  lcd.setCursor(0, 1);
  lcd.print(ms.get_current_menu()->get_selected()->get_name());
  Serial.println("MMEEEENNNNNUUUU!!!!!");
}

boolean readingProgress = false;

void CheckColor(){
  if(readingProgress == true) return;
  uint8_t col[8] = {0,0,0,0,0,0,0,0};
  //uint8_t data[8] = {0,0,0,0,0,0,0,0};
  
  boolean scanBounceChanged = scanBounce.update ();
  if(scanBounce.read() && scanBounceChanged){
    readingProgress = true;
    Serial.println("Checking color!");
    LCDOn();
    lcd.setCursor(0, 1);
    lcd.print("Color reading: ");
    GetColor(col);
    lcd.setCursor(0, 2);
    lcd.print("                ");
    
    lcd.setCursor(0, 2);
    lcd.print("R:");
    lcd.setCursor(2, 2);
    lcd.print(col[0], HEX);
    lcd.setCursor(5, 2);
    lcd.print("G:");
    lcd.setCursor(7, 2);
    lcd.print(col[1], HEX);
    lcd.setCursor(11, 2);
    lcd.print("B:");
    lcd.setCursor(13, 2);
    lcd.print(col[2], HEX);
    
    delay(INIT_DELAY);
    
    //data[1] = col[0];
    //data[2] = col[1];
    //data[3] = col[2];
    
    //if (!nrf24.setTransmitAddress((uint8_t*)"serv1", 5))
    //  Serial.println("setTransmitAddress failed");
    //if (!nrf24.send((uint8_t*)&col, (3*sizeof(uint8_t))))
    //    Serial.println("send failed");  
    // if (!nrf24.waitPacketSent())
    //    Serial.println("waitPacketSent failed"); 
    // Serial.println("Color Sent");
    uint8_t buffer[8];// = {0,0,0,0,0,0,0,0};
    genPacket(0x101, col, 3, buffer);//MESS_SET_COL_IN_SINGLE, data, 3, buffer
      if(!sendData(buffer, sizeof(buffer))){
        ClearLCD();
        lcd.setCursor(0, 1);
        lcd.print("Error sending");
        lcd.setCursor(0, 2);
        lcd.print("color data!");
        Serial.println("Failed to send data");
      }
      readingProgress = false;
      lastAction = 0;
  }
}

void GetColor(uint8_t* color){
  uint16_t clear, red, green, blue;
  
  tcs.setInterrupt(false);      // turn on LED
  delay(100);  // takes 50ms to read 
  tcs.getRawData(&red, &green, &blue, &clear);
  tcs.setInterrupt(true);  // turn off LED
  
  Serial.println("Read data!");
  
  // Figure out some basic hex code for visualization
  float r, g, b;
  r = red/84.1; 
  g = green/84.1;
  b = blue/84.1;
  
  Serial.print("\t");
  Serial.print((int)r, HEX); Serial.print((int)g, HEX); Serial.print((int)b, HEX);
  Serial.println();
  
  color[0] = gammatable[(uint8_t)r]; 
  color[1] = gammatable[(uint8_t)g]; 
  color[2] = gammatable[(uint8_t)b];
}

