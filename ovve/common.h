//Defines
#define STRIPS              5
#define LONGEST_STRIP       23
#define PIN_EQ_STROBE       23
#define PIN_EQ_RESET        22
#define PIN_EQ_SIGNAL       A7
#define PIN_NRF_CS          9
#define PIN_NRF_CSE         10

#define NRF_PACKET_SIZE     8

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

typedef struct{
  uint32_t change_rate;
  uint8_t step_size;
  uint8_t intensity;
}colorwheel_data;

typedef struct {
  uint8_t intensity;
  rgbc color;
  uint32_t rate;
  uint32_t change_rate;
}sparkle_data;

typedef struct {
        rgbc single_color;
        rgbc chamelion_color;
	uint8_t mode;
	sparkle_data sparkle;
        colorwheel_data colorwheel;
	uint16_t eq[7];
        uint16_t eq_thres;
}data;

//Global variables for EQ
int audio_data[7]; // store band values in these arrays
volatile data config_data;

IntervalTimer lcdUpdateTimer;

//Global variables for LEDs
const int ledsPerStrip = LONGEST_STRIP;
const int ledsInStrips[5] = {23,23,21,21,16};
DMAMEM int displayMemory[ledsPerStrip*6];
int drawingMemory[LONGEST_STRIP*6];
const int config = WS2811_GRB | WS2811_800kHz;

//Instansiate classes
NRF24 nrf24(PIN_NRF_CS,PIN_NRF_CSE);
OctoWS2811 leds(LONGEST_STRIP, displayMemory, drawingMemory, config);

