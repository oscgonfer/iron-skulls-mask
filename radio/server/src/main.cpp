// rf69 demo tx rx.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, reliable messaging client
// with the RH_RF69 class. RH_RF69 class does not provide for addressing or
// reliability, so you should only use RH_RF69  if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf69_server.
// Demonstrates the use of AES encryption, setting the frequency and modem
// configuration

#include "Arduino.h"
#include <SPI.h>
#include <RH_RF69.h>
#include <RHReliableDatagram.h>
#include <jled.h>
#include <Adafruit_NeoPixel.h>

// LASER and other things (pwm controlled)
#define LASER_PIN       13
#define LIGHT_PIN_1     6
#define LIGHT_PIN_2     9

JLed pwm_pins[1] = {JLed(LASER_PIN).Off()};
JLedSequence sequence(JLedSequence::eMode::PARALLEL, pwm_pins);


// NEOPIXEL
#define NEOPIXEL_PIN    11
#define NUMPIXELS       30
#define DELAYVAL        10

Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

//--------------------------------------------
// Simple laser and neopixel  animations for testing purposes
void laser_blink(int _wait) {
  pwm_pins[0].Blink(_wait, _wait).Repeat(3); // Blink
  // pwm_pins[0].Breathe(_wait).Repeat(3); // Breathe
}

void laser_on(int _brightness) {
  pwm_pins[0].Set(_brightness);
}

void laser_off() {
  pwm_pins[0].Off();
}

void neopixels_green(){
  pixels.clear();
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 150, 0));
    pixels.show();
    delay(DELAYVAL);
  }
}

void neopixels_red(){
  pixels.clear();
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(150, 0, 0));
    pixels.show();
    delay(DELAYVAL);
  }
}

void neopixels_off(){
  pixels.clear();
  pixels.show();
}
//----------End of test animations---------

//---RADIO---
#define RF69_FREQ     433.0
#define MY_ADDRESS    2
#define RFM69_CS      8

#if defined (__AVR_ATmega32U4__) // Feather 32u4 w/Radio
#define RFM69_INT     7
#else
#define RFM69_INT     3
#endif

#define RFM69_RST     4

#define MAX_MESSAGE_LENGTH 20

//Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram rf69_manager(rf69, MY_ADDRESS);
int16_t packetnum = 0;  // packet counter, we increment per xmission

void setup(){
  Serial.begin(115200);
  // while (!Serial) { delay(1); } // wait until serial console is open, remove if not tethered to computer

  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  Serial.println("Feather Addressed RFM69 RX Test!");
  Serial.println();

  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);

  if (!rf69_manager.init()) {
    Serial.println("RFM69 radio init failed");
    while (1);
  }
  Serial.println("RFM69 radio init OK!");
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(RF69_FREQ)) {
    Serial.println("setFrequency failed");
  }

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);

  Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");

  // Start neopixels
  pixels.begin();

  laser_blink(20);

  Serial.println(RH_RF69_MAX_MESSAGE_LEN);
}

// Dont put this on the stack:
uint8_t checksum_ok[] = "CHK_OK";
uint8_t checksum_error[] = "CHK_ERROR";
// Dont put this on the stack:
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];

void loop() {
  if (rf69_manager.available())
  {
    // Wait for a message addressed to us from the main node
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (rf69_manager.recvfromAck(buf, &len, &from)) {
        buf[len] = 0; // zero out remaining string
        Serial.println();

        // Serial.print("Got packet from #"); Serial.print(from);
        // Serial.print(" [RSSI :");
        // Serial.print(rf69.lastRssi());
        // Serial.print("]: ");
        // Serial.println((char*)buf);
        // Serial.print("Message length: ");
        // Serial.println(len);

        char packet[MAX_MESSAGE_LENGTH]{};
        // Serial.println("Packet ");
        uint16_t checksum = 0;
        uint16_t checksum_sent = 0;

        // Two last ones are the checksum
        for (int i = 0; i < len-2; i++) {
            packet[i] = buf[i];
            // Serial.println(packet[i], HEX);
            checksum += (uint8_t) packet[i];
        }

        // Retrieve checksum from message
        checksum_sent = ((buf[len-2]<<8) + (buf[len-1]));

        Serial.print("Message: ");
        Serial.println(packet);

        // Serial.print("Checksum: ");
        // Serial.print(checksum, HEX);

        // Serial.print(" // Checksum sent: ");
        // Serial.println(checksum_sent, HEX);

        if (checksum != checksum_sent){
            Serial.println("Checksum doesn't match, requesting again");
            // Request again
            if (!rf69_manager.sendtoWait(checksum_error, sizeof(checksum_error), from)) {
                Serial.println("Sending failed (no ack)");
            }
        } else {
            Serial.println("Got packet correctly, checksum matches");
            // Send a reply back to the originator node
            if (!rf69_manager.sendtoWait(checksum_ok, sizeof(checksum_ok), from)){
                Serial.println("Sending failed (no ack)");
            }
            // TODO Make this clean
            // Now perform animations
            if (strstr(packet, "/LED/GREEN")) {
                neopixels_green();
            } else if (strstr(packet, "/LED/RED")) {
                neopixels_red();
            } else if (strstr(packet, "/LED/OFF")) {
                neopixels_off();
            }
        }
    }
  }
  // Update pwm pins sequence
  sequence.Update();
}
