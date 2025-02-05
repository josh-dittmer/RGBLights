#pragma once

#include <homecontroller/util/logger.h>

#include <string>

class Driver {
  public:
    Driver(const std::string& log_context) : m_logger(log_context) {}
    ~Driver() {}

    virtual bool init() = 0;
    virtual void write(uint8_t r, uint8_t g, uint8_t b) = 0;
    virtual void shutdown() = 0;

    const hc::util::Logger& get_logger() { return m_logger; }

  private:
    hc::util::Logger m_logger;
};