#include "rainbow_fade_program.h"

#include "../rgb_lights.h"

#include <thread>

void RainbowFadeProgram::on_start() {}

void RainbowFadeProgram::loop() {
    switch (m_current_color) {
    case Color::RED:
        if (set_color(255, 0, 0))
            m_current_color = Color::ORANGE;
        break;
    case Color::ORANGE:
        if (set_color(255, 69, 0))
            m_current_color = Color::YELLOW;
        break;
    case Color::YELLOW:
        if (set_color(255, 255, 0))
            m_current_color = Color::GREEN;
        break;
    case Color::GREEN:
        if (set_color(0, 255, 0))
            m_current_color = Color::CYAN;
        break;
    case Color::CYAN:
        if (set_color(0, 255, 255))
            m_current_color = Color::BLUE;
        break;
    case Color::BLUE:
        if (set_color(0, 0, 255))
            m_current_color = Color::PURPLE;
        break;
    case Color::PURPLE:
        if (set_color(255, 0, 255))
            m_current_color = Color::RED;
        break;
    default:
        m_current_color = Color::RED;
        break;
    }

    m_app->set_color(m_r, m_g, m_b);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

void RainbowFadeProgram::on_interrupt(const std::vector<uint8_t>& data) {}

void RainbowFadeProgram::on_stop() {}

bool RainbowFadeProgram::set_color(uint8_t r, uint8_t g, uint8_t b) {
    if (r == m_r && g == m_g && b == m_b) {
        return true;
    }

    if (r > m_r)
        m_r += 1;
    else if (r < m_r)
        m_r -= 1;

    if (g > m_g)
        m_g += 1;
    else if (g < m_g)
        m_g -= 1;

    if (b > m_b)
        m_b += 1;
    else if (b < m_b)
        m_b -= 1;

    return false;
}