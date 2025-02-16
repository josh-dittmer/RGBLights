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
    static const std::string INTERFACE_NAME;

    bool set_adapter_property(std::string property, GVariant* value);

    std::thread m_loop_thread;

    std::unique_ptr<GDBusConnection, deleter_of<GDBusConnection>>
        m_dbus_conn_ptr;

    std::unique_ptr<GMainLoop, deleter_of<GMainLoop>> m_g_loop_ptr;
};