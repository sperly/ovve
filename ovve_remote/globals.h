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
#define PIN_ENC_A           99
#define PIN_ENC_B           99
#define PIN_ENC_BUTTON      99

#define NRF_PACKET_SIZE     8
#define LCD_CONTRAST        0x28

typedef struct {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t clear;
}rgbc;

typedef struct {
	uint16_t b1;
	uint16_t b2;
	uint16_t b3;
	uint16_t b4;
	uint16_t b5;
	uint16_t b6;
	uint16_t b7;
} eq_data;

typedef struct {
	uint8_t mode;
	rgbc color;
	uint16_t eq[7];
        uint16_t changetime;
        uint16_t eq_thres;
        uint8_t lcdBacklight;
        uint8_t lcdContrast;
        uint8_t lcdBacklightTime;
}data;
	
//Global variables
uint8_t gammatable[256];
long encPos = -999;
volatile data config_data;
long timeNow, timeLast;

//Menu Variables
MenuSystem ms;
Menu mnuSettings("");
Menu mnuMode("Mode Selection");
MenuItem mitModeColorWheel("Color Wheel");
MenuItem mitModeSparkle("Sparkle");
MenuItem mitModeBassTrigger("Bass trigger");

//Class instantiations
TCS34725 tcs = TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
NRF24 nrf24(PIN_NRF_CS,PIN_NRF_CSE);
Bounce scanBounce = Bounce(PIN_SCAN_BUTTON,10);
Bounce butBounce = Bounce(PIN_ENC_BUTTON,10);
DogLcd lcd(PIN_LCD_SI, PIN_LCD_CLK, PIN_LCD_RS, PIN_LCD_CSB, PIN_LCD_RESET, PIN_LCD_BACK);
Encoder Enc(PIN_ENC_A, PIN_ENC_B);
IntervalTimer CheckColorTimer;
