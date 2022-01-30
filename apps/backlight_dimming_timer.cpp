#include "backlight_dimming_timer.h"
#include "global_preferences.h"

BacklightDimmingTimer::BacklightDimmingTimer() :
  Timer(GlobalPreferences::sharedGlobalPreferences()->idleBeforeDimmingSeconds()*1000/Timer::TickDuration)
{
}

bool BacklightDimmingTimer::fire() {
  k_dimBacklightBrightness = GlobalPreferences::sharedGlobalPreferences()->dimBacklightBrightness();
  if (m_dimerExecutions == 0) {
    m_brightnessLevel = GlobalPreferences::sharedGlobalPreferences()->brightnessLevel();
    m_dimerSteps = m_brightnessLevel / decreaseBy;
    m_timeToSleep = decreasetime / m_dimerSteps;
    m_period = m_timeToSleep / Timer::TickDuration;
    if (m_period == 0) {
      m_period = 1;
    }
    resetTimer();
  }
  if (m_dimerExecutions < m_dimerSteps) {
    m_nextbrightness = (m_brightnessLevel-k_dimBacklightBrightness)/m_dimerSteps * (m_dimerSteps-m_dimerExecutions);
    Ion::Backlight::setBrightness(m_nextbrightness);
    resetTimer();
  } else if (m_dimerExecutions == m_dimerSteps) {
    Ion::Backlight::setBrightness(k_dimBacklightBrightness);
  }
  m_dimerExecutions++;
  return false;
}

void BacklightDimmingTimer::reset(uint32_t NewPeriod) {
  if(NewPeriod != (uint32_t)-1){
    // m_period = NewPeriod;
    k_idleBeforeDimmingDuration = NewPeriod * Timer::TickDuration;
  }
  m_dimerExecutions = 0;
  m_period = k_idleBeforeDimmingDuration / Timer::TickDuration;
  resetTimer();
}

void BacklightDimmingTimer::resetTimer() {
  BacklightDimmingTimer::m_numberOfTicksBeforeFire = BacklightDimmingTimer::m_period;
}
