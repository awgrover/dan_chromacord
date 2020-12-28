#pragma once

#ifndef NeoNumPixels
  #define NeoNumPixels 4 // 1 per zone
#endif
#ifndef NeoI2CPin
  #define NeoI2CPin 6 // typical
#endif

#include <Adafruit_NeoPixel.h>
#include <pwm/PWM_Pins.h>
#include <pwm/RGB.h>

class PWM_NeoPixel : public PWM_Pins {
    // interface for PWM's on a neopixel "Strip"
    // We use the default i2c pins

  public:
    static constexpr int RANGE = (2 ^ 8) - 1;

    // just do 4 for now
    Adafruit_NeoPixel neo = Adafruit_NeoPixel(NeoNumPixels, NeoI2CPin /* pin */, NEO_GRB + NEO_KHZ800);
    boolean inited = false;

    // pin names, end up getting "folded" into rgb per neo
    // pwm1 = pixel[0].red, pwm2=pixel[0].green etc
    enum {
      pwm1, pwm2, pwm3, pwm4, pwm5, pwm6, pwm7, pwm8, pwm9, pwm10, pwm11, pwm12
    };

    // Sadly, no sanity checks
    boolean begin(int pin) {
      // only need to init the connection, not each pin
      if (! inited) {
        neo.begin();
        neo.clear();
        neo.show();
        inited = true;
      }

      return true;
    }

    // Give it a pin and int and you get PWM
    void set(int pin, int brightness) {
      int pixel_i = pin / 3;
      uint32_t rgb_int = neo.getPixelColor( pixel_i );
      RGB<uint8_t> rgb_parts;
      decompose_rgb( rgb_int, rgb_parts.red, rgb_parts.green, rgb_parts.blue );
      rgb_parts[ pin % 3 ] = brightness;
      neo.setPixelColor( pixel_i, rgb_parts.red, rgb_parts.green, rgb_parts.blue);
    }
    // A float is 0.0 ... 1.0, which will be mapped to the RANGE
    void set(int pin, float brightness) {
      set(pin, (int) brightness * RANGE);
    }

    void commit() {
      neo.show();
    }

    void decompose_rgb(uint32_t rgb, uint8_t &r, uint8_t &g, uint8_t &b ) {
      // update r,g,b as the 8bit parts of the int rgb
      r = (rgb & 0xFF0000) >> 16;
      g = (rgb & 0x00F00) >> 8;
      b = rgb & 0x0000FF;
    }

    void demo() {
      // show if rgb is correct
      uint32_t cycle[4] = { 0x880000, 0x008800, 0x000088, 0x888888 };
      for ( uint32_t color : cycle ) {
        for(int i=0; i<NeoNumPixels; i++) {
          neo.setPixelColor(i, color );
        }
        neo.show();
        delay(1000);
      }
      neo.clear();
      neo.show();
    }
};
