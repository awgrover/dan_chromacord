#include <Arduino.h>

#define D Serial.print
#define EOL Serial.println()

class Slopifier {
  public:
  // Classifies a series of values:
  // Reversed-Up
  // Reverse-Down
  // Max/Min

  // We analyze the history using a history-window,
  // and doing math on halves of it
  static const byte HalfClassificationLength = 5;
  static const byte ClassificationLength = 2 * HalfClassificationLength;
  int history[ClassificationLength];
  byte head; // we cheat: assume that we've pushed at least a full buffer, so, no tail.
  bool primed; // goes true when history is full-enough
  byte tail() { return (head + 1) % ClassificationLength; }
  int maxv;
  int minv;
  bool read_reversed;

  static const int Slope_Min = 5; // slope across the history samples to count as a slope, not noise
  static const int Flat_Min = 2; // slope across the history samples to count as a Flat, not noise

  enum Direction {None, Up, Down, Flat};
  Direction shape[3]; // shape is [0]->[1]->has_direction()

  Slopifier() { 
    head = 0;
    primed = false; 
    minv = 10000; 
    shape[0]=None;shape[1]=None;shape[2]=None;
    read_reversed = true;
    D("Init"); D(primed); EOL; 
    }
  ~Slopifier(){}

  Slopifier& operator<<(int newvalue) { 
    history[head] = newvalue; 
    // D("  @");D(head);D("<<");D(newvalue);D(primed ? " +" : " -");EOL;
    head = (head + 1) % ClassificationLength;
    if (!primed) { if (head == 0) {primed=true; D("Primed"); EOL;} } // eh, we miss one
    // update shape
    else {
      Direction now = has_direction();
      if (now) {
        // debug_direction(now);
        if (now != shape[2]) { 
          // debug_history();
          shape[0]=shape[1]; shape[1]=shape[2]; shape[2]=now;
          // D("->");debug_direction(has_reversed());EOL;
          // "clear" the min max for this direction
          if (now == Down) minv = 10000;
          else if (now == Up) maxv = 0;
          read_reversed=false;
          }
        }
      if (maxv < newvalue) {maxv = newvalue; }
      if (minv > newvalue) {minv = newvalue; }
      }
    return *this;
    }

  Direction has_direction() {
    int slope = 
      average(tail(), HalfClassificationLength) -
      average((tail() + HalfClassificationLength) % ClassificationLength, HalfClassificationLength)
      ;
    // D("slope ");D(slope);D(" vs ");D(Slope_Min);EOL; 
    if (slope > Slope_Min) {return Down; }
    else if (-slope > Slope_Min) return Up;
    else if (abs(slope) < Flat_Min) return Flat;
    else return None;
    }

  Direction has_reversed() {
    read_reversed=true;
    if (shape[0] == Up && shape[1] == Flat && shape[2] == Down) return Up;
    else if (shape[0] == Down && shape[1] == Flat && shape[2] == Up) return Down;
    else if (shape[0] == Flat && shape[1] == Flat && shape[2] == Flat) return Flat;
    else return None;
    }

  int average(byte start, byte len) {
    int sum=0;
    for (byte i=0; i < len; i++) { sum += history[(i+start) % ClassificationLength]; }
    // D("avg from ");D(start);D(" for ");D(len);D(" sum ");D(sum);D(" avg ");D(sum/len);EOL;
    return sum / len;
    }

  private:
    Slopifier(const Slopifier&); // none
    Slopifier& operator=(const Slopifier&); // none

    void debug_direction(Direction x) {
      const char* names[] = {"None","Up","Down","Flat"};
      D(names[x]);
      D(" ");
      }
    void debug_history() {
      debug_direction(shape[2]);
      for(byte i=0;i<ClassificationLength;i++) { D(history[(i+tail()) % ClassificationLength]); D(" "); }
      EOL;
      }
  };
