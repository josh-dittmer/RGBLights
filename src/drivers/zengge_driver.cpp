#include "zengge_driver.h"

#include <algorithm>
#include <cassert>
#include <iostream>

bool ZenggeDriver::init() {
    std::set<std::string> addresses = {m_address};

    m_scanner.start(addresses);

    return true;
}

void ZenggeDriver::write(uint8_t r, uint8_t g, uint8_t b) {
    std::shared_ptr<hc::bt::Connection> conn_ptr =
        m_scanner.get_connection(m_address);

    if (conn_ptr) {
        conn_ptr->write_char(10);
    }
}

void ZenggeDriver::shutdown() { m_scanner.shutdown_sync(); }
