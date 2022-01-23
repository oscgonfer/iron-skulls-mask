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
#define LASER         13
#define LIGHT_PIN_1    6
#define LIGHT_PIN_2    9
JLed pwm_pins[3] = {JLed(LASER).Off(),
                    JLed(LIGHT_PIN_1).Off(),
                    JLed(LIGHT_PIN_2).Off()
                    };

JLedSequence sequence(JLedSequence::eMode::PARALLEL, pwm_pins);

// NEOPIXEL
#define NEOPIXEL_PIN     11
#define NUMPIXELS        16
#define DELAYVAL         10

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

void neopixels_animation(){
  pixels.clear(); // Set all pixel colors to 'off'

  // The first NeoPixel in a strand is #0, second is 1, all the way up
  // to the count of pixels minus one.
  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...

    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    // Here we're using a moderately bright green color:
    pixels.setPixelColor(i, pixels.Color(0, 150, 0));

    pixels.show();   // Send the updated pixel colors to the hardware.

    delay(DELAYVAL); // Pause before next pass through loop
  }
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

//Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram rf69_manager(rf69, MY_ADDRESS);
int16_t packetnum = 0;  // packet counter, we increment per xmission

void setup(){
  Serial.begin(115200);
  while (!Serial) { delay(1); } // wait until serial console is open, remove if not tethered to computer

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
}

// Dont put this on the stack:
uint8_t data[] = "Message received";
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

      Serial.print("Got packet from #"); Serial.print(from);
      Serial.print(" [RSSI :");
      Serial.print(rf69.lastRssi());
      Serial.print("] : ");
      Serial.println((char*)buf);

      laser_blink(300);
      neopixels_animation();

      // Send a reply back to the originator node
      if (!rf69_manager.sendtoWait(data, sizeof(data), from))
        Serial.println("Sending failed (no ack)");
    }
  }
  // Update pwm pins sequence
  sequence.Update();
}
