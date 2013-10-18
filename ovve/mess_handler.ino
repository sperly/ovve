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
#define MESS_SET_SPARK_SINGLE_INT        0x701
#define MESS_SET_SPARK_SINGLE_COLOR      0x702
#define MESS_SET_SPARK_SINGLE_RATE       0x703

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

void HandleMess(uint8_t* data, int data_len){
  Serial.println("Handeling...");
  Serial.println(data[0],HEX);
  Serial.println(MESS_TYPE_MASK,HEX);
  Serial.println(MESS_TYPE_SET);
  uint16_t messID = 0;
  uint16_t time = 0;
  if((data[0] & MESS_TYPE_MASK) == MESS_TYPE_SET) {
    //Set values
    messID = data[1] | ((data[0] & (~MESS_TYPE_MASK)) << 8);
    //messID = (data[0] & (!MESS_TYPE_MASK)) << 8;
    
    Serial.print("MessID: 0x");
    Serial.print(messID,HEX);
    Serial.println("");
    
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
        config_data.sparkle.rate = time;
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
        Serial.println("Garbage message");
        break;
    }
  }
  else if(data[0] & MESS_TYPE_MASK == MESS_TYPE_GET){
    //Return data to remote.
    
    
  }
}


