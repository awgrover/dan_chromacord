#pragma once

#include "PWM_Pins.h"

class PWM_NativePin : public PWM_Pins {
  // interface for PWM's on the native pins
  // Acts like an object, but no storage, and inlines, so, efficient

  public:
    static constexpr int RANGE = 255;

    // For each pwm (e.g. pinMode())
    // Sadly, no sanity checks
    boolean begin(int pin) { 
      pinMode(pin, OUTPUT); 
      return true; 
    }

    // Give it a pin and int and you get PWM
    void set(int pin, int brightness) { analogWrite(pin, brightness); }
    // A float is 0.0 ... 1.0, which will be mapped to the RANGE
    void set(int pin, float brightness) { set(pin, (int) brightness * RANGE); }

    // commit is a no-op
    void commit() {}
  };

class PWM_NativePin_Uno : public PWM_NativePin {
    public:

      // pin names
      // fixme: possible this should just be a constexpr array
      enum {
        pwm1=3, pwm2=5, pwm3=6, pwm4=9, pwm5=10, pwm6=11
        };
    };

class PWM_NativePin_MEGA : public PWM_NativePin {
    public:

      // pin names
      // fixme: possible this should just be a constexpr array
      enum {
        pwm1=2, pwm2, pwm3, pwm4, pwm5, pwm6, pwm7, pwm8, pwm9, pwm10, pwm11, pwm12, 
        pwm13=44, pwm14=45, pwm15=46
        };
    };

