#define GATTLIB_LOG_LEVEL GATTLIB_DEBUG

#include "rgb_lights.h"

#include <csignal>
#include <iostream>

RGBLights r;

void display_usage() {}

RGBLights::CommandLineArgs get_args(int argc, char* argv[]) {
    // define defaults
    RGBLights::CommandLineArgs args;
    args.m_config_path = "conf/conf.json";

    static std::map<std::string, std::function<void(std::string)>> arg_map = {
        {"-c", [&](std::string val) { args.m_config_path = val; }}};

    for (int i = 1; i < argc; i += 2) {
        if (i + 1 >= argc) {
            r.get_logger().warn(
                "No value provided for command line argument \"" +
                std::string(argv[i]) + "\"");
            break;
        }

        std::string arg(argv[i]);
        std::string val(argv[i + 1]);

        auto mit = arg_map.find(arg);
        if (mit == arg_map.end()) {
            r.get_logger().warn("Invalid command line argument \"" + arg +
                                "\"");
            continue;
        }

        mit->second(val);
    }

    return args;
}

int main(int argc, char* argv[]) {
    r.set_init_finished_cb(
        []() { std::signal(SIGINT, [](int s) { r.shutdown(); }); });

    // overwritten later by value set in config
    hc::util::Logger::set_log_level(hc::util::Logger::LogLevel::NORMAL);

    r.get_logger().log("RGBLights for HomeController v1.0.0");
    r.get_logger().log("Created by Josh Dittmer");

    if (!r.init(get_args(argc, argv))) {
        r.get_logger().fatal("Exiting with non-zero status code");
        return -1;
    }

    r.get_logger().log("RGBLights stopped, exiting gracefully");

    return 0;
}