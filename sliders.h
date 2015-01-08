const int POT_MIN = 0;
const int POT_MAX = 1024;
RGBPot sliders[Zone_Count] = {
  RGBPot(A0 + 0*3, POT_MIN, POT_MAX),
  RGBPot(A0 + 1*3, POT_MIN, POT_MAX),
  RGBPot(A0 + 2*3, POT_MIN, POT_MAX),
  RGBPot(A0 + 3*3, POT_MIN, POT_MAX),
  };
