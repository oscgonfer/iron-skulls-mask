#include "Arduino.h"
#include <SPI.h>
#include "Animations.h"
#include "Extras.h"

#define MAX_MESSAGE_LENGTH 20

#define SHOT_NUM 5

#define ANIM_LENGTH 12
#define ANIMATION_DURATION 5000
int current_animation = 0;
int animation_duration; 
String animations_list[ANIM_LENGTH][2] = { 
                                            {"LFS/200", "L"},
                                            {"LFS/0", "L"},
                                            {"MFS/550000", "S"},
                                            {"CFS/550000", "S"},
                                            {"JFS/550000", "L"},
                                            {"JFS/0", "S"},
                                            {"CFS/0", "S"},
                                            {"MFS/0", "S"},
                                            {"FFS/200", "L"},
                                            {"FFS/0", "L"},
                                            {"OFS/111111",  "L"},
                                            {"OFS/0", "L"},
                                        };

char buf[MAX_MESSAGE_LENGTH]{}; // Dont put this on the stack:
uint32_t timer = millis();

void setup(){
    Serial.begin(115200);

    // NeoPixelBus
    pixels.Begin();
    pixels.Show();
}

void loop() {
    // Update pwm pins
    pwm_pins[0].Update();
    pwm_pins[1].Update();

    if (animations.IsAnimating())
    {
        // the normal loop just needs these two to run the active animations
        animations.UpdateAnimations();
        pixels.Show();
    }

    if (animations_list[current_animation-1][1] == "L") {
        animation_duration = 5000;
    } else {
        animation_duration = 500;
    }

    if (millis()-timer > animation_duration) {
        timer=millis();
        char packet[MAX_MESSAGE_LENGTH]{};
        strcpy(packet, animations_list[current_animation][0].c_str());
        current_animation++;
        Serial.println(current_animation);

        if (current_animation>ANIM_LENGTH-1){
            current_animation = 0;
        }
        Serial.println(packet);

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
        if (strstr(animation[0], "MTD")) {
            animationRequest.LedStripItem = 1;
            animationRequest.FastTransition = true;
            animationRequest.LedStripItem = 0;
            animationRequest.Loop = false;
            laser_fade_in_fast(0);
            front_fade_in_fast(0);
        // ALL - LedStripItem = 0
        } else if (strstr(animation[0], "AFF")) {
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
