#include <i2c_t3.h>
#include <TCS34725.h>
#include <NRF24.h>
#include <SPI.h>
#include <Bounce.h>
#include <DogLcd.h>
#include <Encoder.h>
#include <EEPROM.h>
#include <MenuSystem.h>
#include "globals.h"

//Set LCD backlight PWM 
//    myDisplay.setBacklight(0,true);  //Off
//    myDisplay.setBacklight(128,true); //Medium on  

void MenuSetup(){
  mnuSettings.add_menu(&mnuMode);
  mnuMode.add_item(&mitModeColorWheel, &selModeColorWheel);
  mnuMode.add_item(&mitModeSparkle, &selModeSparkle);
  mnuMode.add_item(&mitModeBassTrigger, &selModeBassTrigger);
  ms.set_root_menu(&mnuSettings);
}

void selModeColorWheel(MenuItem* p_menu_item){
  
}
void selModeSparkle(MenuItem* p_menu_item){
  
}
void selModeBassTrigger(MenuItem* p_menu_item){
  
}
void selModeColorWheel(){
  
}


void setup() {
  Serial.begin(9600);

  readConfig();  
  MenuSetup();
  
  //Start 3-line, normal contrast, 3.3v
  lcd.begin(DOG_LCD_M163, LCD_CONTRAST, DOG_LCD_VCC_3V3);
  LightLCD();
  
  lcd.setContrast(config_data.lcdContrast);

  lcd.setCursor(0, 0);
  lcd.print("Init Ovve...");
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
  tcs.setGain(TCS34725_GAIN_16X);
  delay(100);
  lcd.setCursor(0, 1);
  lcd.print("Wireless...");
  if (nrf24.init())
  {
    lcd.setCursor(0, 2);
    lcd.print("Found!");
  }
  else {
    lcd.setCursor(0, 2);
    lcd.print("Failed! Halting!");
    Serial.println("NRF24 init failed");
  }
  delay(100);
  if (nrf24.setChannel(2))
  {
    lcd.setCursor(0, 2);
    lcd.print("Channel is 2");
  }
  else
  {
    lcd.setCursor(0, 2);
    lcd.print("Failed! Halting!");
    Serial.println("setChannel failed");
  }
  delay(100);
  if (nrf24.setThisAddress((uint8_t*)"clie1", 5))
  {
    lcd.setCursor(0, 2);
    lcd.print("Address is clie1");
  }
  else
  {
    lcd.setCursor(0, 2);
    lcd.print("Failed! Halting!");
    Serial.println("setThisAddress failed");
  }
  delay(100);
  if (nrf24.setPayloadSize(NRF_PACKET_SIZE*sizeof(uint8_t)))
  {
    lcd.setCursor(0, 2);
    lcd.print("payload: ");
    lcd.print(NRF_PACKET_SIZE,DEC);
    lcd.print("b");
  }
  else
  {
    lcd.setCursor(0, 2);
    lcd.print("Failed! Halting!");
    Serial.println("setPayloadSize failed");
  }
  delay(100);
  if (nrf24.setRF(NRF24::NRF24DataRate2Mbps, NRF24::NRF24TransmitPower0dBm))
  {
    lcd.setCursor(0, 2);
    lcd.print("2Mbps, full TXpow");
  }
  else
  {
    lcd.setCursor(0, 2);
    lcd.print("Failed! Halting!");
    Serial.println("setRF failed");    
  }
  delay(100);
  Serial.println("initialised");
  
  // thanks PhilB for this gamma table!
  // it helps convert RGB colors to what humans see
  for (int i=0; i<256; i++) {
    float x = i;
    x /= 255;
    x = pow(x, 2.5);
    x *= 255;
      
    gammatable[i] = (uint8_t)x;      
  }
  
  //Set up Timer for checking colorread every 10ms
  CheckColorTimer.begin(CheckColor, 10000);
  
  //Setup bouncer pins
  pinMode(PIN_SCAN_BUTTON,INPUT);
  pinMode(PIN_ENC_BUTTON,INPUT);
  
  lcd.setCursor(0, 0);
  lcd.print("***** OVVE *****");
  ClearLCD();
  timeLast = millis();
}

void readConfig(){
  int address = 0;
  config_data.mode = (uint8_t)EEPROM.read(address++);
  config_data.changetime = ((uint8_t)EEPROM.read(address++)) << 8;
  config_data.changetime |= (uint8_t)EEPROM.read(address++);
  config_data.color.red = (uint8_t)EEPROM.read(address++);
  config_data.color.green = (uint8_t)EEPROM.read(address++);
  config_data.color.blue = (uint8_t)EEPROM.read(address++);
  config_data.eq_thres = ((uint8_t)EEPROM.read(address++)) << 8;
  config_data.eq_thres |= (uint8_t)EEPROM.read(address++);
  config_data.lcdBacklight = (uint8_t)EEPROM.read(address++);
  config_data.lcdBacklightTime = (uint8_t)EEPROM.read(address++);
  config_data.lcdContrast = (uint8_t)EEPROM.read(address++);
}

void writeConfig(){
  int address = 0;
  EEPROM.write(address++, (byte)config_data.mode); 
  EEPROM.write(address++, (byte)(config_data.changetime >> 8)); 
  EEPROM.write(address++, (byte)(config_data.changetime & 0xFF)); 
  EEPROM.write(address++, (byte)config_data.color.red); 
  EEPROM.write(address++, (byte)config_data.color.green); 
  EEPROM.write(address++, (byte)config_data.color.blue);
  EEPROM.write(address++, (byte)(config_data.eq_thres >> 8)); 
  EEPROM.write(address++, (byte)(config_data.eq_thres & 0xFF));
  EEPROM.write(address++, (byte)config_data.lcdBacklight);
  EEPROM.write(address++, (byte)config_data.lcdBacklightTime);
  EEPROM.write(address++, (byte)config_data.lcdContrast);
}

void ClearLCD(){
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 2);
  lcd.print("                ");
}

void loop() {
  uint8_t col[3];
  boolean butPressed = false;
  
  if(millis() - timeLast > config_data.lcdBacklightTime)
    lcd.setBacklight(0, true);
  
  butBounce.update();
  if(butBounce.risingEdge()) butPressed = true;
  if(butPressed == true) {
    LightLCD();
  }
  //if(butBounce.fallingEdge()) butPressed = false;
  if((butBounce.duration() > 2000) && (butPressed == true))
  {
     ActivateMenu();
  }
}

void LightLCD()
{
  lcd.setBacklight(config_data.lcdBacklight, true);
  timeLast = millis(); 
}

void ActivateMenu() {
  
  
}

void CheckColor()
{
  uint8_t col[3];
  
  boolean scanBounceChanged = scanBounce.update ();
  if(scanBounce.read() && scanBounceChanged)
  {
    LightLCD();
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

    if (!nrf24.setTransmitAddress((uint8_t*)"serv1", 5))
      Serial.println("setTransmitAddress failed");
    if (!nrf24.send((uint8_t*)&col, (3*sizeof(uint8_t))))
        Serial.println("send failed");  
     if (!nrf24.waitPacketSent())
        Serial.println("waitPacketSent failed"); 
     Serial.println("Color Sent");
  }
}

void GetColor(uint8_t* color)
{
  uint16_t clear, red, green, blue;

  tcs.setInterrupt(false);      // turn on LED
  delay(60);  // takes 50ms to read 
  tcs.getRawData(&red, &green, &blue, &clear);
  tcs.setInterrupt(true);  // turn off LED
  
  // Figure out some basic hex code for visualization
  float r, g, b;
  r = red/84.1; 
  g = green/84.1;
  b = blue/84.1;

  Serial.print("\t");
  Serial.print((int)r, HEX); Serial.print((int)g, HEX); Serial.print((int)b, HEX);
  Serial.println();
  
  color[0] = gammatable[(uint8_t)r]; color[1] = gammatable[(uint8_t)g]; color[2] = gammatable[(uint8_t)b];
}

