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

WebsocketClient::WebsocketClient(Controller *controller, WebsocketCommandHandler *handler,
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
                json data = json::parse(msg->str);
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

    /*
    std::cout << "[WS] Sending share: jobId=" << jobId
              << " diff=" << diff
              << " actual=" << actual << std::endl;
    std::cout << "[WS] Payload: " << msg.dump() << std::endl;
    */

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
