#define NRF_PACKET_SIZE    8

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
	
