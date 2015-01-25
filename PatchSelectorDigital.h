#include "tired_of_serial.h"

class PatchSelectorDigital {
  // first pass at an API for patch-selector: read(). internally deals with where that comes from
  const static int KnobCt=2;
  const static int KnobBits=4;
  const static int PinCt=KnobBits * KnobCt;
  const static int FirstPin= 53 - PinCt + 1; // higher-pins easier to access
  const static int DebounceTime=50; // value has to be stable for this time

  int was;
  unsigned long debounce_start;
  int was_debounce;

  public:
    PatchSelectorDigital() : was(-1), debounce_start(0), was_debounce(-1) {}

    // You should call init() in setup(). It also gets the current patch setting.
    // Patch-setting is an signed int: -1 indicates unknown/indeterminate. positives are the setting.
    int init() {
      // print("Init knobs ");print(KnobCt);print(" with bits each ");print(KnobBits);println();
      for(int i=this->FirstPin; i<this->FirstPin + this->PinCt; i++) {
        pinMode(i,INPUT_PULLUP); // knob pins are inverted: open is HIGH, closed is low
        // print("  pin ");print(i);println();
        }
      this->debounced();
      return this->_read();
      }

    int read() {
      // Returns the debounced value (so delayed!)
      // Returns -1 if we haven't debounced the initial value
      if (debounced()) {
        // print("Debounced ");print(this->was_debounce);println();
        this->was = this->was_debounce;
        }
      return this->was;
      }
   
    bool debounced() {
      // print("Debounce: ");
      // restart debounce if changed & not debounced-time
      int is = _read();
        // print(was_debounce,BIN);print(" vs=");print(is,BIN);print(" ");
      if (was_debounce != is) {
        was_debounce = is;
        this->debounce_start = millis();
        // print("restart");println();
        return false;
        }

      // stable? return it
      if ((millis() - debounce_start) > DebounceTime) {
        // print(" stable after ");print(millis() - debounce_start);print(" ->");print(is);println();
        was_debounce = is;
        return true;
        }

      // Not stable
      // println();
      return false;
      }

  private:
    int _read() {
      // Never returns -1
      int v = 0;
      // NKK FR01-AR16 is counter-clockwise. So, build value in reverse to get clockwise
      int bit = 1<<(KnobBits * KnobCt - 1); // 0x1; start at 1 for clockwise
      // print(" (");
      for(int i=this->FirstPin; i<this->FirstPin +this->PinCt; i++) {
        if (!digitalRead(i)) { v |= bit; /* print(i); print(","); */}
        bit >>=1; // <<=1; left shift for clockwise
        }
      // print("=");print(v),print(") ");
      return v;
      }

  };
