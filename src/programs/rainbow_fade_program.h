#pragma once

#include "program.h"

class RGBLights;

class RainbowFadeProgram : public Program {
  public:
    RainbowFadeProgram(RGBLights* app)
        : Program(app), m_current_color(Color::RED), m_r(0), m_g(0), m_b(0) {}
    ~RainbowFadeProgram() {}

    void on_start() override;
    void loop() override;
    void on_interrupt(const std::vector<uint8_t>& data) override;
    void on_stop() override;

  private:
    bool set_color(uint8_t r, uint8_t g, uint8_t b);

    enum class Color {
        RED,
        ORANGE,
        YELLOW,
        GREEN,
        CYAN,
        BLUE,
        PURPLE
    } m_current_color;

    uint8_t m_r;
    uint8_t m_g;
    uint8_t m_b;
};