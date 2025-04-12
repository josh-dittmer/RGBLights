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
    // std::cout << "ENTER" << std::endl;
    std::shared_ptr<hc::bt::Connection> conn_ptr =
        m_scanner.get_connection(m_address);
    // std::cout << "EXIT" << std::endl;

    if (!conn_ptr) {
        get_logger().verbose("Write failed, BLE connection not established");
        return;
    }

    conn_ptr->enqueue_char_write(10);
}

void ZenggeDriver::shutdown() { m_scanner.await_finish_and_cleanup(); }
