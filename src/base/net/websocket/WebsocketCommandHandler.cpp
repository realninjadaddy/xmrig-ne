#include "base/net/websocket/WebsocketCommandHandler.h"
#include "core/Controller.h"
#include "core/config/Config.h"
#include "base/kernel/Base.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include "base/io/log/Log.h"
#include "base/io/log/Tags.h"

using json = nlohmann::json;
using namespace xmrig;

WebsocketCommandHandler::WebsocketCommandHandler(Controller *controller)
    : m_controller(controller)
{
    //std::cerr << "[DEBUG] WebsocketCommandHandler constructed with controller\n";
}

WebsocketCommandHandler::~WebsocketCommandHandler()
{
    //std::cerr << "[DEBUG] WebsocketCommandHandler is being destroyed\n";
}

void WebsocketCommandHandler::handleArgs(const json &args)
{
    if (!m_controller) {
        //std::cerr << "[DEBUG] m_controller is nullptr in handleArgs()\n";
        return;
    }
    //std::cerr << "[DEBUG] handleArgs() is running with valid m_controller\n";
    if (!this) {
        //std::cerr << "WebsocketCommandHandler is not initialized.\n";
        return;
    }
    if (false) {
  
    } else if (args.contains("type") && args["type"] == "set_ws_args") {

        // Hent hele den eksisterende konfiguration
        auto fullConfig = m_controller->config()->toJSON();
    
        // Kopier originalen for at kunne sammenligne senere
        rapidjson::Document originalConfig;
        originalConfig.CopyFrom(fullConfig, fullConfig.GetAllocator());
    
        // Iterer over args["ws_args"] og opdater fullConfig
        if (args.contains("ws_args") && args["ws_args"].is_object()) {
            for (const auto& item : args["ws_args"].items()) {
                const auto& key = item.key();
                const auto& value = item.value();
    
                //std::cout << "Key: " << key << ", Is boolean: " << value.is_boolean() << ", Is string: " << value.is_string() << std::endl;
                if (key == "--url" || key == "-o") {
                    if (value.is_string() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        const auto& pools = fullConfig["pools"].GetArray();
                        if (!pools.Empty()) {
                            auto& firstPool = pools[0];
                            firstPool["url"].SetString(value.get<std::string>().c_str(), fullConfig.GetAllocator());
                        }
                    }
                } else if (key == "--user" || key == "-u") {
                    if (value.is_string() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        const auto& pools = fullConfig["pools"].GetArray();
                        if (!pools.Empty()) {
                            auto& firstPool = pools[0];
                            firstPool["user"].SetString(value.get<std::string>().c_str(), fullConfig.GetAllocator());
                        }
                    }
                } else if (key == "--algo" || key == "-a") {
                    if (value.is_string() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        const auto& pools = fullConfig["pools"].GetArray();
                        if (!pools.Empty()) {
                            auto& firstPool = pools[0];
                            firstPool["algo"].SetString(value.get<std::string>().c_str(), fullConfig.GetAllocator());
                        }
                    }
                } else if (key == "--pass" || key == "-p") {
                    if (value.is_string() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        const auto& pools = fullConfig["pools"].GetArray();
                        if (!pools.Empty()) {
                            auto& firstPool = pools[0];
                            firstPool["pass"].SetString(value.get<std::string>().c_str(), fullConfig.GetAllocator());
                        }
                    }
                } else if (key == "--tls") {
                    if (value.is_boolean() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        const auto& pools = fullConfig["pools"].GetArray();
                        if (!pools.Empty()) {
                            auto& firstPool = pools[0];
                            firstPool["tls"].SetBool(value.get<bool>());
                        }
                    }
                } else if (key == "--tls-fingerprint") {
                    if (value.is_string() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        const auto& pools = fullConfig["pools"].GetArray();
                        if (!pools.Empty()) {
                            auto& firstPool = pools[0];
                            firstPool["tls-fingerprint"].SetString(value.get<std::string>().c_str(), fullConfig.GetAllocator());
                        }
                    }
                } else if (key == "--coin") {
                    if (value.is_string() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        const auto& pools = fullConfig["pools"].GetArray();
                        if (!pools.Empty()) {
                            auto& firstPool = pools[0];
                            firstPool["coin"].SetString(value.get<std::string>().c_str(), fullConfig.GetAllocator());
                        }
                    }
                } else if (key == "--rig-id") {
                    if (value.is_string() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        const auto& pools = fullConfig["pools"].GetArray();
                        if (!pools.Empty()) {
                            auto& firstPool = pools[0];
                            firstPool["rig-id"].SetString(value.get<std::string>().c_str(), fullConfig.GetAllocator());
                        }
                    }
                } else if (key == "--keepalive" || key == "-k") {
                    if (value.is_boolean() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        const auto& pools = fullConfig["pools"].GetArray();
                        if (!pools.Empty()) {
                            auto& firstPool = pools[0];
                            firstPool["keepalive"].SetBool(value.get<bool>());
                        }
                    }
                } else if (key == "--nicehash") {
                    if (value.is_boolean() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        const auto& pools = fullConfig["pools"].GetArray();
                        if (!pools.Empty()) {
                            auto& firstPool = pools[0];
                            firstPool["nicehash"].SetBool(value.get<bool>());
                        }
                    }
                }                
                // Flere parametre kan tilføjes her
            }
        }
    
        /**/
        // Sammenlign originalConfig med fullConfig
        rapidjson::StringBuffer originalBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> originalWriter(originalBuffer);
        originalConfig.Accept(originalWriter);
    
        rapidjson::StringBuffer updatedBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> updatedWriter(updatedBuffer);
        fullConfig.Accept(updatedWriter);
    
        //std::cout << "[CMD] Original config: " << originalBuffer.GetString() << std::endl;
        //std::cout << "[CMD] Updated config: " << updatedBuffer.GetString() << std::endl;

        if (std::string(originalBuffer.GetString()) != std::string(updatedBuffer.GetString())) {
            //std::cout << "[CMD] Config has changed. Proceeding with reload.\n";

            LOG_INFO(WHITE_ON_GREY(" socket  ") " Config has changed. Proceeding with reload.");

            // Kald reload med den opdaterede fullConfig
            if (m_controller->reload(fullConfig)) {
                LOG_INFO(WHITE_ON_GREY(" socket  ") " Config reloaded successfully.");
            } else {
                LOG_INFO(WHITE_ON_GREY(" socket  ") " Config reload failed.");
            }
        } else {
            //std::cout << "[CMD] Config is identical. No reload needed.\n";
        }
       /**/
  

    } else {
        LOG_INFO(WHITE_ON_GREY(" socket  ") " Unknown command.");
    } 
}
