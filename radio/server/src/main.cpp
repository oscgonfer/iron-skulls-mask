#include "Arduino.h"
#include <SPI.h>
#include <RH_RF69.h>
#include <RHMesh.h>
// #include <Adafruit_NeoPixel.h>
#include "Animations.h"
#include "Extras.h"

// RADIO
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

#define SHOT_NUM 5

RH_RF69 rf69(RFM69_CS, RFM69_INT);
RHMesh rf69_manager(rf69, MASK1_ADDRESS);

int16_t packetnum = 0;  // packet counter, we increment per xmission
uint8_t checksum_ok[] = "CHK_OK";
uint8_t checksum_error[] = "CHK_ER";
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN]; // Dont put this on the stack:

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
    // rf69_manager.addRouteTo(CLIENT_ADDRESS, CLIENT_ADDRESS);
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

    // NeoPixelBus
    pixels.Begin();
    pixels.Show();
}

void loop() {
    // Update pwm pins sequence
    sequence.Update();

    if (animations.IsAnimating())
    {
        // the normal loop just needs these two to run the active animations
        animations.UpdateAnimations();
        pixels.Show();
    }

    // Wait for a message addressed to us from the main node
    uint8_t len = sizeof(buf);
    uint8_t from;

    if (rf69_manager.recvfromAck(buf, &len, &from)) {
        buf[len] = 0; // zero out remaining string
        // Serial.println();

        char packet[MAX_MESSAGE_LENGTH]{};
        // Serial.println("Packet ");
        uint16_t checksum = 0;
        uint16_t checksum_rcv = 0;

        // Two last ones are the checksum
        for (int i = 0; i < len-2; i++) {
            packet[i] = buf[i];
            // Serial.println(packet[i], HEX);
            checksum += (uint8_t) packet[i];
        }

        // Retrieve checksum from message
        checksum_rcv = ((buf[len-2]<<8) + (buf[len-1]));

        Serial.print("Message: ");
        Serial.println(packet);

        if (checksum != checksum_rcv){
            Serial.println("Checksum doesn't match, requesting again");
            // Request again
            if (rf69_manager.sendtoWait(checksum_error, sizeof(checksum_error), from) != RH_ROUTER_ERROR_NONE) {
                Serial.println("Sending failed (no ack)");
            }
        } else {
            // Serial.println("Got packet correctly, checksum matches");
            // Send a reply back to the originator node
            if (rf69_manager.sendtoWait(checksum_ok, sizeof(checksum_ok), from) != RH_ROUTER_ERROR_NONE){
                Serial.println("Sending failed (no ack)");
            }

            // Now perform animations
            char *ptr = NULL;
            char *animation[2];
            byte index = 0;

            // Tokenize input
            ptr = strtok(packet, "/");
            while (ptr != NULL) 
            {
                animation[index] = ptr;
                index++;
                ptr = strtok(NULL, "/");
            }

            AnimationRequest animationRequest;

            // TODO Group all these based on each char
            // ALL - LedStripItem = 0 
            if (strstr(animation[0], "AFF")) {
                // ALL FADE FAST
                animationRequest.Loop = false;
                animationRequest.FastTransition = true;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 0;
            } else if (strstr(animation[0], "AFS")) {
                // ALL FADE SLOW
                animationRequest.Loop = false;
                animationRequest.FastTransition = false;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 0;
            } else if (strstr(animation[0], "ALF")) {
                // ALL LOOP FAST
                animationRequest.Loop = true;
                animationRequest.FastTransition = true;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 0;
            } else if (strstr(animation[0], "ALS")) {
                // ALL LOOP SLOW
                animationRequest.Loop = true;
                animationRequest.FastTransition = false;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 0;
            // EYES - LedStripItem = 1
            } else if (strstr(animation[0], "EFF")){ 
                // EYES FADE FAST
                animationRequest.Loop = false;
                animationRequest.FastTransition = true;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 1;
            } else if (strstr(animation[0], "EFS")){
                // EYES FADE SLOW
                animationRequest.Loop = false;
                animationRequest.FastTransition = false;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 1;
            } else if (strstr(animation[0], "ELF")){ 
                // EYES LOOP FAST
                animationRequest.Loop = true;
                animationRequest.FastTransition = true;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 1;
            } else if (strstr(animation[0], "ELS")){ 
                // EYES LOOP SLOW
                animationRequest.Loop = true;
                animationRequest.FastTransition = false;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 1;
            // EARS - LedStripItem = 2
            } else if (strstr(animation[0], "OFF")){ 
                // EARS FADE FAST
                animationRequest.Loop = false;
                animationRequest.FastTransition = true;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 2;
            } else if (strstr(animation[0], "OFS")){
                // EARS FADE SLOW
                animationRequest.Loop = false;
                animationRequest.FastTransition = false;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 2;
            } else if (strstr(animation[0], "OLF")){ 
                // EARS LOOP FAST
                animationRequest.Loop = true;
                animationRequest.FastTransition = true;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 2;
            } else if (strstr(animation[0], "OLS")){ 
                // EARS LOOP SLOW
                animationRequest.Loop = true;
                animationRequest.FastTransition = false;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 2;
            // CHIN - LedStripItem = 3
            } else if (strstr(animation[0], "CFF")){ 
                // CHIN FADE FAST
                animationRequest.Loop = false;
                animationRequest.FastTransition = true;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 3;
            } else if (strstr(animation[0], "CFS")){
                // CHIN FADE SLOW
                animationRequest.Loop = false;
                animationRequest.FastTransition = false;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 3;
            } else if (strstr(animation[0], "CLF")){ 
                // CHIN LOOP FAST
                animationRequest.Loop = true;
                animationRequest.FastTransition = true;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 3;
            } else if (strstr(animation[0], "CLS")){ 
                // CHIN LOOP SLOW
                animationRequest.Loop = true;
                animationRequest.FastTransition = false;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 3;
            // JAW - LedStripItem = 4
            } else if (strstr(animation[0], "JFF")){ 
                // JAW FADE FAST
                animationRequest.Loop = false;
                animationRequest.FastTransition = true;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 4;
            } else if (strstr(animation[0], "JFS")){
                // JAW FADE SLOW
                animationRequest.Loop = false;
                animationRequest.FastTransition = false;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 4;
            } else if (strstr(animation[0], "JLF")){ 
                // JAW LOOP FAST
                animationRequest.Loop = true;
                animationRequest.FastTransition = true;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 4;
            } else if (strstr(animation[0], "JLS")){ 
                // JAW LOOP SLOW
                animationRequest.Loop = true;
                animationRequest.FastTransition = false;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 4;
            // MOUSTACHE - LedStripItem = 5
            } else if (strstr(animation[0], "MFF")){ 
                // MOUSTACHE FADE FAST
                animationRequest.Loop = false;
                animationRequest.FastTransition = true;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 5;
            } else if (strstr(animation[0], "MFS")){
                // MOUSTACHE FADE SLOW
                animationRequest.Loop = false;
                animationRequest.FastTransition = false;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 5;
            } else if (strstr(animation[0], "MLF")){ 
                // MOUSTACHE LOOP FAST
                animationRequest.Loop = true;
                animationRequest.FastTransition = true;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 5;
            } else if (strstr(animation[0], "MLS")){ 
                // MOUSTACHE LOOP SLOW
                animationRequest.Loop = true;
                animationRequest.FastTransition = false;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 5;
            // REAR - LedStripItem = 6
            } else if (strstr(animation[0], "RFF")){ 
                // REAR FADE FAST
                animationRequest.Loop = false;
                animationRequest.FastTransition = true;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 6;
            } else if (strstr(animation[0], "RFS")){
                // REAR FADE SLOW
                animationRequest.Loop = false;
                animationRequest.FastTransition = false;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 6;
            } else if (strstr(animation[0], "RLF")){ 
                // REAR LOOP FAST
                animationRequest.Loop = true;
                animationRequest.FastTransition = true;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 6;
            } else if (strstr(animation[0], "RLS")){ 
                // REAR LOOP SLOW
                animationRequest.Loop = true;
                animationRequest.FastTransition = false;
                animationRequest.IsLedStrip = true;
                animationRequest.LedStripItem = 6;
            // LASER
            } else if (strstr(animation[0], "LFF")) {
                // LASER FADE-TO FAST (PARAM INTENSITY)
                animationRequest.IsLedStrip = false;
                laser_fade_in_fast(atoi(animation[1]));
            } else if (strstr(animation[0], "LFS")) {
                // LASER FADE-TO SLOW (PARAM INTENSITY)
                animationRequest.IsLedStrip = false;
                laser_fade_in_slow(atoi(animation[1]));
            } else if (strstr(animation[0], "LRF")) {
                // LASER BREATH FOREVER (PARAM SPEED)
                animationRequest.IsLedStrip = false;
                laser_breath(atoi(animation[1]));
            } else if (strstr(animation[0], "LLF")) {
                // LASER BLINK FOREVER (PARAM SPEED)
                animationRequest.IsLedStrip = false;
                laser_blink(atoi(animation[1]));
            } else if (strstr(animation[0], "LRS")) {
                // LASER BREATH SHOT_NUM-SHOTs (PARAM SPEED)
                animationRequest.IsLedStrip = false;
                laser_breath(atoi(animation[1]), SHOT_NUM);
            } else if (strstr(animation[0], "LLS")) {
                // LASER BLINK SHOT_NUM-SHOTs (PARAM SPEED)
                animationRequest.IsLedStrip = false;
                laser_blink(atoi(animation[1]), SHOT_NUM);
            // FRONT
            } else if (strstr(animation[0], "FFF")) {
                // FRONT FADE-TO FAST (PARAM INTENSITY)
                animationRequest.IsLedStrip = false;
                front_fade_in_fast(atoi(animation[1]));
            } else if (strstr(animation[0], "FFS")) {
                // FRONT FADE-TO SLOW (PARAM INTENSITY)
                animationRequest.IsLedStrip = false;
                front_fade_in_slow(atoi(animation[1]));
            } else if (strstr(animation[0], "FRF")) {
                // FRONT BREATH FOREVER (PARAM SPEED)
                animationRequest.IsLedStrip = false;
                front_breath(atoi(animation[1]));
            } else if (strstr(animation[0], "FLF")) {
                // FRONT BLINK FOREVER (PARAM SPEED)
                animationRequest.IsLedStrip = false;
                front_blink(atoi(animation[1]));
            } else if (strstr(animation[0], "FRS")) {
                // FRONT BREATH SHOT_NUM-SHOTs (PARAM SPEED)
                animationRequest.IsLedStrip = false;
                front_breath(atoi(animation[1]), SHOT_NUM);
            } else if (strstr(animation[0], "FLS")) {
                // FRONT BLINK SHOT_NUM-SHOTs (PARAM SPEED)
                animationRequest.IsLedStrip = false;
                front_blink(atoi(animation[1]), SHOT_NUM);
            }

            // Handle with NeoPixelBus
            if (animationRequest.IsLedStrip) {
                animationRequest.LedStripColor = convert_rgb(animation[1]);
                AnimationTrigger(animationRequest);
            }

        }
    }

}
