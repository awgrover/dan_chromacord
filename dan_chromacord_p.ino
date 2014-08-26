#include <Wire.h>
#define TLC59116_WARNINGS 1
#include <TLC59116.h>
#include <MsTimer2.h>
#include "slopifier.h"
#include "zone_patches.h"
#include "patches.h"
#include "sliders.h"
#include "tired_of_serial.h"

const byte Pixel_Count = 5*3;
const byte Pixels_Per_Dandelion = int(16 / 3); // rounds to 5!
const int Test_Pot_Pin = 0;
TLC59116Manager tlcmanager; // defaults


extern int __bss_end;
extern void *__brkval;

int get_free_memory() {
  int free_memory;

  if((int)__brkval == 0)
    free_memory = ((int)&free_memory) - ((int)&__bss_end);
  else
    free_memory = ((int)&free_memory) - ((int)__brkval);

  return free_memory;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Top of setup");
  Serial.print(F("Free memory "));Serial.println(get_free_memory());
  tlcmanager.init();
  // tlcmanager[0].set_milliamps(10);
  Serial.print(F("Set [0] to "));Serial.print(tlcmanager[0].milliamps());Serial.println(F("ma"));
  Serial.print(F("Free memory after init "));Serial.println(get_free_memory());
  
  // check for pixels vs dandelion count
  byte pixel = 0;
  for (const byte*** p=patches; p < patches + Patch_Count; p++) {
    print(F("Check patch "));print(p-patches);print(F("/"));print(Patch_Count);Serial.println();
    for (const byte** zone=*p; zone < *p + Zone_Count; zone++) {
      print(F("  check zone "));print(zone-*p);Serial.println();
      for (const byte *pix=*zone; *pix != 0xFF; pix++) {
        // print(F("    check pixel "));print(*pix);Serial.println();
        pixel = max(pixel, *pix);
        }
      }
    print(F("  max "));print(pixel);Serial.println();
    }
  print(F("Max pixel "));print(pixel);Serial.println();
  if (pixel > tlcmanager.device_count() * Pixels_Per_Dandelion) {
    print(F("ERROR, "));print(tlcmanager.device_count());print(F(" dandelions, which is "));
    print(tlcmanager.device_count() * Pixels_Per_Dandelion);print(F(" rgb pixels, but the patches have a rgb pixel # "));
    print(pixel);
    Serial.println();
    }
  }

TLC59116 *g_tlc; // only for the isr routine

void loop() {
  static TLC59116 *tlc = NULL;
  static char test_num = 'd'; // idle pattern
  static byte current_patch_i = 0;
  if (!tlc) tlc = &(tlcmanager[0]);

  switch (test_num) {

    case '0': // idle/sanity
      prove_on(*tlc);
      test_num = 0xff;
      break;

    case 'd': // Describe actual registers
      Serial.println();
      tlc->describe_actual();
      test_num = 0xff;
      break;

    case 'r': // Reset
      tlcmanager.reset();
      tlcmanager[0].set_milliamps(20);
      test_num = 0xff;
      break;

    case 'C': // get max/min of POT on A0 till 'x' (callibration)
      max_min_pot(Test_Pot_Pin);
      test_num = '?';
      break;

    case 'p': // Track Pot
      while (Serial.available() <= 0) {
        int val = analogRead(Test_Pot_Pin);
        Serial.println(val);
        }
      test_num = 0xff;
      break;
    
    case 'P' : // Track pots with timer & stuff
      track_print_pots();
      test_num = 0xff;
      break;

    case 't' : // Timer test: make something blink every n
      g_tlc = tlc;
      MsTimer2::set(200, on_off_isr);
      MsTimer2::start();
      while(Serial.available() <= 0) {
        Serial.println(analogRead(3));
        }
      MsTimer2::stop();
      test_num = 0xff;
      break;
      
    case 's' : // Show patch
      Serial.print(F("Patch "));Serial.println(current_patch_i);
      show_patch(patches[current_patch_i]);
      test_num = 0xff;
      break;

    case 'g' : // Go into performance mode
      Serial.print(F("Patch "));Serial.println(current_patch_i);
      performance(patches[current_patch_i]);
      test_num = 0xff;
      break;

    case '?' :
      Serial.println();
      // menu made by: make (in examples/, then insert here)
Serial.println(F("0  idle/sanity"));
Serial.println(F("r  Reset"));
Serial.println(F("C  get max/min of POT on A0 till 'x' (callibration)"));
Serial.println(F("p  Track Pot"));
Serial.println(F("P  Track pots with timer & stuff"));
Serial.println(F("g  Go into performance mode"));
Serial.println(F("t  Timer test: make something blink every n"));
      // end menu
      // fallthrough

    case 0xff : // show prompt, get input
      Serial.print(F("Choose (? for help): "));
      while(Serial.available() <= 0) {}
      test_num = Serial.read();
      Serial.println(test_num);
      break;

    default :
      test_num = '?';
      break;

    }
  }

void max_min_pot(int analog_pin) {
  Slopifier history;
  byte measure_ct = 5;
  int max_measure[measure_ct];
  int min_measure[measure_ct];
  int i_max=0;
  int i_min=0;
  for(byte i=0;i<measure_ct;i++) { max_measure[i]=0; min_measure[i]=0; }

  while (Serial.available() <= 0) {
    delay(50);
    int val = analogRead(analog_pin);
    history << val;
    if (!history.primed) continue;

    if (history.read_reversed) continue;

    Slopifier::Direction reversal = history.has_reversed();
    switch (reversal) {
      case Slopifier::Up:
        max_measure[(i_max++) % measure_ct] = history.maxv;
        break;
      case Slopifier::Down:
        min_measure[(i_min++) % measure_ct] = history.minv;
        break;
      case Slopifier::None:
        Serial.print("Min ");
        for(byte i=0; i<measure_ct; i++) { Serial.print(min_measure[i]); Serial.print(" "); }
        Serial.println();
        Serial.print("Max ");
        for(byte i=0; i<measure_ct; i++) { Serial.print(max_measure[i]); Serial.print(" "); }
        Serial.println();
        break;
      }
    }
    Serial.print("Min ");
    for(byte i=0; i<measure_ct; i++) { Serial.print(min_measure[i]); Serial.print(" "); }
    Serial.println();
    Serial.print("Max ");
    for(byte i=0; i<measure_ct; i++) { Serial.print(max_measure[i]); Serial.print(" "); }
    Serial.println();
  }


void prove_on(TLC59116& tlc) {
  Serial.println("TOP");
  tlc
  // .pwm(0, 3, (byte[]) {255,255,255}).delay(500)
  .on(0).on(1).on(2)
  .delay(500);
  Serial.println("off...");
  tlc.off(0).off(1).off(2)
  .delay(500);
  tlc.pwm(0, 3, (byte[]){50, 128, 128})
  .delay(500);
  }

void on_off_isr() {
  static bool oo = 0;
  sei();
  g_tlc->set(1, oo);
  g_tlc->set(2, oo);
  oo = !oo;
  }

void track_print_pots() {
  RGBPot::start_reading(sliders);
  while (Serial.available() <= 0) {
    for (byte i = 0; i<RGBPot::pot_list_count; i++) {
      const RGBPot &this_pot = RGBPot::pot_list[i];
      for (byte rgb_i=0; rgb_i < 3; rgb_i++) {
        print(this_pot.rgb[rgb_i]);print(" ");
        }
      print(F(" | "));
      }
    Serial.println();
    delay(200);

    }
  RGBPot::stop_reading();
  }


void update_by_dandelion(const byte *zone_rgb[3] /* [Zone_Count][3] */, const byte **patch) {
  // Do each dandelion so we only have to "buffer" 15 values at a time
  for(byte dandelion_i=0; dandelion_i < tlcmanager.device_count(); dandelion_i++) {
    // Collect rgb values for this dandelion

    TLC59116& dandelion = tlcmanager[dandelion_i];
    byte pwm_by_rgb[Pixels_Per_Dandelion][3] = {}; // rgb sets
    print(F("Dandelion ")); print(dandelion.address());Serial.println();

    // FIXME: we should construct a reverse table: zone_i -> [dandelion/channels,...]
    for (const byte* zone=*patch; zone < *patch + Zone_Count; zone++) {
      for (const byte zone_i=0; zone_i < Zone_Count; zone++) {
        print(F("  check zone "));print(zone_i);Serial.println();
        for (const byte *pix=patch[zone_i]; *pix != -1; pix++) {
          byte dandelion_for_pixel = *pix / Pixels_Per_Dandelion;
          if (dandelion_for_pixel == dandelion_i) {
            byte rgb_i = *pix % Pixels_Per_Dandelion;
            byte channel = rgb_i * 3; // pixel #2 is at 2*3=channel 6,7,8
            print(*pix); print(F("->")); print(channel);Serial.println();
            memcpy(&pwm_by_rgb[channel], (byte* /*[3]*/) zone_rgb[zone_i], 3);
            }
          }
        }
      }
    dandelion.pwm(*pwm_by_rgb);
    }
  }

void show_patch(const byte** patch) {
  // indicate patch r,g,b,rgb on 1st/last pixel
  // tlcmanager[0].on(0);
  // tlcmanager[tlcmanager.device_count()-1].on(14);
  byte rgb[Zone_Count][3];
  byte *rgb_p[Zone_Count];
  for(byte i = 0; i<Zone_Count; i++) { rgb_p[i] = rgb[i]; }
  for(byte i =0; i < Zone_Count; i++) {
    Serial.print(F("Zone "));Serial.println(i);
    byte rgb_bits = i % 6 + 1; // 1..6, 3bits where 1 or 2 bits are on at a time

    // set this i to interesting value, set others to 0
    for (byte set_i=0; set_i < Zone_Count; set_i++) {
      if (set_i==i) {
        rgb[set_i][0] = (rgb_bits & 0b100) ? 100 : 0;
        rgb[set_i][1] = (rgb_bits & 0b010) ? 100 : 0;
        rgb[set_i][2] = (rgb_bits & 0b001) ? 100 : 0;
        }
      else {
        memset(rgb[set_i], 0, 3);
        }
      }

    update_by_dandelion((const byte**)rgb_p, patch);
    delay(1000);
    }
    
  tlcmanager.reset();
  }

void performance(const byte** patch) {
  update_by_dandelion((const byte**)sliders, patch);
  }

/*
const byte Slider_ct = 3;
void spiral() {
  const byte max_group_led = (2 * 3);
  const unsigned long on_time = 2;
  const byte v_min = 254;
  const byte s_by = 10;
  const byte s_max = (255 / s_by) * s_by;
  const byte second_offset = 128;
  while (1) {
    
    for(byte v=v_min; ; v++) {
      Serial.print("V ");Serial.println(v);

      for (byte s=254; ; ) { // s += s_by) {
        if (!(s % 10)) {Serial.print("S ");Serial.println(s);}

        for (byte h=0; ; h++) {
          // Serial.print("H ");Serial.print(h); Serial.print(" "); Serial.println( h<=255);
          for (TLC59116** a_tlc = tlc; *a_tlc; a_tlc++) {
            // for each rgb
            byte as_array[] = { h,s,v };
            byte as_array2[] = { h-128,s,v };
            tlc_first.pwm(0,3,as_array);
            tlc_first.pwm(3,3,as_array2);
            delay(20);
            delay(on_time);
            }

          // check for keyboard every 16
          if (!(h & 0xF)) { if (Serial.available() > 0) {Serial.println("Done"); return;} }

          if (h==255) break; // h<=255 is ALWAYS true
          }
        // Serial.println("done h");
        if (s==s_max) break; // h<=255 is ALWAYS true
        }
      if (v==255) break;
      }
    }
  }

void pwm_full_range() {
  // while 0..7 goes up, 8..15 goes down & vice versa
  const byte led_count = 6;
  const byte size1 = 8;
  byte direction;
  byte values[16];
  unsigned long timer = millis();

  // init
  for(byte i=0; i < size1; i++) { values[i] = 0; }
  for(byte i=size1; i<=15; i++) { values[i] = 255; }

  // vary
  while (1) {
    // Serial.print("Set 0..15 ");for(byte i=0; i<16; i++) {Serial.print(values[i]);Serial.print(" ");}
    tlc_first.pwm(0,led_count, values);
    delay(10);
    // Serial.print(" next ");Serial.print(direction);Serial.println();

    if (values[0] >= 255) { direction = -1; }
    else if (values[0] <= 0) { 
      Serial.print("Elapsed ");Serial.print(millis()-timer);Serial.println();
      timer = millis();
      direction = +1; 
      }
    if (! (values[0] & 0xf)) { if (Serial.available() > 0) break; }
    for(byte i=0; i < size1; i++) { values[i] += direction; }
    for(byte i=size1; i<=15; i++) { values[i] -= direction; }
    
    }
  }

const int Slider_Pins[] = { 0,1,2,3,4,5,6,7,  8.9,10,11,12,13,14,15 };
const byte Zones = 1;
const byte Sample_Time = 100; // microsec
// 50msec
const int Sample_Interval = Sample_Time * Slider_ct > 50000 ? (Sample_Time * Slider_ct)/1000 : 50;
const byte Lunits_Per_Zone = 2;
const byte LEDs_Per_Lunit = 3;
const byte Max_Lunit = 5;
const byte Zone_Assignment[Slider_ct][Lunits_Per_Zone] = { // in Lunits (rgb #)
  // 0
  { 0, 1 },
  // 1
  { 2,3 },
  // 2
  { 4, -1 } // -1 is filler
  };
const byte RGB_Assignment[Slider_ct] = { //0,1,2 as rgb
  0,1,2
  };

TLC59116 *dandelions[] = {&tlc_first};
byte Targets[Slider_ct];
const byte Total_LEDs = LEDs_Per_Lunit * Max_Lunit;
byte leds[Total_LEDs];


void grab_next_slider() {
  // timer routine
  for(byte i=0; i<Slider_ct ; i++) {
    Targets[i] = analogRead(Slider_Pins[i]) >> 2;
    }
  }


void set_pwm(byte* leds) {
  // any assumption of led->actual? 16 per, continous
  byte number_of_dandelions = (Total_LEDs + 15) / 16;
  Serial.print("d_ct ");Serial.println(number_of_dandelions);
  for(byte dandelion_i = 0; dandelion_i < number_of_dandelions; dandelion_i++) {
    byte start_led_i = dandelion_i * 16;
    TLC59116* dandelion = dandelions[dandelion_i];
    dandelion->pwm(&leds[start_led_i]);
    Serial.print("D ");Serial.print(dandelion_i);
    for(byte led_num = start_led_i; led_num < start_led_i + 16; led_num++) {
      Serial.print(leds[led_num]);Serial.print(" ");
      }
    Serial.println();
    }
  }
  
//algorithms
inline void accumulatepwm_rgb_groups(byte slider_i) {
  // maps a slider_i to the proper rgb element of zoned lunits
  const byte* lunit_list = Zone_Assignment[ slider_i / 3 ]; // "zone x"
  byte rgb_i = RGB_Assignment[slider_i % 3]; // "red of..."
  
  Serial.print("Accum for rgbi ");Serial.print(rgb_i);
  for (byte i=0; i<Lunits_Per_Zone; i++) {
    byte lunit = lunit_list[i];
    if (lunit >= 0) {
      Serial.print("lunit ");Serial.print(lunit);Serial.print(" for ");Serial.print(i);
      byte t_led_num = lunit * LEDs_Per_Lunit + rgb_i; // "red of zone x"
      Serial.print(" led num ");Serial.print(t_led_num); Serial.print("=");Serial.print(Targets[slider_i]);
      leds[t_led_num] = Targets[slider_i];
      }
    }
  Serial.println();
  }
*/
