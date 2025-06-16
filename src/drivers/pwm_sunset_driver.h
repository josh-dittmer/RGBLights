#pragma once

#include "driver.h"

class PWMSunsetDriver : public Driver {
  public:
    PWMSunsetDriver() : Driver("PWMSunset"), m_init(false) {}
    ~PWMSunsetDriver() {}

    bool init() override;
    void write(uint8_t r, uint8_t g, uint8_t b) override;
    void shutdown() override;

  private:
    static const unsigned int PWM_PIN_R;
    static const unsigned int PWM_PIN_G;
    static const unsigned int PWM_PIN_B;
    static const unsigned int PWM_PIN_W;

    bool m_init;
};