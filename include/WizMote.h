#ifndef WizMote_h
#define WizMote_h

#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>

#include <configuration.h>
#include <user_interface.h>
#include <espnow.h>

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



class WizMoteClass {

public:

    WizMoteClass();

    void begin();

    void initializeEspNow();

    uint8_t readButtonPress();

    void nextSequenceNumber();

    void powerOff();

    void setChannel(uint8_t ch);

    void registerSendCallback(esp_now_send_cb_t cb);

    void broadcast();


private:

    void printException(const char *message);

    bool initialized = false;

    uint32_t sequenceNumber;

    static uint8_t broadcastAddress[];
    message_structure_t broadcast_data;
};

#endif