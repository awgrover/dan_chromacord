const byte Zone_Count = 4;

// pattern: const ZonePixels <nameit>[Zone_Count] = {

// const ZonePixels y( (const byte []){0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15});
const byte x[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

const ZonePixels test_patch_all[Zone_Count] = {
  // these are pixel numbers, skip a pixel and it will always be zero'd.
  ZonePixels(x),
  ZonePixels(),
  ZonePixels(),
  ZonePixels()
  };
const ZonePixels test_patch_skip1[Zone_Count] = {
  // this doesn't work in avrgcc
  ZonePixels((const byte []){0,2,4,6,8,10,12,14}),
  ZonePixels((byte[]){1,3,5,9,11,13,15}),
  ZonePixels(),
  ZonePixels()
  };

#define count_of(A) (sizeof(A)/sizeof(A[0]))
const ZonePixels* patches[] = { test_patch_all, test_patch_skip1 };
const byte Patch_Count = count_of(patches);

