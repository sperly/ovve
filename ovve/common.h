//Defines
#define STRIPS              6
#define LONGEST_STRIP       23
#define EQ_BANDS			7
#define PIN_EQ_STROBE       23
#define PIN_EQ_RESET        22
#define PIN_EQ_SIGNAL       A7
#define PIN_NRF_CS          9
#define PIN_NRF_CSE         10

#define NRF_PACKET_SIZE     8
#define NRF_DEFAULT_CHANNEL 2
#define NRF_SERVER_ID       "serv1"
#define NRF_REMOTE_ID       "clie1"

#define NRF_ENABLED
//#define EQ_ENABLED
//#define IMU_ENABLED

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
  uint8_t step_size;
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
}data;

//Global variables for EQ
int audio_data[EQ_BANDS]; // store band values in these arrays
volatile data config_data;
uint8_t current_mode;

IntervalTimer lcdUpdateTimer;

//Global variables for LEDs
const int ledsPerStrip = LONGEST_STRIP;
const int ledsInStrips[STRIPS] = {23,23,21,21,16,1};
//Strips start @ 23,46,69,
DMAMEM int displayMemory[ledsPerStrip*6];
int drawingMemory[LONGEST_STRIP*6];
const int config = WS2811_GRB | WS2811_800kHz;

//Instansiate classes
NRF24 nrf24(PIN_NRF_CS,PIN_NRF_CSE);
OctoWS2811 leds(LONGEST_STRIP, displayMemory, drawingMemory, config);

