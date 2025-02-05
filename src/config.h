#pragma once

#include <homecontroller/util/logger.h>

#include <rapidjson/document.h>

class Config {
  public:
    struct Values {
        std::string m_driver;
        std::string m_gateway_url;
        std::string m_gateway_namespace;
        std::string m_device_id;
        std::string m_secret;
        std::string m_log_level_str;
        int m_reconn_delay;
        int m_reconn_attempts;
    };

    Config() : m_logger("Config") {}
    ~Config() {}

    bool load(const std::string& path);

    void print();

    const Values& get_values() { return m_values; }

    const std::string& get_driver() { return m_values.m_driver; }
    const std::string& get_gateway_url() { return m_values.m_gateway_url; }
    const std::string& get_gateway_namespace() {
        return m_values.m_gateway_namespace;
    }
    const std::string& get_device_id() { return m_values.m_device_id; }
    const std::string& get_secret() { return m_values.m_secret; }
    const std::string& get_log_level_str() { return m_values.m_log_level_str; }
    int get_reconn_delay() { return m_values.m_reconn_delay; }
    int get_reconn_attempts() { return m_values.m_reconn_attempts; }

  private:
    class ReadException : public std::exception {
      public:
        ReadException(const std::string& type, const std::string& name) {
            m_msg = "Failed to read required " + type + " \"" + name + "\"";
        }

        ~ReadException() {}

        const char* what() const throw() override { return m_msg.c_str(); }

      private:
        std::string m_msg;
    };

    std::string read_str(const rapidjson::Document& doc,
                         const std::string& name);
    int read_int(const rapidjson::Document& doc, const std::string& name);

    Values m_values;

    hc::util::Logger m_logger;
};