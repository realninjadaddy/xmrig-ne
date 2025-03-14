#include "base/net/websocket/WebsocketClient.h"
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocketMessageType.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <ctime>
#include "core/Controller.h"
#include "core/config/Config.h"

using json = nlohmann::json;

namespace xmrig {

    WebsocketClient::WebsocketClient(Controller *controller, const std::string &url, const std::string &user, const std::string &secret)
    : m_controller(controller), m_url(url), m_user(user), m_secret(secret)
{
    ix::initNetSystem();
}

WebsocketClient::~WebsocketClient() {
    std::cout << "[WS] ~WebsocketClient() called, stopping thread...\n";
    stop();
}

void WebsocketClient::setupHandlers() {
    m_socket.setOnMessageCallback([this](const ix::WebSocketMessagePtr &msg) {
        if (msg->type == ix::WebSocketMessageType::Open) {
            m_connected = true;
            std::cout << "[WS] Connected\n";

            json hello = {
                {"type", "hello"},
                {"user", m_user},
                {"secret", m_secret}
            };
            m_socket.sendText(hello.dump());
        }
        else if (msg->type == ix::WebSocketMessageType::Close) {
            m_connected = false;
            std::cout << "[WS] Disconnected\n";
            std::cout << "[WS] Attempting reconnect...\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));
            m_socket.start();
        }
        else if (msg->type == ix::WebSocketMessageType::Message) {
            std::cout << "[WS] Received: " << msg->str << "\n";
            try {
                json data = json::parse(msg->str);
                std::cout << "[WS] JSON type: " << data["type"] << std::endl;

                if (data["type"] == "set_diff") {
                    std::cout << "[WS] Triggering set_diff reload\n";
                
                    if (!data.contains("value") || !data["value"].is_number_integer()) {
                        std::cerr << "[WS] Invalid value for set_diff\n";
                        return;
                    }
                
                    // Ny værdi for +XXXXX
                    int value = data["value"].get<int>();
                
                    // Hent den eksisterende config
                    auto config = m_controller->config();
                
                    // Lav et nyt rapidjson-dokument til at opdatere wallet-adressen
                    rapidjson::Document newConfig(rapidjson::kObjectType);
                    rapidjson::Document::AllocatorType &allocator = newConfig.GetAllocator();
                
                    // Byg ny pool med opdateret wallet-adresse
                    rapidjson::Value pools(rapidjson::kArrayType);
                    for (const auto &pool : config->pools().data()) {
                        rapidjson::Value poolObj(rapidjson::kObjectType);
                        std::string newUser = pool.user().data();
                
                        // Fjern evt. +XXXXX, hvis det findes
                        size_t plusPos = newUser.find('+');
                        if (plusPos != std::string::npos) {
                            newUser = newUser.substr(0, plusPos);
                        }
                
                        // Tilføj +value
                        newUser += "+" + std::to_string(value);
                
                        // Byg pool JSON
                        poolObj.AddMember("url", rapidjson::Value(pool.url().data(), allocator), allocator);
                        poolObj.AddMember("user", rapidjson::Value(newUser.c_str(), allocator), allocator);
                
                        pools.PushBack(poolObj, allocator);
                    }
                
                    // Tilføj pools til ny config
                    newConfig.AddMember("pools", pools, allocator);
                
                    // Kald reload på den opdaterede config
                    if (m_controller->reload(newConfig)) {
                        std::cout << "[WS] Config reloaded with new diff value\n";
                    } else {
                        std::cerr << "[WS] Reload failed\n";
                    }
                }
                 else if (data["type"] == "reload_config") {
                    std::cout << "[WS] Trigger config reload!\n";
                }
                else if (data["type"] == "set_args") {
                    std::cout << "[WS] set_args handler triggered\n";
                    if (data.contains("args") && data["args"].is_object()) {
                        const auto &args = data["args"];

                        std::string newUrl;
                        if (args.contains("--url") && args["--url"].is_string()) {
                            std::cout << "[WS] --url found: " << args["--url"] << std::endl;
                            newUrl = args["--url"];
                        } else if (args.contains("-o") && args["-o"].is_string()) {
                            std::cout << "[WS] -o found: " << args["-o"] << std::endl;
                            newUrl = args["-o"];
                        } else {
                            std::cout << "[WS] --url NOT found\n";
                        }
                

                        if (!newUrl.empty() && m_onSetUrl) {
                            std::cout << "[WS] Calling m_onSetUrl with: " << newUrl << "\n";
                            m_onSetUrl(newUrl);
                        }
                    }
                }
            } catch (...) {
                std::cerr << "[WS] Invalid JSON received\n";
            }
        }
    });
}

void WebsocketClient::start() {
    m_running = true;
    m_thread = std::thread([this]() {
        m_socket.setUrl(m_url);
        setupHandlers();
        m_socket.start();

        while (m_running) {
            std::this_thread::sleep_for(std::chrono::seconds(10));

            if (m_connected) {
                json ping = {
                    {"type", "ping"},
                    {"user", m_user},
                    {"timestamp", std::time(nullptr)}
                };
                send(ping.dump());
            }
        }

        m_socket.stop();
    });
}

void WebsocketClient::stop() {
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
    m_socket.stop(1000, "XMRig shutting down");
}

void WebsocketClient::send(const std::string &json) {
    //std::cout << "[WS] Sending payload: " << json << std::endl;
    if (m_connected) {
        m_socket.sendText(json);
    }
}

void WebsocketClient::sendShare(const std::string &jobId, uint64_t diff, uint64_t actual) {
    if (!m_connected) return;

    json msg = {
        {"type", "share"},
        {"user", m_user},
        {"result", "accepted"},
        {"difficulty", diff},
        {"actualDifficulty", actual},
        {"jobId", jobId},
        {"timestamp", std::time(nullptr)}
    };

    std::cout << "[WS] Sending share: jobId=" << jobId
              << " diff=" << diff
              << " actual=" << actual << std::endl;
    std::cout << "[WS] Payload: " << msg.dump() << std::endl;

    send(msg.dump());
}

void WebsocketClient::sendJob(const std::string &algo, uint64_t diff, uint64_t height, int txCount) {
    if (!m_connected) return;

    json msg = {
        {"type", "job"},
        {"user", m_user},
        {"algo", algo},
        {"difficulty", diff},
        {"height", height},
        {"txCount", txCount},
        {"timestamp", std::time(nullptr)}
    };

    std::cout << "[WS] Sending job: height=" << height
              << " algo=" << algo
              << " diff=" << diff
              << " tx=" << txCount << std::endl;

    send(msg.dump());
}

bool WebsocketClient::isConnected() const {
    return m_connected;
}

void WebsocketClient::sendStats(double h10s, double h60s, double h15m, uint64_t uptime) {
    //std::cout << "[WS] sendStats() CALLED\n";
    //std::cout << "[WS] Connected? " << (m_connected ? "YES" : "NO") << std::endl;
    if (!m_connected) return;

    json msg = {
        {"type", "stats"},
        {"user", m_user},
        {"uptime", uptime},
        {"hashrate", {
            {"10s", h10s},
            {"60s", h60s},
            {"15m", h15m}
        }},
        {"timestamp", std::time(nullptr)}
    };

    //std::cout << "[WS] Sending stats: " << msg.dump() << std::endl;
    send(msg.dump());
}

} // namespace xmrig
