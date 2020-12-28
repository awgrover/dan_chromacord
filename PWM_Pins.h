#pragma once

// The interface for PWM pin controllers

class PWM_Pins {
  // don't want to make abstract, because virtual may not optimize away to inlines

  public:

    // For each pwm (e.g. pinMode())
    // static boolean begin(int pin) {};

    // fixme: add a global-brightness
    // which is a *x for native

    // Give it a pin and int and you get PWM
    void set(int pin, int state) {};
    void set(int pin, float state) {};

    // for things like i2c/spi, may need a "doit" because changes are batched
    void commit() {};
  };
