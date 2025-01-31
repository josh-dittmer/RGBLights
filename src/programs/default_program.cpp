#include "default_program.h"

#include "../rgb_lights.h"

#include <iostream>
#include <thread>

void DefaultProgram::on_start() {
    m_app->set_color(m_target_r, m_target_g, m_target_b);
}

void DefaultProgram::loop() {
    static const int fade_amount = 10;

    int r = m_app->get_state().m_color.m_r;
    int g = m_app->get_state().m_color.m_g;
    int b = m_app->get_state().m_color.m_b;

    bool changed = false;

    if (r < m_target_r) {
        r += fade_amount;
        if (r > m_target_r)
            r = m_target_r;
        changed = true;
    } else if (r > m_target_r) {
        r -= fade_amount;
        if (r < m_target_r)
            r = m_target_r;
        changed = true;
    }

    if (g < m_target_g) {
        g += fade_amount;
        if (g > m_target_g)
            g = m_target_g;
        changed = true;
    } else if (g > m_target_g) {
        g -= fade_amount;
        if (g < m_target_g)
            g = m_target_g;
        changed = true;
    }

    if (b < m_target_b) {
        b += fade_amount;
        if (b > m_target_b)
            b = m_target_b;
        changed = true;
    } else if (b > m_target_b) {
        b -= fade_amount;
        if (b < m_target_b)
            b = m_target_b;
        changed = true;
    }

    if (changed) {
        m_app->set_color(r, g, b);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

void DefaultProgram::on_interrupt(const std::vector<uint8_t>& data) {
    if (data.size() < 3) {
        get_logger().debug("Bad interrupt data!");
        return;
    }

    get_logger().verbose("Received color data: " + std::to_string(data[0]) +
                         ", " + std::to_string(data[1]) + ", " +
                         std::to_string(data[2]));

    m_target_r = data[0];
    m_target_g = data[1];
    m_target_b = data[2];
}

void DefaultProgram::on_stop() { m_app->set_color(0, 0, 0); }