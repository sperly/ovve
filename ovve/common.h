/*  Copyright (c) 2013 Eric Lind  
 *  Pulse code Copyrighted to Adafruit
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
 
#define MESS_TYPE_MASK                   0xF0
#define MESS_TYPE_SET                    0x90
#define MESS_TYPE_GET                    0xA0
#define MESS_TYPE_RESP_GET               0x40
#define MESS_TYPE_RESP_SET               0x20

#define MESS_VAL_TYPE_UINT8              0x01
#define MESS_VAL_TYPE_UINT16             0x02
#define MESS_VAL_TYPE_FLOAT              0x04
#define MESS_VAL_TYPE_CHAR               0x08
#define MESS_VAL_TYPE_UINT32             0x10
#define MESS_VAL_TYPE_LONG               0x20
#define MESS_VAL_TYPE_INT8               0x40

#define MESS_SET_LEDS                    0x001
#define MESS_SET_MODE                    0x002
#define MESS_SET_DEFAULT_MODE            0x003
#define MESS_SET_COL_IN_SINGLE           0x101
#define MESS_SET_CW_RATE                 0x201
#define MESS_SET_CW_STEPSIZE             0x202
#define MESS_SET_CW_INT                  0x203
#define MESS_SET_SPARK_CW_INT            0x601
#define MESS_SET_SPARK_CW_RATE           0x602
#define MESS_SET_SPARK_CW_COL_RATE       0x603
#define MESS_SET_SPARK_CW_STEPSIZE       0x604
#define MESS_SET_SPARK_CW_DECAY_RATE     0x605
#define MESS_SET_SPARK_SINGLE_INT        0x701
#define MESS_SET_SPARK_SINGLE_COLOR      0x702
#define MESS_SET_SPARK_SINGLE_RATE       0x703
#define MESS_SET_SPARK_SINGLE_DECAY_RATE 0x704
#define MESS_SET_SPARK_RAND_INT          0x801
#define MESS_SET_SPARK_RAND_RATE         0x802
#define MESS_SET_SPARK_RAND_DECAY_RATE   0x803

//Modes
#define MODE_DEFAULT                     0
#define MODE_SINGLE_COLOR                1
#define MODE_COLORWHEEL                  2
#define MODE_CHAMELION                   3
#define MODE_SOUND_COLORWHEEL            4
#define MODE_SOUND_EQSTYLE               5
#define MODE_SPARKLE_COLORWHEEL          6
#define MODE_SPARKLE_SINGLE_COLOR        7
#define MODE_SPARKLE_RANDOM              8
#define MODE_PULSE_SINGLE_COLOR          9
 
 //Defines
#define STRIPS                           5
#define LONGEST_STRIP                    23
#define EQ_BANDS                         7

#define PIN_EQ_STROBE                    23
#define PIN_EQ_RESET                     20
#define PIN_EQ_SIGNAL                    A7 //21
#define PIN_NRF_CS                       9
#define PIN_NRF_CSE                      10
#define PIN_BUTTON                       22
#define PIN_PULSE			 A3 //17

#define NRF_PACKET_SIZE                  8
#define NRF_DEFAULT_CHANNEL              2
#define NRF_SERVER_ID                    "serv1"
#define NRF_REMOTE_ID                    "clie1"

#define MODES                            10
#define CYCLE_TIME                       50
#define DEBOUNCE_DELAY                   10

#define NRF_ENABLED
//#define EQ_ENABLED
//#define IMU_ENABLED
#define PULSE_ENABLED

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
  uint16_t decay_rate;
}sparkle_data;

typedef struct {
	uint8_t intensity;
	uint8_t decay_time;
	rgbc color;
}pulse_data;

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
	pulse_data pulse;
}data;

typedef struct {
  uint32_t ledColor;
	uint8_t  ledHue;
  uint8_t  ledIntensity;
}led_data;

/*typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RgbColor;*/

typedef struct {
    uint32_t h;
    uint32_t s;
    uint32_t v;
} HsvColor;

//Global variables for EQ
int audio_data[EQ_BANDS]; // store band values in these arrays
volatile data config_data;
uint8_t current_mode;
led_data ledData[LONGEST_STRIP*STRIPS];
elapsedMillis loop_time1, loop_time2, loop_time3;
uint8_t last_color_step = 0;
uint8_t last_mode = 99;
uint8_t lastPulse = 0;

IntervalTimer lcdUpdateTimer;

//Global variables for LEDs
const int ledsPerStrip = LONGEST_STRIP;
const int ledsInStrips[STRIPS] = {23,23,21,21,16};
//Strips start @ 0,23,46,69,92

#ifdef PULSE_ENABLED
volatile int BPM;                   // used to hold the pulse rate
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // holds the time between beats, the Inter-Beat Interval
volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
volatile boolean QS = false;        // becomes true when Arduino finds a beat.
volatile int rate[10];                    // used to hold last ten IBI values
volatile unsigned long sampleCounter = 0;          // used to determine pulse timing
volatile unsigned long lastBeatTime = 0;           // used to find the inter beat interval
volatile int P =512;                      // used to find peak in pulse wave
volatile int T = 512;                     // used to find trough in pulse wave
volatile int thresh = 512;                // used to find instant moment of heart beat
volatile int amp = 100;                   // used to hold amplitude of pulse waveform
volatile boolean firstBeat = true;        // used to seed rate array so we startup with reasonable BPM
volatile boolean secondBeat = true;       // used to seed rate array so we startup with reasonable BPM
#endif


DMAMEM int displayMemory[ledsPerStrip*6];
int drawingMemory[LONGEST_STRIP*6];
const int led_config = WS2811_GRB | WS2811_800kHz;

//Instansiate classes
NRF24 nrf24(PIN_NRF_CS,PIN_NRF_CSE);
OctoWS2811 leds(LONGEST_STRIP, displayMemory, drawingMemory, led_config);
Bounce butBounce = Bounce(PIN_BUTTON, DEBOUNCE_DELAY);
