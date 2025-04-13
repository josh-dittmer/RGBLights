#pragma once

#include <homecontroller/api/device.h>
#include <homecontroller/api/device_data/rgb_lights.h>

#include "config.h"
#include "drivers/driver.h"
#include "programs/program.h"

#include <condition_variable>
#include <memory>
#include <mutex>

class RGBLights : public hc::api::Device<hc::api::rgb_lights::State> {
  public:
    struct CommandLineArgs {
        std::string m_config_path;
    };

    RGBLights()
        : Device("RGBLights"), m_loop_needs_reset(false),
          m_shutting_down(false) {}
    ~RGBLights() {}

    bool init(const CommandLineArgs& args);

    void shutdown();

    void set_init_finished_cb(std::function<void()> init_finished_cb) {
        m_init_finished_cb = init_finished_cb;
    }

    void set_color(uint8_t r, uint8_t g, uint8_t b,
                   bool update_on_server = true);

  private:
    bool init_driver(const std::string& driver_name);

    void loop();

    void on_command_received(
        std::map<std::string, ::sio::message::ptr>& data) override;

    ::sio::message::ptr serialize_state() const override;

    void handle_power_on(hc::api::rgb_lights::State& state);
    void handle_power_off(hc::api::rgb_lights::State& state);
    void handle_set_color(hc::api::rgb_lights::State& state,
                          bool& needs_update_ref,
                          std::map<std::string, ::sio::message::ptr>& data);
    void handle_start_program(hc::api::rgb_lights::State& state,
                              std::map<std::string, ::sio::message::ptr>& data);
    void
    handle_interrupt_program(std::map<std::string, ::sio::message::ptr>& data);
    void handle_stop_program(hc::api::rgb_lights::State& state);

    void reset_program(hc::api::rgb_lights::State::Program program);

    std::function<void()> m_init_finished_cb;

    Config m_config;

    std::unique_ptr<Driver> m_driver;

    std::unique_ptr<Program> m_program_ptr;

    std::mutex m_mutex_loop;
    std::mutex m_mutex_command;

    std::condition_variable m_cv_loop;
    bool m_loop_needs_reset;

    bool m_shutting_down;

    // storage for powering on/off
    hc::api::rgb_lights::State::Program m_last_program;

    uint8_t m_last_r;
    uint8_t m_last_g;
    uint8_t m_last_b;
};