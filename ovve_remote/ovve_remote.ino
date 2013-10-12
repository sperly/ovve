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

//Set LCD backlight PWM 
//    myDisplay.setBacklight(0,true);  //Off
//    myDisplay.setBacklight(128,true); //Medium on  

void MenuSetup(){
  mnuSet.add_menu(&mnuMode);
  mnuMode.add_item(&mitModeColorWheel, &selModeColorWheel);
  mnuMode.add_item(&mitModeSparkle, &selModeSparkle);
  mnuMode.add_item(&mitModeBassTrigger, &selModeBassTrigger);
  mnuSet.add_menu(&mnuSettings);
  //Menu mnuSettings("Settings");
  mnuSettings.add_item(&mitSettingsSave, &selSettingsSave);
  mnuSettings.add_item(&mitSettingsDefault, &selSettingsDefault);
  ms.set_root_menu(&mnuSet);
}

void selModeColorWheel(MenuItem* p_menu_item){
  lcd.setCursor(0, 2);
  lcd.print("0 - Colorwheel");
  config_data.mode = 0;
  MenuExc = true;
  MenuFwd = false;
  lastAction = 0;
}
void selModeSparkle(MenuItem* p_menu_item){
  MenuExc = true;
  MenuFwd = false;
}
void selModeBassTrigger(MenuItem* p_menu_item){
  MenuExc = true;
  MenuFwd = false;
}

void selSettingsSave(MenuItem* p_menu_item){
  writeConfig();
  lcd.setCursor(0, 2);
  lcd.print("Config written!");
  MenuExc = true;
  MenuFwd = false;
  lastAction = 0;
}

void selSettingsDefault(MenuItem* p_menu_item){
  defaultConfig();
  lcd.setCursor(0, 2);
  lcd.print("Default set!");
  MenuExc = true;
  MenuFwd = false;;
}


void setup() {
  Serial.begin(9600);

  readConfig();  
  MenuSetup();
  
  //Start 3-line, normal contrast, 3.3v
  lcd.begin(DOG_LCD_M163, LCD_CONTRAST, DOG_LCD_VCC_3V3);
  LCDOn();
  
  lcd.setContrast(config_data.lcdContrast);
  lcd.noCursor();

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
  delay(100);
  tcs.setInterrupt(true);

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
  
  //Setup bouncer pins
  pinMode(PIN_SCAN_BUTTON,INPUT);
  pinMode(PIN_ENC_BUTTON,INPUT);
  Enc.write(0);
  encPos = 0;
  LCDOn();
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
  config_data.lcdBacklight = (uint8_t)EEPROM.read(address++);
  config_data.lcdBacklightTime = ((uint8_t)EEPROM.read(address++)) << 8;
  config_data.lcdBacklightTime |= (uint8_t)EEPROM.read(address++);
  config_data.lcdContrast = (uint8_t)EEPROM.read(address++);
  
  /*config_data.mode = 0;
  config_data.changetime = 100;
  config_data.color.red = 0xAA;
  config_data.color.green = 0xAA;
  config_data.color.blue = 0xAA;
  config_data.eq_thres = 550;
  config_data.lcdBacklight = 0x80;
  config_data.lcdBacklightTime = 3000;
  config_data.lcdContrast = 0x28;
  writeConfig();*/
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
  EEPROM.write(address++, (byte)config_data.lcdBacklight);
  EEPROM.write(address++, (byte)(config_data.lcdBacklightTime >> 8)); 
  EEPROM.write(address++, (byte)(config_data.lcdBacklightTime & 0xFF));
  EEPROM.write(address++, (byte)config_data.lcdContrast);
}

void defaultConfig()
{
  config_data.mode = 0;
  config_data.changetime = 100;
  config_data.color.red = 0xAA;
  config_data.color.green = 0xAA;
  config_data.color.blue = 0xAA;
  config_data.eq_thres = 550;
  config_data.lcdBacklight = 0x80;
  config_data.lcdBacklightTime = 3000;
  config_data.lcdContrast = 0x28;
}

void LCDOn()
{
  lcdLight = true;
  lcd.setBacklight(config_data.lcdBacklight, true);
  ClearLCD();
  lastAction = 0;
}

void LCDOff()
{
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

  boolean butBounceChanged = butBounce.update();
  if(butBounce.read() && butBounceChanged) 
  {
    butPressed = true;
    LCDOn();
  }
  
  if((butBounce.duration() > 2000) && (butPressed == true))
  {
     ActivateMenu();
  }
  
  long newPosition = Enc.read();
  
  if(newPosition != encPos){
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
  }
  CheckColor();
}



void ActivateMenu() {
  lcd.setCursor(0, 1);
  lcd.print(ms.get_current_menu()->get_selected()->get_name());
  Serial.println("MMEEEENNNNNUUUU!!!!!");
}

boolean readingProgress = false;

void CheckColor()
{
  if(readingProgress == true) return;
  uint8_t col[3];
  
  boolean scanBounceChanged = scanBounce.update ();
  if(scanBounce.read() && scanBounceChanged)
  {
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

    //if (!nrf24.setTransmitAddress((uint8_t*)"serv1", 5))
    //  Serial.println("setTransmitAddress failed");
    //if (!nrf24.send((uint8_t*)&col, (3*sizeof(uint8_t))))
    //    Serial.println("send failed");  
    // if (!nrf24.waitPacketSent())
    //    Serial.println("waitPacketSent failed"); 
    // Serial.println("Color Sent");
    readingProgress = false;
    lastAction = 0;
  }
}

void GetColor(uint8_t* color)
{
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
  
  color[0] = gammatable[(uint8_t)r]; color[1] = gammatable[(uint8_t)g]; color[2] = gammatable[(uint8_t)b];
}

