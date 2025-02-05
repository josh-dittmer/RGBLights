#pragma once

#include "driver.h"

class TestDriver : public Driver {
  public:
    TestDriver() : Driver("TestDriver") {}
    ~TestDriver() {}

    bool init() override;
    void write(uint8_t r, uint8_t g, uint8_t b) override;
    void shutdown() override;
};