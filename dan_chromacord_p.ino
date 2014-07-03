// #include <hsv2rgb.h>
// #include <pixeltypes.h>
#include <Wire.h>
#include <TLC59116.h>
#include <MsTimer2.h>

#define start_sequence { \
  static byte sequence_i = 0; \
  static unsigned long sequence_next = 0; \
  if (millis() >= sequence_next) { \
    /* Serial.print(F("Sequence "));Serial.println(sequence_i); */ \
    switch (sequence_i) { \
      case -1 : /* dumy */
#define sequence(sofar, fn, delay_millis) break; \
      case sofar : sequence_i++; sequence_next = millis() + delay_millis; fn;
#define end_sequence \
      default: sequence_i=0; /* wrap i */ \
      } \
    /* Serial.print(F("Next seq "); Serial.print(sequence_i); Serial.print(F(" in "));Serial.println(sequence_next - millis()); */ \
    } \
  }
#define end_do_sequence end_sequence }

TLC59116 tlc_first;
TLC59116* tlc[14]; // null'd

void setup() {
  TLC59116::DEBUG=1;   // change to 0 to skip warnings/etc.
  // (open the serial monitor window).
  Serial.begin(115200);
  Wire.begin();
  TLC59116::reset();
  tlc_first.enable_outputs();

  TLC59116::Scan scanner = TLC59116::scan();
  byte tlc_count = 0;
  for (byte i=0; i<scanner.count(); i++) {
    byte candidate = scanner.device_addresses()[i];
    if (TLC59116::is_single_device_addr(candidate)) {
      tlc[i] = new TLC59116(candidate );
      tlc_count++;
      }
    }
  Serial.print("Made tlc[0..");Serial.print(tlc_count-1);Serial.print("] out of "); Serial.println(scanner.count());
  }

void loop() {
  static char test_num = '0'; // idle pattern
  switch (test_num) {

    case 0xff: // prompt
      Serial.print("Choose (? for help): ");
      test_num = NULL;
      break;

    case NULL: // means "done with last, do nothing"
      break; // do it


    case '0': // 1st 3 rgb on/off/hsv sanity
      prove_on();
      break;

    case 'o': // on
      tlc_first.on(0).on(1).on(2);
      while (Serial.available() <= 0) {}
      test_num = 0xff;
      break;

    case 'r': // Reset
      tlc_first.reset();
      tlc_first.reset_shadow_registers();
      tlc_first.enable_outputs(); // before reset_shadow!
      test_num = 0xff;
      break;

    case 's': // Spiral through HSB color Space
      tlc_first.reset();
      tlc_first.reset_shadow_registers();
      tlc_first.enable_outputs(); // before reset_shadow!
      spiral();
      test_num = 0xff;
      break;
      
    case 'p' : // Pwm full range
      pwm_full_range();
      test_num = 0xff;
      break;

    case 'z' : // huh
      while(Serial.available() <= 0) {
        }
      test_num = 0xff;
      break;

    case 'g' : // Go into performance mode
      performance();
      test_num = 0xfe;
      break;

    case 't' : // Timer test: make something blink every n
      MsTimer2::set(200, on_off_isr);
      MsTimer2::start();
      while(Serial.available() <= 0) {
        Serial.println(analogRead(3));
        }
      MsTimer2::stop();
      test_num = 0xfe;
      break;
      
    default:
      Serial.println();
      // menu made by: make 
      // include *.ino.menu
Serial.println(F("0  1st 3 rgb on/off/hsv sanity"));
Serial.println(F("o  on"));
Serial.println(F("r  Reset"));
Serial.println(F("s  Spiral through HSB color Space"));
Serial.println(F("p  Pwm full range"));
Serial.println(F("z  huh"));
Serial.println(F("g  Go into performance mode"));
      // end-menu

      Serial.println(F("? Prompt again"));
      test_num = 0xFF; // prompt
    }

  // Change test_num?
  if (Serial.available() > 0) {
    test_num = Serial.read();
    Serial.println(test_num);
    }

  }

void prove_on() {
  Serial.println("TOP");
  tlc_first
  // .pwm(0, 3, (byte[]) {255,255,255}).delay(500)
  .on(0).on(1).on(2)
  .delay(500);
  Serial.println("off...");
  tlc_first.off(0).off(1).off(2)
  .delay(500);
  tlc_first.pwm(0, 3, (byte[]){50, 128, 128})
  .delay(500);
  }

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

void on_off_isr() {
  static bool oo = 0;
  sei();
  tlc_first.on(1, oo);
  tlc_first.on(2, oo);
  oo = !oo;
  }

const byte Slider_ct = 3;
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

void performance() {
  const int break_check_interval = 500;

  // setup interrupt routine to read sliders
  MsTimer2::set(Sample_Interval, grab_next_slider);
  MsTimer2::start();

  // update dandelion, till serial.avail
  unsigned long break_check = millis() + break_check_interval; // every 1/2
  while(1) {

    for(byte slider_i =0; slider_i < Slider_ct; slider_i++) {
      // e.g. slider1 => red of zone[1] of rgbs of dandelions
      Serial.print("Slider ");Serial.print(slider_i);Serial.print(" ");Serial.println(Targets[slider_i]);
      accumulatepwm_rgb_groups( slider_i );
      }
    set_pwm(leds);

    if (millis() > break_check) {
      if (Serial.available() > 0) { break; }
      }
    }

  // cleanup interrupt
  MsTimer2::stop();
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
