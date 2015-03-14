const int POT_MIN = 30;
const int POT_MAX = 900;
RGBPot sliders[Zone_Count] = {
  RGBPot(A0 + 0*3, POT_MIN, POT_MAX),
  RGBPot(A0 + 1*3, POT_MIN, POT_MAX),
  RGBPot(A0 + 2*3, POT_MIN, POT_MAX),
  RGBPot(A0 + 3*3, POT_MIN, POT_MAX),
  };
