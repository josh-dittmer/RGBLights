#include "zengge_driver.h"

bool ZenggeDriver::init() {
    GDBusConnection* dbus_conn_ptr =
        g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, nullptr);

    if (!dbus_conn_ptr) {
        get_logger().error("Failed to connect to system bus!");
        return false;
    }

    get_logger().verbose("Successfully connected to system bus!");

    GMainLoop* main_loop_ptr = g_main_loop_new(nullptr, false);

    

    return true;
}

void ZenggeDriver::write(uint8_t r, uint8_t g, uint8_t b) {}

void ZenggeDriver::shutdown() {}
