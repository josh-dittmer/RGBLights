#include "program.h"

#include "../rgb_lights.h"

#include <iostream>

void Program::execute() {
    switch (m_state) {
    case State::RUNNING:
        loop();
        break;
    case State::INTERRUPTED: {
        std::unique_lock<std::mutex> lock(m_mutex);

        on_interrupt(m_interrupt_data);

        m_state = State::RUNNING;
        m_cv.notify_all();
        break;
    }
    }
}

void Program::interrupt(const std::vector<uint8_t>& data) {
    std::unique_lock<std::mutex> lock(m_mutex);

    m_logger.verbose("interrupt(): Triggering interrupt...");

    m_interrupt_data = data;
    m_state = State::INTERRUPTED;

    m_logger.verbose("interrupt(): Waiting for interrupt to finish...");

    m_cv.wait(lock);

    m_logger.verbose("interrupt(): Interrupt finished!");
}

/*void Program::pause() {
    std::unique_lock<std::mutex> lock(m_mutex);

    m_logger.verbose("pause(): Triggering pause...");

    m_state = State::PAUSING;

    m_logger.verbose("pause(): Waiting for program to pause...");

    m_cv.wait(lock);

    m_logger.verbose("pause(): Pause finished!");

    m_last_r = m_app->get_state().m_color.m_r;
    m_last_g = m_app->get_state().m_color.m_g;
    m_last_b = m_app->get_state().m_color.m_b;

    m_app->set_color(0, 0, 0);
}

void Program::unpause() {
    std::unique_lock<std::mutex> lock(m_mutex);

    if (m_state != State::PAUSED) {
        m_logger.warn("unpause(): Program is not paused!");
        return;
    }

    m_logger.verbose("unpause(): Unpausing...");

    m_app->set_color(m_last_r, m_last_g, m_last_b);

    m_state = State::RUNNING;
}*/