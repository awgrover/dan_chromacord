#include <MsTimer2.h>

class RGBPot {
  public:
    const byte pin;
    const int vmin;
    const int vmax;
    byte value; // scaled 0..255

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
        this_pot.value = map(analogRead(this_pot.pin), this_pot.vmin, this_pot.vmax, 0,255);
        }
      }

    // FIXME: consider smoothing
  };

