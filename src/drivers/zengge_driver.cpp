#include "zengge_driver.h"

#include <cassert>

const std::string ZenggeDriver::BUS_NAME = "org.bluez";
const std::string ZenggeDriver::ADAPTER_PATH = "/org/bluez/hci0";

bool ZenggeDriver::init() {
    m_dbus_conn_ptr.reset(g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, nullptr));

    if (!m_dbus_conn_ptr) {
        get_logger().error("Failed to connect to system bus!");
        return false;
    }

    get_logger().verbose("Successfully connected to system bus!");

    guint adapter_change_signal_id = g_dbus_connection_signal_subscribe(
        m_dbus_conn_ptr.get(), BUS_NAME.c_str(),
        "org.freedesktop.DBus.Properties", "PropertiesChanged", nullptr,
        "org.bluez.Adapter1", G_DBUS_SIGNAL_FLAGS_NONE,
        ZenggeDriver::adapter_change_cb, this, nullptr);

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

void ZenggeDriver::adapter_change_cb(GDBusConnection* dbus_conn,
                                     const gchar* sender,
                                     const gchar* object_path,
                                     const gchar* interface,
                                     const gchar* signal, GVariant* params,
                                     void* user_data) {
    assert(user_data != nullptr);
    ZenggeDriver* instance = static_cast<ZenggeDriver*>(user_data);

    instance->get_logger().verbose(
        "adapter_change_cb(): Adapter change callback invoked");

    g_unique_ptr<GVariant> changed_properties_variant_ptr;
    changed_properties_variant_ptr.reset(g_variant_get_child_value(params, 1));

    if (!changed_properties_variant_ptr) {
        instance->get_logger().warn(
            "adapter_change_cb(): Failed to read adapter properties!");
        return;
    }

    g_unique_ptr<GVariant> discovering_variant_ptr;
    discovering_variant_ptr.reset(g_variant_lookup_value(
        changed_properties_variant_ptr.get(), "Discovering", nullptr));

    g_unique_ptr<GVariant> powered_variant_ptr;
    powered_variant_ptr.reset(g_variant_lookup_value(
        changed_properties_variant_ptr.get(), "Powered", nullptr));

    if (discovering_variant_ptr) {
        if (!g_variant_is_of_type(discovering_variant_ptr.get(),
                                  G_VARIANT_TYPE_BOOLEAN)) {
            instance->get_logger().debug("adapter_change_cb(): \"Discovering\" "
                                         "variant has invalid type");
            return;
        }

        const bool scanning =
            g_variant_get_boolean(discovering_variant_ptr.get());

        instance->get_logger().log("Bluetooth adapter change: Adapter is " +
                                           scanning
                                       ? "now scanning"
                                       : "no longer scanning");
    }

    if (powered_variant_ptr) {
        if (!g_variant_is_of_type(powered_variant_ptr.get(),
                                  G_VARIANT_TYPE_BOOLEAN)) {
            instance->get_logger().debug(
                "adapter_change_cb(): \"Powered\" variant has invalid type");
            return;
        }

        const bool powered = g_variant_get_boolean(powered_variant_ptr.get());

        instance->get_logger().log(
            "Bluetooth adapter change: Adapter is now powered " + powered
                ? "ON"
                : "OFF");
    }
}

void ZenggeDriver::device_appear_cb(GDBusConnection* dbus_conn,
                                    const gchar* sender,
                                    const gchar* object_path,
                                    const gchar* interface, const gchar* signal,
                                    GVariant* params, void* user_data) {}

void ZenggeDriver::device_disappear_cb(GDBusConnection* dbus_conn,
                                       const gchar* sender,
                                       const gchar* object_path,
                                       const gchar* interface,
                                       const gchar* signal, GVariant* params,
                                       void* user_data) {}

bool ZenggeDriver::set_adapter_property(std::string property, GVariant* value) {
    GError* err_raw_ptr = nullptr;

    g_unique_ptr<GVariant> res_ptr;
    res_ptr.reset(g_dbus_connection_call_sync(
        m_dbus_conn_ptr.get(), BUS_NAME.c_str(), ADAPTER_PATH.c_str(),
        "org.freedesktop.DBus.Properties", "Set",
        g_variant_new("(ssv)", "org.bluez.Adapter1", property.c_str(), value),
        nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &err_raw_ptr));

    g_unique_ptr<GError> err_ptr;
    err_ptr.reset(err_raw_ptr);

    if (err_ptr) {
        return false;
    }

    return true;
}
