#include <i2c_t3.h>
#include "TCS34725_T3.h"
#include <NRF24.h>
#include <SPI.h>
#include <Bounce.h>
#include <DogLcd.h>

//defines
#define PIN_NRF_CS          9
#define PIN_NRF_CSE         10
#define PIN_SCAN_BUTTON     5
#define PIN_LCD_SI          0
#define PIN_LCD_CLK         3
#define PIN_LCD_RS          1
#define PIN_LCD_CSB         2
#define PIN_LCD_RESET       -1
#define PIN_LCD_BACK        4

//Global variables
uint8_t gammatable[256];

//Class instantiations
TCS34725_T3 tcs = TCS34725_T3(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
NRF24 nrf24(PIN_NRF_CS,PIN_NRF_CSE);
Bounce bouncer = Bounce(PIN_SCAN_BUTTON,5);
DogLcd lcd(PIN_LCD_SI, PIN_LCD_CLK, PIN_LCD_RS, PIN_LCD_CSB, PIN_LCD_RESET, PIN_LCD_BACK);

void setup() {
  Serial.begin(9600);
  
  //Start 3-line, normal contrast, 3.3v
  lcd.begin(DOG_LCD_M163, 0x28, DOG_LCD_VCC_3V3);
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
  tcs.setGain(TCS34725_GAIN_4X);
  
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
  if (nrf24.setPayloadSize(3*sizeof(uint8_t)))
  {
    lcd.setCursor(0, 2);
    lcd.print("blocksize 3bytes");
  }
  else
  {
    lcd.setCursor(0, 2);
    lcd.print("Failed! Halting!");
    Serial.println("setPayloadSize failed");
  }
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
  
  //Setup pins
  pinMode(PIN_SCAN_BUTTON,INPUT);
  
  
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  lcd.print("***** OVVE *****");
  lcd.setCursor(0, 2);
  lcd.print("                ");

}


void loop() {
  uint8_t col[3];
  
  bouncer.update ();
  int value = bouncer.read();
  if(value == HIGH)
  {
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
  uint32_t sum = red;
  sum += green;
  sum += blue;
  sum += clear;
  float r, g, b;
  r = red; r /= sum;
  g = green; g /= sum;
  b = blue; b /= sum;
  r *= 256; g *= 256; b *= 256;
  Serial.print("\t");
  Serial.print((int)r, HEX); Serial.print((int)g, HEX); Serial.print((int)b, HEX);
  Serial.println();
  
  color[0] = gammatable[(uint8_t)r]; color[1] = gammatable[(uint8_t)g]; color[2] = gammatable[(uint8_t)b];


}

