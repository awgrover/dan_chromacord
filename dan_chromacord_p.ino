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

    case 'c' : // Choose another patch
      current_patch_i = choose_patch(current_patch_i);
      test_num = 0xff;
      break;

    case '?' :
      Serial.println();
      // menu made by: make (in examples/, then insert here)
Serial.println(F("0  idle/sanity"));
Serial.println(F("d  Describe actual registers"));
Serial.println(F("r  Reset"));
Serial.println(F("C  get max/min of POT on A0 till 'x' (callibration)"));
Serial.println(F("p  Track Pot"));
Serial.println(F("P  Track pots with timer & stuff"));
Serial.println(F("t  Timer test: make something blink every n"));
Serial.println(F("s  Show patch"));
Serial.println(F("g  Go into performance mode"));
Serial.println(F("c  Choose another patch"));
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
  print(F("first pixel @"));print((unsigned int)*patch);Serial.println();
  for(byte dandelion_i=0; dandelion_i < tlcmanager.device_count(); dandelion_i++) {
    // Collect rgb values for this dandelion
    TLC59116& dandelion = tlcmanager[dandelion_i];
    byte pwm_buffer[16] = {}; // rgb sets. if no zone-pixel anywhere, then off
    print(F("pwm buffer "));print((unsigned int) pwm_buffer);Serial.println();
    print(F("Dandelion 0x")); print(dandelion.address(),HEX);print(F(" i:"));print(dandelion_i);Serial.println();

    // patch: [ zone:[ pixel:i, ...], ... ]
    // FIXME: we should construct a reverse table: zone_i -> [dandelion/channels,...]
    print(F("patch @"));print((unsigned int)patch);print(F(" ptr size"));print(sizeof(patch));Serial.println();
    for (byte zone_i=0; zone_i < Zone_Count; zone_i++) {
      print(F("  check zone @"));print((unsigned int)patch + zone_i * sizeof(byte**));print(F(" #"));print(zone_i);Serial.println();
      print(F("  check zone 1st pixel@"));print((unsigned int)patch[zone_i]);print(F(" #"));print(zone_i);Serial.println();
      for (const byte* pix = patch[zone_i];  *pix != 0xff; pix++) {
        byte dandelion_for_pixel = *pix / Pixels_Per_Dandelion;
        print(F("    pix @"));print((unsigned int)pix);print(F(" #"));print(*pix);print(F(" -> dandelion i "));print(dandelion_for_pixel);Serial.println();
        if (dandelion_for_pixel == dandelion_i) {
          byte rgb_i = *pix % Pixels_Per_Dandelion;
          byte channel = rgb_i * 3; // pixel #2 is at 2*3=channel 6,7,8
          print(F("    "));print(*pix); print(F("->led #")); print(channel);Serial.println();
          print(F("    copy rgb[zone_i] to pwm "));print((unsigned int) pwm_buffer + channel);Serial.println();
          memcpy(pwm_buffer + channel, (byte* /*[3]*/) zone_rgb[zone_i], 3);
          delay(200);
          }
        }
      }
    print(F("set dandelion"));for(byte i=0; i<16; i++) {print(" ");print(pwm_buffer[i]);}Serial.println();
    dandelion.pwm(pwm_buffer);
    }
  }

void show_patch(const byte** patch) {
  // indicate patch r,g,b,rgb on 1st/last pixel
  // tlcmanager[0].on(0);
  // tlcmanager[tlcmanager.device_count()-1].on(14);
  byte rgb[Zone_Count][3];
  byte *rgb_p[Zone_Count];
  // copy pointers so we have byte**
  for(byte i = 0; i<Zone_Count; i++) { rgb_p[i] = rgb[i]; }

  for(byte i =0; i < Zone_Count; i++) {
    Serial.print(F("Zone "));Serial.println(i);
    byte rgb_bits = i % 6 + 1; // 1..6, 3bits where 1 or 2 bits are on at a time

    // set this i to interesting value, set others to 0
    for (byte set_i=0; set_i < Zone_Count; set_i++) {
      if (set_i==i) {
        rgb[set_i][0] = (rgb_bits & 0b001) ? 100 : 0;
        rgb[set_i][1] = (rgb_bits & 0b010) ? 100 : 0;
        rgb[set_i][2] = (rgb_bits & 0b100) ? 100 : 0;
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

byte choose_patch(byte current) {
  for (byte i=0; i < Patch_Count; i++) {
    print(i==current ? "=" : " ");print(" ");Serial.println(i);
    }
  print(F("Choose: ")); 
  while(Serial.available() <= 0) {}
  char choice = Serial.read();
  Serial.println(choice);
  if (choice >= '0' && choice <= '0'+Patch_Count-1) {
    print(F("Set to "));Serial.println(choice);
    return choice-'0';
    }
  Serial.println(F("What?"));Serial.println();
  return current;
  }
