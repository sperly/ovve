//#include <i2c_t3.h>
//#include <TCS34725.h>
#include <OctoWS2811.h>
#include <NRF24.h>
#include <SPI.h>

//Defines
#define STRIPS              5
#define LONGEST_STRIP       24
#define PIN_EQ_STROBE       23
#define PIN_EQ_RESET        22
#define PIN_EQ_SIGNAL       A7
#define PIN_NRF_CS          9
#define PIN_NRF_CSE         10

//Global variables for EQ
int audio_data[7]; // store band values in these arrays

//Global variables for LEDs
const int ledsPerStrip = 16;
const int ledsInStrips[5] = {20,20,24,24,16};
DMAMEM int displayMemory[ledsPerStrip*6];
int drawingMemory[ledsPerStrip*6];
const int config = WS2811_GRB | WS2811_800kHz;

//Instansiate classes
NRF24 nrf24(PIN_NRF_CS,PIN_NRF_CSE);
OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config);

void setup() {
  Serial.begin(9600);  //Anything goes, USB! :P
  
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

#define RED    0xFF0000
#define GREEN  0x00FF00
#define BLUE   0x0000FF
#define YELLOW 0xFFFF00
#define PINK   0xFF1088
#define ORANGE 0xE05800
#define WHITE  0xFFFFFF

void loop() {
  uint8_t r8, g8, b8;
  
  //Serial.println("EQ: Reading...");
  readMSGEQ7(audio_data);
  for(int i=0;i<7;i++)
  {
    //Serial.print("B");
    //Serial.print(i+1);
   //Serial.print(": ");
   Serial.print(audio_data[i]);
   Serial.print(";"); 
  }
  Serial.println("");
  
  //Serial.println("NRF: ");
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
//  int i = 0;  
//  Serial.print((int)r8, HEX); Serial.print((int)g8, HEX); Serial.print((int)b8, HEX);
//  Serial.println(" ");
//  for(i = 0; i< 21;i++)
//  {
//      leds.setPixel(i, gammatable[r8]*3, gammatable[g8]*3,gammatable[b8]*3);
//  }
  leds.show();
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
  for(band=0; band <7; band++)
  {
    digitalWrite(PIN_EQ_STROBE,LOW);   // strobe pin - kicks the IC up to the next band 
    delayMicroseconds(30);             // Wait for MSGEQ to set output level
    audio_data[band] = analogRead(A7);  // store band reading
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
