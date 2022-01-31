#include "tired_of_serial.h"
#include <RunningAverage.h>
#include <Every.h>
#include <Arduino.h>

struct RGBPot {
  private:
    RunningAverage *avg[3];

  public:
    byte rgb[3]; // scaled 0..255. can cast to (byte[3]) // Must be first for this cast
    const byte pin;
    const int vmin;
    const int vmax;

    RGBPot(byte read_pin, int analog_min, int analog_max)
      : pin(read_pin), vmin(analog_min), vmax(analog_max) {
      // 5 values for averaging
      for (byte rgb_i = 0; rgb_i < 3; rgb_i++) {
        avg[rgb_i] = new RunningAverage(10);
      }
    }

    void read(bool verbose = false, boolean performance=true) {
      //verbose = true;
      if (verbose) {
        print(this->pin);
        print(F(":"));
      }
      for (byte rgb_i = 0; rgb_i < 3; rgb_i++) {
        int raw = analogRead(this->pin + rgb_i);
        avg[rgb_i]->addValue(raw);
        int val = constrain(avg[rgb_i]->getAverage(), this->vmin, this->vmax);
        
        if ( !performance ) {
          // sliders appear to be in wrong order for RGB light-box mode
          byte _i;
          if ( rgb_i % 3 == 0) _i = rgb_i + 2;
          else if (rgb_i % 3 == 2) _i = rgb_i - 2;
          else _i = rgb_i;
          this->rgb[_i] = map(val, this->vmin, this->vmax, 0, 255);
        }
        else {
          this->rgb[rgb_i] = map(val, this->vmin, this->vmax, 0, 255);
        }
        
        if (verbose) {
          print(F(" ")); print(rgb_i); print(F("=")); print(raw); print(F("/")); print(this->rgb[rgb_i]);
        }
      }
      if (verbose) {
        println(F(" "));
      }
    }

    template <int N>
    static void dump(const RGBPot (&_pot_list)[N] ) {
      for (byte i = 0; i < N; i++) {
        print(F("[")); print(i); print(F("] rgb = ("));
        for (byte rgb_i = 0; rgb_i < 3; rgb_i++) {
          print(_pot_list[i].rgb[rgb_i]); print(F(","));
        }
        println(F(")"));
      }
    }

    template <int N>
    static void read_pots( RGBPot (&_pot_list)[N], boolean const performance) {
      // Size of pot_list is auto-magic-N if it was explicitly declared.
      static Every time_to_read(RGBPot::Sample_Interval);

      if ( time_to_read() ) {
        //println(F("Read pots..."));
        for (byte i = 0; i < N; i++) {
          _pot_list[i].read(false, performance);
        }
      }

      // FIXME: consider smoothing
    }
    // Class
    static unsigned long Sample_Interval; // milli-sec

    // Class Private
  private:

};
