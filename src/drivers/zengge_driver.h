#pragma once

#include <gio/gio.h>

#include <memory>

#include "driver.h"

class ZenggeDriver : public Driver {
  public:
    ZenggeDriver() : Driver("ZenggeDriver") {}
    ~ZenggeDriver() {}

    bool init() override;
    void write(uint8_t r, uint8_t g, uint8_t b) override;
    void shutdown() override;

  private:
};