/*  Copyright (c) 2013 Eric Lind  
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

boolean scan(){
  // Standard LINK echo message
  // If there is a Crazyflie present on a channel it will reply with an ACK and no payload
  
  uint8_t msg[] = { 0xBA, 0xDA, 0x55 };
  for (uint8_t channel = 0; channel < 125; channel++){
    nrf24.setChannel(channel);
    if (!nrf24.send(msg, sizeof(msg)))
      Serial.println("scan send failed");
    if (nrf24.waitPacketSent())
      return true; // Got an ACK, expect no payload. This channel works, use it
  }
  return false;
}

void nrfInit(){
  
  lcd.setCursor(0, 1);
  lcd.print("Wireless...");
  
  if (nrf24.init()){
    lcd.setCursor(0, 2);
    lcd.print("Found!");
  }
  else {
    lcd.setCursor(0, 2);
    lcd.print("Failed! Halting!");
    Serial.println("NRF24 init failed");
  }
  
  delay(INIT_DELAY);
  
  if (nrf24.setChannel(2)){
    lcd.setCursor(0, 2);
    lcd.print("Channel is 2");
  }
  else{
    lcd.setCursor(0, 2);
    lcd.print("Failed! Halting!");
    Serial.println("setChannel failed");
  }
  
  delay(INIT_DELAY);
  
  if (nrf24.setThisAddress((uint8_t*)"clie1", 5)){
    lcd.setCursor(0, 2);
    lcd.print("Address is clie1");
  }
  else{
    lcd.setCursor(0, 2);
    lcd.print("Failed! Halting!");
    Serial.println("setThisAddress failed");
  }
  
  delay(INIT_DELAY);
  
  if (nrf24.setPayloadSize(NRF_PACKET_SIZE)){
    lcd.setCursor(0, 2);
    lcd.print("payload: ");
    lcd.print(NRF_PACKET_SIZE,DEC);
    lcd.print("b");
  }
  else{
    lcd.setCursor(0, 2);
    lcd.print("Failed! Halting!");
    Serial.println("setPayloadSize failed");
  }
  
  delay(INIT_DELAY);
  
  if (nrf24.setRF(NRF24::NRF24DataRate2Mbps, NRF24::NRF24TransmitPower0dBm)){
    lcd.setCursor(0, 2);
    lcd.print("2Mbps, full TXpow");
  }
  else{
    lcd.setCursor(0, 2);
    lcd.print("Failed! Halting!");
    Serial.println("setRF failed");    
  }
  delay(INIT_DELAY);
}

boolean sendData(uint8_t* data, uint8_t len){
  if (!nrf24.setTransmitAddress((uint8_t*)"serv1", 5)){
    Serial.println("setTransmitAddress failed");
  return false;
  }
  if (!nrf24.send(data, len)){
    Serial.println("send failed");  
    return false;
  }
  if (!nrf24.waitPacketSent()){
    Serial.println("waitPacketSent failed"); 
    return false;
  }
  Serial.println("Packet sent!");
  return true;
}

boolean recvData(byte* buf, uint16_t len, uint16_t timeout=10){
  
  
}
