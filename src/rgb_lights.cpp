#include "rgb_lights.h"

#include "programs/default_program.h"
#include "programs/rainbow_fade_program.h"

#include <homecontroller/socket.io/client.h>

#include <chrono>
#include <iostream>
#include <thread>

bool RGBLights::init(const CommandLineArgs& args) {
    // initialization sequence
    if (!m_config.load(args.m_config_path)) {
        return false;
    }

    hc::util::Logger::set_log_level(hc::util::Logger::string_to_log_level(
        get_logger(), m_config.get_log_level_str()));

    m_config.print();

    get_logger().verbose("Initialization finished!");

    // create initial state
    hc::api::rgb_lights::State state;
    state.m_powered = true;
    state.m_program = hc::api::rgb_lights::State::Program::None;
    state.m_color = {255, 255, 255};

    // start default program
    m_program_ptr = std::make_unique<DefaultProgram>(
        this, state.m_color.m_r, state.m_color.m_g, state.m_color.m_b);

    // device opts
    Device::StartParams params;
    params.m_gateway = {m_config.get_gateway_url(),
                        m_config.get_gateway_namespace()};
    params.m_device_id = m_config.get_device_id();
    params.m_secret = m_config.get_secret();
    params.m_initial_state = state;
    params.m_reconn_delay = m_config.get_reconn_delay();
    params.m_reconn_attempts = m_config.get_reconn_attempts();

    // starts main device loop
    start(params);

    std::thread loop_thread;
    if (is_client_running()) {
        get_logger().verbose("Starting loop thread...");
        loop_thread = std::thread(&RGBLights::loop, this);
    }

    // blocks until device loop exits
    await_finish_and_cleanup();

    if (loop_thread.joinable()) {
        get_logger().verbose("Waiting for loop thread to exit...");
        loop_thread.join();
    }

    // shutdown sequence
    get_logger().verbose("Shutting down...");

    return true;
}

void RGBLights::shutdown() {
    get_logger().log("Triggering manual shutdown...");

    stop();
}

void RGBLights::set_color(uint8_t r, uint8_t g, uint8_t b,
                          bool update_on_server) {
    if (update_on_server) {
        hc::api::rgb_lights::State new_state = get_state();
        new_state.m_color.m_r = r;
        new_state.m_color.m_g = g;
        new_state.m_color.m_b = b;

        update_state(new_state);
    }

    // update pwm
}

void RGBLights::loop() {
    get_logger().verbose("loop(): Main loop started!");

    while (is_client_running()) {
        if (m_loop_needs_reset) {
            std::unique_lock<std::mutex> lock(m_mutex_loop);

            m_cv_loop.notify_all();

            get_logger().verbose(
                "loop(): Main loop stopped! Waiting for restart signal...");

            m_cv_loop.wait(lock);

            get_logger().verbose("loop(): Received signal! Starting...");
        }

        m_program_ptr->execute();
    }

    get_logger().verbose("loop(): Main loop exited!");
}

void RGBLights::on_command_received(
    std::map<std::string, ::sio::message::ptr>& data) {
    std::unique_lock<std::mutex> lock(m_mutex_command);

    get_logger().verbose("on_command_received(): Reading command...");

    std::string cmd_name = data["command"]->get_string();

    get_logger().verbose("on_command_received(): Command name is \"" +
                         cmd_name + "\"");

    hc::api::rgb_lights::Command cmd =
        hc::api::rgb_lights::string_to_command(cmd_name);

    hc::api::rgb_lights::State new_state = get_state();
    bool needs_update = false;

    switch (cmd) {
    case hc::api::rgb_lights::Command::PowerOn:
        get_logger().verbose(
            "on_command_received(): Executing power on handler");
        handle_power_on(new_state);
        needs_update = true;
        break;
    case hc::api::rgb_lights::Command::PowerOff:
        get_logger().verbose(
            "on_command_received(): Executing power off handler");
        handle_power_off(new_state);
        needs_update = true;
        break;
    case hc::api::rgb_lights::Command::SetColor:
        get_logger().verbose(
            "on_command_received(): Executing set color handler");
        handle_set_color(new_state, needs_update, data["data"]->get_map());
        break;
    case hc::api::rgb_lights::Command::StartProgram:
        get_logger().verbose(
            "on_command_received(): Executing start program handler");
        handle_start_program(new_state, data["data"]->get_map());
        needs_update = true;
        break;
    case hc::api::rgb_lights::Command::InterruptProgram:
        get_logger().verbose(
            "on_command_received(): Executing interrupt program handler");
        handle_interrupt_program(data["data"]->get_map());
        needs_update = false;
        break;
    case hc::api::rgb_lights::Command::StopProgram:
        get_logger().verbose(
            "on_command_received(): Executing stop program handler");
        handle_stop_program(new_state);
        needs_update = true;
        break;
    default:
        get_logger().warn("Unimplemented command");
        break;
    }

    if (needs_update) {
        update_state(new_state);
    }
}

::sio::message::ptr RGBLights::serialize_state() const {
    ::sio::message::ptr state_msg = ::sio::object_message::create();
    state_msg->get_map()["powered"] =
        ::sio::bool_message::create(get_state().m_powered);
    state_msg->get_map()["program"] = ::sio::string_message::create(
        hc::api::rgb_lights::program_to_string(get_state().m_program));
    state_msg->get_map()["r"] =
        ::sio::int_message::create(get_state().m_color.m_r);
    state_msg->get_map()["g"] =
        ::sio::int_message::create(get_state().m_color.m_g);
    state_msg->get_map()["b"] =
        ::sio::int_message::create(get_state().m_color.m_b);

    return state_msg;
}

void RGBLights::handle_power_on(hc::api::rgb_lights::State& state) {
    if (state.m_powered) {
        get_logger().debug("handle_power_on(): Power already on!");
        return;
    }

    if (m_last_program != hc::api::rgb_lights::State::Program::None) {
        get_logger().verbose(
            "handle_power_on(): Switching to last running program...");

        reset_program(m_last_program);
        state.m_program = m_last_program;
    } else {
        m_program_ptr->interrupt({m_last_r, m_last_g, m_last_b});
    }

    state.m_powered = true;

    get_logger().log("Power switched ON");
}

void RGBLights::handle_power_off(hc::api::rgb_lights::State& state) {
    if (!state.m_powered) {
        get_logger().debug("handle_power_off(): Power already off!");
        return;
    }

    m_last_program = state.m_program;

    // start default program to enable fade effect
    if (state.m_program != hc::api::rgb_lights::State::Program::None) {
        get_logger().verbose("handle_power_off(): Switching to default program "
                             "for fade effect...");

        reset_program(hc::api::rgb_lights::State::Program::None);
        state.m_program = hc::api::rgb_lights::State::Program::None;
    } else {
        m_last_r = get_state().m_color.m_r;
        m_last_g = get_state().m_color.m_g;
        m_last_b = get_state().m_color.m_b;
    }

    // set color to black
    m_program_ptr->interrupt({0, 0, 0});

    state.m_powered = false;

    get_logger().log("Power switched OFF");
}

void RGBLights::handle_set_color(
    hc::api::rgb_lights::State& state, bool& needs_update_ref,
    std::map<std::string, ::sio::message::ptr>& data) {
    get_logger().verbose("handle_set_color(): Reading command data...");

    uint8_t r = data["r"]->get_int();
    uint8_t g = data["g"]->get_int();
    uint8_t b = data["b"]->get_int();

    if (get_state().m_program != hc::api::rgb_lights::State::Program::None) {
        get_logger().verbose(
            "handle_set_color(): Program is running, stopping first...");

        reset_program(hc::api::rgb_lights::State::Program::None);
        state.m_program = hc::api::rgb_lights::State::Program::None;
        needs_update_ref = true;
    }

    m_program_ptr->interrupt({r, g, b});

    if (!state.m_powered) {
        state.m_powered = true;
        needs_update_ref = true;
    }

    get_logger().log("Color set to RGB(" + std::to_string(r) + ", " +
                     std::to_string(g) + ", " + std::to_string(b) + ")");
}

void RGBLights::handle_start_program(
    hc::api::rgb_lights::State& state,
    std::map<std::string, ::sio::message::ptr>& data) {
    get_logger().verbose("handle_start_program(): Reading command data...");

    std::string program_name = data["program"]->get_string();

    get_logger().verbose("handle_start_program(): Program name is \"" +
                         program_name + "\"");

    hc::api::rgb_lights::State::Program program =
        hc::api::rgb_lights::string_to_program(program_name);

    reset_program(program);
    state.m_program = program;

    if (!state.m_powered) {
        state.m_powered = true;
    }

    get_logger().log("Program \"" + program_name + "\" started");
}

void RGBLights::handle_interrupt_program(

    std::map<std::string, ::sio::message::ptr>& data) {}

void RGBLights::handle_stop_program(hc::api::rgb_lights::State& state) {
    get_logger().verbose("handle_stop_program(): Stopping program...");

    if (state.m_program == hc::api::rgb_lights::State::Program::None) {
        get_logger().verbose("handle_stop_program(): No program is running!");
        return;
    }

    reset_program(hc::api::rgb_lights::State::Program::None);
    state.m_program = hc::api::rgb_lights::State::Program::None;

    get_logger().log("Program stopped");
}

void RGBLights::reset_program(hc::api::rgb_lights::State::Program program) {
    std::unique_lock<std::mutex> lock(m_mutex_loop);

    get_logger().verbose(
        "reset_program(): Waiting for program loop to stop...");

    m_loop_needs_reset = true;

    m_cv_loop.wait(lock);

    m_program_ptr->on_stop();

    switch (program) {
    case hc::api::rgb_lights::State::Program::RainbowFade:
        m_program_ptr = std::make_unique<RainbowFadeProgram>(this);
        break;
    case hc::api::rgb_lights::State::Program::PsychedelicFade:
        get_logger().warn("Unimplemented program");
    case hc::api::rgb_lights::State::Program::GuitarSync:
        get_logger().warn("Unimplemented program");
    default:
        m_program_ptr = std::make_unique<DefaultProgram>(
            this, get_state().m_color.m_r, get_state().m_color.m_g,
            get_state().m_color.m_b);
        break;
    }

    m_program_ptr->on_start();

    get_logger().verbose("reset_program(): Sending start signal...");

    m_loop_needs_reset = false;
    m_cv_loop.notify_all();

    get_logger().verbose("reset_program(): Reset successful!");
}