#pragma once

// Just do one for now
#include "Adafruit_TLC59711.h"

#include "PWM_Pins.h"
#define USING_PWM_TLC59711

#include <Adafruit_SPIDevice.h>

class PWM_TLC59711 : public PWM_Pins {
    // interface for PWM's on the TLC59711
    // We use the default clock/data pins for SPI.
    int number;
    
  public:
    static constexpr uint16_t RANGE = 0xFFFF;
    static constexpr int ChannelsPerDevice = 12;

    Adafruit_TLC59711 *tlc; // default spi clock/data
    boolean inited = false;

    // pin names
    enum {
      pwm1, pwm2, pwm3, pwm4, pwm5, pwm6, pwm7, pwm8, pwm9, pwm10, pwm11, pwm12
    };

    PWM_TLC59711(int number) :number(number) {
      tlc = new Adafruit_TLC59711(number);
    }
    PWM_TLC59711(int number, int clockpin, int datapin) :number(number) {
      //tlc = new Adafruit_TLC59711(number, clockpin, datapin);
      tlc = new Adafruit_TLC59711(number, new Adafruit_SPIDevice(-1, clockpin, -1, datapin, 1000000/2));
    }
    
    // For each pwm (e.g. pinMode())
    // Sadly, no sanity checks
    boolean begin(int pin) {
      // only need to init the connection, not each pin
      if (! inited) {
        tlc->begin();
        tlc->write();

        inited = true;
      }

      return true;
    }

    // Give it a pin and int and you get PWM
    //void set(int pin, int brightness) { tlc->setPWM(pin, (uint16_t) brightness); }
    void set(int pin, uint16_t brightness) {
      //print("@");print(pin);print(F(" "));print(brightness);println();
      tlc->setPWM(pin, ~brightness); // nb: we use inverting drivers
      commit();
    }

    // A float is 0.0 ... 1.0, which will be mapped to the RANGE
    void set(int pin, double brightness) {
      //print("  @");print(pin);print(F(" "));print(brightness);println();
      set(pin, (uint16_t) (brightness * RANGE) );
    }

    void commit() {
      tlc->write();
    }

    void reset() {
      // there is no .reset in the lib, fake it
      tlc->simpleSetBrightness(127);
      for (int i = 0; i < device_count() * ChannelsPerDevice; i++) {
        tlc->setPWM(i, 0U);
      }
      commit();
    }

    // set a range
    void set(int led_num_start, int led_num_end, uint16_t pwm_value) {
      for (int i = led_num_start; i <= led_num_end; i++) {
        set(i, pwm_value);
      }
      commit();
    }
    void set(const int device_i, const uint16_t buffer[ChannelsPerDevice] ) {
      // set all values at once, [ChannelsPerDevice+1] buffer size
      // we are ignoring device_i here (till we figure out what more than 1 means)
      for (int i = 0; i < ChannelsPerDevice; i++) {
        set(i, buffer[i]);
      }
      commit();
    }

    constexpr int device_count() {
      return number;
    }
};
