#include "zengge_driver.h"

bool ZenggeDriver::init() {
    GDBusConnection* dbus_conn =
        g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, nullptr);

    if (!dbus_conn) {
        get_logger().error("Failed to connect to system bus!");
        return false;
    }

    return true;
}

void ZenggeDriver::write(uint8_t r, uint8_t g, uint8_t b) {}

void ZenggeDriver::shutdown() {}
