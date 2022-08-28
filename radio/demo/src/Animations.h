#include <jled.h>
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

// JLED
#define LASER_PIN       10
#define FRONTLIGHT_PIN  11
#define ANIMAT_CH       1

// TODO Understand why without an initial animation, timing gets wrong and sequence is never updated
JLed pwm_pins[] = {JLed(LASER_PIN).Breathe(255).Forever(), JLed(FRONTLIGHT_PIN).Off()};
// JLedSequence sequence(JLedSequence::eMode::PARALLEL, pwm_pins);

// NEOPIXEL
#define NEOPIXEL_PIN    9
#define NUMPIXELS       45
#define FADESLOW        4000
#define FADEFAST        500
#define ANIMAT_CH       1

// NeoPixelBus
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> pixels(NUMPIXELS, NEOPIXEL_PIN);
NeoPixelAnimator animations(ANIMAT_CH); // NeoPixel animation management object

struct AnimationRequest
{
    bool Loop;
    bool FastTransition;
    bool IsLedStrip;
    uint8_t LedStripItem;
    long LedStripColor;
};

struct AnimationStatus
{
    RgbColor StartingColor;
    RgbColor EndingColor;
    bool Loop;
    uint8_t LedStripItem;    
};

AnimationStatus animationStatus[ANIMAT_CH];

// PROTOTYPE LED RANGES
// #define EYE_RANGES 1
// uint8_t eye_ranges[EYE_RANGES][2] = {{30, 44}};

// #define JAW_RANGES 3
// uint8_t jaw_ranges[JAW_RANGES][2] = {{0, 3},
//                                      {9, 12},
//                                      {61, 72}};

// #define CHIN_RANGES 1
// uint8_t chin_ranges[CHIN_RANGES][2] = {{3, 9}};


// #define MOUSTACHE_RANGES 2
// uint8_t moustache_ranges[MOUSTACHE_RANGES][2] = {{24, 30},
//                                                  {44, 50}};   

// #define EAR_RANGES 2
// uint8_t ear_ranges[EAR_RANGES][2] = {{16, 24}, 
//                                      {50, 58}};

// #define REAR_RANGES 2
// uint8_t rear_ranges[REAR_RANGES][2] = {{13, 16}, 
//                                         {58, 61}};

// NEW RANGES
#define JAW_RANGES 3
uint8_t jaw_ranges[JAW_RANGES][2] = {{34, 38},
                                     {41, 47}};

#define CHIN_RANGES 1
uint8_t chin_ranges[CHIN_RANGES][2] = {{38, 41}};


#define MOUSTACHE_RANGES 2
uint8_t moustache_ranges[MOUSTACHE_RANGES][2] = {{11, 18},
                                                 {18, 23}};

#define EAR_RANGES 2
uint8_t ear_ranges[EAR_RANGES][2] = {{0, 8},
                                     {23, 31}};

#define REAR_RANGES 2
uint8_t rear_ranges[REAR_RANGES][2] = {{8, 11},
                                        {31, 34}};

// LASER ANIMATIONS
void laser_blink(int wait, int repeats = 0) {
  if (repeats) pwm_pins[0].Blink(wait, wait).Repeat(repeats);
  else pwm_pins[0].Blink(wait, wait).Forever();
}

void laser_breath(int wait, int repeats = 0) {
  if (repeats) pwm_pins[0].Breathe(wait).Repeat(repeats);
  else pwm_pins[0].Breathe(wait).Forever();
}

void laser_fade_in_fast(int brightness) {
    pwm_pins[0].Set(brightness);
}

void laser_fade_in_slow(int brightness) {
    if (brightness){
        pwm_pins[0].FadeOn(FADESLOW).MaxBrightness(brightness).Repeat(1).Reset();
    } else {
        pwm_pins[0].FadeOff(FADESLOW).Repeat(1).Reset();
    }
}

// FRONT LIGHT ANIMATIONS
void front_blink(int wait, int repeats = 0) {
    if (repeats) pwm_pins[1].Blink(wait, wait).Repeat(repeats);
    else pwm_pins[1].Blink(wait, wait).Forever();
}

void front_breath(int wait, int repeats = 0) {
    if (repeats) pwm_pins[1].Breathe(wait).Repeat(repeats);
    else pwm_pins[1].Breathe(wait).Forever();
}

void front_fade_in_fast(int brightness) {
    pwm_pins[1].Set(brightness);
}

void front_fade_in_slow(int brightness) {
    if (brightness){
        pwm_pins[1].FadeOn(FADESLOW).MaxBrightness(brightness).Repeat(1).Reset();
    } else {
        pwm_pins[1].FadeOff(FADESLOW).Repeat(1).Reset();
    }
}

// LEDSTRIP
void AnimationUpdate(const AnimationParam& param)
{
    RgbColor updatedColor = RgbColor::LinearBlend(
        animationStatus[param.index].StartingColor,
        animationStatus[param.index].EndingColor,
        param.progress);

    if (animationStatus[param.index].LedStripItem == 0) {
        // FULL STRIP
        for (uint8_t pixel = 0; pixel < NUMPIXELS; pixel++)
        {
            pixels.SetPixelColor(pixel, updatedColor);
        }
    } else {
        // TODO make this as a pointer to the ranges - ?
        if (animationStatus[param.index].LedStripItem == 1){
            Serial.println('Nein');
            // for (uint8_t index = 0; index < EYE_RANGES; index++) {
            //     for (uint8_t pixel = eye_ranges[index][0]; pixel < eye_ranges[index][1]; pixel ++){
            //         pixels.SetPixelColor(pixel, updatedColor);
            //     }
            // }
        } else if (animationStatus[param.index].LedStripItem == 2){
            for (uint8_t index = 0; index < EAR_RANGES; index++) {
                for (uint8_t pixel = ear_ranges[index][0]; pixel < ear_ranges[index][1]; pixel ++){
                    pixels.SetPixelColor(pixel, updatedColor);
                }
            }
        } else if (animationStatus[param.index].LedStripItem == 3){
            for (uint8_t index = 0; index < CHIN_RANGES; index++) {
                for (uint8_t pixel = chin_ranges[index][0]; pixel < chin_ranges[index][1]; pixel ++){
                    pixels.SetPixelColor(pixel, updatedColor);
                }
            }
        } else if (animationStatus[param.index].LedStripItem == 4){
            for (uint8_t index = 0; index < JAW_RANGES; index++) {
                for (uint8_t pixel = jaw_ranges[index][0]; pixel < jaw_ranges[index][1]; pixel ++){
                    pixels.SetPixelColor(pixel, updatedColor);
                }
            }
        } else if (animationStatus[param.index].LedStripItem == 5){
            for (uint8_t index = 0; index < MOUSTACHE_RANGES; index++) {
                for (uint8_t pixel = moustache_ranges[index][0]; pixel < moustache_ranges[index][1]; pixel ++){
                    pixels.SetPixelColor(pixel, updatedColor);
                }
            }
        } else if (animationStatus[param.index].LedStripItem == 6){
            for (uint8_t index = 0; index < REAR_RANGES; index++) {
                for (uint8_t pixel = rear_ranges[index][0]; pixel < rear_ranges[index][1]; pixel ++){
                    pixels.SetPixelColor(pixel, updatedColor);
                }
            }
        }

    }

    if (animationStatus[param.index].Loop) {
        if (param.state == AnimationState_Completed){
            RgbColor sc = animationStatus[0].StartingColor;
            RgbColor ec = animationStatus[0].EndingColor;
            animationStatus[0].StartingColor = ec;
            animationStatus[0].EndingColor = sc;

            animations.RestartAnimation(param.index);
        }
    }
}

void AnimationTrigger(AnimationRequest animationRequest)
{
    uint8_t red = (animationRequest.LedStripColor & 0xFF0000) >> 16;
    uint8_t green = (animationRequest.LedStripColor & 0x00FF00) >> 8;
    uint8_t blue = (animationRequest.LedStripColor & 0x0000FF);

    RgbColor target = RgbColor(red, green, blue);
    uint16_t time;
    if (animationRequest.FastTransition) time = FADEFAST;
    else time = FADESLOW;

    animationStatus[0].StartingColor = pixels.GetPixelColor(0);
    animationStatus[0].EndingColor = target;
    // We pass this only because of the inputs to AnimationUpdate
    animationStatus[0].Loop = animationRequest.Loop;
    animationStatus[0].LedStripItem = animationRequest.LedStripItem;

    animations.StartAnimation(0, time, AnimationUpdate);
}