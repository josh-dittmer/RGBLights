#include "zengge_driver.h"

#include <algorithm>
#include <cassert>

const std::string ZenggeDriver::DEVICE_ADDRESS = "08:65:F0:68:2C:92";

bool ZenggeDriver::init() {
    m_bz_conn.start();

    std::shared_ptr<hc::bt::Device> device =
        m_bz_conn.get_device(DEVICE_ADDRESS);

    return true;
}

void ZenggeDriver::write(uint8_t r, uint8_t g, uint8_t b) {}

void ZenggeDriver::shutdown() {}
