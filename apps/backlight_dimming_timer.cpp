#include "backlight_dimming_timer.h"
#include "global_preferences.h"

BacklightDimmingTimer::BacklightDimmingTimer() :
  Timer(GlobalPreferences::sharedGlobalPreferences()->idleBeforeDimmingSeconds()*1000/Timer::TickDuration)
{
}

bool BacklightDimmingTimer::fire() {
  m_dimBacklightBrightness = GlobalPreferences::sharedGlobalPreferences()->dimBacklightBrightness();
  m_decreasetime = GlobalPreferences::sharedGlobalPreferences()->dimmerDuration() * 1000;
  if (m_dimerExecutions == 0) {
    m_brightnessLevel = GlobalPreferences::sharedGlobalPreferences()->brightnessLevel();
    m_dimerSteps = m_brightnessLevel / decreaseBy;
    m_timeToSleep = m_decreasetime / m_dimerSteps;
    m_period = m_timeToSleep / Timer::TickDuration;
    if (m_period == 0) {
      m_period = 1;
    }
    resetTimer();
  }
  if (m_dimerExecutions < m_dimerSteps) {
    m_nextbrightness = (m_brightnessLevel-m_dimBacklightBrightness)/m_dimerSteps * (m_dimerSteps-m_dimerExecutions);
    Ion::Backlight::setBrightness(m_nextbrightness);
    resetTimer();
  } else if (m_dimerExecutions == m_dimerSteps) {
    Ion::Backlight::setBrightness(m_dimBacklightBrightness);
  }
  m_dimerExecutions++;
  return false;
}

void BacklightDimmingTimer::reset(uint32_t NewPeriod) {
  if(NewPeriod != (uint32_t)-1){
    m_idleBeforeDimmingDuration = NewPeriod * Timer::TickDuration;
  }
  m_dimerExecutions = 0;
  m_period = m_idleBeforeDimmingDuration / Timer::TickDuration;
  resetTimer();
}

void BacklightDimmingTimer::resetTimer() {
  BacklightDimmingTimer::m_numberOfTicksBeforeFire = BacklightDimmingTimer::m_period;
}
