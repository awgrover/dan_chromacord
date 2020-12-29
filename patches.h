const byte Zone_Count = 4;

#define count_of(A) (sizeof(A)/sizeof(A[0]))
const byte patch_empty_zone[] = {0xFF};
const byte* empty_patch[Zone_Count] = {patch_empty_zone, patch_empty_zone, patch_empty_zone, patch_empty_zone};

// We have a list of patches
//  patches[N] = { patch... }
//  patch_names[N] = { F("name0"), ... } // cf print_pgm_string(patch_names,i)
// A Patch is a list of zones, an entry for the zone being the list of lunits for that zone
//  apatch[Zone_Count] = { <zone0>={ luna, lunb,..., 0xFF }, zone1, zone2, zone3... Zone_Count }
//  Note that a zone's-list-of-lunits is terminated by 0xFF, since those lists are of possibly varying lengths.
//  A list-of-lunits maybe 0 length: {0xFF}
// The code maps lunits to dandelions and outputs (in order).
// A lunit is a rgb "pixel": lunit0 is the first 3 dandelion outputs controlling rgb.
// The rgb of a lunit is controlled by the rgb slider of a zone.
// The rgb order is actually bgr
// Thus, zone0, slider0 controls the blue of each lunit in the zone0 list.
// The code maps lunits to dandeliions and outputs (in order).

// Generated by make patches.h:

// Patch built from check_sliders.patch
  const byte patch_check_sliders_z0[] = {0,1,2,3,4,0xFF};
const byte* patch_check_sliders[Zone_Count] = {patch_check_sliders_z0, patch_empty_zone, patch_empty_zone, patch_empty_zone};

// Patch built from [1] check_sliders.patch
  const byte patch_check_sliders_1_z1[] = {0,1,2,3,4,0xFF};
const byte* patch_check_sliders_1[Zone_Count] = {patch_empty_zone, patch_check_sliders_1_z1, patch_empty_zone, patch_empty_zone};

// Patch built from [2] check_sliders.patch
  const byte patch_check_sliders_2_z2[] = {0,1,2,3,4,0xFF};
const byte* patch_check_sliders_2[Zone_Count] = {patch_empty_zone, patch_empty_zone, patch_check_sliders_2_z2, patch_empty_zone};

// Patch built from [3] check_sliders.patch
  const byte patch_check_sliders_3_z3[] = {0,1,2,3,4,0xFF};
const byte* patch_check_sliders_3[Zone_Count] = {patch_empty_zone, patch_empty_zone, patch_empty_zone, patch_check_sliders_3_z3};

// Patch built from [across] check_sliders.patch
  const byte patch_check_sliders_across_z0[] = {0,1,2,3,4,0xFF};
  const byte patch_check_sliders_across_z1[] = {5,6,7,8,9,0xFF};
  const byte patch_check_sliders_across_z2[] = {10,11,12,13,14,0xFF};
const byte* patch_check_sliders_across[Zone_Count] = {patch_check_sliders_across_z0, patch_check_sliders_across_z1, patch_check_sliders_across_z2, patch_empty_zone};

// Patch built from [down] check_sliders.patch
  const byte patch_check_sliders_down_z0[] = {0,1,5,6,10,11,0xFF};
  const byte patch_check_sliders_down_z1[] = {2,7,11,0xFF};
  const byte patch_check_sliders_down_z2[] = {3,8,13,0xFF};
  const byte patch_check_sliders_down_z3[] = {4,9,14,0xFF};
const byte* patch_check_sliders_down[Zone_Count] = {patch_check_sliders_down_z0, patch_check_sliders_down_z1, patch_check_sliders_down_z2, patch_check_sliders_down_z3};

// Patch built from [hypercube1] dan.patch
  const byte patch_dan_hypercube1_z0[] = {0,4,5,9,10,14,0xFF};
  const byte patch_dan_hypercube1_z1[] = {1,13,0xFF};
  const byte patch_dan_hypercube1_z2[] = {2,7,12,0xFF};
  const byte patch_dan_hypercube1_z3[] = {3,6,8,11,0xFF};
const byte* patch_dan_hypercube1[Zone_Count] = {patch_dan_hypercube1_z0, patch_dan_hypercube1_z1, patch_dan_hypercube1_z2, patch_dan_hypercube1_z3};

// Patch built from [hypercube2] dan.patch
  const byte patch_dan_hypercube2_z0[] = {0,4,5,9,10,14,0xFF};
  const byte patch_dan_hypercube2_z1[] = {1,3,6,8,11,13,0xFF};
  const byte patch_dan_hypercube2_z2[] = {2,7,12,0xFF};
const byte* patch_dan_hypercube2[Zone_Count] = {patch_dan_hypercube2_z0, patch_dan_hypercube2_z1, patch_dan_hypercube2_z2, patch_empty_zone};

// Patch built from [all1] test.patch
  const byte patch_test_all1_z0[] = {0,1,2,3,4,0xFF};
const byte* patch_test_all1[Zone_Count] = {patch_test_all1_z0, patch_empty_zone, patch_empty_zone, patch_empty_zone};

// Patch built from [even_odd] test.patch
  const byte patch_test_even_odd_z0[] = {0,2,4,0xFF};
  const byte patch_test_even_odd_z1[] = {1,3,0xFF};
const byte* patch_test_even_odd[Zone_Count] = {patch_test_even_odd_z0, patch_test_even_odd_z1, patch_empty_zone, patch_empty_zone};

// Patch built from [zones_one_per] test.patch
  const byte patch_test_zones_one_per_z0[] = {0,0xFF};
  const byte patch_test_zones_one_per_z1[] = {1,0xFF};
  const byte patch_test_zones_one_per_z2[] = {2,0xFF};
  const byte patch_test_zones_one_per_z3[] = {3,0xFF};
const byte* patch_test_zones_one_per[Zone_Count] = {patch_test_zones_one_per_z0, patch_test_zones_one_per_z1, patch_test_zones_one_per_z2, patch_test_zones_one_per_z3};

const byte** patches[] = {patch_check_sliders, patch_check_sliders_1, patch_check_sliders_2, patch_check_sliders_3, patch_check_sliders_across, patch_check_sliders_down, patch_dan_hypercube1, patch_dan_hypercube2, patch_test_all1, patch_test_even_odd, patch_test_zones_one_per};

  const char patch_name_0[] PROGMEM = "check_sliders";
  const char patch_name_1[] PROGMEM = "check_sliders_1";
  const char patch_name_2[] PROGMEM = "check_sliders_2";
  const char patch_name_3[] PROGMEM = "check_sliders_3";
  const char patch_name_4[] PROGMEM = "check_sliders_across";
  const char patch_name_5[] PROGMEM = "check_sliders_down";
  const char patch_name_6[] PROGMEM = "dan_hypercube1";
  const char patch_name_7[] PROGMEM = "dan_hypercube2";
  const char patch_name_8[] PROGMEM = "test_all1";
  const char patch_name_9[] PROGMEM = "test_even_odd";
  const char patch_name_10[] PROGMEM = "test_zones_one_per";
PGM_P const  patch_names[] PROGMEM = {patch_name_0, patch_name_1, patch_name_2, patch_name_3, patch_name_4, patch_name_5, patch_name_6, patch_name_7, patch_name_8, patch_name_9, patch_name_10};

const byte Patch_Count = count_of(patches);

// End Generated by make patches.h
