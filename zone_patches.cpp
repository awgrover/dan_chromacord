#include <Arduino.h>
#include "zone_patches.h"

unsigned long RGBPot::Sample_Interval=20;
const RGBPot* RGBPot::pot_list = NULL;
byte RGBPot::pot_list_count = 0;

