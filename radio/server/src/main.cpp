#include "Arduino.h"
#include <SPI.h>
#include <RH_RF69.h>
#include <RHMesh.h>
#include <jled.h>
#include <Adafruit_NeoPixel.h>

// JLED
#define LASER_PIN       10
#define LIGHT_PIN       11

// TODO Understand why without an initial animation, timing gets wrong and sequence is never updated
JLed pwm_pins[] = {JLed(LASER_PIN).Breathe(2000).Forever(), JLed(LIGHT_PIN).Off()};
JLedSequence sequence(JLedSequence::eMode::PARALLEL, pwm_pins);

// NEOPIXEL
#define NEOPIXEL_PIN    9
#define NUMPIXELS       30
#define DELAYVAL        10

Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

//RADIO
#define CLIENT_ADDRESS      1
#define REPEATER1_ADDRESS   2
#define REPEATER2_ADDRESS   3
#define BASE1_ADDRESS       4
#define BASE2_ADDRESS       5
#define BASE3_ADDRESS       6
#define BASE4_ADDRESS       7
// MASKS WILL START AT 10 TO HAVE ROOM FOR MORE REPEATERS/BASES
#define MASK1_ADDRESS       10
#define MASK2_ADDRESS       11
#define MASK3_ADDRESS       12
#define MASK4_ADDRESS       13

#define RF69_FREQ     433.0
#define RFM69_CS      8
#define RFM69_RST     4

#if defined (__AVR_ATmega32U4__) // Feather 32u4 w/Radio
#define RFM69_INT     7
#else
#define RFM69_INT     3
#endif

#define MAX_MESSAGE_LENGTH 20

RH_RF69 rf69(RFM69_CS, RFM69_INT);
RHMesh rf69_manager(rf69, MASK1_ADDRESS);

int16_t packetnum = 0;  // packet counter, we increment per xmission
uint8_t checksum_ok[] = "CHK_OK";
uint8_t checksum_error[] = "CHK_ERROR";
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN]; // Dont put this on the stack:

//--------------------------------------------
// Simple laser and neopixel animations for testing purposes
//--------------------------------------------

void laser_blink(int _wait, int repeats) {
  if (repeats) pwm_pins[0].Blink(_wait, _wait).Repeat(repeats);
  else pwm_pins[0].Blink(_wait, _wait).Forever();
}

void laser_breath(int _wait, int repeats) {
  if (repeats) pwm_pins[0].Breathe(_wait).Repeat(repeats);
  else pwm_pins[0].Breathe(_wait).Forever();
}

void laser_on(int _brightness) {
  pwm_pins[0].Set(_brightness);
}

void laser_off() {
  pwm_pins[0].Off();
}

void light_blink(int _wait, int repeats) {
  if (repeats) pwm_pins[1].Blink(_wait, _wait).Repeat(repeats);
  else pwm_pins[1].Blink(_wait, _wait).Forever();
}

void light_breath(int _wait, int repeats) {
  if (repeats) pwm_pins[1].Breathe(_wait).Repeat(repeats);
  else pwm_pins[1].Breathe(_wait).Forever();
}

void light_on(int _brightness) {
  pwm_pins[1].Set(_brightness);
}

void light_off() {
  pwm_pins[1].Off();
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
//--------------------------------------------
//------------End of test animations----------
//--------------------------------------------

void setup(){
    Serial.begin(115200);
    //while (!Serial) { delay(1); } // wait until serial console is open, remove if not tethered to computer

    pinMode(RFM69_RST, OUTPUT);
    digitalWrite(RFM69_RST, LOW);

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

    // Manually define the routes for this network
    rf69_manager.addRouteTo(CLIENT_ADDRESS, CLIENT_ADDRESS);
    rf69_manager.addRouteTo(REPEATER1_ADDRESS, REPEATER1_ADDRESS);
    rf69_manager.addRouteTo(REPEATER2_ADDRESS, REPEATER2_ADDRESS);

    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
    // No encryption
    if (!rf69.setFrequency(RF69_FREQ)) {
        Serial.println("setFrequency failed");
    }

    // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
    // ishighpowermodule flag set like this:
    rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

    // The encryption key has to be the same on both ends
    uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    rf69.setEncryptionKey(key);

    // Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");

    // Start neopixels
    pixels.begin();

    // laser_blink(200, 5);
    // pinMode(LASER_PIN, OUTPUT);
    // pinMode(LIGHT_PIN, OUTPUT);
}

void loop() {
    // Update pwm pins sequence
    sequence.Update();

    // Wait for a message addressed to us from the main node
    uint8_t len = sizeof(buf);
    uint8_t from;

    if (rf69_manager.recvfromAck(buf, &len, &from)) {
        buf[len] = 0; // zero out remaining string
        Serial.println();

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
            if (rf69_manager.sendtoWait(checksum_error, sizeof(checksum_error), from) != RH_ROUTER_ERROR_NONE) {
                Serial.println("Sending failed (no ack)");
            }
        } else {
            Serial.println("Got packet correctly, checksum matches");
            // Send a reply back to the originator node
            if (rf69_manager.sendtoWait(checksum_ok, sizeof(checksum_ok), from) != RH_ROUTER_ERROR_NONE){
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
            } else if (strstr(packet, "/LAS/ON")){
                laser_on(255);
                // digitalWrite(LASER_PIN, HIGH);
            } else if (strstr(packet, "/LAS/OFF")){
                laser_off();
                // digitalWrite(LASER_PIN, LOW);
            } else if (strstr(packet, "/LAS/BLI")){
                laser_blink(30, 0);
            } else if (strstr(packet, "/L1G/ON")){
                light_on(255);
            } else if (strstr(packet, "/L1G/OFF")){
                light_off();
            } else if (strstr(packet, "/L1G/BLI")){
                light_blink(30,0);
            }
        }
    }

}
