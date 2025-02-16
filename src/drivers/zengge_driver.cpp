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

    m_g_loop_ptr.reset(g_main_loop_new(nullptr, false));
    m_loop_thread = std::thread([&]() { g_main_loop_run(m_g_loop_ptr.get()); });

    return true;
}

void ZenggeDriver::write(uint8_t r, uint8_t g, uint8_t b) {}

void ZenggeDriver::shutdown() {
    g_main_loop_quit(m_g_loop_ptr.get());

    get_logger().verbose("Waiting for event loop to quit...");

    if (m_loop_thread.joinable()) {
        m_loop_thread.join();
    }

    get_logger().verbose("Event loop has quit!");

    m_g_loop_ptr.reset();
    m_dbus_conn_ptr.reset();

    get_logger().log("Driver shutdown successfully!");
}

bool ZenggeDriver::set_adapter_property(std::string property, GVariant* value) {
    GError* err = nullptr;

    std::unique_ptr<GVariant, deleter_of<GVariant>> res_ptr;
    res_ptr.reset(g_dbus_connection_call_sync(
        m_dbus_conn_ptr.get(), BUS_NAME.c_str(), ADAPTER_PATH.c_str(),
        "org.freedesktop.DBus.Properties", "Set",
        g_variant_new("(ssv)", INTERFACE_NAME.c_str(), property.c_str(), value),
        nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &err));

    if (err != nullptr) {
        return false;
    }

    return true;
}
