#pragma once

#include <Wire.h>
#define TLC59116_WARNINGS 1
#include <TLC59116_Unmanaged.h>
#include <TLC59116.h>
#include "PWM_Pins.h"
#define USING_PWM_TLC59116

class PWM_TLC59116 : public PWM_Pins {
    // interface for PWM's on the PWM_TLC59116
    // We use the default clock/data pins for I2C.

  public:
    static constexpr int RANGE = (2 ^ 8) - 1;
    static constexpr int ChannelsPerDevice = 16;

    TLC59116Manager tlc = TLC59116Manager(); // (Wire, 5000); // defaults
    boolean inited = false;

    // pin names
    enum {
      pwm1, pwm2, pwm3, pwm4, pwm5, pwm6, pwm7, pwm8, pwm9, pwm10, pwm11, pwm12
      // ... to ChannelsPerDevice * number-of-tlc's
    };

    // For each pwm (e.g. pinMode())
    // Sadly, no sanity checks
    boolean begin(int pin) {
      // only need to init the connection, not each pin
      if (! inited) {
        tlc.init();

        inited = true;
      }

      return true;
    }

    // Give it a pin and int and you get PWM
    void set(int pin, byte brightness) {
      tlc[pin / ChannelsPerDevice].pwm(pin % ChannelsPerDevice, brightness);
    }
    void set(int pin, int brightness) {
      tlc[pin / ChannelsPerDevice].pwm(pin % ChannelsPerDevice, (byte) brightness);
    }

    // A float is 0.0 ... 1.0, which will be mapped to the RANGE
    void set(int pin, double brightness) {
      set(pin, (byte) (brightness * RANGE));
    }

    void commit() { } // the "managed" tlc does not need a commit

    void reset() {
      tlc.reset();
    }

    // set a range
    void set(byte led_num_start, byte led_num_end, byte pwm_value) {
      for (byte i = led_num_start; i <= led_num_end; i++) {
        set(i, pwm_value);
      }
    }
    void set(const byte device_i, const byte (&buffer)[ChannelsPerDevice] ) {
      // set all values at once, [ChannelsPerDevice+1] buffer size
      tlc[device_i].set_outputs( buffer );
    }
    
    int device_count() {
      return tlc.device_count();
    }
};
