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

#define MESS_SET_MODE                    0x001
#define MESS_SET_LEDS                    0x002
#define MESS_SET_COL_IN_SINGLE           0x101
#define MESS_SET_CW_RATE                 0x201
#define MESS_SET_CW_STEPSIZE             0x202
#define MESS_SET_CW_INT                  0x203
#define MESS_SET_SPARK_CW_INT            0x601
#define MESS_SET_SPARK_CW_RATE           0x602
#define MESS_SET_SPARK_CW_COL_RATE       0x603
#define MESS_SET_SPARK_SINGLE_INT        0x701
#define MESS_SET_SPARK_SINGLE_RATE       0x702
#define MESS_SET_SPARK_SINGLE_COL_RATE   0x703

uint32_t bytes2udword(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3){
  uint32_t retval = 0;
  retval = ((uint32_t)byte0 << 24) || ((uint32_t)byte1 << 16) || ((uint32_t)byte2 << 8) || (uint32_t)byte3 ;
  return retval;
}

uint32_t bytes2uword(uint8_t byte0, uint8_t byte1){
  uint16_t retval = 0;
  retval = ((uint32_t)byte0 << 8) || (uint32_t)byte1;
  return retval;
}

void HandleMess(byte* data, int data_len){
  uint16_t messID = 0;
  uint32_t time = 0;
  if(data[0] & MESS_TYPE_MASK == MESS_TYPE_SET){
    //Set values
    messID = data[1];
    messID |= (data[0] & !MESS_TYPE_MASK) << 8;
    
    switch(messID){
      case MESS_SET_MODE:
        Serial.print("Setting mode to: ");
        Serial.println(data[2]);
        config_data.mode == data[2];
        break;
      case MESS_SET_COL_IN_SINGLE:
        Serial.print("Setting color to: ");
        Serial.print(data[2], HEX);
        Serial.print(data[3], HEX);
        Serial.println(data[4], HEX);
        config_data.single_color.red = data[2];
        config_data.single_color.green = data[3];
        config_data.single_color.blue = data[4];
        break;
      case MESS_SET_CW_RATE:
        time = bytes2udword(data[2], data[3], data[4], data[5]);
        Serial.print("Setting colorwheel cycletime to: ");
        Serial.println(time, DEC);
        config_data.colorwheel.change_rate = time;
        break;
      case MESS_SET_CW_STEPSIZE:
        Serial.print("Setting colorwheel color step to: ");
        Serial.println(data[2], DEC);
        config_data.colorwheel.step_size = data[2];
        break;
      case MESS_SET_CW_INT:
        Serial.print("Setting colorwheel color intensity to: ");
        Serial.println(data[2], DEC);
        config_data.colorwheel.intensity = data[2];
        break;
      case MESS_SET_SPARK_CW_INT:
        Serial.print("Setting sparkle colorwheel color intensity to: ");
        Serial.println(data[2], DEC);
        config_data.colorwheel.intensity = data[2];
        break;
      default:
        break;
    }
  }
  else if(data[0] & MESS_TYPE_MASK == MESS_TYPE_GET){
    //Return data to remote.
    
    
  }
}


