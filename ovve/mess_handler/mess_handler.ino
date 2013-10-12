#define MESS_TYPE_MASK         0xF0
#define MESS_TYPE_SET          0x90
#define MESS_TYPE_GET          0xA0
#define MESS_TYPE_RESP_GET     0x40
#dfeine MESS_TYPE_RESP_SET     0x20

#define MESS_VAL_TYPE_UINT8    0x01
#define MESS_VAL_TYPE_UINT16   0x02
#define MESS_VAL_TYPE_FLOAT        0x04
#define MESS_VAL_TYPE_CHAR         0x08
#define MESS_VAL_TYPE_UINT32       0x10
#define MESS_VAL_TYPE_LONG         0x20
#define MESS_VAL_TYPE_INT8         0x40

#define MESS_SET_MODE              0x001
#define MESS_SET_COLOR_IN_SINGLE   0x101
#define MESS_SET_CYCLE_TIME        0x201

void HandleMess(byte* data, int data_len){
  uint16_t messID = 0;
  if(data[0] & MESS_TYPE_MASK == MESS_TYPE_SET){
    //Set values
    messID = data[1];
    messID |= (data[0] & !MESS_TYPE_MASK) << 8;
    
    switch(messID){
      case MESS_SET_MODE:
        config_data.mode == data[2];
        break;
      default:
        break;
    }
  }
}
