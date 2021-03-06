#include "tired_of_serial.h"

// "1" for performance box, "0" for lightpainting tlc59116 boxes
#define PERFORMANCE 0
#if PERFORMANCE
// This is normal performance display: "dandelion"
#include "PWM_TLC59116.h"
PWM_TLC59116 PWM;
using pwmrange_t = byte;
#else
// This is "manual lightpainting" 16 bit mode: tlc59711
#include "PWM_TLC59711.h"
PWM_TLC59711 PWM(1, 20, 21); // just 1, nb, data on 21 for phone cord;
using pwmrange_t = uint16_t;
#endif

// NB: MsTimer2 needs to be version 0.6+ for mega2560
#include <MsTimer2.h>
#include "every.h"
#include "slopifier.h"
#include "zone_patches.h"
#include "patches.h"
#include "sliders.h"
#include <RunningAverage.h>
#include "PatchSelectorDigital.h"

const byte Pixel_Count = 5 * 3;
const byte Pixels_Per_Dandelion = int(16 / 3); // rounds to 5!
const int Test_Pot_Pin = A1;

PatchSelectorDigital patch_selector;

extern int __bss_end;
extern void *__brkval;

void get_free_memory() {
  int free_memory;

  if ((int)__brkval == 0)
    free_memory = ((int)&free_memory) - ((int)&__bss_end);
  else
    free_memory = ((int)&free_memory) - ((int)__brkval);
  Serial.print(F("Free memory ")); Serial.println(free_memory);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Top of setup");
  // print("gcc ver ");println(__VERSION__);
  get_free_memory();

  PWM.begin(0);
  // for native pwm, you'd have to PWM.begin( each pin )

  // debug test if tlc* works at all: pins 0/1 blink
  if (0) {
    bool onoff = 0;
    println("blinking");

    while (Serial.available() <= 0) {
      PWM.set(0, onoff * 1.0);
      PWM.set(1, ( ! onoff ) * 1.0);
      //tlc.write();
      delay(200);
      onoff = ! onoff;
    }
  }
  if (0) {
    Adafruit_TLC59711 tlc = Adafruit_TLC59711(1, 20, 21);
    bool onoff = 0;
    println("blinking");

    while (Serial.available() <= 0) {
      tlc.setPWM(0, onoff * 65535u);
      tlc.setPWM(1, ( ! onoff ) * 65535u);
      tlc.write();
      delay(200);
      onoff = ! onoff;
    }
  }

  // PWM.tlc->set_milliamps(10);
  //Serial.print(F("[0] is "));Serial.print(PWM.tlc[0]->milliamps());Serial.println(F("ma max"));

  Serial.print(F("After init:")); get_free_memory();

  // check for pixels vs dandelion count
  byte pixel = 0;
  for (const byte*** p = patches; p < patches + Patch_Count; p++) {
    print(F("Check patch ")); print(p - patches); print(F("/")); print(Patch_Count); Serial.println();
    for (const byte** zone = *p; zone < *p + Zone_Count; zone++) {
      print(F("  check zone ")); print(zone - *p); Serial.println();
      for (const byte *pix = *zone; *pix != 0xFF; pix++) {
        // print(F("    check pixel "));print(*pix);Serial.println();
        pixel = max(pixel, *pix);
      }
    }
    print(F("  max ")); print(pixel); Serial.println();
  }
  print(F("Max pixel ")); print(pixel); Serial.println();
  if (pixel >= PWM.device_count() * Pixels_Per_Dandelion) {
    print(F("ERROR, ")); print(PWM.device_count()); print(F(" dandelions, which is 0.."));
    print(PWM.device_count() * Pixels_Per_Dandelion - 1 ); print(F(" rgb pixels, but the patches have a rgb pixel # "));
    print(pixel);
    Serial.println();
  }

  // We don't use the initial patch-setting (we do "attractor" on reset).
  int p = patch_selector.init();
  print(F("Initial Patch Setting ")); print(p, HEX); println();
}

void x_knob_test_loop() {
  int was_knob = -1;
  while (Serial.available() <= 0) {
    int is = patch_selector.read();
    if (is != was_knob) {
      print(F("KNOB "));
      print(is, HEX);
      println();
    }
    was_knob = is;
  }
}

void loop() {
  static char test_num = '0';
  static byte current_patch_i = patch_selector.read();

  switch (test_num) {

    case '0': // (zero) show I'm working: attractor loop
      Serial.print(F("Choose (? for help): "));
      prove_on();
      if (current_patch_i == 0xff) {
        current_patch_i = patch_selector.read();
        if (current_patch_i == 0xff) {
          current_patch_i = 0;
        }
      }
      test_num = 'g';
      break;

#ifdef USING_PWM_TLC59116
    case 'd': // Describe actual registers
      Serial.println();
      PWM.tlc[0].describe_actual();
      test_num = 0xff;
      break;
#endif

    case 'r': // Reset
      PWM.reset();
      test_num = 0xff;
      break;

    case 'm': // Free memory
      get_free_memory();
      test_num = 0xff;
      break;

    case 'R': // Reset
      while (1) {
        PWM.reset();
      }
      test_num = 0xff;
      break;

    case 'C': // get max/min of POT on A0 till 'x' (callibration)
      max_min_pot(Test_Pot_Pin);
      test_num = '?';
      break;

    case 'p': // Track Pot
      while (Serial.available() <= 0) {
        for (int i = 0; i < Zone_Count; i++) {
          sliders[i].read(true);
        }
        println();
      }
      test_num = 0xff;
      break;

    case 'P' : // Track pots with timer & stuff
      track_print_pots();
      test_num = 0xff;
      break;

    case 'k' : // Track knob
      {
        int was_knob = -1;
        while (Serial.available() <= 0) {
          int is = patch_selector.read();
          if (is != was_knob) {
            print(F("KNOB "));
            print(is, HEX);
            println();
          }
          was_knob = is;
        }
      }
      test_num = 0xff;
      break;

    case 'b' : // blink pins 0,1
      {
        bool onoff = 0;

        while (Serial.available() <= 0) {
          PWM.set(0, onoff * 1.0);
          PWM.set(1, ( ! onoff ) * 1.0);
          delay(200);
          onoff = ! onoff;
        }
      }
      test_num = '?';
      break;


    case 't' : // Timer test: make something blink every n till serial input
#ifdef USING_PWM_TLC59116
      print("on/off for D"); print(PWM.tlc[0].address(), HEX); println();
#else
      print("on/off for pwm 0"); println();
#endif
      PWM.set(1, 1.0);
      MsTimer2::set(200, on_off_isr);
      MsTimer2::start();
      while (Serial.available() <= 0) {
        // Serial.println(analogRead(3));
      }
      MsTimer2::stop();
      PWM.reset();
      test_num = 0xff;
      break;

    case 's' : // Demo Patch
      Serial.print(F("Patch ")); Serial.println(current_patch_i, HEX);
      if (current_patch_i != 0xff) {
        show_patch(patches[current_patch_i]);
      }
      test_num = 0xff;
      break;

    case 'S' : // Show patch
      print(F("Analog pins ")); print(NUM_ANALOG_INPUTS); print(F(" : "));
      print(F("A0=")); print(A0); print(F(" "));
      print(F("A1=")); print(A1); print(F(" "));
      print(F("A2=")); print(A2); print(F(" "));
      print(F("A3=")); print(A3); print(F(" "));
      print(F("A4=")); print(A4); print(F(" "));
      print(F("A5=")); print(A5); print(F(" "));
      print(F("A")); print((Zone_Count - 1) * 3); print(F(" max used")); print(F(" "));
      print(F("A")); print(NUM_ANALOG_INPUTS - 1); print(F(" max")); println();

      if (current_patch_i == 0xff) {
        print(F("Patch is -1"));
        println();
        test_num = 0xff;
        break;
      }
      {
        const byte **patch = patches[current_patch_i];
        for (byte zone_i = 0; zone_i < Zone_Count; zone_i++) {
          int pin = sliders[zone_i].pin;
          print(F("Zone ")); print(zone_i); print(F(" pins ")); print(pin); print(F(" - ")); print(pin + 2); print(F(" lunits "));
          for (const byte* pix = patch[zone_i];  *pix != 0xff; pix++) {
            print(*pix); print(F(" "));
          }
          println();
        }
      }
      test_num = 0xFE;
      break;

    case 'g' : // Go into performance mode
      while (current_patch_i == 0xff && Serial.available() <= 0) {
        current_patch_i = patch_selector.read();
      }
      if (current_patch_i != 0xff) {
        if (current_patch_i >= Patch_Count) {
          current_patch_i = 0;
          print(F("Force patch to 0, because it was ")); print(current_patch_i, HEX); println();
        }
        performance(current_patch_i);
      }
      test_num = 0xff;
      break;

    case 'c' : // Choose another patch
      current_patch_i = choose_patch(current_patch_i);
      test_num = 'g';
      break;

    case '?' :
      Serial.println();
      // menu made by: make (in examples/, then insert here)
      Serial.println(F("0  (zero) show I'm working"));
      Serial.println(F("d  Describe actual registers"));
      Serial.println(F("r  Reset"));
      Serial.println(F("m  Free memory"));
      Serial.println(F("R  Reset"));
      Serial.println(F("C  get max/min of POT on A0 till 'x' (callibration)"));
      Serial.println(F("p  Track Pot"));
      Serial.println(F("P  Track pots with timer & stuff"));
      Serial.println(F("k  Track knob"));
      Serial.println(F("b  blink pins 0,1"));
      Serial.println(F("t  Timer test: make something blink every n till serial input"));
      Serial.println(F("s  Demo Patch"));
      Serial.println(F("S  Show patch"));
      Serial.println(F("g  Go into performance mode"));
      Serial.println(F("c  Choose another patch"));
    // end menu
    // fallthrough

    case 0xff : // show prompt, get input
      Serial.print(F("Choose (? for help): "));
    // fallthrough

    case 0xfe : // just get input
      while (Serial.available() <= 0) {}
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
  int i_max = 0;
  int i_min = 0;
  for (byte i = 0; i < measure_ct; i++) {
    max_measure[i] = 0;
    min_measure[i] = 0;
  }

  while (Serial.available() <= 0) {
    delay(50);
    int val = analogRead(analog_pin);
    history << val;
    if (!history.primed) continue;

    if (history.read_reversed) continue;

    Slopifier::Direction reversal = history.has_reversed();
    switch (reversal) {
      case Slopifier::Flat:
        // nothing to do I think?
        break;
      case Slopifier::Up:
        max_measure[(i_max++) % measure_ct] = history.maxv;
        break;
      case Slopifier::Down:
        min_measure[(i_min++) % measure_ct] = history.minv;
        break;
      case Slopifier::None:
        Serial.print("Min ");
        for (byte i = 0; i < measure_ct; i++) {
          Serial.print(min_measure[i]);
          Serial.print(" ");
        }
        Serial.println();
        Serial.print("Max ");
        for (byte i = 0; i < measure_ct; i++) {
          Serial.print(max_measure[i]);
          Serial.print(" ");
        }
        Serial.println();
        break;
    }
  }
  Serial.print("Min ");
  for (byte i = 0; i < measure_ct; i++) {
    Serial.print(min_measure[i]);
    Serial.print(" ");
  }
  Serial.println();
  Serial.print("Max ");
  for (byte i = 0; i < measure_ct; i++) {
    Serial.print(max_measure[i]);
    Serial.print(" ");
  }
  Serial.println();
}


void prove_on() {
  // i.e. show something happening: 2 level reds for each rgb set
  Serial.println("demo");

  PWM.reset();
  // FIXME PWM.broadcast().pwm(0,15,100);
  PWM.set(0, PWM.ChannelsPerDevice, 100);

  const byte max_rgb = PWM.device_count() * (PWM.ChannelsPerDevice / 3); // rgb-pixels-per-dandelion
  const byte rgb_per_device = PWM.ChannelsPerDevice / 3;

  byte rgb_i = 0;

  print(F("attractor loop....")); println();

  // Exit if knobs move
  int was = patch_selector.read();

  // Exit if slider[0][0] moves
  RGBPot &s0 = sliders[0];
  s0.read();
  byte s00 = s0.rgb[0];
  print(F("slider0.0 is ")); print(s00); println();

  while (Serial.available() <= 0) {
    // setting only the reds

    // may not have a multiple of 3 channels per device (i.e. 16 channels)
    byte device_num =  rgb_i / rgb_per_device;
    byte rgb_set = rgb_i % rgb_per_device;

    byte r = (device_num * PWM.ChannelsPerDevice) + (rgb_set * 3);
    // print(F("Will set D"));print(rgb_i/5);print(F(" C"));print(r);Serial.println();

    PWM.set(r, 0.35);
    delay(200);
    PWM.set(r, 0.4);

    rgb_i++; if (rgb_i >= max_rgb) rgb_i = 0;

    if (was != patch_selector.read()) {
      break;
    }
    s0.read(); if (abs(s0.rgb[0] - s00) > 30) {
      break;  // if slider moves "30"
    }
    // print(s0.rgb[0] - s00);println();
  }
}

void on_off_isr() {
  static bool oo = 0;
  sei();
  PWM.set(0, oo * 1.0);
  PWM.set(1, oo * 1.0);
  oo = !oo;
}

void track_print_pots() {
  RGBPot::start_reading(sliders);
  for (byte i = 0; i < RGBPot::pot_list_count; i++) {
    const RGBPot &this_pot = RGBPot::pot_list[i];
    print(this_pot.pin); print("    ");
    print(F(" | "));
  }
  println();
  while (Serial.available() <= 0) {
    for (byte i = 0; i < RGBPot::pot_list_count; i++) {
      const RGBPot &this_pot = RGBPot::pot_list[i];
      for (byte rgb_i = 0; rgb_i < 3; rgb_i++) {
        print(this_pot.rgb[rgb_i]); print(" ");
      }
      print(F(" | "));
    }
    Serial.println();
    delay(200);

  }
  RGBPot::stop_reading();
}


void update_by_dandelion(const RGBPot zone_rgb[Zone_Count], const byte **patch) {
  // Do each dandelion so we only have to "buffer" 15 values at a time
  for (byte dandelion_i = 0; dandelion_i < PWM.device_count(); dandelion_i++) {
    // Collect rgb values for this dandelion
    pwmrange_t pwm_buffer[16] = {}; // rgb sets. if no zone-pixel anywhere, then off
    // print(F("pwm buffer "));print((unsigned int) pwm_buffer);println();
    // print(F("Dandelion 0x")); print(dandelion.address(),HEX);print(F(" i:"));print(dandelion_i);println();

    // FIXME: we should construct a reverse table: zone_i -> [dandelion/channels,...]
    // print(F("patch @"));print((unsigned int)patch);print(F(" ptr size"));print(sizeof(patch));Serial.println();
    for (byte zone_i = 0; zone_i < Zone_Count; zone_i++) {
      // print(F("  check zone @"));print((unsigned int)patch + zone_i * sizeof(byte**));print(F(" #"));print(zone_i);println();
      // print(F("  check zone 1st pixel@"));print((unsigned int)patch[zone_i]);print(F(" #"));print(zone_i);println();
      const byte *rgb_values = zone_rgb[zone_i].rgb;
      for (const byte* pix = patch[zone_i];  *pix != 0xff; pix++) {
        byte dandelion_for_pixel = *pix / Pixels_Per_Dandelion;
        // print(F("    pix @"));print((unsigned int)pix);print(F(" #"));print(*pix);
        //   print(F(" -> dandelion i "));print(dandelion_for_pixel);println();
        if (dandelion_for_pixel == dandelion_i) {
          byte rgb_i = *pix % Pixels_Per_Dandelion;
          byte channel = rgb_i * 3; // pixel #2 is at 2*3=channel 6,7,8
          /*
            print(F("    "));print(*pix); print(F("->led #")); print(channel);
            print(F("    copy rgb[zone_i] @"));print((unsigned int) rgb_values);print(F(" to pwm "));
            print((unsigned int) pwm_buffer + channel);
            print(F(" rgb")); print(*rgb_values);print(F(","));print(*(rgb_values+1));print(F(","));print(*(rgb_values+2));
            println(); delay(200);
          */

          if (sizeof(pwmrange_t) == sizeof(byte)) {
            memcpy(pwm_buffer + channel, rgb_values, 3);
          }
          else {
            // have to map the values to pwmrange_t
            for (rgb_i = 0; rgb_i < 3; rgb_i++) {
              // slightly clever to get the max value of pwmrange_t assuming it is unsigned!
              pwm_buffer[channel + rgb_i] = map(rgb_values[rgb_i], 0, 255, 0, static_cast<pwmrange_t>(-1) );
              /*print(F("  buffer[i"));print(rgb_i);print(F("+c")); print(channel);print(F("] : "));
                print(rgb_values[rgb_i]);print(F(" = "));
                print( map(rgb_values[rgb_i], 0, 255, 0, static_cast<pwmrange_t>(-1) ) );
                println();
              */
            }
          }
        }
      }
    }
    // print(F("set dandelion["));print(dandelion_i);print(F("]"));for(byte i=0; i<16; i++) {print(" ");print(pwm_buffer[i]);}Serial.println();
#ifdef USING_PWM_TLC59711
    // Show the color settings
    static Every print_color(100);
    if (print_color() ) {
      for (int i = 0; i < 12; i++) {
        print(pwm_buffer[i]); print(( i % 3 == 2) ? F(" : ") : F(" "));
      }
      println();
    }
#endif

    PWM.set(dandelion_i, pwm_buffer);
  }
}

void show_patch(const byte** patch) {
  // indicate patch r,g,b,rgb on 1st/last pixel
  // Uses extant "sliders" list (assume reading them is off)
  /*
    print(F("sliders list @"));print((unsigned int) sliders);print(F("[z=0]@"));print((unsigned int) &sliders[0]);
    print(F(" [0][2]@"));print((unsigned int) &sliders[0].rgb[2]);
    print(F(" [1][0]@"));print((unsigned int) &sliders[1].rgb[0]);
    println();
    for(byte z=0; z<Zone_Count; z++) {
    print(F(" z"));print(z);print(F("@'s:"));
    for(byte i=0; i<3; i++) { print((unsigned int) (&sliders[z].rgb[i])); print(F(",")); }
    }
    println();
  */

  // highlight each zone
  for (byte i = 0; i < Zone_Count; i++) {
    print(F("Zone ")); print(i); println();
    byte rgb_bits = i % 6 + 1; // up to 6 zones mapped to 1..6, 3bits where 1 or 2 bits are on at a time

    for (byte xi = 0; xi < Zone_Count; xi++) {
      // set this zone to interesting value
      if (xi == i) {
        sliders[i].rgb[0] = (rgb_bits & 0b001) ? 100 : 0;
        sliders[i].rgb[1] = (rgb_bits & 0b010) ? 100 : 0;
        sliders[i].rgb[2] = (rgb_bits & 0b100) ? 100 : 0;
        print(F("  want 0b")); print(rgb_bits, BIN); print(F("=>rgb @")); print((unsigned int) &sliders[i].rgb[0]); print(F(" "));
        for (byte x = 0; x < 3; x++) {
          print(sliders[i].rgb[x]);
          print(F(","));
        }
        println();
      }
      // Set other zones to 0
      else {
        // print(F("  zap "));print(i);print(F(" @"));print((unsigned int)&sliders[i]);println();
        memset(sliders[xi].rgb, 0, 3);
      }
    }

    update_by_dandelion(sliders, patch);
    delay(1000);
  }

  print(F("resetting on purpose")); println();
  PWM.reset();
}


void performance(byte & patch_i) {
  const byte** patch = patches[patch_i];
  /*
    print(F("nlist* "));println((long)patch_names);
    print(F("n* "));println((long)patch_name_0);
    print(F("i "));println(patch_i);
    print(F("nlistr+i "));println((long) (patch_names + patch_i));
  */
  print(F("Patch ")); print(patch_i); print(F("@")); print((long)patch); print(F(" ")); Serial.print(patch_names[patch_i]); println();
  Every next_knob_check(200); // doesn't need to be static, we stay in a loop

  RGBPot::start_reading(sliders);
  delay(100);

  while (Serial.available() <= 0) {
    update_by_dandelion(sliders, patch);

    // Don't check knob each time, only every 300 or so
    if ( next_knob_check() ) {
      int new_patch_i = patch_selector.read();
#ifdef USING_PWM_TLC59711
      //pause so you can copy the printed values
      while (new_patch_i != 0) {
        new_patch_i = patch_selector.read();
      }
#endif
      if (new_patch_i != patch_i && new_patch_i != -1 && new_patch_i < Patch_Count) {
        patch_i = new_patch_i;
        patch = patches[patch_i];
      }
    }
  }
  RGBPot::stop_reading();
}

byte choose_patch(byte current) {
  print(F("current ")); println(current);
  for (byte i = 0; i < Patch_Count; i++) {
    print(i == current ? "=" : " "); print(" "); print(i); print(" "); Serial.print(patch_names[i]); println();
  }
  //println();
  //return;

  print(F(" = use knob")); println();
  print(F("Choose: "));
  int knob = -1 ;
  while (Serial.available() <= 0) {
    knob = patch_selector.read();
  }
  char choice = Serial.read();
  Serial.println(choice);
  if (choice == '=') {
    if (knob == -1) {
      print(F("Knob hadn't changed"));
      println();
      return current;
    }
    print(knob); println();
    choice = '0' + knob;
    return choice;
  }
  if (choice >= '0' && choice <= '0' + Patch_Count - 1) {
    print(F("Set to ")); print(choice); print(F(" ")); Serial.print(patch_names[choice - '0']); println();
    return choice - '0';
  }
  Serial.println(F("What?")); Serial.println();
  return current;
}
