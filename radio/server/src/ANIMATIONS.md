## LEDSTRIP ANIMATIONS

__ __ __
|  |  |
|  |  |__ F: FAST / S: SLOW
|  |
|  |__ F: FADE / L: LOOP
|
|__ A: ALL / E: EYES / O: EARS

### Full Table

| CODE | DESCRIPTION | PARAM |
| :-: | :- | :- |
| "AFF"  | ALL FADE FAST | COLOR (RGB_HEX) |
| "AFS"  | ALL FADE SLOW | COLOR (RGB_HEX) |
| "ALF"  | ALL LOOP FAST | COLOR (RGB_HEX) |
| "ALS"  | ALL LOOP SLOW | COLOR (RGB_HEX) |
| "EFF"  | EYES FADE FAST | COLOR (RGB_HEX) |
| "EFS"  | EYES FADE SLOW | COLOR (RGB_HEX) |
| "ELF"  | EYES LOOP FAST | COLOR (RGB_HEX) |
| "ELS"  | EYES LOOP SLOW | COLOR (RGB_HEX) |
| "OFF"  | EARS FADE FAST | COLOR (RGB_HEX) |
| "OFS"  | EARS FADE SLOW | COLOR (RGB_HEX) |
| "OLF"  | EARS LOOP FAST | COLOR (RGB_HEX) |
| "OLS"  | EARS LOOP SLOW | COLOR (RGB_HEX) |

### Example

```
AFF/00FF00
--> All Leds Fade-in to Green
```

## PWM ANIMATIONS (LASER OR HIGH POWER LEDS)

__ __ __
|  |  |
|  |  |__ S: SHOT / F: FOREVER
|  |
|  |__ F: FADE / R: BREATHE / L: BLINK
|
|__ L: LASER / F: FRONTLIGHTS

### Full Table

| CODE | DESCRIPTION | PARAM | 
| :-: | :- | :- |
| "LFF"  | LASER FADE-TO FAST            | INTENSITY |
| "LFS"  | LASER FADE-TO SLOW            | INTENSITY |
| "LRF"  | LASER BREATH FOREVER          | PERIOD |
| "LLF"  | LASER BLINK FOREVER           | PERIOD |
| "LRS"  | LASER BREATH SHOT_NUM-SHOTs   | PERIOD |
| "LLS"  | LASER BLINK SHOT_NUM-SHOTs    | PERIOD |
| "FFF"  | FRONT FADE-TO FAST            | INTENSITY |
| "FFS"  | FRONT FADE-TO SLOW            | INTENSITY |
| "FRF"  | FRONT BREATH FOREVER          | PERIOD |
| "FLF"  | FRONT BLINK FOREVER           | PERIOD |
| "FRS"  | FRONT BREATH SHOT_NUM-SHOTs   | PERIOD |
| "FLS"  | FRONT BLINK SHOT_NUM-SHOTs    | PERIOD |

### Example

```
FRF/3000
--> Front Lights Breathe Forever with 3000ms PERIOD
```
