#include "base/net/websocket/WebsocketCommandHandler.h"
#include "core/Controller.h"
#include "3rdparty/rapidjson/document.h"
#include "3rdparty/rapidjson/stringbuffer.h"
#include "3rdparty/rapidjson/writer.h"
#include "base/io/json/Json.h"
#include "base/net/stratum/Pool.h"
#include "base/net/stratum/Pools.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include "base/io/log/Log.h"
#include "core/config/Config.h"

namespace xmrig {

WebsocketCommandHandler::WebsocketCommandHandler(Controller *controller)
    : m_controller(controller)
{
}

WebsocketCommandHandler::~WebsocketCommandHandler() = default;

void WebsocketCommandHandler::handleArgs(const nlohmann::json &message) {
    if (!message.is_object()) {
        LOG_ERR("[WebSocket:ERR] Invalid args payload (not an object)");
        return;
    }

    if (!message.contains("ws_args") || !message["ws_args"].is_object()) {
        LOG_ERR("[WebSocket:ERR] Missing or invalid 'ws_args'");
        return;
    }

    const nlohmann::json &args = message["ws_args"];
    LOG_INFO("[WebSocket:INFO] Received set_ws_args");

    const Config *config = m_controller->config();
    if (!config) {
        LOG_ERR("[WebSocket:ERR] Config missing in controller");
        return;
    }

    rapidjson::Document doc;
    doc.CopyFrom(config->toJSON(), doc.GetAllocator());
    rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();

    // Modify pools[0] fields if they exist in args
    if (doc.HasMember("pools") && doc["pools"].IsArray() && !doc["pools"].Empty()) {
        rapidjson::Value &pool = doc["pools"][0];

        for (auto it = args.begin(); it != args.end(); ++it) {
            const std::string key = it.key();
            const std::string val = it.value().get<std::string>();

            std::string keyStripped = key;
            if (keyStripped.rfind("--", 0) == 0) keyStripped = keyStripped.substr(2);
            else if (keyStripped.rfind("-", 0) == 0) keyStripped = keyStripped.substr(1);

            if (keyStripped == "url") {
                pool["url"].SetString(val.c_str(), allocator);
            }
            else if (keyStripped == "user") {
                pool["user"].SetString(val.c_str(), allocator);
            }
            else if (keyStripped == "pass") {
                pool["pass"].SetString(val.c_str(), allocator);
            }
            else if (keyStripped == "algo") {
                pool["algo"].SetString(val.c_str(), allocator);
            }
            else if (keyStripped == "rig-id") {
                pool["rig-id"].SetString(val.c_str(), allocator);
            }
            else if (keyStripped == "tls") {
                pool["tls"].SetBool(val == "true");
            }
            else if (keyStripped == "keepalive") {
                pool["keepalive"].SetBool(val == "true");
            }
            else if (keyStripped == "nicehash") {
                pool["nicehash"].SetBool(val == "true");
            }
            else {
                LOG_INFO("[WebSocket:INFO] Unhandled key: %s", key.c_str());
            }
        }

        // Apply the updated JSON config
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        LOG_INFO("[WebSocket:INFO] Reloading config with modified ws_args");
        m_controller->reload(doc);
    }
    else {
        LOG_ERR("[WebSocket:ERR] Invalid or missing 'pools' section in config");
    }
}

} // namespace xmrig
