//Defines
#define STRIPS              5
#define LONGEST_STRIP       24
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

typedef struct {
	uint8_t mode;
	rgbc color;
	uint16_t eq[7];
        uint16_t changetime;
        uint16_t eq_thres;
}data;

//Global variables for EQ
int audio_data[7]; // store band values in these arrays
volatile data config_data;

IntervalTimer lcdUpdateTimer;

//Global variables for LEDs
const int ledsPerStrip = 16;
const int ledsInStrips[5] = {20,20,24,24,16};
DMAMEM int displayMemory[ledsPerStrip*6];
int drawingMemory[ledsPerStrip*6];
const int config = WS2811_GRB | WS2811_800kHz;

//Instansiate classes
NRF24 nrf24(PIN_NRF_CS,PIN_NRF_CSE);
OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config);

