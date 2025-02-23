#include <WizMote.h>

uint8_t WizMoteClass::broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

WizMoteClass::WizMoteClass() {}

void WizMoteClass::begin() {
    // Set the voltage regulator pin as an output pin
    pinMode(VOLTAGE_REGULATOR_PIN, OUTPUT);

    // Enable the voltage regulator, so the remote stays awake
    digitalWrite(VOLTAGE_REGULATOR_PIN, HIGH);

    // Initialize I2C Bus
    Wire.begin(SDA_PIN, SCL_PIN);
}

void WizMoteClass::initializeEspNow() {

    // Set device as a Wi-Fi Station
    if (WiFi.mode(WIFI_STA) != true) {
        printException("setting Wi-Fi mode failed");
    }

    // Immediately disconnect from any networks
    if (WiFi.disconnect() != true) {
        printException("disconnecting Wi-Fi failed");
    }

    // Initialize ESP-NOW
    if (esp_now_init() != OK) {
        printException("initializing ESP-NOW failed");    
    }

    // Set this device's role to CONTROLLER
    if (esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER) != OK) {
        printException("setting ESP-NOW role failed");
    };

}

void WizMoteClass::setChannel(uint8_t ch)
{
    esp_now_del_peer(WizMoteClass::broadcastAddress);
    wifi_set_channel(ch);
    esp_now_add_peer(WizMoteClass::broadcastAddress, ESP_NOW_ROLE_SLAVE, ch, NULL, 0);
}


uint8_t WizMoteClass::readButtonPress() {
    // Select input port 0
    Wire.beginTransmission(PCA6416A_I2C_ADDR);
    Wire.write(PCA6416A_INPORT0);
    Wire.endTransmission();

    // Request one byte and read input port 0 value
    if (Wire.requestFrom(PCA6416A_I2C_ADDR, 2) > 1) {
        broadcast_data.program = 0x81;
        broadcast_data.byte5 = 0x20;
        broadcast_data.byte8 = 0x01;
        broadcast_data.byte9 = 0x64;

        uint8_t d1 = ~Wire.read();
        uint8_t d2 = ~Wire.read();

        broadcast_data.byte12 = d1;
        broadcast_data.byte13 = d2;
        if(d1 == 2) {
            broadcast_data.button = 1;
            broadcast_data.program = 0x91;
        }
        else if(d1 == 1) broadcast_data.button = 2;
        else if(d1 == 4) broadcast_data.button = 17;
        else if(d1 == 8) broadcast_data.button = 16;
        else if(d1 == 16) broadcast_data.button = 19;
        else if(d1 == 32) broadcast_data.button = 18;
        else if(d1 == 64) broadcast_data.button = 9;
        else if(d1 == 128) broadcast_data.button = 8;
        else if(d2 == 255) broadcast_data.button = 3;
        else return 0;
        return 1;
    }
    return 0;
}

void WizMoteClass::nextSequenceNumber() {

    // Initialize EEPROM
    EEPROM.begin(EEPROM_SIZE);

    // Read sequence number from EEPROM
    EEPROM.get(EEPROM_SEQUENCE_OFFSET, sequenceNumber);

    // Increment sequence number
    sequenceNumber++;

    // Write back sequence number into EEPROM
    EEPROM.put(EEPROM_SEQUENCE_OFFSET, sequenceNumber);
    EEPROM.commit();

    memcpy(broadcast_data.seq, &sequenceNumber, sizeof(sequenceNumber));
}

void WizMoteClass::broadcast() {
    if (esp_now_send(WizMoteClass::broadcastAddress, (uint8_t *) &broadcast_data, sizeof(message_structure_t)) != OK) {
        printException("sending ESP-NOW message failed");
    }
}

void WizMoteClass::powerOff() {

    // Disable the voltage regulator, so the remote turns off
    digitalWrite(VOLTAGE_REGULATOR_PIN, LOW);
}

void WizMoteClass::registerSendCallback(esp_now_send_cb_t cb) {
    if (esp_now_register_send_cb(cb) != OK) {
        printException("registering ESP-NOW send callback failed");
    }
}

void WizMoteClass::printException(const char* message) {
    Serial.println();
    Serial.println();
    Serial.println("========================");
    Serial.println("  An unexpected error occured.");
    Serial.printf("  message: %s\n", message);
    Serial.println("========================");
    Serial.println();
    Serial.println("System will restart in 5 seconds...");

    delay(5000);
    system_restart();
}