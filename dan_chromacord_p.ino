#include <hsv2rgb.h>
#include <pixeltypes.h>
#include <Wire.h>
#include <TLC59116.h>

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

// CHSV hsv;
CRGB rgb;
CRGB rgb2;
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

byte* rgb2array(CRGB &rgb) {
  // 3 bytes {r,g,b}
  static byte as_array[3];
  as_array[0] = rgb.red;
  as_array[1] = rgb.green;
  as_array[2] = rgb.blue;
  return as_array;
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

    default:
      Serial.println();
      // menu made by: make 
      // include dan_callibration.ino.menu
      // end-menu
Serial.println(F("0  1st 3 rgb on/off/hsv sanity"));
Serial.println(F("r  Reset"));
Serial.println(F("s  Spiral through HSB color Space"));
Serial.println(F("p  Pwm full range"));

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
  rgb = CHSV( 50, 128, 128);
  tlc_first.pwm(0, 3, rgb2array(rgb))
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
          rgb = CHSV(h,s,v);
          rgb2 = CHSV(second_offset-h,s,v);
          for (TLC59116** a_tlc = tlc; *a_tlc; a_tlc++) {
            // for each rgb
            byte *as_array = rgb2array(rgb);
            tlc_first.pwm(0,3,as_array);
            tlc_first.pwm(3,3,rgb2array(rgb2));
            delay(20);
            /*
            for (byte group=0; group < max_group_led; group += 3) {
              (*a_tlc)->pwm(group, 3, as_array);
              }
            */
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
