class ZonePixels {
  public:
    const int pixel_ct;
    byte* pixels; // each pixel is a rgb

    template <int N> ZonePixels(byte _pixels[N]) : pixel_ct(N), pixels(_pixels) {};
    ZonePixels(void *x) : pixel_ct(0), pixels(NULL) {};
  private:
    ZonePixels();
    // ZonePixels(const ZonePixels&);
    ZonePixels& operator=(const ZonePixels&);
  };


class RGBPot {
  public:
    const byte pin;
    const int vmin;
    const int vmax;

    RGBPot(byte read_pin, int analog_min, int analog_max) : pin(read_pin), vmin(analog_min), vmax(analog_max) {}
  };

