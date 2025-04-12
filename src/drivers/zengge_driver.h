#pragma once

#include <homecontroller/bt/scanner.h>

#include "driver.h"

class ZenggeDriver : public Driver {
  public:
    ZenggeDriver(const std::string& address)
        : Driver("ZenggeDriver"), m_address(address) {}
    ~ZenggeDriver() {}

    bool init() override;
    void write(uint8_t r, uint8_t g, uint8_t b) override;

    void shutdown() override;

  private:
    hc::bt::Scanner m_scanner;

    std::string m_address;
};