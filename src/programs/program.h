#pragma once

#include <homecontroller/util/logger.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <vector>

class RGBLights;

class Program {
  public:
    Program(RGBLights* app)
        : m_logger("Program"), m_app(app), m_state(State::RUNNING) {}
    ~Program() {}

    void execute();
    void interrupt(const std::vector<uint8_t>& data);

    void pause();
    void unpause();

    virtual void on_start() = 0;
    virtual void loop() = 0;
    virtual void on_interrupt(const std::vector<uint8_t>& data) = 0;
    virtual void on_stop() = 0;

    const hc::util::Logger& get_logger() { return m_logger; }

  protected:
    RGBLights* m_app;

  private:
    enum class State { RUNNING, INTERRUPTED };

    std::atomic<State> m_state;

    std::vector<uint8_t> m_interrupt_data;

    hc::util::Logger m_logger;

    std::mutex m_mutex;
    std::condition_variable m_cv;
};