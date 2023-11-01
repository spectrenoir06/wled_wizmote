#include <Arduino.h>
#include <WizMote.h>

#include <configuration.h>


// This is kind of an esoteric strucure because it's pulled from the "Wizmote"
// product spec. That remote is used as the baseline for behavior and availability
// since it's broadly commercially available and works out of the box as a drop-in
typedef struct WizMoteMessageStructure {
  uint8_t program;  // 0x91 for ON button, 0x81 for all others
  uint8_t seq[4];   // Incremetal sequence number 32 bit unsigned integer LSB first
  uint8_t byte5;    // Unknown (seen 0x20)
  uint8_t button;   // Identifies which button is being pressed
  uint8_t byte8;    // Unknown, but always 0x01
  uint8_t byte9;    // Unnkown, but always 0x64

  uint8_t byte10;   // Unknown, maybe checksum
  uint8_t byte11;   // Unknown, maybe checksum
  uint8_t byte12;   // Unknown, maybe checksum
  uint8_t byte13;   // Unknown, maybe checksum
} message_structure_t;

message_structure_t broadcast_data;

WizMoteClass WizMote;

uint8_t ch = 1;
uint8_t repeat = 10;

void on_data_sent(uint8_t *mac_addr, uint8_t sendStatus) {
  delay(0.1);
  ch++;
  if(ch <= 14) {  
    WizMote.setChannel(ch);
    WizMote.broadcast((uint8_t *) &broadcast_data, sizeof(message_structure_t));
  }
  else {
    if(repeat > 0) {
      repeat--;
      ch = 1;
      WizMote.setChannel(ch);
      WizMote.broadcast((uint8_t *) &broadcast_data, sizeof(message_structure_t));
    }
    else {
      WizMote.powerOff();
    }
  }

}

void setup() {

  // Initialize Serial communication
  Serial.begin(BAUD_RATE);

  // Initialize the WizMote
  WizMote.begin();

  broadcast_data.program = 0x81;
  broadcast_data.byte5 = 0x20;
  broadcast_data.byte8 = 0x01;
  broadcast_data.byte9 = 0x64;

  // Read button press as soon as possible
  broadcast_data.button = WizMote.readButtonPress();

  if(broadcast_data.button == 2) {
    broadcast_data.button = 1;
    broadcast_data.program = 0x91;
  }
  else if(broadcast_data.button == 1) broadcast_data.button = 2;
  else if(broadcast_data.button == 0) broadcast_data.button = 3;
  else if(broadcast_data.button == 4) broadcast_data.button = 17;
  else if(broadcast_data.button == 8) broadcast_data.button = 16;
  else if(broadcast_data.button == 16) broadcast_data.button = 19;
  else if(broadcast_data.button == 32) broadcast_data.button = 18;
  else if(broadcast_data.button == 64) broadcast_data.button = 9;
  else if(broadcast_data.button == 128) broadcast_data.button = 8;

  // Set sequence number
  uint32_t seq = WizMote.nextSequenceNumber();
  memcpy(broadcast_data.seq, &seq, sizeof(seq));

  // Initialize ESP-NOW
  WizMote.initializeEspNow();
  WizMote.setChannel(ch);

  // Register send callback
  WizMote.registerSendCallback(on_data_sent);

  // Broadcast message to ESP-NOW receivers
  WizMote.broadcast((uint8_t *) &broadcast_data, sizeof(message_structure_t));
}

void loop() {}