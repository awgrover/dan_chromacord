#include <Arduino.h>

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
  byte tail() { return (head + (ClassificationLength-1)) % ClassificationLength; }
  int maxv;
  int minv;

  static const int Slope_Min = 5; // slope across the history samples to count as a slope, not noise
  static const int Flat_Min = 2; // slope across the history samples to count as a Flat, not noise

  enum Direction {None, Up, Down, Flat};
  Direction shape[3]; // shape is [0]->[1]->has_direction()

  Slopifier() { minv = 10000; };
  ~Slopifier(){}

  Slopifier& operator<<(byte newvalue) { 
    history[head] = newvalue; 
    head = (head + 1) % ClassificationLength;
    if (!primed) { if (head == 0) primed=true; } // eh, we miss one
    // update shape
    else {
      Direction now = has_direction();
      if (now) {
        if (now != shape[1]) { 
          shape[0]=shape[1]; shape[1]=shape[2]; shape[2]=now;
          // "clear" the min max for this direction
          if (now == Down) minv = 10000;
          else if (now == Up) maxv = 0;
          }
        }
      if (maxv < newvalue) maxv = newvalue;
      if (minv > newvalue) minv = newvalue;
      }
    return *this;
    }

  Direction has_direction() {
    int slope = 
      average(tail(), HalfClassificationLength) -
      average((tail() + HalfClassificationLength) % ClassificationLength, HalfClassificationLength)
      ;
    if (slope > Slope_Min) return Down;
    else if (-slope > Slope_Min) return Up;
    else if (abs(slope) < Flat_Min) return Flat;
    else return None;
    }

  Direction has_reversed() {
    if (shape[0] == Up && shape[1] == Flat && shape[2] == Down) return Up;
    else if (shape[0] == Down && shape[1] == Flat && shape[2] == Up) return Down;
    else if (shape[0] == Flat && shape[1] == Flat && shape[2] == Flat) return Flat;
    else return None;
    }

  int average(byte i, byte len) {
    int sum;
    for (;i < len; i++) { sum += history[i % ClassificationLength]; }
    return sum / len;
    }

  private:
    Slopifier(const Slopifier&); // none
    Slopifier& operator=(const Slopifier&); // none
  };

