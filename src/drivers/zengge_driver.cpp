#include "zengge_driver.h"

const std::string ZenggeDriver::BUS_NAME = "org.bluez";
const std::string ZenggeDriver::ADAPTER_PATH = "/org/bluez/hci0";
const std::string ZenggeDriver::INTERFACE_NAME = "org.bluez.Adapter1";

bool ZenggeDriver::init() {
    m_dbus_conn_ptr.reset(g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, nullptr));

    if (!m_dbus_conn_ptr) {
        get_logger().error("Failed to connect to system bus!");
        return false;
    }

    get_logger().verbose("Successfully connected to system bus!");

    if (!set_adapter_property("Powered", g_variant_new("b", true))) {
        get_logger().error("Failed to enable Bluetooth adapter!");
        return false;
    }

    GMainLoop* main_loop_ptr = g_main_loop_new(nullptr, false);
    g_main_loop_run(main_loop_ptr);

    return true;
}

void ZenggeDriver::write(uint8_t r, uint8_t g, uint8_t b) {}

void ZenggeDriver::shutdown() {}

bool ZenggeDriver::set_adapter_property(std::string property, GVariant* value) {
    GError* err = nullptr;

    std::unique_ptr<GVariant, deleter_of<GVariant>> res_ptr;
    res_ptr.reset(g_dbus_connection_call_sync(
        m_dbus_conn_ptr.get(), BUS_NAME.c_str(), ADAPTER_PATH.c_str(),
        "org.freedesktop.DBus.Properties", "Set",
        g_variant_new("(ssv)", INTERFACE_NAME.c_str(), property, value),
        nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &err));

    if (err != nullptr) {
        return false;
    }

    return true;
}
