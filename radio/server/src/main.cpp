#include "Arduino.h"
#include <SPI.h>
#include <RH_RF69.h>
#include <RHMesh.h>
#include <jled.h>
#include <Adafruit_NeoPixel.h>

#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

// JLED
#define LASER_PIN       10
#define LIGHT_PIN       11
#define ANIMAT_CH    1

// TODO Understand why without an initial animation, timing gets wrong and sequence is never updated
JLed pwm_pins[] = {JLed(LASER_PIN).Breathe(2000).Forever(), JLed(LIGHT_PIN).Off()};
JLedSequence sequence(JLedSequence::eMode::PARALLEL, pwm_pins);

// NEOPIXEL
#define NEOPIXEL_PIN    9
#define NUMPIXELS       30
#define DELAYVAL        10
#define FADESLOW        3000
#define FADEFAST        500

// Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// NeoPixelBus
boolean fadeToColor = true;
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> pixels(NUMPIXELS, NEOPIXEL_PIN);
NeoPixelAnimator animations(ANIMAT_CH); // NeoPixel animation management object

struct MyAnimationState
{
    RgbColor StartingColor;
    RgbColor EndingColor;
};

// one entry per pixel to match the animation timing manager
MyAnimationState animationState[ANIMAT_CH];

// LED RANGES
uint8_t eye_left_0 =  5;
uint8_t eye_left_1 =  8;
uint8_t eye_left_2 =  10;
uint8_t eye_left_3 =  13;

uint8_t eye_right_0 =  15;
uint8_t eye_right_1 =  18;
uint8_t eye_right_2 =  20;
uint8_t eye_right_3 =  24;

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

RH_RF69 rf69(RFM69_CS, RFM69_INT);
RHMesh rf69_manager(rf69, MASK1_ADDRESS);

int16_t packetnum = 0;  // packet counter, we increment per xmission
uint8_t checksum_ok[] = "CHK_OK";
uint8_t checksum_error[] = "CHK_ERROR";
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN]; // Dont put this on the stack:

//--------------------------------------------
// Simple laser and neopixel animations for testing purposes
//--------------------------------------------

void laser_blink(int _wait, int repeats = 0) {
  if (repeats) pwm_pins[0].Blink(_wait, _wait).Repeat(repeats);
  else pwm_pins[0].Blink(_wait, _wait).Forever();
}

void laser_breath(int _wait, int repeats = 0) {
  if (repeats) pwm_pins[0].Breathe(_wait).Repeat(repeats);
  else pwm_pins[0].Breathe(_wait).Forever();
}

void laser_static(int _brightness) {
  pwm_pins[0].Set(_brightness);
}

void front_blink(int _wait, int repeats = 0) {
  if (repeats) pwm_pins[1].Blink(_wait, _wait).Repeat(repeats);
  else pwm_pins[1].Blink(_wait, _wait).Forever();
}

void front_breath(int _wait, int repeats = 0) {
  if (repeats) pwm_pins[1].Breathe(_wait).Repeat(repeats);
  else pwm_pins[1].Breathe(_wait).Forever();
}

void front_static(int _brightness) {
  pwm_pins[1].Set(_brightness);
}

// simple blend function
void BlendAnimUpdate(const AnimationParam& param)
{
    // this gets called for each animation on every time step
    // progress will start at 0.0 and end at 1.0
    // we use the blend function on the RgbColor to mix
    // color based on the progress given to us in the animation
    RgbColor updatedColor = RgbColor::LinearBlend(
        animationState[param.index].StartingColor,
        animationState[param.index].EndingColor,
        param.progress);

    // apply the color to the strip
    for (uint16_t pixel = 0; pixel < NUMPIXELS; pixel++)
    {
        pixels.SetPixelColor(pixel, updatedColor);
    }
}

void EyeAnimUpdate(const AnimationParam& param)
{
    // this gets called for each animation on every time step
    // progress will start at 0.0 and end at 1.0
    // we use the blend function on the RgbColor to mix
    // color based on the progress given to us in the animation
    RgbColor updatedColor = RgbColor::LinearBlend(
        animationState[param.index].StartingColor,
        animationState[param.index].EndingColor,
        param.progress);

    // apply the color to the strip
    for(int i=eye_left_0; i<eye_left_1; i++) {
        pixels.SetPixelColor(i, updatedColor);
    }

    for(int i=eye_left_2; i<eye_left_3; i++) {
        pixels.SetPixelColor(i, updatedColor);
    }

    for(int i=eye_right_0; i<eye_right_1; i++) {
        pixels.SetPixelColor(i, updatedColor);
    }

    for(int i=eye_right_2; i<eye_right_3; i++) {
        pixels.SetPixelColor(i, updatedColor);
    }  
}

// TODO Make this as a handler for all animations
void FadeInFadeOut(long color, bool fast = false, uint8_t what = 0)
{
    uint8_t red = (color & 0xFF0000) >> 16;
    uint8_t green = (color & 0x00FF00) >> 8;
    uint8_t blue = (color & 0x0000FF);

    RgbColor target = RgbColor(red, green, blue);
    uint16_t time;
    if (fast) time = FADEFAST;
    else time = FADESLOW;

    animationState[0].StartingColor = pixels.GetPixelColor(0);
    animationState[0].EndingColor = target;

    // TODO Make this nicer
    if (what == 0) {
        animations.StartAnimation(0, time, EyeAnimUpdate);
    } else if (what == 1) {
        animations.StartAnimation(0, time, BlendAnimUpdate);
    }
}

// void eyes(long color){
//     pixels.clear();
//     for(int i=eye_left_0; i<eye_left_1; i++) {
//         pixels.setPixelColor(i, color);
//     }

//     for(int i=eye_left_2; i<eye_left_3; i++) {
//         pixels.setPixelColor(i, color);
//     }

//     for(int i=eye_right_0; i<eye_right_1; i++) {
//         pixels.setPixelColor(i, color);
//     }

//     for(int i=eye_right_2; i<eye_right_3; i++) {
//         pixels.setPixelColor(i, color);
//     }  

//     pixels.show();
// }

// void all_leds(long color){
//     strip.SetPixelColor(pixel, color);
//     // pixels.clear();
//     // for(int i=0; i<NUMPIXELS; i++) {
//     //     pixels.setPixelColor(i, color);
//     //     pixels.show();
//     // }
// }

//--------------------------------------------
//------------End of test animations----------
//--------------------------------------------

byte x2b(char c) {
  if (isdigit(c)) {  // 0 - 9
    return c - '0';
  } 
  else if (isxdigit(c)) { // A-F, a-f
    return (c & 0xF) + 9;
  }

}

long convert_rgb(char* rgb_str) {
    long rgb = 0;

    for (int i= 0; i < strlen(rgb_str); i++) {
        rgb = (rgb * 16) + x2b(rgb_str[i]);
    }

    return rgb;
}

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

    // Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");

    // Start neopixels
    // pixels.begin();

    // NeoPixelBus
    pixels.Begin();
    pixels.Show();

    // laser_blink(200, 5);
    // pinMode(LASER_PIN, OUTPUT);
    // pinMode(LIGHT_PIN, OUTPUT);
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

            char *ptr = NULL;
            char *animation[2];
            byte index = 0;

            // Tokenize input
            ptr = strtok(packet, "/");
            while (ptr != NULL) 
            {
                Serial.println(ptr);
                animation[index] = ptr;
                index++;
                ptr = strtok(NULL, "/");
            }

            if (strstr(animation[0], "EYF")){
                // eyes(convert_rgb(animation[1]));
                FadeInFadeOut(convert_rgb(animation[1]), true, 0);
            } else if (strstr(animation[0], "EYS")){
                // eyes(convert_rgb(animation[1]));
                FadeInFadeOut(convert_rgb(animation[1]), false, 0);
            } else if (strstr(animation[0], "ALF")) {
                // all_leds(convert_rgb(animation[1]));
                FadeInFadeOut(convert_rgb(animation[1]), true, 1);
                // FadeInFadeOutRinseRepeat(0.2f); 
            } else if (strstr(animation[0], "ALS")) {
                // all_leds(convert_rgb(animation[1]));
                FadeInFadeOut(convert_rgb(animation[1]), false, 1);
                // FadeInFadeOutRinseRepeat(0.2f);
            } else if (strstr(animation[0], "LAS")) {
                laser_static(atoi(animation[1]));
            } else if (strstr(animation[0], "LBL")) {
                laser_blink(atoi(animation[1]));
            } else if (strstr(animation[0], "LBR")) {
                laser_breath(atoi(animation[1]));
            } else if (strstr(animation[0], "FRO")) {
                front_static(atoi(animation[1]));
            } else if (strstr(animation[0], "FBL")) {
                front_blink(atoi(animation[1]));
            } else if (strstr(animation[0], "FBR")) {
                front_breath(atoi(animation[1]));
            }

            // if (strstr(animation[0], "/LED/GREEN")) {
            //     neopixels_green();
            // } else if (strstr(packet, "/LED/RED")) {
            //     neopixels_red();
            // } else if (strstr(packet, "/LED/OFF")) {
            //     neopixels_off();
            // } else if (strstr(packet, "/LAS/ON")){
            //     laser_on(255);
            //     // digitalWrite(LASER_PIN, HIGH);
            // } else if (strstr(packet, "/LAS/OFF")){
            //     laser_off();
            //     // digitalWrite(LASER_PIN, LOW);
            // } else if (strstr(packet, "/LAS/BLI")){
            //     laser_blink(30, 0);
            // } else if (strstr(packet, "/L1G/ON")){
            //     light_on(255);
            // } else if (strstr(packet, "/L1G/OFF")){
            //     light_off();
            // } else if (strstr(packet, "/L1G/BLI")){
            //     light_blink(30,0);
            // }
        }
    }

}
