#include "zengge_driver.h"

#include <algorithm>
#include <cassert>

const std::string ZenggeDriver::BUS_NAME = "org.bluez";
const std::string ZenggeDriver::ADAPTER_PATH = "/org/bluez/hci0";
const std::string ZenggeDriver::DEVICE_ADDRESS = "08:65:F0:20:92:94";
const int ZenggeDriver::CONNECT_TIMEOUT = 10000;

bool ZenggeDriver::init() {
    m_dbus_conn_ptr.reset(g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, nullptr));

    if (!m_dbus_conn_ptr) {
        get_logger().error("Failed to connect to system bus!");
        return false;
    }

    get_logger().verbose("init(): Successfully connected to system bus!");

    m_signal_ids.push_back(g_dbus_connection_signal_subscribe(
        m_dbus_conn_ptr.get(), BUS_NAME.c_str(),
        "org.freedesktop.DBus.Properties", "PropertiesChanged", nullptr,
        "org.bluez.Adapter1", G_DBUS_SIGNAL_FLAGS_NONE,
        ZenggeDriver::adapter_change_cb, this, nullptr));

    m_signal_ids.push_back(g_dbus_connection_signal_subscribe(
        m_dbus_conn_ptr.get(), BUS_NAME.c_str(),
        "org.freedesktop.DBus.ObjectManager", "InterfacesAdded", nullptr,
        nullptr, G_DBUS_SIGNAL_FLAGS_NONE, ZenggeDriver::device_appear_cb, this,
        nullptr));

    if (!set_adapter_property("Powered", g_variant_new("b", true))) {
        get_logger().error("Failed to enable Bluetooth adapter!");
        remove_signals();
        return false;
    }

    if (!call_adapter_method("StartDiscovery")) {
        get_logger().error("Failed to start Bluetooth device discovery!");
        remove_signals();
        return false;
    }

    std::unique_lock<std::mutex> lock(m_mutex);

    m_g_loop_ptr.reset(g_main_loop_new(nullptr, false));
    m_loop_thread = std::thread([&]() { g_main_loop_run(m_g_loop_ptr.get()); });

    get_logger().verbose("Connecting to Bluetooth device...");

    auto now = std::chrono::system_clock::now();
    while (!m_device_connected) {
        if (m_cv.wait_until(lock, now + std::chrono::milliseconds(
                                            ZenggeDriver::CONNECT_TIMEOUT)) ==
            std::cv_status::timeout) {
            get_logger().error("Bluetooth connection timed out!");
            shutdown();
            return false;
        }
    }

    get_logger().verbose("init(): Initialization complete!");

    return true;
}

void ZenggeDriver::write(uint8_t r, uint8_t g, uint8_t b) {}

void ZenggeDriver::shutdown() {
    g_main_loop_quit(m_g_loop_ptr.get());

    get_logger().verbose("shutdown(): Waiting for event loop to quit...");

    if (m_loop_thread.joinable()) {
        m_loop_thread.join();
    }

    get_logger().verbose("shutdown(): Event loop has quit!");

    remove_signals();

    m_g_loop_ptr.reset();
    m_dbus_conn_ptr.reset();

    get_logger().log("shutdown(): Driver shutdown successfully!");
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

    g_unique_ptr<GVariant> changed_properties_variant_ptr(
        g_variant_get_child_value(params, 1));

    if (!changed_properties_variant_ptr) {
        instance->get_logger().warn(
            "adapter_change_cb(): Failed to read adapter properties!");
        return;
    }

    g_unique_ptr<GVariant> discovering_variant_ptr(g_variant_lookup_value(
        changed_properties_variant_ptr.get(), "Discovering", nullptr));

    g_unique_ptr<GVariant> powered_variant_ptr(g_variant_lookup_value(
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
                                    GVariant* params, void* user_data) {
    assert(user_data != nullptr);
    ZenggeDriver* instance = static_cast<ZenggeDriver*>(user_data);

    g_unique_ptr<GVariant> arg0_variant_ptr(
        g_variant_get_child_value(params, 0));

    const gchar* arg0_str =
        g_variant_get_string(arg0_variant_ptr.get(), nullptr);

    g_unique_ptr<GVariant> interfaces_and_properties_variant_ptr(
        g_variant_get_child_value(params, 1));

    GVariant* properties_variant_raw_ptr = nullptr;
    if (g_variant_lookup(interfaces_and_properties_variant_ptr.get(),
                         "org.bluez.Device1", "@a{sv}",
                         &properties_variant_raw_ptr)) {
        g_unique_ptr<GVariant> properties_variant_ptr(
            properties_variant_raw_ptr);

        g_unique_ptr<GVariant> name_variant_ptr(g_variant_lookup_value(
            properties_variant_ptr.get(), "Name", nullptr));

        const std::string name(
            name_variant_ptr
                ? g_variant_get_string(name_variant_ptr.get(), nullptr)
                : "unknown");

        g_unique_ptr<GVariant> addr_variant_ptr(g_variant_lookup_value(
            properties_variant_ptr.get(), "Address", nullptr));

        const std::string addr(
            addr_variant_ptr
                ? g_variant_get_string(addr_variant_ptr.get(), nullptr)
                : "unknown");

        instance->get_logger().verbose("device_appear_cb(): Found " + name +
                                       " [" + addr + "]");

        if (addr == ZenggeDriver::DEVICE_ADDRESS) {
            instance->get_logger().verbose(
                "device_appear_cb(): Found device with matching address! "
                "Connecting...");

            instance->get_logger().verbose("device_appear_cb(): Invoking BlueZ "
                                           "connect method...");
            if (!instance->call_device_method(
                    "Connect", nullptr,
                    [](GObject* dbus_conn, GAsyncResult* result,
                       gpointer user_data) {
                        assert(user_data != nullptr);
                        ZenggeDriver* instance =
                            static_cast<ZenggeDriver*>(user_data);

                        std::unique_lock<std::mutex> lock(instance->m_mutex);

                        instance->get_logger().verbose(
                            "device_appear_cb(): Connection method "
                            "finished! "
                            "Notifying...");

                        instance->m_device_connected = true;
                        instance->m_cv.notify_all();
                    })) {
                instance->get_logger().verbose(
                    "device_appear_cb(): Connection failed!");
            }
        }
    }
}

void ZenggeDriver::device_disappear_cb(GDBusConnection* dbus_conn,
                                       const gchar* sender,
                                       const gchar* object_path,
                                       const gchar* interface,
                                       const gchar* signal, GVariant* params,
                                       void* user_data) {}

bool ZenggeDriver::set_adapter_property(const std::string& property,
                                        GVariant* value) {
    GError* err_raw_ptr = nullptr;

    g_unique_ptr<GVariant> res_ptr(g_dbus_connection_call_sync(
        m_dbus_conn_ptr.get(), BUS_NAME.c_str(), ADAPTER_PATH.c_str(),
        "org.freedesktop.DBus.Properties", "Set",
        g_variant_new("(ssv)", "org.bluez.Adapter1", property.c_str(), value),
        nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &err_raw_ptr));

    g_unique_ptr<GError> err_ptr(err_raw_ptr);

    if (err_ptr) {
        return false;
    }

    return true;
}

bool ZenggeDriver::call_dbus_method(const std::string& object_path,
                                    const std::string& interface_name,
                                    const std::string& method,
                                    g_unique_ptr<GVariant> params_ptr,
                                    GAsyncReadyCallback callback) {
    GError* err_raw_ptr = nullptr;

    g_dbus_connection_call(m_dbus_conn_ptr.get(), BUS_NAME.c_str(),
                           object_path.c_str(), interface_name.c_str(),
                           method.c_str(),
                           params_ptr ? params_ptr.get() : nullptr, nullptr,
                           G_DBUS_CALL_FLAGS_NONE, -1, nullptr, callback, this);

    g_unique_ptr<GError> err_ptr(err_raw_ptr);

    if (err_ptr) {
        return false;
    }

    return true;
}

bool ZenggeDriver::call_adapter_method(const std::string& method) {
    return call_dbus_method(ADAPTER_PATH.c_str(), "org.bluez.Adapter1",
                            method.c_str(), nullptr, nullptr);
}

bool ZenggeDriver::call_device_method(const std::string& method,
                                      g_unique_ptr<GVariant> params_ptr,
                                      GAsyncReadyCallback callback) {
    std::string formatted_addr = DEVICE_ADDRESS;
    std::replace(formatted_addr.begin(), formatted_addr.end(), ':', '_');
    const std::string path = ADAPTER_PATH + "/" + "dev_" + formatted_addr;

    return call_dbus_method(path, "org.bluez.Device1", method,
                            std::move(params_ptr), callback);
}

void ZenggeDriver::remove_signals() {
    if (!m_dbus_conn_ptr) {
        return;
    }

    for (guint signal_id : m_signal_ids) {
        g_dbus_connection_signal_unsubscribe(m_dbus_conn_ptr.get(), signal_id);
    }

    get_logger().verbose("remove_signals(): All signals unsubscribed");
}
