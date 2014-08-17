const byte Zone_Count = 4;

// pattern: const ZonePixels <nameit>[Zone_Count] = {
const ZonePixels test_patch_all[Zone_Count] = {
  // these are pixel numbers, skip a pixel and it will always be zero'd.
  ZonePixels((byte[]){0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}),
  ZonePixels(NULL),
  ZonePixels(NULL),
  ZonePixels(NULL)
  };
const ZonePixels test_patch_skip1[Zone_Count] = {
  ZonePixels((byte[]){0,2,4,6,8,10,12,14}),
  ZonePixels((byte[]){1,3,5,9,11,13,15}),
  ZonePixels(NULL),
  ZonePixels(NULL)
  };

#define count_of(A) (sizeof(A)/sizeof(A))
const ZonePixels* patches[] = { test_patch_all, test_patch_skip1 };
const byte Patch_Count = count_of(patches);

