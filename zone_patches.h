#include <MsTimer2.h>

struct RGBPot {
  byte rgb[3]; // scaled 0..255. can cast to (byte[3]) // Must be first for this cast

  public:
    const byte pin;
    const int vmin;
    const int vmax;

    RGBPot(byte read_pin, int analog_min, int analog_max) : pin(read_pin), vmin(analog_min), vmax(analog_max) {}

  // Class
    static unsigned long Sample_Interval; // milli-sec
    static const RGBPot* pot_list;
    static byte pot_list_count;

    template <int N> static void start_reading(const RGBPot (&_pot_list)[N]) {
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
        for(byte rgb_i=0; rgb_i < 3; rgb_i++) {
          this_pot.rgb[i] = map(analogRead(this_pot.pin + rgb_i), this_pot.vmin, this_pot.vmax, 0,255);
          }
        }
      }

    // FIXME: consider smoothing
  };
