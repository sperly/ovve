

uint32_t bytes2udword(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3){
  uint32_t retval = 0;
  retval = ((uint32_t)byte0 << 24) | ((uint32_t)byte1 << 16) | ((uint32_t)byte2 << 8) | (uint32_t)byte3 ;
  return retval;
}

uint32_t bytes2uword(uint8_t byte0, uint8_t byte1){
  uint16_t retval = 0;
  retval = ((uint32_t)byte0 << 8) | (uint32_t)byte1;
  return retval;
}

void reciveMess(byte* data, int data_len){
  uint16_t messID = 0;
  uint16_t time = 0;
  if(data[0] & MESS_TYPE_MASK == MESS_TYPE_SET){
    //Set values
    messID = data[1];
    messID |= (data[0] & !MESS_TYPE_MASK) << 8;
    
    switch(messID){
      // 0x001
      case MESS_SET_LEDS:
	Serial.print("Turning Leds: ");
	data[2] == 0 ? Serial.println("On") : Serial.println("Off");
	data[2] == 0 ? config_data.leds = true : config_data.leds = false;
	break;
	//0x002
      case MESS_SET_MODE:
	Serial.print("Setting mode to: ");
	Serial.println(data[2]);
	config_data.mode = data[2];
	break;
	//0x004
      case MESS_SET_DEFAULT_MODE:
	Serial.print("Setting mode to: ");
	Serial.println(data[2]);
	config_data.default_mode = data[2];
	break;
	//0x101
      case MESS_SET_COL_IN_SINGLE:
	Serial.print("Setting color to: ");
	Serial.print(data[2], HEX);
	Serial.print(data[3], HEX);
	Serial.println(data[4], HEX);
	config_data.single_color.red = data[2];
	config_data.single_color.green = data[3];
	config_data.single_color.blue = data[4];
	break;
	//0x201
      case MESS_SET_CW_RATE:
	time = bytes2uword(data[2], data[3]);
	Serial.print("Setting colorwheel cycletime to: ");
	Serial.println(time, DEC);
	config_data.colorwheel.change_rate = time;
	break;
	//0x202
      case MESS_SET_CW_STEPSIZE:
	Serial.print("Setting colorwheel color step to: ");
	Serial.println(data[2], DEC);
	config_data.colorwheel.step_size = data[2];
	break;
	//0x203
      case MESS_SET_CW_INT:
	Serial.print("Setting colorwheel color intensity to: ");
	Serial.println(data[2], DEC);
	config_data.colorwheel.intensity = data[2];
	break;
	//0x601
      case MESS_SET_SPARK_CW_INT:
	Serial.print("Setting sparkle colorwheel color intensity to: ");
	Serial.println(data[2], DEC);
	config_data.sparkle.intensity = data[2];
	break;
	//0x602
      case MESS_SET_SPARK_CW_RATE:
	time = bytes2uword(data[2], data[3]);
	Serial.print("Setting sparkle colorwheel sparkle rate to: ");
	Serial.println(time, DEC);
	config_data.sparkle.change_rate = time;
	break;
	//0x603
      case MESS_SET_SPARK_CW_COL_RATE:
	time = bytes2uword(data[2], data[3]);
	Serial.print("Setting sparkle colorwheel change rate to: ");
	Serial.println(time, DEC);
	config_data.sparkle.change_rate = time;
	break;
	//0x604
      case MESS_SET_SPARK_CW_STEPSIZE:
	Serial.print("Setting sparkle colorwheel color step to: ");
	Serial.println(data[2], DEC);
	config_data.sparkle.step_size = data[2];
	break;
	//0x702
      case MESS_SET_SPARK_SINGLE_COLOR:
	Serial.print("Setting color to: ");
	Serial.print(data[2], HEX);
	Serial.print(data[3], HEX);
	Serial.println(data[4], HEX);
	config_data.single_color.red = data[2];
	config_data.single_color.green = data[3];
	config_data.single_color.blue = data[4];
	break;
	//0x701
      case MESS_SET_SPARK_SINGLE_INT:
	Serial.print("Setting colorwheel color intensity to: ");
	Serial.println(data[2], DEC);
	config_data.sparkle.intensity = data[2];
	break;
	//0x703
      case MESS_SET_SPARK_SINGLE_RATE:
	time = bytes2uword(data[2], data[3]);
	Serial.print("Setting sparkle rate to: ");
	Serial.println(time, DEC);
	config_data.sparkle.rate = time;
	break;
      default:
	break;
    }
  }
  else if(data[0] & MESS_TYPE_MASK == MESS_TYPE_GET){
    //Return data to remote.
    
    
  }
}

void genPacket(uint16_t id, uint8_t* data, uint8_t len, uint8_t* tx_buffer){
  tx_buffer[0] = MESS_TYPE_SET | ((id & 0xf00) >> 8);
  tx_buffer[1] = (id & 0x0ff);
  for(int i = 0; i < len; i++) {
    tx_buffer[2+i] = data[i];
  }  
}

boolean SendMess(char* hostId, byte* data, uint16_t len) {
  if (!nrf24.setTransmitAddress((uint8_t*)hostId, 5))
    Serial.println("setTransmitAddress failed");
  if (!nrf24.send((uint8_t*)&data, (len)))
    Serial.println("send failed");  
  if (!nrf24.waitPacketSent())
    Serial.println("waitPacketSent failed"); 
  Serial.println("Color Sent");
  //#ifdef NRF_LOW_POWER
  //nrf24.powerDown();
  //Serial.println("NRF power down");
  //#endif
}

