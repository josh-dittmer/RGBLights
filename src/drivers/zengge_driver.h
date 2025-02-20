#pragma once

#include <gio/gio.h>

#include <memory>
#include <thread>

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
    ZenggeDriver() : Driver("ZenggeDriver") {}
    ~ZenggeDriver() {}

    bool init() override;
    void write(uint8_t r, uint8_t g, uint8_t b) override;
    void shutdown() override;

  private:
    static const std::string BUS_NAME;
    static const std::string ADAPTER_PATH; // should be found in runtime

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

    bool set_adapter_property(std::string property, GVariant* value);

    std::thread m_loop_thread;

    g_unique_ptr<GDBusConnection> m_dbus_conn_ptr;

    g_unique_ptr<GMainLoop> m_g_loop_ptr;
};