#include <Arduino.h>
#include <WizMote.h>

#include <configuration.h>

WizMoteClass WizMote;


void on_data_sent(uint8_t *mac_addr, uint8_t sendStatus) {
  WizMote.powerOff();
}

void setup() {
  // Initialize the WizMote
  WizMote.begin();

  // Read button press as soon as possible
  if (WizMote.readButtonPress()) {
    // Set sequence number
    WizMote.nextSequenceNumber();

    // Initialize ESP-NOW
    WizMote.initializeEspNow();

    // Register send callback
    WizMote.registerSendCallback(on_data_sent);

    // Broadcast message to ESP-NOW receivers
    WizMote.broadcast();
  } else {
    WizMote.powerOff();
  }
}

void loop() {}