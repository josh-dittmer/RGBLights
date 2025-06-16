#pragma once

#include "driver.h"

class PWMStripDriver : public Driver {
  public:
    PWMStripDriver() : Driver("PWMStrip"), m_init(false) {}
    ~PWMStripDriver() {}

    bool init() override;
    void write(uint8_t r, uint8_t g, uint8_t b) override;
    void shutdown() override;

  private:
    static const unsigned int PWM_PIN_R;
    static const unsigned int PWM_PIN_G;
    static const unsigned int PWM_PIN_B;

    bool m_init;
};