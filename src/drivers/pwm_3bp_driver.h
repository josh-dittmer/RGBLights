#pragma once

#include "driver.h"

class PWM3BPDriver : public Driver {
  public:
    PWM3BPDriver() : Driver("PWM3BP"), m_init(false) {}
    ~PWM3BPDriver() {}

    bool init() override;
    void write(uint8_t r, uint8_t g, uint8_t b) override;
    void shutdown() override;

  private:
    static const unsigned int PWM_PIN_R;
    static const unsigned int PWM_PIN_G;
    static const unsigned int PWM_PIN_B;

    bool m_init;
};