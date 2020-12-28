#pragma once

// Just do one for now
#define NUM_TLC59711 1
#include "Adafruit_TLC59711.h"

#include "PWM_Pins.h"

class PWM_TLC59711 : public PWM_Pins {
  // interface for PWM's on the TLC59711
  // We use the default clock/data pins for SPI.

  public:
    static constexpr int RANGE =(2^16) - 1;

    // just do 1 for now
    Adafruit_TLC59711 tlc = Adafruit_TLC59711(NUM_TLC59711); // default spi clock/data
    boolean inited = false;

      // pin names
      enum {
        pwm1, pwm2, pwm3, pwm4, pwm5, pwm6, pwm7, pwm8, pwm9, pwm10, pwm11, pwm12
        };

    // For each pwm (e.g. pinMode())
    // Sadly, no sanity checks
    boolean begin(int pin) { 
      // only need to init the connection, not each pin
      if (! inited) {
        tlc.begin();
        tlc.write();

        inited = true;
        }

      return true; 
    }

    // Give it a pin and int and you get PWM
    void set(int pin, int brightness) { tlc.setPWM(pin, brightness); }
    // A float is 0.0 ... 1.0, which will be mapped to the RANGE
    void set(int pin, float brightness) { set(pin, (int) brightness * RANGE); }

    void commit() { tlc.write(); }
  };
