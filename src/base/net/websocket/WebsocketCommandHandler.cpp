#include "base/net/websocket/WebsocketCommandHandler.h"
#include "core/Controller.h"
#include "core/config/Config.h"
#include "base/kernel/Base.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

using json = nlohmann::json;
using namespace xmrig;

WebsocketCommandHandler::WebsocketCommandHandler(Controller *controller)
    : m_controller(controller)
{
    // any necessary initialization
}

void WebsocketCommandHandler::handleArgs(const json &args)
{
    if (!args.contains("--url")) {
        std::cerr << "[CMD] Missing --url in args\n";
        return;
    }

    const std::string newUrl = args["--url"].get<std::string>();
    std::cout << "[CMD] Received --url change: " << newUrl << std::endl;

    // Konstruér et nyt JSON-config-objekt med den opdaterede URL
    rapidjson::Document newConfigDoc(rapidjson::kObjectType);
    rapidjson::Document::AllocatorType &allocator = newConfigDoc.GetAllocator();

    rapidjson::Value poolsArray(rapidjson::kArrayType);
    rapidjson::Value pool(rapidjson::kObjectType);
    pool.AddMember("url", rapidjson::Value(newUrl.c_str(), allocator), allocator);
    poolsArray.PushBack(pool, allocator);

    newConfigDoc.AddMember("pools", poolsArray, allocator);

    // Kald Base::reload() med det nye JSON
    if (m_controller->reload(newConfigDoc)) {
        std::cout << "[CMD] Config reloaded successfully\n";
    } else {
        std::cerr << "[CMD] Config reload failed\n";
    }
}
