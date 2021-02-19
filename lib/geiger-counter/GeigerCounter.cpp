#include "Arduino.h"
#include "GeigerCounter.h"


bool GeigerCounter::hasData(unsigned long currentTime) {
  return currentTime >= 60000 && _head != nullptr;
}

void GeigerCounter::addPulse(unsigned long pulseTime) {
  auto *measure = new Pulse;
  measure->time = pulseTime;
  measure->next = _head == nullptr ? nullptr : _head;
  _head = measure;
}

double GeigerCounter::getPulsesPerSecond(unsigned long currentTime) {
  double pulses = 0;

  Pulse *sumIterator;
  Pulse *cleanIterator = nullptr;

  unsigned long thresholdTime = currentTime - 60000;

  if (_head->time <= thresholdTime) {
    if (_head->next != nullptr) {
      cleanIterator = _head->next;
    }
    delete _head;
    _head = nullptr;
  } else {
    sumIterator = _head;
    while (true) {
      if (sumIterator->next != nullptr && sumIterator->next->time <= thresholdTime) {
        cleanIterator = sumIterator->next;
        sumIterator->next = nullptr;
      }

      pulses += 1;

      if (sumIterator->next != nullptr) {
        sumIterator = sumIterator->next;
      } else {
        break;
      }
    }
  }

  if (cleanIterator != nullptr) {
    Pulse *tmp;

    while (true) {
      if (cleanIterator->next == nullptr) {
        delete cleanIterator;
        break;
      }

      tmp = cleanIterator;
      cleanIterator = cleanIterator->next;
      delete tmp;
    }
  }

  return pulses / 60;
}
