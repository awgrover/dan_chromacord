#include <MsTimer2.h>
#include "tired_of_serial.h"

struct RGBPot {
  byte rgb[3]; // scaled 0..255. can cast to (byte[3]) // Must be first for this cast

  public:
    const byte pin;
    const int vmin;
    const int vmax;

    RGBPot(byte read_pin, int analog_min, int analog_max) : pin(read_pin), vmin(analog_min), vmax(analog_max) {}
    void read(bool verbose=false) {
        if (verbose) {print(F("   read "));print(this->pin);}
        for(byte rgb_i=0; rgb_i < 3; rgb_i++) {
          this->rgb[rgb_i] = map(analogRead(this->pin + rgb_i), this->vmin, this->vmax, 0,255);
          if (verbose) {
            print(F(" "));print(rgb_i);print(F("="));print(this->rgb[rgb_i]);
            }
          }
        if (verbose) {println();}
        }

  // Class
    static unsigned long Sample_Interval; // milli-sec
    static const RGBPot* pot_list;
    static byte pot_list_count;

    template <int N> static void start_reading(const RGBPot (&_pot_list)[N]) {
      // Size of pot_list is auto-magic-N if it was explicitly declared.
      pot_list_count = N;
      pot_list = _pot_list;

      MsTimer2::set(Sample_Interval, RGBPot::read_pots);
      MsTimer2::start();
      }

    static void stop_reading() { MsTimer2::stop(); }

  // Class Private
  private:
    static void read_pots() {
      for (byte i = 0; i<pot_list_count; i++) {
        RGBPot &this_pot = (RGBPot&)(pot_list[i]);
        this_pot.read();
        }
      }

    // FIXME: consider smoothing
  };

