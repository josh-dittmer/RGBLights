#pragma once

#include "program.h"

#include <vector>

class RGBLights;

class DefaultProgram : public Program {
  public:
    DefaultProgram(RGBLights* app, uint8_t default_r, uint8_t default_g,
                   uint8_t default_b)
        : Program(app), m_target_r(default_r), m_target_g(default_g),
          m_target_b(default_b) {}
    ~DefaultProgram() {}

    void on_start() override;
    void loop() override;
    void on_interrupt(const std::vector<uint8_t>& data) override;
    void on_stop() override;

  private:
    uint8_t m_target_r;
    uint8_t m_target_g;
    uint8_t m_target_b;
};