const int POT_MIN = 68;    // default=70   min tried=65
const int POT_MAX = 610;   // default=600  max tried=620
RGBPot sliders[Zone_Count] = {
  RGBPot(A0 + 0*3, POT_MIN, POT_MAX),
  RGBPot(A0 + 1*3, POT_MIN, POT_MAX),
  RGBPot(A0 + 2*3, POT_MIN, POT_MAX),
  RGBPot(A0 + 3*3, POT_MIN, POT_MAX),
  };
