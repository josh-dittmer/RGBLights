#pragma once

#include "driver.h"

class PWMZDriver : public Driver {
  public:
    PWMZDriver() : Driver("PWMZ"), m_init(false) {}
    ~PWMZDriver() {}

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