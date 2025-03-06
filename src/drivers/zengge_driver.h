#pragma once

#include <homecontroller/bt/bluez_connection.h>

#include "driver.h"

class ZenggeDriver : public Driver {
  public:
    ZenggeDriver() : Driver("ZenggeDriver") {}
    ~ZenggeDriver() {}

    bool init() override;
    void write(uint8_t r, uint8_t g, uint8_t b) override;
    void shutdown() override;

  private:
    static const std::string DEVICE_ADDRESS;

    hc::bt::BlueZConnection m_bz_conn;
};