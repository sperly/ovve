//defines
#define PIN_NRF_CS          9
#define PIN_NRF_CSE         10
#define PIN_SCAN_BUTTON     14
#define PIN_LCD_SI          0
#define PIN_LCD_CLK         3
#define PIN_LCD_RS          1
#define PIN_LCD_CSB         2
#define PIN_LCD_RESET       -1
#define PIN_LCD_BACK        6
#define PIN_ENC_A           22
#define PIN_ENC_B           23
#define PIN_ENC_BUTTON      21

#define NRF_PACKET_SIZE     8
#define LCD_CONTRAST        0x28

#define INIT_DELAY			200
#define DEBOUNCE_DELAY		10
#define NRF_ENABLED

typedef struct {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t clear;
}rgbc;

typedef struct{
  uint16_t change_rate;
  uint8_t step_size;
  uint8_t intensity;
}colorwheel_data;

typedef struct {
  uint8_t intensity;
  rgbc color;
  uint16_t rate;
  uint16_t change_rate;
}sparkle_data;

typedef struct {
		boolean leds;
        rgbc single_color;
        rgbc chamelion_color;
		uint8_t mode;
		uint8_t default_mode;
		sparkle_data sparkle;
        colorwheel_data colorwheel;
		uint16_t eq[EQ_BANDS];
        uint16_t eq_thres;
		uint8_t lcdBacklight;
		uint16_t lcdBacklightTime;
		uint8_t lcdContrast;
}data;
	
//Global variables
uint8_t gammatable[256];
long encPos = -999;
volatile data config_data;
elapsedMillis lastAction;
boolean lcdLight;
boolean MenuExc = false;
boolean MenuFwd = true;
boolean butPressed = false;

//Menu Variables
MenuSystem ms;
Menu mnuSet("");
Menu mnuMode("Mode Selection");
MenuItem mitModeColorWheel("Color Wheel");
MenuItem mitModeSparkle("Sparkle");
MenuItem mitModeBassTrigger("Bass trigger");
Menu mnuSettings("Settings");
MenuItem mitSettingsSave("Save Settings");
MenuItem mitSettingsDefault("Return to Default");

//Class instantiations
TCS34725_T3 tcs = TCS34725_T3(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_16X);
NRF24 nrf24(PIN_NRF_CS,PIN_NRF_CSE);
Bounce scanBounce = Bounce(PIN_SCAN_BUTTON, DEBOUNCE_DELAY);
Bounce butBounce = Bounce(PIN_ENC_BUTTON, DEBOUNCE_DELAY);
DogLcd lcd(PIN_LCD_SI, PIN_LCD_CLK, PIN_LCD_RS, PIN_LCD_CSB, PIN_LCD_RESET, PIN_LCD_BACK);
Encoder enc(PIN_ENC_A, PIN_ENC_B);
