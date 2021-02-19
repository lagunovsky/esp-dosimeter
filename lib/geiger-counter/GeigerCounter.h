#ifndef DOSIMETER_GEIGERCOUNTER_H
#define DOSIMETER_GEIGERCOUNTER_H

struct Pulse {
  unsigned long time;
  Pulse *next;
};

class GeigerCounter {

protected:
  Pulse *_head = nullptr;

public:
  void addPulse(unsigned long time);

  bool hasData(unsigned long currentTime);

  double getPulsesPerSecond(unsigned long currentTime);
};


#endif //DOSIMETER_GEIGERCOUNTER_H
