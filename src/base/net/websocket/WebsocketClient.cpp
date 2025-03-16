/* XMRig
 * Copyright 2025      NinjaDaddy https://github.com/realninjadaddy/xmrig-ne
 *
 *   This file is part of XMRig and is licensed under the GNU General Public License v3 (GPLv3).
 *   You can find a copy of the license at <http://www.gnu.org/licenses/>.
 */
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
#include "base/net/websocket/WebsocketCommandHandler.h"
#include "base/io/log/Log.h"
#include "base/io/log/Tags.h"

using json = nlohmann::json;

namespace xmrig {

WebsocketClient::WebsocketClient(Controller *controller, std::shared_ptr<WebsocketCommandHandler> handler,
        const std::string &url, const std::string &user,
        const std::string &secret)
: m_controller(controller), m_handler(handler), m_url(url), m_user(user), m_secret(secret)
{

    ix::initNetSystem();
}

WebsocketClient::~WebsocketClient() {
    //std::cout << "[WS] ~WebsocketClient() called, stopping thread...\n";
    stop();
}

void WebsocketClient::setupHandlers() {
    m_socket.setOnMessageCallback([this](const ix::WebSocketMessagePtr &msg) {
        //LOG_INFO("WebSocket", "onMessageCallback triggered"); std::cout << "[WebSocket:INFO] " << "onMessageCallback triggered" << std::endl;
        if (!msg) {
            //LOG_ERR("WebSocket", "Null WebSocket message received."); std::cout << "[WebSocket:ERR] " << "Null WebSocket message received." << std::endl;
            return;
        }
        //LOG_INFO("WebSocket", "Message type: " + std::to_string(static_cast<int>(msg->type))); std::cout << "[WebSocket:INFO] " << "Message type: " + std::to_string(static_cast<int>(msg->type)) << std::endl;
        //LOG_INFO("WebSocket", "Raw message content: " + msg->str); std::cout << "[WebSocket:INFO] " << "Raw message content: " + msg->str << std::endl;
        if (msg->type == ix::WebSocketMessageType::Open) {
            m_connected = true;
            LOG_INFO(WHITE_ON_GREY(" socket  ") " Connected.");

            json hello = {
                {"type", "hello"},
                {"user", m_user},
                {"secret", m_secret}
            };
            m_socket.sendText(hello.dump());
        }
        else if (msg->type == ix::WebSocketMessageType::Close) {
            m_connected = false;
            LOG_INFO(WHITE_ON_GREY(" socket  ") " Disconnected.");
            LOG_INFO(WHITE_ON_GREY(" socket  ") " Attempting reconnect....");
            std::this_thread::sleep_for(std::chrono::seconds(2));
            m_socket.start();
        }
        else if (msg->type == ix::WebSocketMessageType::Message) {
            //std::cout << "[WS] Received: " << msg->str << "\n";
            try {
                //std::cout << "[WS] msg->str: " << msg->str << std::endl;
                //LOG_INFO("[WS DEBUG] msg ptr = %p", (void*)msg.get());
                //LOG_INFO("[WS DEBUG] msg->str = %s", msg->str.c_str());
                json data;
                try {
                    data = json::parse(msg->str);
                } catch (const std::exception &e) {
                    //LOG_ERR("WebSocket", "JSON parse failed: " + std::string(e.what())); std::cout << "[WebSocket:ERR] " << "JSON parse failed: " + std::string(e.what()) << std::endl;
                    return;
                }
                //std::cout << "[WS] JSON type: " << data["type"] << std::endl;
                //std::cout << "[WS] JSON : " << data << std::endl;

                if (data.contains("type") && data["type"] == "set_ws_args") {
                    //std::cout << "[WS] set_args handler triggered\n";

                    if (data.contains("ws_args") && data["ws_args"].is_object()) {

                        if (m_handler) {
                            m_handler->handleArgs(data);
                        }                        
                    }
                }
            } catch (const json::exception& e) {
                //std::cerr << "[WS] JSON parsing error: " << e.what() << std::endl;
            }
        }
    });
}

void WebsocketClient::start() {
    //LOG_INFO("WebSocket", "start() called"); std::cout << "[WebSocket:INFO] " << "start() called" << std::endl;
    //LOG_INFO("WebSocket", "Using URL: " + m_url); std::cout << "[WebSocket:INFO] " << "Using URL: " + m_url << std::endl;
    //LOG_INFO("WebSocket", "Using user: " + m_user); std::cout << "[WebSocket:INFO] " << "Using user: " + m_user << std::endl;
    //LOG_INFO("WebSocket", "Using secret: " + m_secret); std::cout << "[WebSocket:INFO] " << "Using secret: " + m_secret << std::endl;
    m_running = true;
    m_thread = std::thread([this]() {
        m_socket.setUrl(m_url);
        setupHandlers();
        m_socket.start();

        using namespace std::chrono;

        while (m_running) {
            std::this_thread::sleep_for(std::chrono::seconds(10));

            auto now = system_clock::now();
            auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();
        
            if (m_connected) {
                json ping = {
                    {"type", "ping"},
                    {"user", m_user},
                    {"timestamp", std::time(nullptr)},
                    {"timestamp_ms", ms}
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

void WebsocketClient::sendShare(const std::string &jobId, uint64_t diff, uint64_t actual, const char *error) {
    if (!m_connected) return;

    using namespace std::chrono;
    auto now = system_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();

    if (!error) {
        json msg = {
            {"type", "share"},
            {"user", m_user},
            {"result", "accepted"},
            {"difficulty", diff},
            {"actualDifficulty", actual},
            {"jobId", jobId},
            {"timestamp", std::time(nullptr)},
            {"timestamp_ms", ms}
        };
        send(msg.dump());
    } else {
        json msg = {
            {"type", "share"},
            {"user", m_user},
            {"result", "rejected"},
            {"difficulty", diff},
            {"actualDifficulty", actual},
            {"jobId", jobId},
            {"timestamp", std::time(nullptr)},
            {"timestamp_ms", ms}
        };
        send(msg.dump());
    }
}


void WebsocketClient::sendJob(const std::string &algo, uint64_t diff, uint64_t height, int txCount) {
    if (!m_connected) return;

    using namespace std::chrono;
    auto now = system_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();

    json msg = {
        {"type", "job"},
        {"user", m_user},
        {"algo", algo},
        {"difficulty", diff},
        {"height", height},
        {"txCount", txCount},
        {"timestamp", std::time(nullptr)},
        {"timestamp_ms", ms}
    };

    send(msg.dump());
}

bool WebsocketClient::isConnected() const {
    return m_connected;
}

void WebsocketClient::sendStats(double h10s, double h60s, double h15m, uint64_t uptime) {

    if (!m_connected) return;

    using namespace std::chrono;
    auto now = system_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();

    json msg = {
        {"type", "stats"},
        {"user", m_user},
        {"uptime", uptime},
        {"hashrate", {
            {"10s", h10s},
            {"60s", h60s},
            {"15m", h15m}
        }},
        {"timestamp", std::time(nullptr)},
        {"timestamp_ms", ms}
    };

    send(msg.dump());
}

} // namespace xmrig
