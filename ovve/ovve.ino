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
 
//#include <i2c_t3.h>
#include <OctoWS2811.h>
#include <NRF24.h>
#include <SPI.h>
#include <EEPROM.h>
#include <Bounce.h>
#include "common.h"

boolean nrfInit() {
	//Set up wireless
	Serial.println("NRF24: Init...");
	if (!nrf24.init()) {
		Serial.println("NRF24: init failed");
		return false;
	}
	if (!nrf24.setChannel(NRF_DEFAULT_CHANNEL)) {
		Serial.println("NRF24: setChannel failed");
		return false;
	}
	if (!nrf24.setThisAddress((uint8_t*)"serv1", 5)) {
		Serial.println("NRF24: setThisAddress failed");
		return false;
	}
	if (!nrf24.setPayloadSize(NRF_PACKET_SIZE)) {
		Serial.println("NRF24: setPayloadSize failed");
		return false;
	}
	if (!nrf24.setRF(NRF24::NRF24DataRate2Mbps, NRF24::NRF24TransmitPower0dBm)) {
		Serial.println("NRF24: setRF failed");
		return false;
	}
	Serial.println("NRF24: Initialised!");
	return true;
}

void eqInit() {
	//Set up EQ pins
	Serial.print("EQ: Init... ");
	pinMode(PIN_EQ_RESET, OUTPUT); // reset
	pinMode(PIN_EQ_STROBE, OUTPUT); // strobe
	digitalWrite(PIN_EQ_RESET,LOW); // reset low
	digitalWrite(PIN_EQ_STROBE,HIGH); //pin 5 is RESET on the shield
	Serial.print("EQ: Initialised!");
}

void setup() {
	Serial.begin(9600);  //Anything goes, USB! :P
	delay(5000);
	readConfig();
	
	//Set up LEDs
	Serial.println("LED: Init... ");
	leds.begin();
	Serial.println("LED: Initialised!");
	
	#ifdef EQ_ENABLED
	eqInit();
	#endif
	
	#ifdef NRF_ENABLED
	if(!nrfInit()) {
		//Blinka error!!!
	}
	#endif
	
	Serial.println("All systems GO!");
	loop_time1 = 0;
	loop_time2 = 0;
	loop_time3 = 0;
	last_mode = config_data.default_mode;
	for(int i = 0; i < LONGEST_STRIP*STRIPS; i++) {
		ledData[i].ledColor = 0;
		ledData[i].ledIntensity = 0;
	}
	//Hitta en tom port...
	randomSeed(analogRead(A9));
}

void readConfig() {
	int address = 0;
	Serial.println("Reading EEPOROM config");
	config_data.mode = (uint8_t)EEPROM.read(address++);
	config_data.colorwheel.change_rate = ((uint8_t)EEPROM.read(address++)) << 8;
	config_data.colorwheel.change_rate |= (uint8_t)EEPROM.read(address++);
	config_data.single_color.red = (uint8_t)EEPROM.read(address++);
	config_data.single_color.green = (uint8_t)EEPROM.read(address++);
	config_data.single_color.blue = (uint8_t)EEPROM.read(address++);
	config_data.eq_thres = ((uint8_t)EEPROM.read(address++)) << 8;
	config_data.eq_thres |= (uint8_t)EEPROM.read(address++);
}

void writeConfig() {
	int address = 0;
	Serial.println("Writing EEPOROM config");
	EEPROM.write(address++, (byte)config_data.mode); 
	EEPROM.write(address++, (byte)(config_data.colorwheel.change_rate >> 8)); 
	EEPROM.write(address++, (byte)(config_data.colorwheel.change_rate & 0xFF)); 
	EEPROM.write(address++, (byte)config_data.single_color.red); 
	EEPROM.write(address++, (byte)config_data.single_color.green); 
	EEPROM.write(address++, (byte)config_data.single_color.blue);
	EEPROM.write(address++, (byte)(config_data.eq_thres >> 8)); 
	EEPROM.write(address++, (byte)(config_data.eq_thres & 0xFF));
}

void defaultConfig() {
	config_data.mode = 0;
	config_data.single_color.red = 0xAA;
	config_data.single_color.green = 0xAA;
	config_data.single_color.blue = 0xAA;
	config_data.eq_thres = 550;
	
	config_data.sparkle.intensity = 0xFF;
	config_data.sparkle.color.red = 0xFF;
	config_data.sparkle.color.green = 0xFF;
	config_data.sparkle.color.blue = 0xFF;
	config_data.sparkle.rate = 10;
	config_data.sparkle.change_rate = 100;
	
	config_data.colorwheel.change_rate = 100;
	config_data.colorwheel.step_size = 1;
	config_data.colorwheel.intensity = 0xFF;
	
	config_data.eq[7];
	config_data.eq_thres;
}

void UpdateLEDS() {
	//Serial.println("+");
	leds.show(); 
}

//Dirty trick!!!
boolean validLed(uint8_t led) {
	boolean valid = true;
	for(uint8_t i = 1; i <= STRIPS;i++) {
		uint8_t start_leds = ((i-1)*LONGEST_STRIP) + ledsInStrips[i-1];
		uint8_t stop_leds = i * LONGEST_STRIP;
		if(led >= start_leds && led < stop_leds)
			valid = false;
	}
	return valid;
}


void loop() {
	uint8_t r, g, b;
	uint8_t data[8] = {0,0,0,0,0,0,0,0};
	
	boolean butBounceChanged = butBounce.update ();
	if(butBounce.read() && butBounceChanged){
	  config_data.mode = (config_data.mode + 1) % MODES;
	}
	
	#ifdef PULSE_ENABLED
	Signal = analogRead(PIN_PULSE);              // read the Pulse Sensor 
	sampleCounter += 2;                         // keep track of the time in mS with this variable
	int N = sampleCounter - lastBeatTime;       // monitor the time since the last beat to avoid noise

	//find the peak and trough of the pulse wave
	if(Signal < thresh && N > (IBI/5)*3) {       // avoid dichrotic noise by waiting 3/5 of last IBI
		if (Signal < T){                        // T is the trough
			T = Signal;                         // keep track of lowest point in pulse wave 
		}
	}
		
	if(Signal > thresh && Signal > P){          // thresh condition helps avoid noise
		P = Signal;                             // P is the peak
	}                                        // keep track of highest point in pulse wave
		
	//  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
	// signal surges up in value every time there is a pulse
	if (N > 250) {                                   // avoid high frequency noise
		if ( (Signal > thresh) && (Pulse == false) && (N > (IBI/5)*3) ) {        
			Pulse = true; 			// set the Pulse flag when we think there is a pulse
			Serial.println("PULSE DETECTED!");
			//digitalWrite(blinkPin,HIGH);                // turn on pin 13 LED
			IBI = sampleCounter - lastBeatTime;         // measure time between beats in mS
			lastBeatTime = sampleCounter;               // keep track of time for next pulse

			if(firstBeat){                         // if it's the first time we found a beat, if firstBeat == TRUE
				firstBeat = false;                 // clear firstBeat flag
				return;                            // IBI value is unreliable so discard it
			}   
			if(secondBeat){                        // if this is the second beat, if secondBeat == TRUE
				secondBeat = false;                 // clear secondBeat flag
				for(int i=0; i<=9; i++){         // seed the running total to get a realisitic BPM at startup
					rate[i] = IBI;                      
				}
			}

			// keep a running total of the last 10 IBI values
			uint16_t runningTotal = 0;                   // clear the runningTotal variable    

			for(int i=0; i<=8; i++) {                // shift data in the rate array
				rate[i] = rate[i+1];              // and drop the oldest IBI value 
				runningTotal += rate[i];          // add up the 9 oldest IBI values
			}

			rate[9] = IBI;                          // add the latest IBI to the rate array
			runningTotal += rate[9];                // add the latest IBI to runningTotal
			runningTotal /= 10;                     // average the last 10 IBI values 
			BPM = 60000/runningTotal;               // how many beats can fit into a minute? that's BPM!
			QS = true;                              // set Quantified Self flag 
			// QS FLAG IS NOT CLEARED INSIDE THIS ISR
		}                       
	}

	if (Signal < thresh && Pulse == true){     // when the values are going down, the beat is over
		//digitalWrite(blinkPin,LOW);            // turn off pin 13 LED
		Pulse = false;                         // reset the Pulse flag so we can do it again
		amp = P - T;                           // get amplitude of the pulse wave
		thresh = amp/2 + T;                    // set thresh at 50% of the amplitude
		P = thresh;                            // reset these for next time
		T = thresh;
	}

	if (N > 2500){                             // if 2.5 seconds go by without a beat
		thresh = 512;                          // set thresh default
		P = 512;                               // set P default
		T = 512;                               // set T default
		lastBeatTime = sampleCounter;          // bring the lastBeatTime up to date        
		firstBeat = true;                      // set these to avoid noise
		secondBeat = true;                     // when we get the heartbeat back
	}
	#endif
	
	#ifdef EQ_ENABLED
	if(config_data.mode == MODE_SOUND_COLORWHEEL || config_data.mode == MODE_SOUND_EQTYLE) {
	  Serial.println("EQ: Reading...");
		readMSGEQ7(audio_data);
		for(int i=0;i<7;i++) {
			Serial.print("B");
			Serial.print(i+1);
			Serial.print(": ");
			Serial.print(audio_data[i]);
			Serial.print(";"); 
		}
		Serial.println("");
		
		//Handle data and set colors...
		
	}
	#endif
	
	#ifdef NRF_ENABLED
	if(nrf24.available()) {
	  //data = {0,0,0,0,0,0,0,0};
		//data[0]=0;data[1]=0;data[2]=0;data[3]=0;data[4]=0;data[5]=0;data[6]=0;data[7]=0;
		Serial.println("Available!");
		uint8_t len = sizeof(data);
		if (!nrf24.recv((uint8_t*)&data, &len)) {
			Serial.println("read failed");
		}
		else {
			Serial.print("Bytes recieved: ");
			Serial.println(len);
			for(int i = 0; i < len; i++) {
				Serial.print(" 0x");
				Serial.print(data[i], HEX);
			}
			Serial.println("");
			HandleMess(data, len);
		}
	}
	#endif

	HsvColor hsvColors;
	switch(config_data.mode){
	
		case 0:
			if(last_mode != 0) {
				for(int j = 0; j< LONGEST_STRIP*STRIPS; j++) {
					leds.setPixel(j, 0x11, 0x11, 0x11);
				}
				last_mode = 0;
			}
			break;
			
		case MODE_SINGLE_COLOR:
			for(int j = 0; j< LONGEST_STRIP*STRIPS; j++) {
				leds.setPixel(j, config_data.single_color.red, config_data.single_color.green, config_data.single_color.blue);
			}
			break;
			
		case MODE_COLORWHEEL:
			if(loop_time1 > config_data.colorwheel.change_rate) {
				uint8_t colorStep = 0;
				if(config_data.mode != last_mode) 
					last_color_step = 0;
				for(int j = 0; j< LONGEST_STRIP*STRIPS; j++) {
					if(validLed(j)) {
						leds.setPixel(j, makeColor(colorStep + last_color_step%360, 100, config_data.colorwheel.intensity));
						colorStep++;
					}
				}
				loop_time1 = loop_time1 - config_data.colorwheel.change_rate;
				last_color_step = (last_color_step + config_data.colorwheel.step_size) % 360;
			}
			
		case MODE_CHAMELION:
			for(int j = 0; j< LONGEST_STRIP*STRIPS; j++) {
				leds.setPixel(j, config_data.single_color.red, config_data.single_color.green, config_data.single_color.blue);
			}
			break;
			
		case MODE_SPARKLE_SINGLE_COLOR:
		  hsvColors = RgbToHsv(config_data.sparkle.color.red, config_data.sparkle.color.green, config_data.sparkle.color.blue);
			if(last_mode != config_data.mode) {
				for(int j = 0; j< LONGEST_STRIP*STRIPS; j++) {
					ledData[j].ledIntensity = 0;
					ledData[j].ledHue = hsvColors.h;
				}
			}
			if(loop_time2 > config_data.sparkle.decay_rate) {
				for(int j = 0; j< LONGEST_STRIP*STRIPS; j++) {
					ledData[j].ledIntensity <= 0 ? ledData[j].ledIntensity = 0 : ledData[j].ledIntensity--;
					//led_data[j] = led_data[j] - 1;
					leds.setPixel(j, makeColor(ledData[j].ledHue, 100, ledData[j].ledIntensity));
				}
				loop_time2 = loop_time2 - config_data.sparkle.decay_rate;
			}
			if(loop_time1 > config_data.sparkle.rate) {
				uint16_t  randLed = random(LONGEST_STRIP*STRIPS);
				leds.setPixel(randLed, makeColor(ledData[randLed].ledHue, 100, config_data.sparkle.intensity));
				ledData[randLed].ledIntensity = config_data.sparkle.intensity;
				loop_time1 = loop_time1 - config_data.sparkle.rate;
			}
			break;
			
		case MODE_SPARKLE_COLORWHEEL:
			if(last_mode != config_data.mode) {
				for(int j = 0; j< LONGEST_STRIP*STRIPS; j++) {
					ledData[j].ledIntensity = 0;
					ledData[j].ledHue = 0;
				}
			}
			if(loop_time2 > config_data.sparkle.decay_rate) {
				for(int j = 0; j< LONGEST_STRIP*STRIPS; j++) {
					ledData[j].ledIntensity <= 0 ? ledData[j].ledIntensity = 0 : ledData[j].ledIntensity--;
					//led_data[j] = led_data[j] - 1;
					leds.setPixel(j, makeColor(ledData[j].ledHue, 100, ledData[j].ledIntensity));
				}
				loop_time2 = loop_time2 - config_data.sparkle.decay_rate;
			}
			if(loop_time1 > config_data.sparkle.rate) {
				uint16_t  randLed = random(LONGEST_STRIP*STRIPS);
				leds.setPixel(randLed, makeColor(last_color_step, 100, config_data.sparkle.intensity));
				ledData[randLed].ledHue = last_color_step;
				ledData[randLed].ledIntensity = config_data.sparkle.intensity;
				last_color_step = (last_color_step + config_data.colorwheel.step_size) % 360;
				loop_time1 = loop_time1 - config_data.sparkle.rate;
			}
			break;
			
		case MODE_SPARKLE_RANDOM:
			if(loop_time2 > config_data.sparkle.decay_rate) {
				for(int j = 0; j< LONGEST_STRIP*STRIPS; j++) {
					ledData[j].ledIntensity <= 0 ? ledData[j].ledIntensity=0 : ledData[j].ledIntensity--;
					//led_data[j] = led_data[j] - 1;
					leds.setPixel(j, makeColor(ledData[j].ledHue, 100, ledData[j].ledIntensity));
				}
				loop_time2 = loop_time2 - config_data.sparkle.decay_rate;
			}
			if(loop_time1 > config_data.sparkle.rate) {
				uint16_t randColor = random(360);
				uint16_t  randLed = random(LONGEST_STRIP*STRIPS);				
				ledData[randLed].ledHue = randColor;
				ledData[randLed].ledIntensity = config_data.sparkle.intensity;
				leds.setPixel(randLed, makeColor(ledData[randLed].ledHue, 100, ledData[randLed].ledIntensity));
				loop_time1 = loop_time1 - config_data.sparkle.rate;
			}
			break;
			
		case MODE_PULSE_SINGLE_COLOR:
			if(last_mode != config_data.mode) {
				hsvColors = RgbToHsv(config_data.pulse.color.red, config_data.pulse.color.green, config_data.pulse.color.blue);
				for(int j = 0; j< LONGEST_STRIP*STRIPS; j++) {
					ledData[j].ledIntensity = 0;
					ledData[j].ledHue = hsvColors.h;
				}
			}
			if(loop_time1 > config_data.pulse.decay_time) {
				for(int j = 0; j< LONGEST_STRIP*STRIPS; j++) {
					ledData[j].ledIntensity <= 0 ? ledData[j].ledIntensity=0 : ledData[j].ledIntensity--;
					//led_data[j] = led_data[j] - 1;
					leds.setPixel(j, makeColor(ledData[j].ledHue, 100, ledData[j].ledIntensity));
				}
				loop_time2 = loop_time2 - config_data.sparkle.decay_rate;
			}
			if(QS == true && Pulse == true && lastPulse == false) {
				for(int j = 0; j< LONGEST_STRIP*STRIPS; j++) {
					ledData[j].ledIntensity = 50;
					leds.setPixel(j, makeColor(0, 100, 50));
				}
				lastPulse == true;	
			}
			else if(QS == true && Pulse == false && lastPulse == true) {
				lastPulse = false;
			}
			break;
		default:
			Serial.println("WRONG MODE!");
			break;
	}
	
	#ifndef NRF_ENABLED
	for(int i = 0; i < 360; i++) {
		for(int j = 0; j< 185;j++) {
			leds.setPixel(j, i, 100, 100);
		}
		delay(10);
	}
	#endif
	
	UpdateLEDS();
}
	
// Function to read 7 band equalizers
void readMSGEQ7(int *audio_data) {
	int band;
	digitalWrite(PIN_EQ_RESET, HIGH);
	digitalWrite(PIN_EQ_RESET, LOW);
	for(band=0; band < 7; band++) {
		digitalWrite(PIN_EQ_STROBE,LOW);   // strobe pin - kicks the IC up to the next band 
		delayMicroseconds(30);             // Wait for MSGEQ to set output level
		audio_data[band] = analogRead(PIN_EQ_SIGNAL);  // store band reading
		digitalWrite(PIN_EQ_STROBE,HIGH); 
	}
}

//uint32_t rgb2uint32(rgbc color){
//	
//
//}

// algorithm from: http://www.easyrgb.com/index.php?X=MATH&H=19#text19
int makeColor(unsigned int hue, unsigned int saturation, unsigned int lightness) {
	unsigned int red, green, blue;
	unsigned int var1, var2;
	
	if (hue > 359) 
		hue = hue % 360;
	if (saturation > 100) 
		saturation = 100;
	if (lightness > 100) 
		lightness = 100;
	if (saturation == 0) {
		red = green = blue = lightness * 255 / 100;
	} 
	else {
		if (lightness < 50) {
			var2 = lightness * (100 + saturation);
		} 
		else {
			var2 = ((lightness + saturation) * 100) - (saturation * lightness);
		}
		var1 = lightness * 200 - var2;
		red = h2rgb(var1, var2, (hue < 240) ? hue + 120 : hue - 240) * 255 / 600000;
		green = h2rgb(var1, var2, hue) * 255 / 600000;
		blue = h2rgb(var1, var2, (hue >= 120) ? hue - 120 : hue + 240) * 255 / 600000;
	}
	return (red << 16) | (green << 8) | blue;
}

unsigned int h2rgb(unsigned int v1, unsigned int v2, unsigned int hue) {
	if (hue < 60) 
		return v1 * 60 + (v2 - v1) * hue;
	if (hue < 180) 
		return v2 * 60;
	if (hue < 240) 
		return v1 * 60 + (v2 - v1) * (240 - hue);
	return v1 * 60;
}


//Algorithm from http://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb
/*RgbColor HsvToRgb(HsvColor hsv)
{
		RgbColor rgb;
		uint8_t region, remainder, p, q, t;

		if (hsv.s == 0)
		{
				rgb.r = hsv.v;
				rgb.g = hsv.v;
				rgb.b = hsv.v;
				return rgb;
		}

		region = hsv.h / 43;
		remainder = (hsv.h - (region * 43)) * 6; 

		p = (hsv.v * (255 - hsv.s)) >> 8;
		q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
		t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

		switch (region)
		{
				case 0:
						rgb.r = hsv.v; rgb.g = t; rgb.b = p;
						break;
				case 1:
						rgb.r = q; rgb.g = hsv.v; rgb.b = p;
						break;
				case 2:
						rgb.r = p; rgb.g = hsv.v; rgb.b = t;
						break;
				case 3:
						rgb.r = p; rgb.g = q; rgb.b = hsv.v;
						break;
				case 4:
						rgb.r = t; rgb.g = p; rgb.b = hsv.v;
						break;
				default:
						rgb.r = hsv.v; rgb.g = p; rgb.b = q;
						break;
		}

		return rgb;
}*/

HsvColor RgbToHsv(uint8_t red, uint8_t green, uint8_t blue){
		HsvColor hsv;
		uint8_t rgbMin, rgbMax;

		rgbMin = red < green ? (red < blue ? red : blue) : (green < blue ? green : blue);
		rgbMax = red > green ? (red > blue ? red : blue) : (green > blue ? green : blue);

		hsv.v = rgbMax;
		if (hsv.v == 0)
		{
				hsv.h = 0;
				hsv.s = 0;
				return hsv;
		}

		hsv.s = 255 * long(rgbMax - rgbMin) / hsv.v;
		if (hsv.s == 0)
		{
				hsv.h = 0;
				return hsv;
		}

		if (rgbMax == red)
				hsv.h = 0 + 43 * (green - blue) / (rgbMax - rgbMin);
		else if (rgbMax == green)
				hsv.h = 85 + 43 * (blue - red) / (rgbMax - rgbMin);
		else
				hsv.h = 171 + 43 * (red - green) / (rgbMax - rgbMin);

		return hsv;
}
