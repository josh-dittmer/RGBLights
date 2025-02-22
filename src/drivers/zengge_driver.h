#pragma once

#include <gio/gio.h>

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "driver.h"

template <class T> struct deleter_of;

template <> struct deleter_of<GDBusConnection> {
    void operator()(GDBusConnection* p) const { g_object_unref(p); }
};

template <> struct deleter_of<GMainLoop> {
    void operator()(GMainLoop* p) const { g_main_loop_unref(p); }
};

template <> struct deleter_of<GVariant> {
    void operator()(GVariant* p) const { g_variant_unref(p); }
};

template <> struct deleter_of<GVariantIter> {
    void operator()(GVariantIter* p) const { g_variant_iter_free(p); }
};

template <> struct deleter_of<GError> {
    void operator()(GError* p) const { g_error_free(p); }
};

template <class g_type>
using g_unique_ptr = std::unique_ptr<g_type, deleter_of<g_type>>;

class ZenggeDriver : public Driver {
  public:
    ZenggeDriver() : Driver("ZenggeDriver"), m_device_connected(false) {}
    ~ZenggeDriver() {}

    bool init() override;
    void write(uint8_t r, uint8_t g, uint8_t b) override;
    void shutdown() override;

  private:
    static const std::string BUS_NAME;
    static const std::string ADAPTER_PATH;   // should be found in runtime
    static const std::string DEVICE_ADDRESS; // will be read from file in future
    static const int CONNECT_TIMEOUT;        // will be read from file in future

    static void adapter_change_cb(GDBusConnection* dbus_conn,
                                  const gchar* sender, const gchar* object_path,
                                  const gchar* interface, const gchar* signal,
                                  GVariant* params, void* user_data);
    static void device_appear_cb(GDBusConnection* dbus_conn,
                                 const gchar* sender, const gchar* object_path,
                                 const gchar* interface, const gchar* signal,
                                 GVariant* params, void* user_data);
    static void device_disappear_cb(GDBusConnection* dbus_conn,
                                    const gchar* sender,
                                    const gchar* object_path,
                                    const gchar* interface, const gchar* signal,
                                    GVariant* params, void* user_data);

    bool set_adapter_property(const std::string& property, GVariant* value);

    bool call_dbus_method(const std::string& object_path,
                          const std::string& interface_name,
                          const std::string& method,
                          g_unique_ptr<GVariant> params_ptr,
                          GAsyncReadyCallback callback);

    bool call_adapter_method(const std::string& method);

    bool call_device_method(const std::string& method,
                            g_unique_ptr<GVariant> params_ptr,
                            GAsyncReadyCallback callback);

    void remove_signals();

    std::thread m_loop_thread;
    std::mutex m_mutex;
    std::condition_variable m_cv;

    bool m_device_connected;

    g_unique_ptr<GDBusConnection> m_dbus_conn_ptr;

    g_unique_ptr<GMainLoop> m_g_loop_ptr;

    std::vector<guint> m_signal_ids;
};