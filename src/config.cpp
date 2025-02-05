#include "config.h"

#include <rapidjson/error/en.h>

#include <fstream>
#include <sstream>

bool Config::load(const std::string& path) {
    std::ifstream conf_file(path);
    if (!conf_file.good()) {
        m_logger.error("Failed to open config file: \"" + path + "\"");
        return false;
    }

    std::ostringstream ss;
    ss << conf_file.rdbuf();

    rapidjson::Document doc;
    rapidjson::ParseResult res = doc.Parse(ss.str().c_str());

    if (!res) {
        m_logger.error("Failed to parse config file: \"" +
                       std::string(rapidjson::GetParseError_En(res.Code())) +
                       "\"");

        return false;
    }

    try {
        m_values.m_driver = read_str(doc, "driver");
        m_values.m_gateway_url = read_str(doc, "gateway_url");
        m_values.m_gateway_namespace = read_str(doc, "gateway_namespace");
        m_values.m_device_id = read_str(doc, "device_id");
        m_values.m_secret = read_str(doc, "secret");
        m_values.m_log_level_str = read_str(doc, "log_level");
        m_values.m_reconn_delay = read_int(doc, "reconn_delay");
        m_values.m_reconn_attempts = read_int(doc, "reconn_attempts");
    } catch (ReadException& e) {
        m_logger.error(e.what());
        return false;
    }

    m_logger.log("Successfully loaded config file \"" + path + "\"");

    return true;
}

void Config::print() {
    m_logger.verbose("gateway_url: " + m_values.m_gateway_url);
    m_logger.verbose("gateway_namespace: " + m_values.m_gateway_namespace);
    m_logger.verbose("device_id: " + m_values.m_device_id);
    m_logger.verbose("secret: [hidden]");
    m_logger.verbose("log_level: " + m_values.m_log_level_str);
    m_logger.verbose("reconn_delay: " +
                     std::to_string(m_values.m_reconn_delay));
    m_logger.verbose("reconn_attempts: " +
                     std::to_string(m_values.m_reconn_attempts));
}

std::string Config::read_str(const rapidjson::Document& doc,
                             const std::string& name) {
    if (!doc.HasMember(name.c_str()) || !doc[name.c_str()].IsString()) {
        throw ReadException("string", name);
    }

    return std::string(doc[name.c_str()].GetString());
}

int Config::read_int(const rapidjson::Document& doc, const std::string& name) {
    if (!doc.HasMember(name.c_str()) || !doc[name.c_str()].IsInt()) {
        throw ReadException("integer", name);
    }

    return doc[name.c_str()].GetInt();
}