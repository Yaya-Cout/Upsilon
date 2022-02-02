#ifndef APPS_BACKLIGHT_DIMMING_TIMER_H
#define APPS_BACKLIGHT_DIMMING_TIMER_H

#include "global_preferences.h"
#include <escher.h>

class BacklightDimmingTimer : public Timer {
public:
  BacklightDimmingTimer();
  void reset(uint32_t NewPeriod = -1);
private:
  constexpr static int decreaseBy = 15;
  int m_dimerExecutions = 0;
  int m_dimBacklightBrightness;
  int m_idleBeforeDimmingDuration; // In miliseconds
  int m_decreasetime; // In miliseconds
  int m_brightnessLevel;
  int m_dimerSteps;
  int m_nextbrightness;
  float m_timeToSleep; // In miliseconds
  bool fire() override;
  void resetTimer();
};

#endif

